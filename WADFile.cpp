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
// WAD file implementation
//
#include "StdAfx.h"
#include "ResourceFile.h"

//==========================================================================
//
//
//
//==========================================================================

CWADFile::CWADFile(const char * filename)
{
	char type[4];
	int diroffset;
	int numlumps;

	m_Filename = _strdup(filename);
	m_File=NULL;

	FILE *f = fopen(filename, "rb");
	if (!f) return;

	m_File = f;
	fread(&type, 1, 4, f);
	fread(&numlumps, 4, 1, f);
	fread(&diroffset, 4, 1, f);

	IWAD = (type[0] == 'I');

	fseek(f, diroffset, SEEK_SET);

	CWADLump lump;

	for (int i = 0; i < numlumps; i++)
	{
		fread(&lump.offset, 4, 1, f);
		fread(&lump.length, 4, 1, f);
		fread(lump.name, 1, 8, f);
		lump.name[8]=0;
		lump.data=NULL;

		lumps.Push(lump);

	}

	for (int i = 1; i < numlumps; i++)
	{
		// we can't look for the level header. It can be any name with some ports
		// and 0-length is also no useful indicator.

		if (i<numlumps-9 &&
			!stricmp(lumps[i].name, "THINGS") &&
			!stricmp(lumps[i+1].name, "LINEDEFS") &&
			!stricmp(lumps[i+2].name, "SIDEDEFS") &&
			!stricmp(lumps[i+3].name, "VERTEXES"))// &&
			/* the rest can't be used because the BSP lumps can be omitted.
			!stricmp(lumps[i+4].name, "SEGS") &&
			!stricmp(lumps[i+5].name, "SSECTORS") &&
			!stricmp(lumps[i+6].name, "NODES") &&
			!stricmp(lumps[i+7].name, "SECTORS") &&
			!stricmp(lumps[i+8].name, "REJECT") &&
			!stricmp(lumps[i+9].name, "BLOCKMAP"))
			*/
		{
			// Do a sanity check to verify that this map is properly accessible
			if (FindLump(lumps[i-1].name) == i-1)
			{
				mapnames.Push(lumps[i-1].name);
			}
		}
		else if (!stricmp(lumps[i].name, "TEXTMAP"))
		{
			if (FindLump(lumps[i-1].name) == i-1)
			{
				mapnames.Push(lumps[i-1].name);
			}
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

CWADFile::CWADFile(const char * name, const char * memory)
{
	int diroffset;
	int numlumps;

	m_Filename = _strdup(name);
	m_File=NULL;

	numlumps=*(int*)(memory+4);
	diroffset=*(int*)(memory+8);

	IWAD = false;

	const char * readptr = memory+diroffset;

	CWADLump lump;

	for (int i = 0; i < numlumps; i++)
	{
		lump.offset=*(int*)readptr;
		lump.length=*(int*)(readptr+4);
		memcpy(lump.name, readptr+8, 8);
		readptr+=16;
		lump.name[8]=0;
		lump.data = new char[lump.length];
		memcpy(lump.data, memory+lump.offset, lump.length);

		lumps.Push(lump);
	}

	for (int i = 1; i < numlumps-9; i++)
	{
		// we can't look for the level header. It can be any name with some ports
		// and 0-length is also no useful indicator.

		if (!stricmp(lumps[i].name, "THINGS") &&
			!stricmp(lumps[i+1].name, "LINEDEFS") &&
			!stricmp(lumps[i+2].name, "SIDEDEFS") &&
			!stricmp(lumps[i+3].name, "VERTEXES"))// &&
			/*
			!stricmp(lumps[i+4].name, "SEGS") &&
			!stricmp(lumps[i+5].name, "SSECTORS") &&
			!stricmp(lumps[i+6].name, "NODES") &&
			!stricmp(lumps[i+7].name, "SECTORS") &&
			!stricmp(lumps[i+8].name, "REJECT") &&
			!stricmp(lumps[i+9].name, "BLOCKMAP"))
			*/
		{
			// Do a sanity check to verify that this map is properly accessible
			if (FindLump(lumps[i-1].name) == i-1)
			{
				mapnames.Push(lumps[i-1].name);
			}
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

CWADFile::~CWADFile(void)
{
	for(unsigned int i=0;i<lumps.Size(); i++)
	{
		if (lumps[i].data!=NULL) delete[] lumps[i].data;
	}
	if (m_File) fclose(m_File);
}

//==========================================================================
//
//
//
//==========================================================================

int CWADFile::FindLump(const char * name)
{
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

void CWADFile::FindLumps(const char * name, TArray<QWORD> & list)
{
	for(unsigned int i=0;i<lumps.Size(); i++)
	{
		if (!stricmp(name, lumps[i].name)) list.Push(i+m_LumpStart);
	}
}


//==========================================================================
//
//
//
//==========================================================================

int CWADFile::GetLumpSize(int lump)
{
	if ((unsigned)lump < lumps.Size() ) return lumps[lump].length;
	else return -1;
}

//==========================================================================
//
//
//
//==========================================================================

void * CWADFile::ReadLump(int lump)
{
	if ((unsigned)lump < lumps.Size() )
	{
		void * Ptr= malloc(lumps[lump].length);
		ReadLump(lump, Ptr);
		return Ptr;
	}
	return NULL;
}

//==========================================================================
//
//
//
//==========================================================================

void CWADFile::ReadLump(int lump, void * Ptr)
{
	if ((unsigned)lump < lumps.Size() )
	{
		if (lumps[lump].data) memcpy(Ptr, lumps[lump].data, lumps[lump].length);
		else
		{
			fseek(m_File, lumps[lump].offset, SEEK_SET);
			fread(Ptr, 1, lumps[lump].length, m_File);
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

const char * CWADFile::GetLumpName(int lump)
{
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

void CWADFile::ReplaceLump(int lump, void * data, int length)
{
	if ((unsigned)lump < lumps.Size() )
	{
		if (lumps[lump].data) delete [] lumps[lump].data;
		lumps[lump].length = length;
		lumps[lump].data = new char[length];
		memcpy(lumps[lump].data, data, length);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CWADFile::Save(const char * filename)
{
	// later
}

//==========================================================================
//
//
//
//==========================================================================

void * CWADFile::CreateInMemory(int * length)
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

void CWADFile::GetMapList(TArray<const char *> & array)
{
	for(unsigned int i=0;i<mapnames.Size ();i++)
	{
		array.Push(mapnames[i]);
	}
	SortMapList(array);
}

//==========================================================================
//
//
//
//==========================================================================

void CWADFile::GetAllSprites(TArray<QWORD> & list) 
{
	bool adding = false;
	for(unsigned i=0;i<lumps.Size();i++)
	{
		if (!stricmp(lumps[i].name, "S_START") || !stricmp(lumps[i].name, "SS_START"))
		{
			adding=true;
		}
		else if (!stricmp(lumps[i].name, "S_END") || !stricmp(lumps[i].name, "SS_END"))
		{
			adding=false;
		}
		else if (adding)
		{
			list.Push(i+m_LumpStart);
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CWADFile::GetAllFlats(TArray<QWORD> & list) 
{
	bool adding = false;
	for(unsigned i=0;i<lumps.Size();i++)
	{
		if (!stricmp(lumps[i].name, "F_START") || !stricmp(lumps[i].name, "FF_START"))
		{
			adding=true;
		}
		else if (!stricmp(lumps[i].name, "F_END") || !stricmp(lumps[i].name, "FF_END"))
		{
			adding=false;
		}
		else if (adding)
		{
			list.Push(i+m_LumpStart);
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CWADFile::GetAllTextures(TArray<QWORD> & list) 
{
	bool adding = false;
	for(unsigned i=0;i<lumps.Size();i++)
	{
		if (!stricmp(lumps[i].name, "TX_START"))
		{
			adding=true;
		}
		else if (!stricmp(lumps[i].name, "TX_END"))
		{
			adding=false;
		}
		else if (adding)
		{
			list.Push(i+m_LumpStart);
		}
	}
}
