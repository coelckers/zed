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
// Implements a single lump resource file
//

#include "StdAfx.h"
#include "ResourceFile.h"


//==========================================================================
//
//
//
//==========================================================================

static inline bool IsSeperator (int c)
{
	if (c == '/') return true;
#ifdef WIN32
	if (c == '\\' || c == ':') return true;
#endif
	return false;
}

//==========================================================================
//
//
//
//==========================================================================

void ExtractFileBase (const char *path, char *dest)
{
	const char *src;

	src = path + strlen(path) - 1;

	if (src >= path)
	{
		// back up until a / or the start
		while (src != path && !IsSeperator(*(src-1)))
			src--;

		// Check for files with drive specification but no path
#if defined(_WIN32) || defined(DOS)
		if (src == path && src[0] != 0)
		{
			if (src[1] == ':')
				src += 2;
		}
#endif

		while (*src && *src != '.')
		{
			*dest++ = *src++;
		}
	}
	*dest = 0;
}

//==========================================================================
//
//
//
//==========================================================================

CSingleLump::CSingleLump(const char * filename)
{
	char buffer[256];
	m_NumLumps=1;
	m_Filename=_strdup(filename);
	m_Data=NULL;
	ExtractFileBase(filename, buffer);
	buffer[8]=0;
	strcpy(lumpname, buffer);
	strupr(lumpname);

	FILE * f = fopen(filename, "rb");
	if (f!=NULL)
	{
		fseek(f, 0 ,SEEK_END);
		m_Length=ftell(f);
		fseek(f, 0, SEEK_SET);
		m_Data = new char[m_Length];
		fread(m_Data, 1, m_Length, f);
		fclose(f);
	}
}

//==========================================================================
//
//
//
//==========================================================================

CSingleLump::~CSingleLump(void)
{
	if (m_Data!=NULL) delete [] m_Data;
}

//==========================================================================
//
//
//
//==========================================================================

void CSingleLump::GetMapList(TArray<const char *> & array)
{
}


//==========================================================================
//
//
//
//==========================================================================

int CSingleLump::FindLump(const char * name)
{
	if (!stricmp(name, lumpname)) return 0;
	return -1;
}

//==========================================================================
//
//
//
//==========================================================================

void CSingleLump::FindLumps(const char * name, TArray<QWORD> & list)
{
	if (!stricmp(name, lumpname)) list.Push(m_LumpStart);
}

//==========================================================================
//
//
//
//==========================================================================

int CSingleLump::GetLumpSize(int lump)
{
	if (lump==0) return m_Length;
	else return -1;
}

//==========================================================================
//
//
//
//==========================================================================

void * CSingleLump::ReadLump(int lump)
{
	if (lump==0)
	{
		void * Ptr= malloc(m_Length);
		memcpy(Ptr, m_Data, m_Length);
		return Ptr;
	}
	return NULL;
}

//==========================================================================
//
//
//
//==========================================================================

void CSingleLump::ReadLump(int lump, void * buffer)
{
	if (lump==0)
	{
		memcpy(buffer, m_Data, m_Length);
	}
}

//==========================================================================
//
//
//
//==========================================================================

const char * CSingleLump::GetLumpName(int lump)
{
	if (lump==0)
	{
		return lumpname;
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

void CSingleLump::ReplaceLump(int lump, void * data, int length)
{
	if (m_Data) delete [] m_Data;
	m_Length=length;
	m_Data = new char[length];
	memcpy(m_Data, data, length);
}

//==========================================================================
//
//
//
//==========================================================================

void CSingleLump::Save(const char * filename)
{
	// This can never be a map so don't bother!
}

//==========================================================================
//
//
//
//==========================================================================

void * CSingleLump::CreateInMemory(int * length)
{
	// This can never be embedded into a .zip and won't ever be called
	*length=0;
	return NULL;
}

