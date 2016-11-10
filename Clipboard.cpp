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
// Clipboard.cpp : implementation of the CClipboard class
//

#include "StdAfx.h"
#include "Level.h"

CClipboard::CClipboard(void)
{
}

CClipboard::~CClipboard(void)
{
	Clear();
}


void CClipboard::Clear()
{
	//for(unsigned i=0;i<m_Things.Size();i++) m_Things[i].ReleaseProperties();
	//for(unsigned i=0;i<m_Sectors.Size();i++) m_Sectors[i].ReleaseProperties();
	//for(unsigned i=0;i<m_Lines.Size();i++) m_Lines[i].ReleaseProperties(); 

	m_Things.Clear();
	m_Lines.Clear();
	m_Sectors.Clear();
	m_Vertices.Clear();
}

void CClipboard::Copy(CLevel * l)
{
	TArray<int> linestocopy;
	TArray<int> sectorstocopy;
	TArray<int> verticestocopy;
	int minx=32767;
	int maxx=-32767;
	int miny=32767;
	int maxy=-32767;

	if (l->Selection.Size()==0 && l->m_Selection==-1) return;

	Clear();

	if (l->m_Mode == l->modeThings)
	{
		if (l->Selection.Size() > 0)
		{
			for (unsigned i = 0; i < l->Selection.Size(); i++)
			{
				CThing * t = l->GetThing(l->Selection[i]);
				int x = int(t->X());
				int y = int(t->Y());

				if (x < minx) minx = x;
				if (x > maxx) maxx = x;
				if (y < miny) miny = y;
				if (y > maxy) maxy = y;
			}

			minx += ((maxx - minx) / 2);
			miny += ((maxy - miny) / 2);

			// Add things
			for (unsigned i = 0; i < l->Selection.Size(); i++)
			{
				CThing * t = l->GetThing(l->Selection[i]);
				CThing * thing = &m_Things[m_Things.Push(*t)];
				thing->SetX(thing->X() - minx, true);
				thing->SetY(thing->Y() - minx, true);
			}
		}
		else
		{
			CThing * t = l->GetThing(l->m_Selection);
			m_Things.Push(*t);
			m_Things[0].SetX(0, false);
			m_Things[0].SetY(0, false);
		}
		return;
	}
	switch (l->m_Mode)
	{
	case l->modeLineDefs:
		if (l->Selection.Size())
		{
			linestocopy = l->Selection;
		}
		else
		{
			linestocopy.Push (l->m_Selection);
		}
		break;

	case l->modeSectors:
		if (l->Selection.Size())
		{
			for (int i = 0; i < l->NumLines(); i++)
			{
				CLine * ln = l->GetLine(i);
				// Add all lines touching a selected sector
				if (l->Selection.Find( ln->sides[0].sector) ||
					l->Selection.Find( ln->sides[1].sector))
				{
					linestocopy.Push(i);
				}
			}
		}
		else
		{
			for (int i = 0; i < l->NumLines(); i++)
			{
				CLine * ln = l->GetLine(i);
				if (ln->sides[0].sector == l->m_Selection ||
					ln->sides[1].sector == l->m_Selection)
				{
					linestocopy.Push(i);
				}
			}
		}
		break;

	case l->modeVertexes:
		// copy all lines which have both ends selected
		if (l->Selection.Size())
		{
			for (int i = 0; i < l->NumLines(); i++)
			{
				if (l->Selection.Find( l->StVtNo(i)) &&	l->Selection.Find( l->EnVtNo(i)))
				{
					linestocopy.Push(i);
				}
			}
		}
		break;
	}

	if (linestocopy.Size()==0) return;	// nothing to copy


	for(unsigned i=0;i<linestocopy.Size();i++)
	{
		CLine * ln = l->GetLine(linestocopy[i]);
		int s1 = ln->sides[0].sector;
		int s2 = ln->sides[1].sector;

		int v1 = ln->line.Start;
		int v2 = ln->line.End;

		if (s1!=-1) sectorstocopy.Add(s1);
		if (s2!=-1) sectorstocopy.Add(s2);

		verticestocopy.Add(v1);
		verticestocopy.Add(v2);

		m_Lines.Push(*ln);
		//m_Lines[i].CopyProperties();
	}

	for(unsigned i=0;i<sectorstocopy.Size();i++)
	{
		m_Sectors.Push(*l->GetSector(sectorstocopy[i]));
		//m_Sectors[i].CopyProperties();
	}

	for(unsigned i=0;i<verticestocopy.Size();i++)
	{
		CVertex * v = l->GetVertex(verticestocopy[i]);
		int x = int(v->X());
		int y = int(v->Y());

		if (x < minx) minx = x;
		if (x > maxx) maxx = x;
		if (y < miny) miny = y;
		if (y > maxy) maxy = y;
	}

	minx += ((maxx - minx) / 2);
	miny += ((maxy - miny) / 2);
	for(unsigned i=0;i<verticestocopy.Size();i++)
	{
		CVertex v = *l->GetVertex(verticestocopy[i]);
		v.SetX(v.X()-minx, l->m_TextMap);
		v.SetY(v.Y()-miny, l->m_TextMap);

		m_Vertices.Push(v);
	}

	// There's still some relocation work to do for the vertex and sector
	// references in the line array.

	// First create some inverse mappings to make this more efficient.
	// Sectors are mapped so that index 0 refers to the unused marker '-1'.

	int * invsectors = new int[l->NumSectors()+1];
	int * invvertices= new int[l->NumVertices()];

	invsectors[0]=-1;
	for(unsigned i=0;i<sectorstocopy.Size();i++)
	{
		invsectors[sectorstocopy[i]+1]=i;
	}
	for(unsigned i=0;i<verticestocopy.Size();i++)
	{
		invvertices[verticestocopy[i]]=i;
	}

	for(unsigned i=0;i<m_Lines.Size();i++)
	{
		m_Lines[i].line.Start = invvertices[m_Lines[i].line.Start];
		m_Lines[i].line.End   = invvertices[m_Lines[i].line.End  ];

		m_Lines[i].sides[0].sector = invsectors[m_Lines[i].sides[0].sector+1];
		m_Lines[i].sides[1].sector = invsectors[m_Lines[i].sides[1].sector+1];
	}
	delete [] invsectors;
	delete [] invvertices;
}

void CClipboard::Paste(CLevel * l, int x, int y)
{
	if (m_Things.Size()>0)
	{
		l->MakeBackup("Paste Things", false, false, false, true);
		l->OnModeThings();
		for(unsigned i=0;i<m_Things.Size();i++)
		{
			CThing t=m_Things[i];
			int index = l->InsertThing(&t, t.X()+x, t.Y()+y);
			l->Select(index, true);
		}
	}
	else
	{
		int verts = l->NumVertices();
		int sectors = l->NumSectors();

		l->MakeBackup("Paste Geometry", true, true, true, false);

		for(unsigned i=0;i<m_Vertices.Size();i++)
		{
			m_Vertices[i].SetX(m_Vertices[i].X()+x, l->m_TextMap);
			m_Vertices[i].SetY(m_Vertices[i].Y()+y, l->m_TextMap);
			if (l->m_SnapToGrid) l->SnapToGrid(&m_Vertices[i]);

			l->InsertVertex(m_Vertices[i].X(), m_Vertices[i].Y());
		}

		for(unsigned i=0;i<m_Sectors.Size();i++)
		{
			l->InsertSector(&m_Sectors[i]);
		}

		l->OnModeLinedefs();
		for(unsigned i=0;i<m_Lines.Size();i++)
		{
			m_Lines[i].line.Start += verts;
			m_Lines[i].line.End += verts;
			if (m_Lines[i].sides[0].sector!=-1) m_Lines[i].sides[0].sector += sectors;
			if (m_Lines[i].sides[1].sector!=-1) m_Lines[i].sides[1].sector += sectors;

			int line = l->InsertLineDef(&m_Lines[i]);
			//l->GetLine(line)->CopyProperties();
		}

		// l->AutoMergeVertices();
		//l->OnDeleteunusedsectors(wxCommandEvent());
	}
}

void CClipboard::DrawSelection(wxDC & DC, CLevel * l, int x, int y)
{
	DC.SetLogicalFunction(wxXOR);
	if (m_Things.Size() > 0)
	{
		for(unsigned i=0;i<m_Things.Size();i++)
		{
			l->DrawThing(DC, &m_Things[i], m_Things[i].X()+x, m_Things[i].Y()+y);
		}
	}
	else
	{
		for(unsigned i=0;i<m_Lines.Size();i++)
		{
			CVertex v1 = m_Vertices[m_Lines[i].line.Start];
			CVertex v2 = m_Vertices[m_Lines[i].line.End];
			v1.SetX(v1.X()+x, l->m_TextMap);
			v1.SetY(v1.Y()+y, l->m_TextMap);
			v2.SetX(v2.X()+x, l->m_TextMap);
			v2.SetY(v2.Y()+y, l->m_TextMap);
			if (l->m_SnapToGrid) 
			{
				l->SnapToGrid(&v1);
				l->SnapToGrid(&v2);
			}
			l->DrawLinedef(DC, l->hpLr, &v1, &v2);
		}
	}
	DC.SetLogicalFunction(wxCOPY);
}
