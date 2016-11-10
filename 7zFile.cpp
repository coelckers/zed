//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2009 Christoph Oelckers
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
// 7Z file as WAD replacement
//

#include "StdAfx.h"
#include "ResourceFile.h"
#include "doomerrors.h"
#include "files.h"
#include "templates.h"
#include "w_zip.h"


extern "C" {
#include "zip/lzma/C/Archive/7z/7zHeader.h"
#include "zip/lzma/C/Archive/7z/7zExtract.h"
#include "zip/lzma/C/Archive/7z/7zIn.h"
#include "zip/lzma/C/7zCrc.h"
}

extern ISzAlloc g_Alloc;

struct CZDFileInStream
{
	ISeekInStream s;
	FileReader *File;

	CZDFileInStream(FileReader *_file)
	{
		s.Read = Read;
		s.Seek = Seek;
		File = _file;
	}

	static SRes Read(void *pp, void *buf, size_t *size)
	{
		CZDFileInStream *p = (CZDFileInStream *)pp;
		long numread = p->File->Read(buf, (long)*size);
		if (numread < 0)
		{
			*size = 0;
			return SZ_ERROR_READ;
		}
		*size = numread;
		return SZ_OK;
	}

	static SRes Seek(void *pp, Int64 *pos, ESzSeek origin)
	{
		CZDFileInStream *p = (CZDFileInStream *)pp;
		int move_method;
		int res;
		if (origin == SZ_SEEK_SET)
		{
			move_method = SEEK_SET;
		}
		else if (origin == SZ_SEEK_CUR)
		{
			move_method = SEEK_CUR;
		}
		else if (origin == SZ_SEEK_END)
		{
			move_method = SEEK_END;
		}
		else
		{
			return 1;
		}
		res = p->File->Seek((long)*pos, move_method);
		*pos = p->File->Tell();
		return res;
	}
};

struct C7zArchive
{
	CSzArEx DB;
	CZDFileInStream ArchiveStream;
	CLookToRead LookStream;
	UInt32 BlockIndex;
	Byte *OutBuffer;
	size_t OutBufferSize;

	C7zArchive(FileReader *file) : ArchiveStream(file)
	{
		if (g_CrcTable[1] == 0)
		{
			CrcGenerateTable();
		}
		file->Seek(0, SEEK_SET);
		LookToRead_CreateVTable(&LookStream, false);
		LookStream.realStream = &ArchiveStream.s;
		LookToRead_Init(&LookStream);
		SzArEx_Init(&DB);
		BlockIndex = 0xFFFFFFFF;
		OutBuffer = NULL;
		OutBufferSize = 0;
	}

	~C7zArchive()
	{
		if (OutBuffer != NULL)
		{
			IAlloc_Free(&g_Alloc, OutBuffer);
		}
		SzArEx_Free(&DB, &g_Alloc);
	}

	SRes Open()
	{
		return SzArEx_Open(&DB, &LookStream.s, &g_Alloc, &g_Alloc);
	}

	SRes Extract(UInt32 file_index, char *buffer)
	{
		size_t offset, out_size_processed;
		SRes res = SzAr_Extract(&DB, &LookStream.s, file_index,
			&BlockIndex, &OutBuffer, &OutBufferSize,
			&offset, &out_size_processed,
			&g_Alloc, &g_Alloc);
		if (res == SZ_OK)
		{
			memcpy(buffer, OutBuffer + offset, out_size_processed);
		}
		return res;
	}
};


void FixPathSeperator (char *path)
{
	while (*path)
	{
		if (*path == '\\')
			*path = '/';
		path++;
	}
}



//==========================================================================
//
//
//
//==========================================================================

C7zFile::C7zFile(const char * filename) : CZipFile(filename, false)
{
	ReadDirectory(fin);
}

//==========================================================================
//
//
//
//==========================================================================

C7zFile::~C7zFile(void)
{
}


//==========================================================================
//
//
//
//==========================================================================

void C7zFile::ReadDirectory(FileReader *wadinfo)
{
	int skipped = 0;
	SRes res;
	TArray<LumpRecord> EmbeddedWADs;

	Archive = new C7zArchive(wadinfo);
	res = Archive->Open();
	if (res != SZ_OK)
	{
		delete Archive;
		delete wadinfo;
		if (res == SZ_ERROR_UNSUPPORTED)
		{
			I_Error("%s: Decoder does not support this archive\n", m_Filename);
		}
		else if (res == SZ_ERROR_MEM)
		{
			I_Error("%s: Cannot allocate memory\n", m_Filename);
		}
		else if (res == SZ_ERROR_CRC)
		{
			I_Error("%s: CRC error\n", m_Filename);
		}
		else
		{
			I_Error("%s: error #%d\n", m_Filename, res);
		}
		return;
	}

	for (unsigned i = 0; i < Archive->DB.db.NumFiles; ++i)
	{
		CSzFileItem *file = &Archive->DB.db.Files[i];
		char name[256];
		char base[256];

		// skip Directories
		if (file->IsDir)
		{
			skipped++;
			continue;
		}

		strncpy(name, file->Name, 255);
		name[255] = 0;
		FixPathSeperator(name);
		strlwr(name);

		// Check for embedded WADs in the root directory. 
		char *c = strstr(name, ".wad");
		if (c && strlen(c) == 4 && !strchr(name, '/'))
		{
			LumpRecord l;

			memset(&l, 0, sizeof(l));

			l.fullname = _strdup(name);
			l.size = file->Size;
			l.position = i;
			l.compressedsize = -1;

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

		lump_p->size = file->Size;
		lump_p->position = i;
		lump_p->compressedsize = -1;

		if (!strncmp(lump_p->fullname, "maps/", 5))
		{
			mapnames.Push(_strdup(lump_p->name));
		}
	}

	// Entries in archives are sorted alphabetically
	//qsort(&LumpInfo[startlump], NumLumps - startlump, sizeof(LumpRecord), lumpcmp);

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

void C7zFile::ReadLump(LumpRecord *l, void *Ptr)
{
	Archive->Extract(l->position, (char*)Ptr);
}
