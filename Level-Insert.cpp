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
// Object insertion
//

#include "StdAfx.h"
#include "View2D.h"
#include "Level.h"
#include "ZEd.h"




//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdDoInsert(bool shift, wxPoint pt)
{
	if (m_Mode==mode3D) return;

	if (m_Mode==modeLineDraw) 
	{
		CancelLineDraw();
		return;
	}

	/* first special case: if several Vertices are selected, add new LineDefs */
	if (m_Mode==modeVertexes && Selection.Size())
	{
		InsertLinesFromVertices(shift);
	}
	/* second special case: if several LineDefs are selected, add new SideDefs and one Sector */
	else if (m_Mode==modeLineDefs && Selection.Size())
	{
		InsertSectorsFromLines();
	}
	/* normal case: add a new object of the current type */
	else
	{
		wxRealPoint p(MX(pt.x),MY(pt.y));
		wxPoint pi(int(p.x+0.5), int(p.y+0.5));

		// Grid!!!
		switch(m_Mode)
		{
		case modeThings:
			MakeBackup("Insert Thing", false, false, false, true);
			m_Selection=InsertThing(GetThing(m_Selection),pi.x, pi.y);
			break;

		case modeVertexes:
			// Only insert if there isn't already a vertex here!
			if (GetVertexFromPoint(pi)<0)
			{
				MakeBackup("Insert Vertex", true, false, false, false);
				m_Selection=InsertVertex(pi.x, pi.y);
				CheckForLineSplit(m_Selection, false);
			}
			break;
		}
	}
	m_DrawWindow->Refresh();
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdInsert(wxCommandEvent & event)
{
	wxPoint pt = m_DrawWindow->ScreenToClient(wxGetMousePosition());
	OnCmdDoInsert(false, pt);
}

void CLevel::OnCmdInsertClose(wxCommandEvent & event)
{
	wxPoint pt = m_DrawWindow->ScreenToClient(wxGetMousePosition());
	OnCmdDoInsert(true, pt);
}

void CLevel::OnCmdInsertM(wxCommandEvent & event)
{
	OnCmdDoInsert(false, m_ContextPoint);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::InsertLinesFromVertices(bool shift)
{
	unsigned int i;
	TArray<int> newselect;
	int n;

	CVertex * cur;
	int firstv=Selection.Size()>=3;

	MakeBackup("Insert Linedefs", false, true, false, false);

	/* create LineDefs between the Vertices */
	for(i=0;i<Selection.Size()-1;i++)
	{
		cur=GetVertex(Selection[i]);
		/* check if there is already a LineDef between the two Vertices */
		for (n = 0; n < NumLines(); n++)
		{
			CLine * ln = GetLine(n);
			if ((ln->line.Start==Selection[i] && ln->line.End==Selection[i+1])
				|| (ln->line.End==Selection[i] && ln->line.Start==Selection[i+1]))
				break;
		}
		if (n >= NumLines())
		{
			n=InsertLineDef(NULL);
			CLine * ln = GetLine(n);
			ln->line.Start = Selection[i+1];
			ln->line.End = Selection[i];
		}
		newselect.Push(n);
	}

	/* close the polygon if there are more than 2 Vertices */
	if (firstv && shift)
	{
		for (n = 0; n < NumLines(); n++)
		{
			CLine * ln = GetLine(n);

			if ((ln->line.Start==Selection[0] && ln->line.End==Selection[Selection.Size()-1])
				|| (ln->line.End==Selection[0] && ln->line.Start==Selection[Selection.Size()-1]))
				break;
		}
		if (n >= NumLines())
		{
			n=InsertLineDef(NULL);
			CLine * ln = GetLine(n);
			ln->line.Start = Selection[0];
			ln->line.End = Selection[Selection.Size()-1];
		}
		newselect.Push(n);
	}
	OnModeLinedefs();
	UncheckAll();
	Selection=newselect;
	for(i=0;i<Selection.Size ();i++)
	{
		checked[Selection[i]]=true;
	}
	m_DrawWindow->Refresh();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::InsertSectorsFromLines()
{
	TArray<int> newselect;
	int CurObject;
	int n;
	CSideDef * sd;

	for (n=0;n<NumLines();n++) if (checked[n])
	{
		if (FrontSecNo(n)!=-1 && BackSecNo(n)!=-1)
		{
			wxMessageBox(wxString::Format("Error: cannot add the new Sector. LineDef #%d already has two SideDefs",n), ZED_CAPTION);
			return;
		}
	}

	MakeBackup("Insert Sectors", false, true, true, false);

	CurObject=InsertSector(NULL);

	int bit = 2;
	if (m_TextMap) bit = cgc->CheckTextMapThingFlag("twosided");

	for (n=0;n<NumLines();n++) if (checked[n])
	{
		CLine * ln = GetLine(n);
		if (ln->sides[0].sector==-1) sd=&ln->sides[0];
		else sd=&ln->sides[1];

		InitSideDef(sd, NULL, ln->sides[0].sector!=-1);
		sd->sector=CurObject;

		if (sd==&ln->sides[1])
		{
			map.CopySector(CurObject, FrontSec(ln));

			ln->line.Flags.SetBit(bit);
			strncpy( ln->sides[0].texNormal, "-", 8);
			strncpy( ln->sides[1].texNormal, "-", 8);
		}
	}
	OnModeSectors();
	UncheckAll();
	m_Selection=CurObject;
	m_DrawWindow->Refresh();
}


//==========================================================================
//
//
//
//==========================================================================

int CLevel::InsertThing(CThing * copything, double xpos, double ypos) 
{
	CThing copy;
	
	if (copything) copy = *copything;	// this may get reallocated so save it locally

	CThing * newthing = map.AddThing();

	if (copything!=NULL)
	{
		*newthing=copy;
		//newthing->CopyProperties();
	}
	else
	{
		newthing->type  = 1;
		if (m_TextMap)
		{
			static const char *keys[] = { "skill1", "skill2", "skill3", "skill4", 
				"skill5", "class1", "class2", "class3", "single", "dm", "coop", NULL};

			newthing->Flags.Clear();
			for(int i=0; keys[i]; i++)
			{
				int bit = cgc->CheckTextMapThingFlag(keys[i]);
				if (bit != -1) newthing->Flags.SetBit(bit);
			}
		}
		else if (!m_Extended) newthing->Flags.SetShort(0x07);
		else newthing->Flags.SetShort(0x7c7);
	}
	if (m_SnapToGrid) SnapToGrid(&xpos, &ypos);
	newthing->SetX(xpos, m_TextMap);
	newthing->SetY(ypos, m_TextMap);
	m_changed=true;
	CRect r = {int(xpos)-512,int(ypos)-512,int(xpos)+512,int(ypos)+512};
	CRect r2 = {-32767,-32767,32767,32767};

	r.IntersectRect(r2);
	m_Bounds.UnionRect(r);
	m_DrawWindow->SetScrollBar(false);
	UpdateStatusBar();
	return NumThings()-1;
}


//==========================================================================
//
//
//
//==========================================================================

int CLevel::InsertVertex(double xpos, double ypos, bool nosnap) 
{
	CVertex * vnew=map.AddVertex();

	if (m_SnapToGrid) SnapToGrid(&xpos, &ypos);
	vnew->SetX(xpos, m_TextMap);
	vnew->SetY(ypos, m_TextMap);
	CRect r = {int(xpos)-512,int(ypos)-512,int(xpos)+512,int(ypos)+512};
	CRect r2 = {-32767,-32767,32767,32767};

	r.IntersectRect(r2);
	m_Bounds.UnionRect(r);
	m_NeedNodeBuild=true;
	m_changed=true;
	UpdateStatusBar();
	return NumVertices()-1;
}


//==========================================================================
//
//
//
//==========================================================================

int CLevel::InsertLineDef(CLine * copyfrom)
{
	CLine copy;

	if (copyfrom) copy = *copyfrom;
	CLine * ln = map.AddLine();
	if (m_Extended) ln->line.tag = -1;

	if (copyfrom )
	{
		*ln = copy;
	}
	else
	{
		InitSideDef(&ln->sides[0], NULL);
		InitSideDef(&ln->sides[1], NULL, true);
		ln->sides[0].sector=ln->sides[1].sector=-1;
	}
	m_NeedNodeBuild=true;
	m_changed=true;
	UpdateStatusBar();
	return NumLines()-1;
}


//==========================================================================
//
//
//
//==========================================================================

int CLevel::InsertSector(CSector * copyfrom)
{
	CSector copy;

	if (copyfrom) copy = *copyfrom;

	CSector * newsec = map.AddSector();

	if (copyfrom)
	{
		*newsec=copy;
	}
	else
	{
		newsec->floorh = DEF_FLOORHEIGHT;
		newsec->ceilh = DEF_CEILINGHEIGHT;
		strncpy( newsec->floort, DEF_FLOORTEX, 8);	// change!
		strncpy( newsec->ceilt, DEF_CEILTEX, 8);
		newsec->light = DEF_LIGHT;
		newsec->special = 0;
		newsec->tag = 0;
	}
	m_NeedNodeBuild=true;
	m_changed=true;
	UpdateStatusBar();
	return NumSectors()-1;
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::InsertPolygonVertices( int centerx, int centery, int sides, int radius)
{
	int n;

	for (n = 0; n < sides; n++)
		InsertVertex(centerx + (int) ((double) radius * cos( 6.28 * (double) n / (double) sides)), centery + (int) ((double) radius * sin( 6.2832 * (double) n / (double) sides)));
}


//==========================================================================
//
//
//
//==========================================================================


int CLevel::SplitLine(int l, int vertex)
{
	int line = InsertLineDef(GetLine(l));
	GetLine(l)->line.End = GetLine(line)->line.Start = vertex;
	return line;
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::CheckForLineSplit(int refv, bool force)
{
	int Distance(CVertex * v1,CVertex * v2);

	//if (!force && !m_autosplit) return;

	CVertex * vt = GetVertex(refv);

	for(int ld = NumLines()-1; ld>=0; ld--)
	{
		CLine * ln = GetLine(ld);
		if (refv==StVtNo(ln) || refv==EnVtNo(ln)) continue;

		double d = distance_to_line(StVt(ln)->X(), StVt(ln)->Y(), EnVt(ln)->X(), EnVt(ln)->Y(),	vt->X(), vt->Y());

		if (d<0.5)
		{
			int orglength=Distance(StVt(ln), EnVt(ln));
			int newlength1=Distance(StVt(ln), vt);
			int newlength2=Distance(vt, EnVt(ln));

			if (newlength1>0 && newlength2>0)
			{
				SplitLine(ld, refv);
				// may have been reallocated!
				GetLine(ld)->sides[1].xoff=newlength2;
				GetLine(NumLines()-1)->sides[0].xoff=newlength1;
			}
		}
	}
	m_changed=true;
	m_NeedNodeBuild=true;
}


