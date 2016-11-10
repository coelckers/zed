//
//-----------------------------------------------------------------------------
//
// Copyright (C) 1995-2005 Christoph Oelckers
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// Texture manager
//

#include "stdafx.h"
#include "ResourceFile.h"
#include "texture.h"
#include "m_swap.h"
#include "zip/zlib/zlib.h"


DWORD LumpNameHash (const char *s)
{
	char b[9];
	strncpy(b,s,8);
	b[8]=0;
	strupr(b);
	return crc32(0, (const Bytef*)b, 8);
}

const char * TextureTypeNames[]={
	"",
	"Sprite",
	"Patch",
	"",
	"Flat",
	"Texture",
	"",
	"TX_"
};


unsigned char * playpal=NULL;
TextureManager * texman;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Texture::Texture(QWORD lump, tx_namespace _prio)
{
	strncpy(name, CResourceFile::GlobalGetLumpName(lump), 8);
	name[8]=0;

	prio=_prio;

	width=-1;
	height=-1;

	gltexture=NULL;

	patchcount=1;
	patches=new TexPatch;
	patches->patch=lump;
	patches->originx=0;
	patches->originy=0;
	owner=int(lump>>32);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Texture::Texture(int _owner, maptexture_t * mtx,tx_namespace _prio, QWORD * patchlookup, const char * pnames)
{
	int i;

	strncpy(name,mtx->name,8);
	name[8]=0;

	prio=_prio;

	width=LittleShort(mtx->width);
	height=LittleShort(mtx->height);

	gltexture=NULL;

	patchcount=mtx->patchcount;
	patches=new TexPatch[patchcount];

	TexPatch * patch;
	int pc=patchcount;
	for(i=0, patch=patches;i<pc;i++, patch++)
	{
		int mpatch = mtx->patches[i].patch;
		patch->originx=mtx->patches[i].originx;
		patch->originy=mtx->patches[i].originy;
		patch->patch=patchlookup[mpatch];
		if (patch->patch==~0)
		{
			patch--;
			patchcount--;
		}
	}
	owner=_owner;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Texture::Texture(int _owner, strifemaptexture_t * mtx,tx_namespace _prio, QWORD * patchlookup, const char * pnames)
{
	int i;

	strncpy(name,mtx->name,8);
	name[8]=0;

	prio=_prio;

	width=LittleShort(mtx->width);
	height=LittleShort(mtx->height);

	gltexture=NULL;

	patchcount=mtx->patchcount;
	patches=new TexPatch[patchcount];

	TexPatch * patch;
	int pc=patchcount;
	for(i=0, patch=patches;i<pc;i++, patch++)
	{
		int mpatch = mtx->patches[i].patch;
		patch->originx=mtx->patches[i].originx;
		patch->originy=mtx->patches[i].originy;
		patch->patch=patchlookup[mpatch];
		if (patch->patch==~0)
		{
			patch--;
			patchcount--;
		}
	}
	owner=_owner;
}


void Texture::Clean()
{
	if (gltexture) delete gltexture;
	gltexture=NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// PCX identification code taken from DevIL

#ifdef _WIN32
#pragma pack(push, packed_struct, 1)
#endif
typedef struct PCXHEAD
{
	BYTE	Manufacturer;
	BYTE	Version;
	BYTE	Encoding;
	BYTE	Bpp;
	WORD	Xmin, Ymin, Xmax, Ymax;
	WORD	HDpi;
	WORD	VDpi;
	BYTE	ColMap[48];
	BYTE	Reserved;
	BYTE	NumPlanes;
	WORD	Bps;
	WORD	PaletteInfo;
	WORD	HScreenSize;
	WORD	VScreenSize;
	BYTE	Filler[54];
} /*IL_PACKSTRUCT*/ PCXHEAD;
#ifdef _WIN32
#pragma pack(pop, packed_struct)
#endif

// Internal function used to check if the HEADER is a valid .pcx header.
// Should we also do a check on Header->Bpp?
static bool CheckPcx(PCXHEAD *Header)
{
	unsigned int	Test, i;

	if (Header->Manufacturer != 10 || Header->Encoding != 1) return false;

	// Try to support all pcx versions, as they only differ in allowed formats...
	// Let's hope it works.
	if(Header->Version != 5 && Header->Version != 0 && Header->Version != 2 && Header->VDpi != 3 && Header->VDpi != 4)
		return false;

	// See if the padding size is correct
	Test = Header->Xmax - Header->Xmin + 1;
	if (Header->Bpp >= 8) 
	{
		if (Test & 1) 
		{
			if (Header->Bps != Test + 1)
				return false;
		}
		else 
		{
			if (Header->Bps != Test)  // No padding
				return false;
		}
	}

	for (i = 0; i < 54; i++) 
	{
		if (Header->Filler[i] != 0)
			return false;
	}

	return true;
}


bool Texture::Register()
{
	if (!gltexture)
	{
		if (width<0)
		{
			if (patchcount!=1)
			{
				// this can only happen when TEXTUREx is corrupted
				return false;
			}
			int l = CResourceFile::GlobalGetLumpSize(patches->patch);
			if (l<16) return false;

			unsigned char * data = (unsigned char*)CResourceFile::GlobalReadLump(patches->patch);
			if (!data) return false;

			if (!memcmp(data,"\x89PNG\xd\xa\x1a\xa\0\0\0\xdIHDR",16))
			{
				width = BigLong( *(int*) (((char*)data)+16));
				height = BigLong( *(int*) (((char*)data)+20));
			}
			else if (data[0]==0xff && data[1]==0xd8 && !memcmp(data+6, "JFIF", 4))
			{
				// Is there a simpler way to get to this information?
				wxImage img(wxMemoryInputStream(data, CResourceFile::GlobalGetLumpSize(patches->patch)));
				width=img.GetWidth();
				height=img.GetHeight();
			}
			// I'd like to support TGA as well but wxWidgets doesn't have
			// a handler for it. :(
			else if (prio==tx_flat)
			{
					// highest power of 2 whose square is smaller than the lump size
				float q=log10f(sqrtf(float(l)))/log10f(2.f); 
				width=height=1<<(int)q;
			}
			else
			{
				width=*(short*)(data);
				height=*(short*)(data+2);
			}
			free(data);
		}
		gltexture=new GLTexture(this);
	}
	return true;
}



Texture * TextureManager::AddLumpTexture(QWORD lumpno,tx_namespace prio)
{
	if (lumpno>=0)
	{
		Texture * tx=new Texture(lumpno, prio);
		textures.Push(tx);
		return tx;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TextureManager::AddTextureLump(QWORD lump, QWORD pnlump, tx_namespace prio)
{
	if (lump==-1 || pnlump==-1) return;	// nothing there!

	char * pnames1 = (char*)CResourceFile::GlobalReadLump(pnlump);
	int * texturel= (int*)CResourceFile::GlobalReadLump(lump);
	int numtextures=LittleLong(*texturel);
	int maxoff = CResourceFile::GlobalGetLumpSize(lump);
	bool isStrife=false;
	int owner=int(lump>>32);

	int i;

	int nummappatches = LittleLong(*((const int *)pnames1));
	QWORD * patchlookup = new QWORD[nummappatches];
	
	char * pnames = pnames1 + 4;
	for (i=0 ; i<nummappatches ; i++)
    {
		char name[9];

		memcpy(name, pnames+i*8, 8);
		name[8]=0;
		patchlookup[i] = CResourceFile::GlobalFindLump(name);
    }


	// Scan the texture lump to decide if it contains Doom or Strife textures
	int * directory;
	for (i = 0, directory = texturel+1; i < numtextures; ++i)
	{
		int offset = LittleLong(directory[i]);

		maptexture_t *tex = (maptexture_t *)((BYTE *)texturel + offset);

		if (LittleShort(tex->patchcount) <= 0 ||
			tex->columndirectory[2] != 0 ||
			tex->columndirectory[3] != 0)
		{
			isStrife = true;
			break;
		}
	}


	for(i=0;i<numtextures;i++)
	{
		int offset=LittleLong(texturel[i+1]);

		if (offset > maxoff)
		{
			//lprintf(LO_ERROR,"TextureManager: bad texture directory in %s\n",W_GetLumpName(lump));
			break;
		}

		maptexture_t * mtx= (maptexture_t *) ( (BYTE *)texturel + offset);

		Texture * tx;
		
		if (!isStrife) tx = new Texture(owner, mtx, prio, patchlookup, pnames);
		else  tx = new Texture(owner, (strifemaptexture_t*)mtx, prio, patchlookup, pnames);
		textures.Push(tx);
	}
	free(pnames1);
	free(texturel);
}

void TextureManager::AddPatches (QWORD lumpnum)
{
	// maybe later. This creates a lot of overhead for little gain.

	/*
	FMemLump l = Wads.ReadLump (lumpnum);
	DWORD * lump=(DWORD*)l.GetMem();
	DWORD numpatches, i;
	char name[9];

	numpatches=*lump++;
	name[8] = 0;

	for (i = 0; i < numpatches; ++i)
	{
		memcpy(name,lump,8);
		lump+=2;

		if (CheckTextureNumForName (name, tx_pname) == -1)
		{
			AddLumpTexture(Wads.CheckNumForName (name), tx_pname);
		}
	}
	*/
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int __cdecl icmp (const void *a, const void *b)
{
	return (*(int*)a)-(*(int*)b);
}

void TextureManager::Initialize()
{
	TArray<QWORD> lumplist;
	TArray<QWORD> t1list;
	TArray<QWORD> t2list;
	unsigned i,j;


	CResourceFile::GlobalGetAllSprites(lumplist);
	for(i=0;i<lumplist.Size();i++)
	{
		AddLumpTexture(lumplist[i],tx_sprite);
	}
	lumplist.Clear();

	CResourceFile::GlobalGetAllFlats(lumplist);
	for(i=0;i<lumplist.Size();i++)
	{
		AddLumpTexture(lumplist[i],tx_flat);
	}
	lumplist.Clear();

	CResourceFile::GlobalFindLumps("PNAMES", lumplist);
	CResourceFile::GlobalFindLumps("TEXTURE1", t1list);
	CResourceFile::GlobalFindLumps("TEXTURE2", t2list);

	for(i=0;i<lumplist.Size();i++)
	{
		AddPatches (lumplist[i]);

		for(j=0;j<t1list.Size();j++)
		{
			if ((t1list[j]&0xffffffffffff0000) == (lumplist[i]&0xffffffffffff0000))
			{
				AddTextureLump(t1list[j], lumplist[i], tx_texture);
				break;
			}
		}
		for(j=0;j<t2list.Size();j++)
		{
			if ((t2list[j]&0xffffffffffff0000) == (lumplist[i]&0xffffffffffff0000))
			{
				AddTextureLump(t2list[j], lumplist[i], tx_texture);
				break;
			}
		}
	}

	// If the final TEXTURE1 and/or TEXTURE2 lumps are in a wad without a PNAMES lump,
	// they have not been loaded yet, so load them now.
	if (t1list.Size() >0 && lumplist.Size()>0 &&
		(t1list[t1list.Size()-1]&0xffffffffffff0000) != (lumplist[lumplist.Size()-1]&0xffffffffffff0000))
	{
		AddTextureLump(t1list[t1list.Size()-1], lumplist[lumplist.Size()-1], tx_texture);
	}
	if (t2list.Size() >0 && lumplist.Size()>0 &&
		(t2list[t2list.Size()-1]&0xffffffffffff0000) != (lumplist[lumplist.Size()-1]&0xffffffffffff0000))
	{
		AddTextureLump(t2list[t2list.Size()-1], lumplist[lumplist.Size()-1], tx_texture);
	}

	lumplist.Clear();

	CResourceFile::GlobalGetAllTextures(lumplist);
	for(i=0;i<lumplist.Size();i++)
	{
		AddLumpTexture(lumplist[i],tx_txlump);
	}

	textures.ShrinkToFit();

	// Initialize texture hash table
	for (i = 0; (size_t)i<textures.Size(); i++)
	{
		textures[i]->index = -1;
	}

	for (i = 0; (size_t)i<textures.Size(); i++)
	{
		int j = LumpNameHash(textures[i]->name) % (unsigned) textures.Size();
		textures[i]->next = textures[j]->index;   // Prepend to chain
		textures[j]->index = i;
	}

	for (i = 0; (size_t)i<textures.Size(); i++)
	{
		int j=CheckTextureNumForName(textures[i]->name,tx_texture);
		int k=CheckTextureNumForName(textures[i]->name,tx_flat);
		textures[i]->isflat=(k==i);
		textures[i]->iswall=(j==i);
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// R_CheckTextureNumForName
// Check whether texture is available.
// Filter out NoTexture indicator.
//
// Rewritten by Lee Killough to use hash table for fast lookup. Considerably
// reduces the time needed to start new levels. See w_wad.c for comments on
// the hashing algorithm, which is also used for lump searches.
//
// killough 1/21/98, 1/31/98
//

int TextureManager::CheckTextureNumForName(const char *name, tx_namespace ns)
{
	int i = 0;

	int found=0;

	if (!name) return 0;
	if (ns==tx_texture)
	{
		if (*name=='-') return -1;
	}

	i = textures[LumpNameHash(name) % (unsigned) textures.Size()]->index;

	while (i>=0 && i<(int)textures.Size())
	{
		if (!strnicmp(textures[i]->name, name, 8))
		{
			
			if (textures[i]->prio>=tx_hiprio) return i;
			else if (textures[i]->prio==ns) return i;
			else if (ns==tx_texture || ns==tx_flat)
			{
				if (textures[i]->prio==tx_texture+tx_flat-ns) found=i;
				else if (found) return found;
				else return i;
			}
		}
		i=textures[i]->next;
    }
	return found? found:-1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Texture * TextureManager::RegisterTexture(unsigned int texno)
{
	if (texno>=textures.Size() || texno==0) return NULL;
	if (textures[texno]->Register()) return textures[texno];
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TextureManager::CleanWorldTextures()
{
	for(int i=0;(size_t)i<textures.Size();i++) textures[i]->Clean();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TextureManager::CleanMemory()
{
	CleanWorldTextures();
}


wxBitmap * TextureManager::CreateTexture(int tnum)
{
	if (tnum<0 || tnum>=(int)textures.Size() || !textures[tnum]->Register()) return NULL;
	return textures[tnum]->gltexture->CreateTexBuffer();
}

wxBitmap * TextureManager::CreateTexture(const char * name,tx_namespace type)
{
	int t=CheckTextureNumForName(name, type);
	if (t==-1) return NULL;
	return CreateTexture(t);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TextureManager::GetWallTextureSize(SIZE * sz,char * texName)
{
	if (*texName=='-')
	{
		if (sz) sz->cx=sz->cy=0;
		return true;
	}
	int v=CheckTextureNumForName(texName,tx_texture);
	if (v>0)
	{
		Texture * t=RegisterTexture(v);
		if (sz)
		{
			sz->cx=t->width;
			sz->cy=t->height;
		}
		return true;
	}
	if (sz) sz->cx=sz->cy=0;
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TextureManager::GetFlatTextureSize(SIZE * sz,char * texName)
{
	int v=CheckTextureNumForName(texName,tx_flat);
	if (v>0)
	{
		Texture * t=RegisterTexture(v);
		if (sz)
		{
			sz->cx=t->width;
			sz->cy=t->height;
		}
		return true;
	}
	if (sz) sz->cx=sz->cy=0;
	return false;
}


static int tccmp(const void * a, const void * b)
{
	return strcmp(texman->textures[*(int*)a]->GetName(), texman->textures[*(int*)b]->GetName());
}

void TextureManager::CollectTextures(TArray<int> & list, tx_namespace prio, int mask, int owner)
{
	list.Clear();
	if (prio==tx_texture) list.Push(-1);
	for(unsigned int i=0;i<textures.Size();i++)
	{
		Texture * tx = textures[i];

		if (owner>=0 && tx->owner!=owner) continue;
		if (tx->prio==tx_sprite && !(mask&16)) continue;
		if (tx->prio==tx_pname && !(mask&8)) continue;
		if (tx->prio==tx_txlump && !(mask&4)) continue;
		if (tx->prio==tx_flat && !(mask&2)) continue;
		if (tx->prio==tx_texture && !(mask&1)) continue;

		// Only list textures that are accessible
		if (CheckTextureNumForName(tx->name, prio)==i)
		{
			list.Push(i);
		}
	}
	if (prio==tx_texture) qsort(&list[1], list.Size()-1, sizeof(list[0]), tccmp);
	else qsort(&list[0], list.Size(), sizeof(list[0]), tccmp);
}


void ValidateTextureManager()
{
	static int init=0;

	if (!texman)
	{
		texman=new TextureManager;
		texman->Initialize();
		QWORD pp=CResourceFile::GlobalFindLump("PLAYPAL");

		if (pp>=0) 
		{
			if (playpal) free(playpal);
			playpal=(unsigned char*)CResourceFile::GlobalReadLump(pp);
			//ilSetSharedPal(playpal, 256, IL_PAL_RGB24);
		}
	}
}

