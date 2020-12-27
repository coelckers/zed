/*
** file_grp.cpp
**
**---------------------------------------------------------------------------
** Copyright 1998-2009 Randy Heit
** Copyright 2005-2009 Christoph Oelckers
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
**
*/

#include "stdafx.h"
#include "ResourceFile.h"


//==========================================================================
//
//
//
//==========================================================================

CGrpFile::CGrpFile(const char * filename)
{
	
	char type[12];
	int numlumps;

	m_Filename = _strdup(filename);
	m_File=NULL;

	FILE *f = fopen(filename, "rb");
	if (!f) return;

	m_File = f;
	fread(&type, 1, 12, f);
	fread(&numlumps, 4, 1, f);

	CGrpLump lump;
	
	int Position = 16 + numlumps * sizeof(GrpEntry);

	for (int i = 0; i < numlumps; i++)
	{
		lump.offset = Position;
		fread(lump.name, 1, 12, f);
		lump.name[12] = 0;
		fread(&lump.length, 4, 1, f);
		lump.data = nullptr;
		Position += lump.length;
		lumps.Push(lump);
	}

	for (int i = 1; i < numlumps; i++)
	{
		if (strstr(lumps[i].name, ".map") || strstr(lumps[i].name, ".MAP"))
		{
			// Do a sanity check to verify that this map is properly accessible
			if (FindLump(lumps[i-1].name) == i-1)
			{
				mapnames.Push({ QWORD(i - 1), lumps[i - 1].name });
			}
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

CGrpFile::~CGrpFile(void)
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

int CGrpFile::FindLump(const char * name)
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

void CGrpFile::FindLumps(const char * name, TArray<QWORD> & list)
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

int CGrpFile::GetLumpSize(int lump)
{
	if ((unsigned)lump < lumps.Size() ) return lumps[lump].length;
	else return -1;
}

//==========================================================================
//
//
//
//==========================================================================

void * CGrpFile::ReadLump(int lump)
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

void CGrpFile::ReadLump(int lump, void * Ptr)
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

const char * CGrpFile::GetLumpName(int lump)
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

void CGrpFile::ReplaceLump(int lump, void * data, int length)
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

void CGrpFile::Save(const char * filename)
{
	// later
}

//==========================================================================
//
//
//
//==========================================================================

void * CGrpFile::CreateInMemory(int * length)
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

void CGrpFile::GetMapList(TArray<MapRecord> & array)
{
	for(unsigned int i=0;i<mapnames.Size ();i++)
	{
		array.Push(mapnames[i]);
	}
	SortMapList(array);
}


