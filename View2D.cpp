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
// The 2D map view window
//

#include "stdafx.h"
#include "ZEd.h"
#include "Level.h"
#include "View2D.h"
#include "resourcefile.h"
#include "sc_man.h"


BEGIN_EVENT_TABLE(C2DMapview, wxWindow)
    EVT_PAINT(C2DMapview::OnPaint)
	EVT_SCROLLWIN(C2DMapview::OnScroll)
	EVT_KEY_DOWN(C2DMapview::OnKeyDown) 
	EVT_LEFT_DOWN(C2DMapview::OnLButtonDown)
	EVT_RIGHT_DOWN(C2DMapview::OnRButtonDown)
	EVT_LEFT_UP(C2DMapview::OnLButtonUp)
	EVT_MOTION(C2DMapview::OnMouseMove)

END_EVENT_TABLE()


//==========================================================================
//
//
//
//==========================================================================

C2DMapview::C2DMapview(wxWindow * parent) : wxWindow(parent, -1, wxDefaultPosition, wxDefaultSize,
													wxSUNKEN_BORDER|wxVSCROLL|wxHSCROLL|wxALWAYS_SHOW_SB) 
{
	m_Level=new CLevel(false);
	m_Level->m_DrawWindow=this;
	SetOwnBackgroundColour(wxColour(0,0,0));
}


//==========================================================================
//
//
//
//==========================================================================

C2DMapview::~C2DMapview()
{
	if (m_Level) delete m_Level;
}


//==========================================================================
//
//
//
//==========================================================================

bool C2DMapview::Detach(bool force)
{
	if (m_Level && m_Level->Changed())
	{
		int res = wxMessageBox("Level has not been saved. Do you want to continue?", ZED_CAPTION, wxYES_NO);
		if (res==wxNO) return false;
		delete m_Level;
		m_Level=NULL;
	}
	return true;
}

//==========================================================================
//
//
//
//==========================================================================

void C2DMapview::Attach(CLevel * level)
{
	wxSize sz;
	bool confdone = false;
	wxString gamename;

	if (!m_Level || Detach(false))
	{
		int wad = (level->m_MapLump>>32)-1;
		CResourceFile *resf = CResourceFile::GetResourceFile(wad);
		if (resf != NULL)
		{
			const char * fn = resf->GetName();

			if (config.SetSection("Game"))
			{
				const char *key, *value;
				while (config.NextInSection (key, value))
				{
					if (strchr(key, '*'))
					{
						size_t cmplen = strcspn(key, "*");
						if (!strnicmp(key, fn, cmplen)) gamename = value;
					}
					else if (!stricmp(key, fn))
					{
						gamename = value;
						break;
					}
				}
			}
			if (gamename.length() > 0)
			{
				gamename.MakeUpper();
				GameConfig *newgc = confman.FindDefaultConfig(gamename.c_str(), level->m_Extended, level->m_TextMap);
				if (newgc)
				{
					cgc = newgc;
					confdone = true;
					config.SetSection("IWADs");
					wxString conf = cgc->thisconfig;
					conf.MakeLower();
					size_t pos = conf.find(".cfg");
					if (pos > 0) conf = conf.SubString(0, pos-1);
					const char * iwad = config.GetValueForKey(conf);
					if (iwad != NULL) CResourceFile::SwitchIWAD(iwad);
				}
			}
		}
		if (!confdone)
		{
			cgc=confman.GetConfig(CurrentConfig+".cfg");
			cgc=confman.GetRealConfig(level->Extended(), cgc);
		}

		//LoadPreferences();
		m_Level=level;

		sz = GetClientSize();
		m_Level->m_ScreenCenterX = sz.x >> 1;
		m_Level->m_ScreenCenterY = sz.y >> 1;
		m_Level->OnViewCenterview();

		SetScrollBar(true);
		Refresh();
		m_Level->m_DrawWindow=this;
		m_Level->m_Selection=0;
		m_Level->UpdateStatusBar();
	}
}


//==========================================================================
//
//
//
//==========================================================================

void C2DMapview::OnPaint(wxPaintEvent& event)
{
	wxPaintDC paintdc(this);
	wxSize sz = GetClientSize();
    wxBitmap offscreen(sz.x, sz.y);
	wxMemoryDC dc;

	dc.SetPen(wxPen(wxColour(255,255,255), 1));
	dc.SetBrush(wxBrush(wxColour(255,255,255), wxTRANSPARENT));
	dc.SelectObject(offscreen);

	if (m_Level)
	{
		m_Level->m_ScreenCenterX = sz.x>>1;
		m_Level->m_ScreenCenterY = sz.y>>1;

		CRectFloat crf;
		crf.Set(0, sz.y, sz.x, 0);
		m_Level->OnDraw(dc, &crf);
	}

	paintdc.Blit(0, 0, sz.x, sz.y, &dc, 0, 0, wxCOPY);
	dc.SelectObject(wxNullBitmap);
}


//==========================================================================
//
//
//
//==========================================================================

void C2DMapview::SetScrollBar(bool initcenter)
{
	CRect * lprc=&m_Level->m_Bounds;

	if (initcenter)
	{
		m_Level->m_CenterX = lprc->left + lprc->Width()/2;
		m_Level->m_CenterY = lprc->top + lprc->Height()/2;
	}
	SetScrollbar(wxHORIZONTAL, lprc->Width()/2, 1, lprc->Width());
	SetScrollbar(wxVERTICAL, lprc->Height()/2, 1, lprc->Height());
}


//==========================================================================
//
//
//
//==========================================================================

void C2DMapview::OnScroll(wxScrollWinEvent & event)
{
	WXTYPE ev = event.GetEventType();
	if (m_Level)
	{
		CRect * lpr=&m_Level->m_Bounds;
		int CenterX=m_Level->m_CenterX;
		int Midlevel = (lpr->left+lpr->right)>>1;
		float ZoomFactor=m_Level->m_ZoomFactor;
		double newx=CenterX;
		double zf = max(ZoomFactor, 10);

		if (event.GetOrientation()==wxHORIZONTAL)
		{
			if (ev == wxEVT_SCROLLWIN_TOP)
			{
				newx=lpr->left;
			}
			else if (ev == wxEVT_SCROLLWIN_BOTTOM)
			{
				newx=lpr->right;
			}
			else if (ev == wxEVT_SCROLLWIN_LINEDOWN)
			{
				if (newx>=lpr->right) return;
				newx+=zf;
			}
			else if (ev == wxEVT_SCROLLWIN_LINEUP)
			{
				if (newx<=lpr->left) return;
				newx-=zf;
			}
			else if (ev == wxEVT_SCROLLWIN_PAGEDOWN)
			{
				if (newx>=lpr->right) return;
				newx+=10*ZoomFactor;
				if (newx>lpr->right) newx=lpr->right;
			}
			else if (ev == wxEVT_SCROLLWIN_PAGEUP)
			{
				if (newx<=lpr->left) return;
				newx-=10*ZoomFactor;
				if (newx<lpr->left) newx=lpr->left;
			}
			else if (ev == wxEVT_SCROLLWIN_THUMBRELEASE)
			{
				newx=event.GetPosition()+lpr->left;
			}
			else
				return;

			if (int(newx)==CenterX) return;
			double delta=newx-Midlevel;
			if (ZoomFactor>=1.0f)
			{
				int z=(int)ZoomFactor;
				if (delta>0) delta=((int(delta)+z-1)/z)*z;
				else delta=-((-int(delta)+z-1)/z)*z;
			}
			newx=delta+Midlevel;
			delta=CenterX-newx;
			m_Level->m_CenterX=int(newx);
			SetScrollPos(wxHORIZONTAL, int(newx)-lpr->left);
			Refresh();
		}
		else
		{
			CRect * lpr=&m_Level->m_Bounds;
			int CenterY=m_Level->m_CenterY;
			int Midlevel = (lpr->top+lpr->bottom)>>1;
			float ZoomFactor=m_Level->m_ZoomFactor;
			double newy=CenterY;
			double zf = max(ZoomFactor, 10);
			int range = GetScrollRange(wxVERTICAL);

			if (ev == wxEVT_SCROLLWIN_BOTTOM)
			{
				newy=lpr->top;
			}
			else if (ev == wxEVT_SCROLLWIN_LINEDOWN)
			{
				if (newy<=lpr->top) return;
				newy-=zf;
			}
			else if (ev == wxEVT_SCROLLWIN_LINEUP)
			{
				if (newy>=lpr->bottom) return;
				newy+=zf;
			}
			else if (ev == wxEVT_SCROLLWIN_PAGEDOWN)
			{
				if (newy<=lpr->top) return;
				newy-=10*ZoomFactor;
				if (newy<lpr->top) newy=lpr->top;
			}
			else if (ev == wxEVT_SCROLLWIN_PAGEUP)
			{
				if (newy>=lpr->bottom) return;
				newy+=10*ZoomFactor;
				if (newy>lpr->bottom) newy=lpr->bottom;
			}
			else if (ev == wxEVT_SCROLLWIN_TOP)
			{
				newy=lpr->bottom;
			}
			else if (ev == wxEVT_SCROLLWIN_THUMBRELEASE)
			{
				newy=range-event.GetPosition();
			}
			else
				return;

			if (int(newy)==CenterY) return;
			double delta=newy-Midlevel;
			if (ZoomFactor>=1.0f)
			{
				int z=(int)ZoomFactor;
				if (delta>0) delta=((int(delta)+z-1)/z)*z;
				else delta=-((-int(delta)+z-1)/z)*z;
			}
			newy=delta+Midlevel;
			delta=CenterY-newy;
			m_Level->m_CenterY=int(newy);
			SetScrollPos(wxVERTICAL, range-(int(newy)-lpr->top));
			Refresh();
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void C2DMapview::OnKeyDown(wxKeyEvent & event)
{
	WXTYPE scrollevent;
	int orientation=wxHORIZONTAL;

	switch (event.GetKeyCode())
	{
	default:
		if (m_Level)
		{
			int cmdID = GetCommandFromKey(MODE_2D, event.GetKeyCode(), 
							event.ShiftDown()*S_SHIFT + event.ControlDown()*S_CONTROL + event.AltDown()*S_ALT);
			if (cmdID!=-1) m_Level->ProcessEvent(wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, cmdID));
			else event.Skip();
		}
		return;

	case WXK_UP:
	case WXK_NUMPAD_UP:
		scrollevent = wxEVT_SCROLLWIN_LINEUP;
		orientation = wxVERTICAL;
		break;

	case WXK_DOWN:
	case WXK_NUMPAD_DOWN:
		scrollevent = wxEVT_SCROLLWIN_LINEDOWN;
		orientation = wxVERTICAL;
		break;

	case WXK_LEFT:
	case WXK_NUMPAD_LEFT:
		scrollevent = wxEVT_SCROLLWIN_LINEUP;
		break;

	case WXK_RIGHT:
	case WXK_NUMPAD_RIGHT:
		scrollevent = wxEVT_SCROLLWIN_LINEDOWN;
		break;

	case WXK_PRIOR:
	case WXK_NUMPAD_PRIOR:
		scrollevent = wxEVT_SCROLLWIN_PAGEUP;
		orientation = wxVERTICAL;
		break;

	case WXK_NEXT:
	case WXK_NUMPAD_NEXT:
		scrollevent = wxEVT_SCROLLWIN_PAGEDOWN;
		orientation = wxVERTICAL;
		break;

	case WXK_HOME:
	case WXK_NUMPAD_HOME:
		scrollevent = wxEVT_SCROLLWIN_TOP;
		if (!event.ControlDown()) orientation = wxVERTICAL;
		break;

	case WXK_END:
	case WXK_NUMPAD_END:
		scrollevent = wxEVT_SCROLLWIN_BOTTOM;
		if (!event.ControlDown()) orientation = wxVERTICAL;
		break;
	}
	OnScroll(wxScrollWinEvent(scrollevent, 0, orientation));
}


//==========================================================================
//
//
//
//==========================================================================

void C2DMapview::OnLButtonDown(wxMouseEvent & event)
{
	if (m_Level)
	{
		m_Level->OnLButtonDown(event);
	}
	event.Skip();
}

void C2DMapview::OnMouseMove(wxMouseEvent & event)
{
	if (m_Level)
	{
		m_Level->OnMouseMove(event);
	}
}

void C2DMapview::OnLButtonUp(wxMouseEvent & event)
{
	if (m_Level)
	{
		m_Level->OnLButtonUp(event);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void C2DMapview::OnRButtonDown(wxMouseEvent & event)
{
	if (m_Level)
	{
		m_Level->ContextMenu(event);
	}
}


/*
BOOL C2DMapview::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}
*/

