//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2006 Christoph Oelckers
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
// Container class for map data
//


#include "StdAfx.h"
#include "MapData.h"



Map::Map()
{
	m_Things.Reserve(1024);
	m_Things.Clear();
	m_Sectors.Reserve(1024);
	m_Sectors.Clear();
	m_Lines.Reserve(2048);
	m_Lines.Clear();
	m_Vertices.Reserve(2048);
	m_Vertices.Clear();
}

Map::~Map()
{
	ClearThings();
	ClearSectors();
	ClearVertices();
	ClearLines();
}

void Map::ShrinkToFit()
{
	m_Things.ShrinkToFit();
	m_Sectors.ShrinkToFit();
	m_Vertices.ShrinkToFit();
	m_Lines.ShrinkToFit();
}

void Map::ClearThings()
{
	m_Things.Clear();
}

void Map::DeleteThing(int index)
{
	if ((unsigned)index>=m_Things.Size()) return;
	m_Things.Delete(index);
}

CThing * Map::AddThing()
{
	CThing empty;

	memset(&empty, 0, sizeof(CThing));
	m_Things.Push(empty);
	return &m_Things.Last();
}

void Map::CopyThing(int index, CThing * thing)
{
	if ((unsigned)index>=m_Things.Size()) return;

	m_Things[index] = *thing;
}

void Map::ClearSectors()
{
	m_Sectors.Clear();
}

void Map::DeleteSector(int index)
{
	if ((unsigned)index>=m_Sectors.Size()) return;
	m_Sectors.Delete(index);
}

CSector * Map::AddSector()
{
	CSector empty;

	empty.Init();
	m_Sectors.Push(empty);
	return &m_Sectors.Last();
}

void Map::CopySector(int index, CSector * Sector)
{
	if ((unsigned)index>=m_Sectors.Size()) return;

	m_Sectors[index] = *Sector;
	m_Sectors[index].buggy=false;
}

void Map::ClearVertices()
{
	m_Vertices.Clear();
}

void Map::DeleteVertex(int index)
{
	if ((unsigned)index>=m_Vertices.Size()) return;
	m_Vertices.Delete(index);
}

CVertex * Map::AddVertex()
{
	CVertex empty;

	memset(&empty, 0, sizeof(CVertex));
	m_Vertices.Push(empty);
	return &m_Vertices.Last();
}

void Map::ClearLines()
{
	m_Lines.Clear();
}

void Map::DeleteLine(int index)
{
	if ((unsigned)index>=m_Lines.Size()) return;
	m_Lines.Delete(index);
}

CLine * Map::AddLine()
{
	CLine empty;

	memset(&empty, 0, sizeof(CLine));
	m_Lines.Push(empty);
	return &m_Lines.Last();
}

void Map::CopyLine(int index, CLine * Line)
{
	if ((unsigned)index>=m_Lines.Size()) return;

	m_Lines[index] = *Line;
	m_Lines[index].buggy=false;
}


int Distance(CVertex * v1,CVertex * v2)
{
	double dx=fabs(v1->X()-v2->X());
	double dy=fabs(v1->Y()-v2->Y());

	return (int)(hypot(dx,dy)+0.5);
}

// Yes, this leaks memory but considering the normal use it's not worth bothering
// Doing a proper implementation of string properties will cost a lot more than this leaks.

static TArray<char> PropStrings;

void ClearPropertyStrings()
{
	PropStrings.Clear();
}

int MakeStringIndex(const char *s)
{
	unsigned index = PropStrings.Size();
	if (index == 0) index = 1;
	PropStrings.Resize(index + strlen(s) + 1);
	strcpy(&PropStrings[index], s);
	return index;
}

const char *GetStringFromIndex(int i)
{
	if (unsigned(i) < PropStrings.Size()) return &PropStrings[i];
	return "";
}
