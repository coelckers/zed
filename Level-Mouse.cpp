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
// Mouse handling
//

#include "StdAfx.h"
#include "ResourceFile.h"
#include "ZEd.h"
#include "View2D.h"
//#include "Clipboard.h"
#include "Level.h"
#include "GameConfig.h"




//==========================================================================
//
//
//
//==========================================================================

void CLevel::ContextMenu(wxMouseEvent & event)
{

	if (m_Mode==modeLineDraw)
	{
		CancelLineDraw();
		return;
	}

	ZEdFrame * frame = wxGetApp().GetFrame();
	wxMenu * menu = m_Mode==modeThings? frame->m_ThingMenu: 
					m_Mode==modeVertexes? frame->m_VertexMenu:
					m_Mode==modeSectors? frame->m_SectorMenu:
					m_Mode==modeLineDefs? frame->m_LineMenu : NULL;

	if (menu==NULL || m_Selection<0) return;

	if (m_Mode==modeLineDefs)
	{
		bool second=false;
		int j;

		for(j=0;j<NumLines();j++) if (checked[j]) break;

		if (j==NumLines()) 
		{
			if (m_Selection > -1 && BackSecNo(m_Selection)!=-1) second=true;		
		}
		else for(j=0;j<NumLines();j++) 
		{
			if (checked[j] && BackSecNo(j)!=-1)
			{
				second=true;
				break;
			}
		}
		menu->Enable(CMD_DEL2ND, second);
	}

	m_DrawWindow->PopupMenu(menu);
	m_ContextPoint=event.GetPosition();
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnLButtonUp(wxMouseEvent & event)
{
	CLevelDC DC(this);

	switch(m_MouseMode)
	{
	case MMODE_MARK:
		DrawMarkRect(DC,m_MousePoint,m_LastMousePoint);

	case MMODE_MOVE:
		m_MouseMode=MMODE_NONE;
		//AutoMergeVertices(-1);
		m_DrawWindow->ReleaseMouse();
		break;
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnLButtonDown(wxMouseEvent & event)
{
	CLevelDC DC(this);
	wxRealPoint pt(MX(event.GetX()),MY(event.GetY()));

	if (m_drawClipboard)
	{

		m_drawClipboard=false;
		clipboard.Paste(this, (int)pt.x, (int)pt.y);
		m_DrawWindow->Refresh();
		UpdateStatusBar();
		return;
	}

	/*
	if (m_Mode == modeLineDraw)
	{
		OnLButtonLineDraw(DC,pt);
	}
	else*/ if (event.ShiftDown())
	{
		MakeBackup("Move stuff", true, true, true, true);
		m_LastMousePoint=m_MousePoint=pt;
		m_MouseMode=MMODE_MOVE;
		m_DrawWindow->CaptureMouse();
	}
	else if (event.ControlDown())
	{
		m_LastMousePoint=m_MousePoint=pt;
		m_MouseMode=MMODE_MARK;
		m_DrawWindow->CaptureMouse();
		DrawMarkRect(DC,pt,pt);
	}
	else switch(m_Mode)
	{
		case modeThings:
			OnLButtonThings(DC,pt);
			break;

		case modeVertexes:
			OnLButtonVertexes(DC,pt);
			break;

		case modeSectors:
			OnLButtonSectors(DC,pt);
			break;

		case modeLineDefs:
			OnLButtonLineDefs(DC,pt);
			break;
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnLButtonThings(wxDC & DC,const wxRealPoint & p)
{
	CThing * pth;

	if (m_Selection>=0 && NumThings()>0)
	{
		pth=GetThing(m_Selection);
		Select(m_Selection, !checked[m_Selection]);
		MarkThing(DC,pth, hpGr, false);
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnLButtonVertexes(wxDC & DC,const wxRealPoint & p)
{
	CVertex * pth;

	if (m_Selection>=0 && NumVertices()>0)
	{
		pth=GetVertex(m_Selection);
		Select(m_Selection, !checked[m_Selection]);
		MarkVertex(DC,pth,false);
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnLButtonSectors(wxDC & DC,const wxRealPoint & p)
{
	if (m_Selection>=0 && NumSectors()>0)
	{
		Select(m_Selection, !checked[m_Selection]);
		HighlightSector(DC, m_Selection, hpGr, false);
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnLButtonLineDefs(wxDC & DC,const wxRealPoint & p)
{
	if (m_Selection>=0 && NumLines())
	{
		Select(m_Selection, !checked[m_Selection]);
		HighlightLineDef(DC, m_Selection, hpGr, false);
	}
}


//==========================================================================
//
//
//
//==========================================================================

bool CLevel::OnMouseMove(wxMouseEvent & event)
{
	CLevelDC DC(this);
	wxRealPoint pt(MX(event.GetX()),MY(event.GetY()));

	wxGetApp().GetFrame()->SetCoordinates(int(pt.x), int(pt.y));

	switch(m_MouseMode)
	{
	case MMODE_NONE:
		if (!event.ShiftDown())
		{
			switch(m_Mode)
			{
			case modeThings:
				OnMMoveThings(DC, pt);
				break;

			case modeVertexes:
				OnMMoveVertexes(DC, pt);
				break;

			case modeSectors:
				OnMMoveSectors(DC, pt);
				break;

			case modeLineDefs:
				OnMMoveLineDefs(DC, pt);
				break;

				/*
			case modeLineDraw:
				OnMMoveLineDraw(DC,pt);
				break;
				*/

			}
			if (m_drawClipboard)
			{
				clipboard.DrawSelection(DC, this, m_drawclipx, m_drawclipy);
				m_drawclipx = (int)pt.x;
				m_drawclipy = (int)pt.y;
				clipboard.DrawSelection(DC, this, m_drawclipx, m_drawclipy);
			}
		}
		break;

	case MMODE_MARK: 
		{
			DrawMarkRect(DC,m_MousePoint,m_LastMousePoint);
			m_LastMousePoint=pt;
			DrawMarkRect(DC,m_MousePoint,m_LastMousePoint);
			switch(m_Mode)
			{
			case modeThings:
				OnMMoveMarkThings(DC);
				break;

			case modeVertexes:
				OnMMoveMarkVertexes(DC);
				break;

			case modeSectors:
				// costs too much time!
				//OnMMoveMarkSectors(DC);
				break;

			case modeLineDefs:
				OnMMoveMarkLineDefs(DC);
				break;
			}
			break;
		}

	case MMODE_MOVE: 
		{
			double xoff, yoff;

			PushChecked();
			switch(m_Mode)
			{
			case modeThings:
				OnMMoveMoveThings(DC,int(pt.x-m_LastMousePoint.x),int(pt.y-m_LastMousePoint.y));
				break;

			case modeSectors:
				CheckLineDefsFromSectors();
			case modeLineDefs:
				CheckVertexesFromLineDefs();
			case modeVertexes:
				xoff=(pt.x)-m_LastMousePoint.x;
				if (xoff<0) xoff=floor(xoff);
				else xoff=ceil(xoff);
				yoff=(pt.y)-m_LastMousePoint.y;
				if (yoff<0) yoff=floor(yoff);
				else yoff=ceil(yoff);
				OnMMoveMoveVertexes(DC, int(xoff), int(yoff));
				break;

			}
			PopChecked();
			m_LastMousePoint.x=pt.x;
			m_LastMousePoint.y=pt.y;
			break;
		}
	}
	return true;
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnMMoveThings(wxDC & DC, const wxRealPoint & p)
{
	int pth;
	float ptx=0.4f * m_ZoomFactor;
	CRectFloat r;

	r.Set(p.x-ptx,p.y-ptx,p.x+ptx,p.y+ptx);
	pth=GetThingFromPos(&r);
	if (pth!=-1 && pth!=m_Selection)
	{
		if (m_Selection>=0) MarkThing(DC,GetThing(m_Selection),hpYe,true);
		MarkThing(DC,GetThing(pth),hpYe,true);
		m_Selection=pth;
		UpdateStatusBar();
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnMMoveVertexes(wxDC & DC, const wxRealPoint & p)
{
	int i;
	float ptx=0.4f * m_ZoomFactor;
	CRectFloat r;

	r.Set(p.x-ptx,p.y-ptx,p.x+ptx,p.y+ptx);
	i=GetVertexFromPos(&r);
	if (i!=-1 && i!=m_Selection)
	{
		if (m_Selection>=0) MarkVertex(DC,GetVertex(m_Selection),true);
		MarkVertex(DC,GetVertex(i),true);
		m_Selection=i;
		UpdateStatusBar();
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnMMoveSectors(wxDC & DC, const wxRealPoint & p)
{
	int i;

	i=GetSectorFromPos(p);
	if (i!=-1 && i!=m_Selection )
	{
		if (m_Selection>=0) HighlightSector(DC,m_Selection,hpYe,true);
		HighlightSector(DC,i,hpYe,true);
		m_Selection=i;
		UpdateStatusBar();
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnMMoveLineDefs(wxDC & DC,const wxRealPoint & p)
{
	float ptx=0.4f * m_ZoomFactor;
	CRectFloat r;
	int i;

	r.Set(p.x-ptx,p.y-ptx,p.x+ptx,p.y+ptx);
	i=GetLineDefFromPos(&r);
	if (i!=-1 && i!=m_Selection)
	{
		if (m_Selection>=0) HighlightLineDef(DC,m_Selection,hpYe,true);
		HighlightLineDef(DC,i,hpYe,true);
		m_Selection=i;
		UpdateStatusBar();
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnMMoveMarkThings(wxDC & DC)
{
	int i;
	CRectFloat r;

	r.left=min(m_MousePoint.x,m_LastMousePoint.x);
	r.right=max(m_MousePoint.x,m_LastMousePoint.x);
	r.top=min(m_MousePoint.y,m_LastMousePoint.y);
	r.bottom=max(m_MousePoint.y,m_LastMousePoint.y);

	for(i=0;i<NumThings();i++)
	{
		CThing * pth = GetThing(i);
		if (r.PointInRect(*pth))
		{
			if (!checked[i]) 
			{
				MarkThing(DC,pth, hpGr, false);
				Select(i, true);
			}
		}
		else
		{
			if (checked[i]) 
			{
				MarkThing(DC,pth, hpGr, false);
				Select(i, false);
			}
		}
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnMMoveMarkVertexes(wxDC & DC)
{
	int i;
	CRectFloat r;

	r.left=min(m_MousePoint.x,m_LastMousePoint.x);
	r.right=max(m_MousePoint.x,m_LastMousePoint.x);
	r.top=min(m_MousePoint.y,m_LastMousePoint.y);
	r.bottom=max(m_MousePoint.y,m_LastMousePoint.y);

	for(i=0;i<NumVertices();i++)
	{
		CVertex * pth=GetVertex(i);
		if (r.PointInRect(wxRealPoint(*pth)))
		{
			if (!checked[i]) 
			{
				MarkVertex(DC,pth, false);
				Select(i, true);
			}
		}
		else
		{
			if (checked[i]) 
			{
				MarkVertex(DC,pth, false);
				Select(i, false);
			}
		}
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnMMoveMarkSectors(wxDC & DC)
{
	CSector * pth;
	int i;
	CRectFloat r;

	r.left=min(m_MousePoint.x,m_LastMousePoint.x);
	r.right=max(m_MousePoint.x,m_LastMousePoint.x);
	r.top=min(m_MousePoint.y,m_LastMousePoint.y);
	r.bottom=max(m_MousePoint.y,m_LastMousePoint.y);


	for(i=0;i<NumSectors();i++)
	{
		pth = GetSector(i);
		if (IsSectorInside(i,r.left,r.top,r.right,r.bottom))
		{
			if (!checked[i])
			{
				HighlightSector(DC,i,hpGr,false);
				Select(i, true);
			}
		}
		else
		{
			if (checked[i])
			{
				HighlightSector(DC,i,hpGr,false);
				Select(i, false);
			}
		}
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnMMoveMarkLineDefs(wxDC & DC)
{
	CRectFloat r;
	int i;

	r.left=min(m_MousePoint.x,m_LastMousePoint.x);
	r.right=max(m_MousePoint.x,m_LastMousePoint.x);
	r.top=min(m_MousePoint.y,m_LastMousePoint.y);
	r.bottom=max(m_MousePoint.y,m_LastMousePoint.y);

	for(i=0;i<NumLines();i++)
	{
		if (IsLineDefInside(i,r.left,r.top,r.right,r.bottom))
		{
			if (!checked[i]) 
			{
				HighlightLineDef(DC,i,hpGr,false);
				Select(i, true);
			}
		}
		else
		{
			if (checked[i]) 
			{
				HighlightLineDef(DC,i,hpGr,FALSE);
				Select(i, false);
			}
		}
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnMMoveMoveThings(wxDC & DC,int xoff,int yoff)
{
	int i;
	CThing * pth;

	if (NumThings()==0) return;
	if (m_Selection<0) m_Selection=0;
	for(i=0;i<NumThings();i++) if (checked[i]) break;
	if (i==NumThings()) 
	{
		pth=GetThing(m_Selection);
		pth->SetX(pth->X() + xoff, m_TextMap);
		pth->SetY(pth->Y() + yoff, m_TextMap);
	}
	else for(i=0;i<NumThings();i++) if (checked[i]) 
	{
		pth=GetThing(i);
		pth->SetX(pth->X() + xoff, m_TextMap);
		pth->SetY(pth->Y() + yoff, m_TextMap);
	}
	m_DrawWindow->Refresh();
	m_changed=true;
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnMMoveMoveVertexes(wxDC & DC,int xoff,int yoff)
{
	int i;

	if (NumVertices()==0) return;
	if (m_Selection<0) m_Selection=0;
	for(i=0;i<NumVertices();i++) if (checked[i]) break;
	if (i==NumVertices()) 
	{
		CVertex * pth=GetVertex(m_Selection);
		pth->SetX(pth->X()+xoff, m_TextMap);
		pth->SetY(pth->Y()+yoff, m_TextMap);
	}
	else for(i=0;i<NumVertices();i++) if (checked[i]) 
	{
		CVertex * pth=GetVertex(i);
		pth->SetX(pth->X()+xoff, m_TextMap);
		pth->SetY(pth->Y()+yoff, m_TextMap);
	}
	m_DrawWindow->Refresh();
	m_changed=true;
}
