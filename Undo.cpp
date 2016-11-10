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
// Implementation of the Undo buffer
// This uses zlib for compression because uncompressed undo records
// can become quite large. Compression method 1 is used to reduce 
// compression time. Absolute compression ratio is not that important here.
//
#include "StdAfx.h"
#include "zip/zlib/zlib.h"
#include "Level.h"

//static TArray<CProperty> propsave;

int CUndoBuffer::ProcessBuffer(void * s_data, void ** bufpAddr, int buflen)
{
	Bytef * data = (Bytef *)malloc(buflen+12);
	uLongf dstbufsize=buflen+12;

	compress2(data, &dstbufsize, (Bytef*)s_data, buflen, 1);
	data = (Bytef*)realloc(data, dstbufsize);
	*bufpAddr = (void*)data;
	return dstbufsize;
}

CUndoBuffer::CUndoBuffer(CLevel * t, const char * description, bool vertices, bool lines, bool sectors, bool things)
{
	target=t;
	m_descript=_strdup(description);
	m_Vertices=NULL;
	m_Sectors=NULL;
	m_Things=NULL;
	m_Lines=NULL;

	if (vertices)
	{
		m_nVertices = t->NumVertices();
		m_cVertices = ProcessBuffer(&t->map.m_Vertices[0], &m_Vertices, m_nVertices * sizeof(CVertex));
	}
	if (lines)
	{
		m_nLines = t->NumLines();
		m_cLines = ProcessBuffer(&t->map.m_Lines[0], &m_Lines, m_nLines * sizeof(CLine));

	}
	if (sectors)
	{
		m_nSectors = t->NumSectors();
		m_cSectors = ProcessBuffer(&t->map.m_Sectors[0], &m_Sectors, m_nSectors * sizeof(CSector));
	}
	if (things)
	{
		m_nThings = t->NumThings();
		m_cThings = ProcessBuffer(&t->map.m_Things[0], &m_Things, m_nThings * sizeof(CThing));
	}
}


CUndoBuffer::~CUndoBuffer()
{
	if (m_Lines) free(m_Lines);
	if (m_Vertices) free(m_Vertices);
	if (m_Sectors) free(m_Sectors);
	if (m_Things) free(m_Things);
	free((void*)m_descript);
}


void CUndoBuffer::Apply()
{
	uLongf siz;
	int propindex=0;

	if (m_Vertices) 
	{
		target->map.ClearVertices();
		target->map.m_Vertices.Resize(m_nVertices);
		siz=m_nVertices*sizeof(CVertex);
		uncompress( (Bytef*)&target->map.m_Vertices[0], &siz, (Bytef*)m_Vertices, m_cVertices);
	}
	if (m_Lines) 
	{
		target->map.ClearLines();
		target->map.m_Lines.Resize(m_nLines);
		siz=m_nLines*sizeof(CLine);
		uncompress( (Bytef*)&target->map.m_Lines[0], &siz, (Bytef*)m_Lines, m_cLines);
	}
	if (m_Sectors) 
	{
		target->map.ClearSectors();
		target->map.m_Sectors.Resize(m_nSectors);
		siz=m_nSectors*sizeof(CSector);
		uncompress( (Bytef*)&target->map.m_Sectors[0], &siz, (Bytef*)m_Sectors, m_cSectors);
	}
	if (m_Things) 
	{
		target->map.ClearThings();
		target->map.m_Things.Resize(m_nThings);
		siz=m_nThings*sizeof(CThing);
		uncompress( (Bytef*)&target->map.m_Things[0], &siz, (Bytef*)m_Things, m_cThings);
	}
}
