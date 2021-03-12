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
// ZIP file as WAD replacement
//

#include "StdAfx.h"
#include "ResourceFile.h"
#include "doomerrors.h"
#include "files.h"
#include "templates.h"
#include "w_zip.h"
#include "zip/explode/explode.h"

void ExtractFileBase (const char *path, char *dest);

enum
{
	LUMPF_BLOODCRYPT	= 1,	// Lump uses Blood-style encryption
	LUMPF_COMPRESSED	= 2,	// Zip-compressed
	LUMPF_ZIPFILE		= 4,	// Inside a Zip file - used to enforce use of special directories insize Zips
	LUMPF_NEEDFILESTART	= 8,	// Still need to process local file header to find file start inside a zip
	LUMPF_EXTERNAL		= 16,	// Lump is from an external file that won't be kept open permanently
};

//==========================================================================
//
//
//
//==========================================================================

CZipFile::CZipFile(const char * filename, bool readdir)
{
	fin = new FileReader(filename);
	m_Filename=_strdup(filename);

	if (readdir) ReadDirectory(fin);
}

//==========================================================================
//
//
//
//==========================================================================

CZipFile::~CZipFile(void)
{
	for(unsigned int i=0;i<mapnames.Size ();i++)
	{
		free((void*)mapnames[i].mapname);
	}
	for(unsigned i=0;i<embeddedWADs.Size();i++)
	{
		delete embeddedWADs[i];
	}
	for(unsigned i=0;i<lumps.Size();i++)
	{
		if (lumps[i].data!=NULL) delete [] lumps[i].data;
		if (lumps[i].fullname!=NULL) free(lumps[i].fullname);
	}
}

//-----------------------------------------------------------------------
//
// Finds the central directory end record in the end of the file.
// Taken from Quake3 source
//
//-----------------------------------------------------------------------

DWORD CZipFile::FindCentralDir(FileReader * fin)
{
	const int BUFREADCOMMENT = 0x400;

	unsigned char buf[BUFREADCOMMENT + 4];
	DWORD FileSize;
	DWORD uBackRead;
	DWORD uMaxBack; // maximum size of global comment
	DWORD uPosFound=0;

	fin->Seek(0, SEEK_END);

	FileSize = fin->Tell();
	uMaxBack = MIN<DWORD>(0xffff, FileSize);

	uBackRead = 4;
	while (uBackRead < uMaxBack)
	{
		DWORD uReadSize, uReadPos;
		int i;
		if (uBackRead + BUFREADCOMMENT > uMaxBack) 
			uBackRead = uMaxBack;
		else
			uBackRead += BUFREADCOMMENT;
		uReadPos = FileSize - uBackRead;

		uReadSize = MIN<DWORD>((BUFREADCOMMENT + 4), (FileSize - uReadPos));

		if (fin->Seek(uReadPos, SEEK_SET) != 0) break;

		if (fin->Read(buf, (SDWORD)uReadSize) != (SDWORD)uReadSize) break;

		for (i = (int)uReadSize - 3; (i--) > 0;)
		{
			if (buf[i] == 'P' && buf[i+1] == 'K' && buf[i+2] == 5 && buf[i+3] == 6)
			{
				uPosFound = uReadPos + i;
				break;
			}
		}

		if (uPosFound != 0)
			break;
	}
	return uPosFound;
}

//-----------------------------------------------------------------------
//
// Reads a zip's directory
//
//-----------------------------------------------------------------------

void CZipFile::ReadDirectory(FileReader *wadinfo)
{
	DWORD centraldir = FindCentralDir(wadinfo);
	FZipEndOfCentralDirectory info;
	TArray<LumpRecord> EmbeddedWADs;
	
	int skipped = 0;

	if (centraldir == 0)
	{
		I_Error("\n%s: ZIP file corrupt!\n", m_Filename);
		return;
	}

	// Read the central directory info.
	wadinfo->Seek(centraldir, SEEK_SET);
	wadinfo->Read(&info, sizeof(FZipEndOfCentralDirectory));

	// No multi-disk zips!
	if (info.NumEntries != info.NumEntriesOnAllDisks ||
		info.FirstDisk != 0 || info.DiskNumber != 0)
	{
		I_Error("\n%s: Multipart Zip files are not supported.\n", m_Filename);
		return;
	}

	int NumLumps = LittleShort(info.NumEntries);


	// Load the entire central directory. Too bad that this contains variable length entries...
	void *directory = malloc(LittleLong(info.DirectorySize));
	wadinfo->Seek(LittleLong(info.DirectoryOffset), SEEK_SET);
	wadinfo->Read(directory, LittleLong(info.DirectorySize));

	char *dirptr = (char*)directory;
	for (int i = 0; i < NumLumps; i++)
	{
		FZipCentralDirectoryInfo *zip_fh = (FZipCentralDirectoryInfo *)dirptr;

		char name[256];
		char base[256];

		int len = LittleShort(zip_fh->NameLength);
		strncpy(name, dirptr + sizeof(FZipCentralDirectoryInfo), MIN<int>(len, 255));
		name[len] = 0;
		dirptr += sizeof(FZipCentralDirectoryInfo) + 
				  LittleShort(zip_fh->NameLength) + 
				  LittleShort(zip_fh->ExtraLength) + 
				  LittleShort(zip_fh->CommentLength);
			
		// skip Directories
		if(name[len - 1] == '/' && LittleLong(zip_fh->UncompressedSize) == 0) 
		{
			skipped++;
			continue;
		}

		// Ignore unknown compression formats
		zip_fh->Method = LittleShort(zip_fh->Method);
		if (zip_fh->Method != METHOD_STORED &&
			zip_fh->Method != METHOD_DEFLATE &&
			zip_fh->Method != METHOD_LZMA &&
			zip_fh->Method != METHOD_BZIP2 &&
			zip_fh->Method != METHOD_SHRINK &&
			zip_fh->Method != METHOD_IMPLODE)
		{
			//Printf("\n: %s: '%s' uses an unsupported compression algorithm (#%d).\n", m_Filename, name, zip_fh->Method);
			//skipped++;
			continue;
		}
		// Also ignore encrypted entries
		if(LittleShort(zip_fh->Flags) & ZF_ENCRYPTED)
		{
			//Printf("\n%s: '%s' is encrypted. Encryption is not supported.\n", m_Filename, name);
			//skipped++;
			continue;
		}

		//FixPathSeperator(name);
		strlwr(name);

		// Check for embedded WADs in the root directory. 
		// They must be extracted and added separately to the lump list.
		// WADs in subdirectories are added to the lump directory.
		// Embedded .zips are ignored for now. But they should be allowed later!
		char * c = strstr(name, ".wad");
		if (c && strlen(c)==4 && !strchr(name, '/'))
		{
			LumpRecord l;

			memset(&l, 0, sizeof(l));

			l.fullname = _strdup(name);
			l.size = LittleLong(zip_fh->UncompressedSize);

			l.method = zip_fh->Method;
			l.compressedsize = LittleLong(zip_fh->CompressedSize);

			// The start of the file will be determined the first time it is accessed.
			l.flags = LUMPF_NEEDFILESTART;
			l.position = LittleLong(zip_fh->LocalHeaderOffset);


			EmbeddedWADs.Push(l);
			continue;
		}

		LumpRecord *lump_p = &lumps[lumps.Reserve(1)];

		//ExtractFileBase(name, base);
		char *lname = strrchr(name,'/');
		if (!lname) lname = name;
		else lname++;
		strcpy(base, lname);
		char *dot = strrchr(base, '.');
		if (dot) *dot = 0;
		strncpy(lump_p->name, base,8);
		lump_p->data = NULL;
		lump_p->name[8] = 0;
		_strupr(lump_p->name);
		lump_p->fullname = _strdup(name);
		lump_p->size = LittleLong(zip_fh->UncompressedSize);

		lump_p->method = zip_fh->Method;
		lump_p->compressedsize = LittleLong(zip_fh->CompressedSize);

		// The start of the file will be determined the first time it is accessed.
		lump_p->flags = LUMPF_NEEDFILESTART;
		lump_p->position = LittleLong(zip_fh->LocalHeaderOffset);

		if (!strncmp(lump_p->fullname, "maps/", 5))
		{
			mapnames.Push({ lumps.Size() - 1, _strdup(lump_p->name) });
		}
		else
		{
			char* c = strstr(name, ".map");
			if (c && strlen(c) == 4 && !strchr(name, '/'))
			{
				mapnames.Push({ lumps.Size() - 1, _strdup(lump_p->name) });
			}
		}

		// Entries in Zips are sorted alphabetically.
		//qsort(&LumpInfo[startlump], NumLumps - startlump, sizeof(LumpRecord), lumpcmp);
	}

	for(unsigned i=0; i<EmbeddedWADs.Size(); i++)
	{
		char * buffer = new char[EmbeddedWADs[i].size];
		QWORD cnt = m_LumpStart;

		ReadLump(&EmbeddedWADs[i], buffer);
		if (!memcmp(buffer+1, "WAD", 3))
		{
			CWADFile * wadf = new CWADFile(EmbeddedWADs[i].fullname, buffer);
			cnt+=0x10000;
			wadf->m_LumpStart=cnt;
			embeddedWADs.Push(wadf);
			free(EmbeddedWADs[i].fullname);
		}
		delete buffer;
	}
}

//==========================================================================
//
//
//
//==========================================================================

int CZipFile::FindLump(const char * name)
{
	for(int i=embeddedWADs.Size()-1;i>=0;i--)
	{
		int ret = embeddedWADs[i]->FindLump(name);
		if (ret>=0)
		{
			return ((i+1)<<16)+ret;
		}
	}
	for (int i = lumps.Size()-1; i>=0; i--)
	{
		if (!stricmp(name, lumps[i].name)) return i;
	}
	return -1;
}

//==========================================================================
//
//
//
//==========================================================================

int CZipFile::FindLumpByFullName(const char * name)
{
	for (int i = lumps.Size()-1; i>=0; i--)
	{
		if (!stricmp(name, lumps[i].fullname)) return i;
	}
	return -1;
}

//==========================================================================
//
//
//
//==========================================================================

void CZipFile::FindLumps(const char * name, TArray<QWORD> & list)
{
	for(unsigned int i=0;i<lumps.Size(); i++)
	{
		if (!stricmp(name, lumps[i].name)) list.Push(i+m_LumpStart);
	}
	unsigned index=list.Size();
	for(unsigned i=0;i<embeddedWADs.Size();i++)
	{
		embeddedWADs[i]->FindLumps(name,list);
	}
	for(unsigned i=index;i<list.Size();i++) list[i]+=m_LumpStart;
}

//==========================================================================
//
//
//
//==========================================================================

int CZipFile::GetLumpSize(int lump)
{
	if (lump&0xffff0000)
	{
		unsigned wad = (lump>>16)-1;
		if (wad>=embeddedWADs.Size()) return -1;
		return embeddedWADs[wad]->GetLumpSize(lump&0xffff);
	}
	if ((unsigned)lump < lumps.Size() ) return lumps[lump].size;
	else return -1;
}

//==========================================================================
//
//
//
//==========================================================================

void * CZipFile::ReadLump(int lump)
{
	if (lump&0xffff0000)
	{
		unsigned wad = (lump>>16)-1;
		if (wad>=embeddedWADs.Size()) return NULL;
		return embeddedWADs[wad]->ReadLump(lump&0xffff);
	}

	if ((unsigned)lump < lumps.Size() )
	{
		void * Ptr= malloc(lumps[lump].size);
		ReadLump(&lumps[lump], Ptr);
		return Ptr;
	}
	return NULL;
}

//==========================================================================
//
//
//
//==========================================================================

void CZipFile::ReadLump(int lump, void * Ptr)
{
	if (lump&0xffff0000)
	{
		unsigned wad = (lump>>16)-1;
		if (wad>=embeddedWADs.Size()) return;
		embeddedWADs[wad]->ReadLump(lump&0xffff, Ptr);
	}
	else if ((unsigned)lump < lumps.Size() )
	{
		// read the lump here
		ReadLump(&lumps[lump], Ptr);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CZipFile::ReadLump(LumpRecord *l, void *Ptr)
{
	if (l->data != NULL)
	{
		memcpy(Ptr, l->data, l->size);
		return;
	}

	if (l->flags & LUMPF_NEEDFILESTART)
	{
		// This file is inside a zip and has not been opened before.
		// Position points to the start of the local file header, which we must
		// read and skip so that we can get to the actual file data.
		FZipLocalFileHeader localHeader;
		int skiplen;
		int address;

		address = fin->Tell();
		fin->Seek(l->position, SEEK_SET);
		fin->Read(&localHeader, sizeof(localHeader));
		skiplen = LittleShort(localHeader.NameLength) + LittleShort(localHeader.ExtraLength);
		l->position += sizeof(localHeader) + skiplen;
		l->flags &= ~LUMPF_NEEDFILESTART;
		l->gpflags = LittleShort(localHeader.Flags);
	}
	if (fin != NULL) fin->Seek (l->position, SEEK_SET);

	switch (l->method)
	{
		case METHOD_STORED:
		{
			fin->Read(Ptr, l->size);
			break;
		}

		case METHOD_DEFLATE:
		{
			FileReaderZ frz(*fin, true);
			frz.Read(Ptr, l->size);
			break;
		}

		case METHOD_BZIP2:
		{
			FileReaderBZ2 frz(*fin);
			frz.Read(Ptr, l->size);
			break;
		}

		case METHOD_LZMA:
		{
			FileReaderLZMA frz(*fin, l->size, true);
			frz.Read(Ptr, l->size);
			break;
		}

		case METHOD_IMPLODE:
		{
			FZipExploder exploder;
			exploder.Explode((unsigned char *)Ptr, l->size, fin, l->compressedsize, l->gpflags);
			break;
		}
		
		case METHOD_SHRINK:
		{
			ShrinkLoop((unsigned char *)Ptr, l->size, fin, l->compressedsize);
			break;
		}

		default:
			assert(0);
			break;
	}
}

//==========================================================================
//
//
//
//==========================================================================

const char * CZipFile::GetLumpName(int lump)
{
	if (lump&0xffff0000)
	{
		unsigned wad = (lump>>16)-1;
		if (wad>=embeddedWADs.Size()) return NULL;
		return embeddedWADs[wad]->GetLumpName(lump&0xffff);
	}

	if ((unsigned)lump < lumps.Size() )
	{
		return lumps[lump].name;
	}
	else
	{
		return NULL;
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CZipFile::ReplaceLump(int lump, void * data, int length)
{
	if (lump&0xffff0000)
	{
		unsigned wad = (lump>>16)-1;
		if (wad>=embeddedWADs.Size()) return;
		embeddedWADs[wad]->ReplaceLump(lump&0xffff, data, length);
	}

	if ((unsigned)lump < lumps.Size() )
	{
		/* not needed yet!
		if (lumps[lump].data) delete [] lumps[lump].data;
		lumps[lump].length = length;
		lumps[lump].data = new char[length];
		memcpy(lumps[lump].data, data, length);
		*/
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CZipFile::Save(const char * filename)
{
	// later
}

//==========================================================================
//
//
//
//==========================================================================

void * CZipFile::CreateInMemory(int * length)
{
	// later
	*length=0;
	return NULL;
}

//==========================================================================
//
//
//
//==========================================================================

void CZipFile::GetMapList(TArray<MapRecord> & array)
{
	for(unsigned int i=0;i<mapnames.Size ();i++)
	{
		array.Push(mapnames[i]);
	}
	for(unsigned int i=0;i<embeddedWADs.Size ();i++)
	{
		embeddedWADs[i]->GetMapList(array);
	}
	SortMapList(array);
}

//==========================================================================
//
//
//
//==========================================================================

void CZipFile::GetAllSprites(TArray<QWORD> & list) 
{
	for(unsigned i=0;i<lumps.Size();i++)
	{
		if (!strnicmp(lumps[i].fullname, "sprites/", 8)) list.Push(i+m_LumpStart);
	}
	unsigned index=list.Size();
	for(unsigned i=0;i<embeddedWADs.Size();i++)
	{
		embeddedWADs[i]->GetAllSprites(list);
	}
	for(unsigned i=index;i<list.Size();i++) list[i]+=m_LumpStart;
}

//==========================================================================
//
//
//
//==========================================================================

void CZipFile::GetAllFlats(TArray<QWORD> & list) 
{
	for(unsigned i=0;i<lumps.Size();i++)
	{
		if (!strnicmp(lumps[i].fullname, "flats/", 6)) list.Push(i+m_LumpStart);
	}
	unsigned index=list.Size();
	for(unsigned i=0;i<embeddedWADs.Size();i++)
	{
		embeddedWADs[i]->GetAllFlats(list);
	}
	for(unsigned i=index;i<list.Size();i++) list[i]+=m_LumpStart;
}

//==========================================================================
//
//
//
//==========================================================================

void CZipFile::GetAllTextures(TArray<QWORD> & list) 
{
	for(unsigned i=0;i<lumps.Size();i++)
	{
		if (!strnicmp(lumps[i].fullname, "textures/", 9)) list.Push(i+m_LumpStart);
	}
	unsigned index=list.Size();
	for(unsigned i=0;i<embeddedWADs.Size();i++)
	{
		embeddedWADs[i]->GetAllTextures(list);
	}
	for(unsigned i=index;i<list.Size();i++) list[i]+=m_LumpStart;
}


const char *CZipFile::GetFullLumpName(int lump)
{
	if (lump&0xffff0000)
	{
		return NULL;
	}

	if ((unsigned)lump < lumps.Size() )
	{
		return lumps[lump].fullname;
	}

	return NULL;
}
