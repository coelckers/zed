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
// Object deletion code
//

#include "StdAfx.h"
#include "ZEd.h"
#include "View2D.h"
#include "Level.h"


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdDelete(wxCommandEvent & event)
{
	switch(m_Mode)
	{
		case modeThings:
			OnCmdDeleteThings();
			break;

		case modeVertexes:
			OnCmdDeleteVertexes();
			break;

		case modeSectors:
			OnCmdDeleteSectors();
			break;

		case modeLineDefs:
			OnCmdDeleteLineDefs();
			break;

		/*
		case modeLineDraw:
			DeleteLineDrawVertex();
			break;
		*/
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdDeleteThings()
{
	int i;

	if (NumThings()==0) return;
	if (m_Selection<0) m_Selection=0;
	for(i=0;i<NumThings();i++) if (checked[i]) break;
	if (i==NumThings()) Select(m_Selection, true);


	MakeBackup("Delete Things", false, false, false, true);
	for (i=NumThings()-1;i>=0;i--)
	{
		if (checked[i])	
		{
			map.DeleteThing(i);
		}
	}
	m_Selection=NumThings()-1;
	m_DrawWindow->Refresh();
	UncheckAll();
	m_changed=true;
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdDeleteVertexes()
{
	int i;

	if (NumVertices()==0) return;
	if (m_Selection<0) m_Selection=0;
	for(i=0;i<NumVertices();i++) if (checked[i]) break;
	if (i==NumVertices()) Select(m_Selection, true);

	MakeBackup("Delete Vertices", true, true, true, false);
	for(i=NumVertices()-1;i>=0;i--)
	{
		if (checked[i])	DeleteOneVertex(i);
	}
	m_Selection=NumVertices()-1;
	m_DrawWindow->Refresh();
	UncheckAll();
	m_NeedNodeBuild=true;
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::DeleteOneVertex(int i)
{
	int n;

	if (m_Mode==modeVertexes)
	{
		Select(i, false);
		checked.Delete(i);
	}

	for(n=NumLines()-1;n>=0;n--)
	{
		CLine * ln = GetLine(n);
		if (ln->line.Start==i || ln->line.End==i)
		{
			DeleteOneLineDef(n);
		}
		else
		{
			if (ln->line.Start>i) ln->line.Start--;
			if (ln->line.End>i) ln->line.End--;
		}
	}

	map.DeleteVertex(i);
	m_NeedNodeBuild=true;
	m_changed=true;
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdDeleteLineDefs()
{
	int i;

	if (NumLines()==0) return;
	if (m_Selection<0) m_Selection=0;
	for(i=0;i<(int)NumLines();i++) if (checked[i]) break;
	if (i==NumLines()) Select(m_Selection, true);

	MakeBackup("Delete Linedefs", false, true, true, false);
	for(i=NumLines()-1;i>=0;i--)
	{
		if (checked[i]) DeleteOneLineDef(i);
	}
	m_Selection=0;
	m_DrawWindow->Refresh();
	UncheckAll();
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::DeleteOneLineDef(int no)
{
	CLine * pth=GetLine(no);

	if (m_Mode==modeLineDefs)
	{
		Select(no, false);
		checked.Delete(no);
	}

	for(int k=0;k<2;k++)
	{
		if (pth->sides[k].sector!=-1)
		{
			int sec = pth->sides[k].sector;
			pth->sides[k].sector=-1;

			for(int l=0;l<NumLines();l++) for(int j=0;j<2;j++)
			{
				if (GetLine(l)->sides[j].sector==sec) goto no;
			}
			DeleteOneSector(sec);
		no:;
		}
	}

	map.DeleteLine(no);
	m_NeedNodeBuild=true;
	m_changed=true;
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdDeleteSectors()
{
	int i;

	if (NumSectors()==0) return;
	if (m_Selection<0) m_Selection=0;
	for(i=0;i<NumSectors();i++) if (checked[i]) break;
	if (i==NumSectors()) Select(m_Selection, true);

	MakeBackup("Delete Sectors", false, true, true, false);
	for(i=NumSectors();i>=0;i--)
	{
		if (checked[i]) DeleteOneSector(i);
	}
	m_Selection=NumSectors()-1;
	m_DrawWindow->Refresh();
	UncheckAll();
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::DeleteOneSector(int no)
{
	int i;

	if (m_Mode==modeSectors)
	{
		Select(no, false);
		checked.Delete(no);
	}


	for(i=0;i<NumLines();i++)
	{
		CLine * ln = GetLine(i);
		if (ln->sides[0].sector==no) ln->sides[0].sector=-1;
		if (ln->sides[0].sector>no) ln->sides[0].sector--;
		if (ln->sides[1].sector==no) ln->sides[1].sector=-1;
		if (ln->sides[1].sector>no) ln->sides[1].sector--;
	}
	map.DeleteSector(no);
	m_NeedNodeBuild=true;
	m_changed=true;
}

