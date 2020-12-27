//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2005 Christoph Oelckers
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
// Base class for resource files (WADs, ZIPs or Lumps)
//

#include "StdAfx.h"
#include "ResourceFile.h"
#include "texture.h"

TArray<CResourceFile *> CResourceFile::OpenFiles;

//==========================================================================
//
//
//
//==========================================================================

CResourceFile::CResourceFile(void)
{
	m_NumLumps=0;
	m_Filename=NULL;
	m_LumpStart=0;
}

//==========================================================================
//
//
//
//==========================================================================

CResourceFile::~CResourceFile(void)
{
	if (m_Filename!=NULL) free(m_Filename);
}


//==========================================================================
//
//
//
//==========================================================================

QWORD CResourceFile::GlobalFindLump(const char * name)
{
	for(int i=OpenFiles.Size()-1;i>=0;i--)
	{
		int ret = OpenFiles[i]->FindLump(name);
		if (ret>=0) return ret|(QWORD(i+1)<<32);
	}
	return ~0;
}

//==========================================================================
//
//
//
//==========================================================================

QWORD CResourceFile::GlobalFindLumpByFullName(const char * name)
{
	for(int i=OpenFiles.Size()-1;i>=0;i--)
	{
		int ret = OpenFiles[i]->FindLumpByFullName(name);
		if (ret>=0) return ret|(QWORD(i+1)<<32);
	}
	return ~0;
}

//==========================================================================
//
//
//
//==========================================================================

bool CResourceFile::GlobalFindLumps(const char * name, TArray<QWORD> & list)
{
	list.Clear();

	for(unsigned int i=0; i<OpenFiles.Size();i++)
	{
		OpenFiles[i]->FindLumps(name, list);
	}
	return list.Size()>0;
}

//==========================================================================
//
//
//
//==========================================================================

int CResourceFile::GlobalGetLumpSize(QWORD lump)
{
	unsigned wad=unsigned((lump>>32)-1);
	if (wad>=0 && wad<OpenFiles.Size()) return OpenFiles[wad]->GetLumpSize(int(lump&0xffffffff));
	return -1;
}

//==========================================================================
//
//
//
//==========================================================================

const char * CResourceFile::GlobalGetLumpName(QWORD lump)
{
	unsigned wad=unsigned((lump>>32)-1);
	if (wad>=0 && wad<OpenFiles.Size()) return OpenFiles[wad]->GetLumpName(int(lump&0xffffffff));
	return NULL;
}

//==========================================================================
//
//
//
//==========================================================================

const char * CResourceFile::GlobalGetFullLumpName(QWORD lump)
{
	unsigned wad=unsigned((lump>>32)-1);
	if (wad>=0 && wad<OpenFiles.Size()) return OpenFiles[wad]->GetFullLumpName(int(lump&0xffffffff));
	return NULL;
}

//==========================================================================
//
//
//
//==========================================================================

void * CResourceFile::GlobalReadLump(QWORD lump)
{
	unsigned wad=unsigned((lump>>32)-1);
	if (wad>=0 && wad<OpenFiles.Size()) return OpenFiles[wad]->ReadLump(int(lump&0xffffffff));
	return NULL;
}

//==========================================================================
//
//
//
//==========================================================================

void CResourceFile::GlobalReadLump(QWORD lump, void * buffer)
{
	unsigned wad=unsigned((lump>>32)-1);
	if (wad>=0 && wad<OpenFiles.Size()) OpenFiles[wad]->ReadLump(int(lump&0xffffffff), buffer);
}

//==========================================================================
//
//
//
//==========================================================================

void CResourceFile::GlobalReplaceLump(QWORD lump, void * data, int length)
{
	unsigned wad=unsigned((lump>>32)-1);
	if (wad>=0 && wad<OpenFiles.Size()) OpenFiles[wad]->ReplaceLump(int(lump&0xffffffff), data, length);
}

//==========================================================================
//
//
//
//==========================================================================

void CResourceFile::GlobalGetAllSprites(TArray<QWORD> & list)
{
	for(unsigned i=0;i<OpenFiles.Size();i++)
	{
		OpenFiles[i]->GetAllSprites(list);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CResourceFile::GlobalGetAllFlats(TArray<QWORD> & list)
{
	for(unsigned i=0;i<OpenFiles.Size();i++)
	{
		OpenFiles[i]->GetAllFlats(list);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CResourceFile::GlobalGetAllTextures(TArray<QWORD> & list)
{
	for(unsigned i=0;i<OpenFiles.Size();i++)
	{
		OpenFiles[i]->GetAllTextures(list);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CResourceFile::AddResourceFile(const char * filename)
{
	FILE * f = fopen(filename, "rb");
	CResourceFile * resf;
	QWORD cnt=0;

	if (f!=NULL)
	{
		char id[4];
		fread(id, 1, 4, f);
		fclose(f);
		if (!memcmp(id, "IWAD", 4) || !memcmp(id, "PWAD", 4))
		{
			resf = new CWADFile(filename);
		}
		else if (!memcmp(id, "PK\3\4", 4))
		{
			resf = new CZipFile(filename);
		}
		else if (!memcmp(id, "7z\xbc\xaf", 4))
		{
			resf = new  C7zFile(filename);
		}
		else if (!memcmp(id, "KenS", 4))
		{
			resf = new CGrpFile(filename);
		}
		else if (!memcmp(id, "RFF\x1a", 4))
		{
			resf = new CRffFile(filename);
		}
		else
		{
			resf = new  CSingleLump(filename);
		}
		if (resf!=NULL) 
		{
			cnt=0x100000000*(OpenFiles.Size()+1);
			resf->m_LumpStart=cnt;
			OpenFiles.Push(resf);
			if (texman) delete texman;
			texman=NULL;
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CResourceFile::CloseResourceFile(const char * filename)
{
	for(unsigned i=0;i<OpenFiles.Size();i++) if (!stricmp(OpenFiles[i]->GetName(),filename))
	{
		delete OpenFiles[i];
		OpenFiles.Delete(i);
		if (texman) delete texman;
		texman=NULL;

		for(i++;i<OpenFiles.Size();i++)
		{
			OpenFiles[i]->m_LumpStart--;
		}
		return;
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CResourceFile::CloseResourceFiles()
{
	for(unsigned i=0;i<OpenFiles.Size();i++) 
	{
		delete OpenFiles[i];
	}
	OpenFiles.Clear();
	if (texman) delete texman;
	texman=NULL;
}

//==========================================================================
//
//
//
//==========================================================================

void CResourceFile::OpenResourceFiles(const TArray<const char *> & filenames)
{
	for(unsigned i=0;i<OpenFiles.Size();i++)
	{
		delete OpenFiles[i];
	}
	OpenFiles.Clear();
	for(unsigned i=0;i<filenames.Size();i++)
	{
		AddResourceFile(filenames[i]);
	}
}

void CResourceFile::OpenResourceFiles(const wxArrayString & filenames)
{
	for(unsigned i=0;i<OpenFiles.Size();i++)
	{
		delete OpenFiles[i];
	}
	OpenFiles.Clear();
	for(unsigned i=0;i<filenames.GetCount();i++)
	{
		AddResourceFile(filenames[i].c_str());
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CResourceFile::SwitchIWAD(const char *newiwad)
{
	wxArrayString files;

	files.Add(newiwad);
	for(unsigned i=1;i<OpenFiles.Size();i++)
	{
		files.Add(OpenFiles[i]->GetName());
	}
	OpenResourceFiles(files);
}

//==========================================================================
//
//
//
//==========================================================================

void CResourceFile::GetMapList(const char * filename, TArray<MapRecord> & array)
{
	for(unsigned i=0;i<OpenFiles.Size();i++) if (!stricmp(OpenFiles[i]->GetName(),filename))
	{
		OpenFiles[i]->GetMapList(array);
	}
}

static int mapcmp(const void *a, const void *b)
{
	char * A = (char*)((MapRecord*)a)->mapname;
	char * B = (char*)((MapRecord*)b)->mapname;

	while (*A && *B)
	{
		char ca = tolower(*A);
		char cb = tolower(*B);
		if (!isdigit(ca) || !isdigit(cb))
		{
			if (ca != cb) return ca - cb;
			A++;
			B++;
		}
		else
		{
			int va = strtol(A, &A, 10);
			int vb = strtol(B, &B, 10);
			if (va != vb) return va - vb;
		}
	}
	return 0;
}


void CResourceFile::SortMapList(TArray<MapRecord> & array)
{
	qsort(&array[0], array.Size(), sizeof(array[0]), mapcmp);
}
