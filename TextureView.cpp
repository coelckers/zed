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
// Texture browser control
//

#include "stdafx.h"
#include "ZEd.h"
#include "texture.h"

#define TEX_WIDTH 100
#define TEX_SPACE_X 20
#define TEX_HEIGHT 100
#define TEX_SPACE_Y 30


//==========================================================================
//
//
//
//==========================================================================

TextureList::TextureList(wxWindow * parent, int id)
:	wxWindow(parent, id, wxDefaultPosition, wxDefaultSize, 
			 wxSUNKEN_BORDER|wxFULL_REPAINT_ON_RESIZE|wxVSCROLL|wxALWAYS_SHOW_SB),
	m_topleft(0), m_Selection(0)
{
	SetOwnBackgroundColour(wxColour(0,0,0));
	SetMinSize(wxSize(4*(TEX_WIDTH+TEX_SPACE_X), 3*(TEX_HEIGHT+TEX_SPACE_Y)));
}

//==========================================================================
//
//
//
//==========================================================================

TextureList::~TextureList()
{
}



BEGIN_EVENT_TABLE(TextureList, wxWindow)
    EVT_PAINT(TextureList::OnPaint)
	EVT_SCROLLWIN(TextureList::OnScroll)
	EVT_KEY_DOWN(TextureList::OnKeyDown) 
	EVT_LEFT_DOWN(TextureList::OnLButtonDown)
	EVT_SIZE(TextureList::OnSize)
	EVT_LEFT_DCLICK(TextureList::OnLButtonDblClk)
END_EVENT_TABLE()

//==========================================================================
//
//
//
//==========================================================================

void TextureList::OnPaint(wxPaintEvent & event)
{
	wxPaintDC dc(this);
	int tx,ty;
	wxSize size;

	size = GetClientSize();
	tx=size.x/(TEX_WIDTH+TEX_SPACE_X);
	ty=size.y/(TEX_HEIGHT+TEX_SPACE_Y);

	wxFont * Font = wxFont::New(wxSize(5,12), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false,
		"Arial", wxFONTENCODING_SYSTEM);

	dc.SetFont(*Font);
	for(int y=0;y<ty;y++) for(int x=0;x<tx;x++) if (m_topleft+x+y*tx<(int)m_texturelist.Size())
	{
		Texture * tex = texman->RegisterTexture(m_texturelist[m_topleft+x+y*tx]);

		if (tex)
		{
			const char * b=NULL;
			int draww=TEX_WIDTH;
			int drawh=TEX_HEIGHT;
			int drawx=0;
			int drawy=0;

			GLTexture * gltex = tex->gltexture;
			wxBitmap * bits = gltex->CreateTexBuffer();

			if (bits)
			{
				wxSize sz = wxSize( bits->GetWidth(), bits->GetHeight() );
				wxMemoryDC bitmap_dc;

				bitmap_dc.SelectObject(*bits);

				float drawratio = (float)TEX_WIDTH/TEX_HEIGHT;
				float picratio = (float)sz.x/sz.y;

				if (drawratio>picratio)
				{
					int newdraw = int(TEX_WIDTH * picratio / drawratio); 
					drawx = (TEX_WIDTH-newdraw)/2;
					draww = newdraw;
				}
				else
				{
					int newdraw = int(TEX_HEIGHT * drawratio / picratio); 
					drawy = (TEX_HEIGHT-newdraw)/2;
					drawh = newdraw;
				}

				float scalex = (float)sz.x/TEX_WIDTH;
				float scaley = (float)sz.y/TEX_HEIGHT;
				float scale = max(scalex, scaley);
				bitmap_dc.SetUserScale(scale, scale);

				drawx+=TEX_SPACE_X/2 + (TEX_WIDTH+TEX_SPACE_X)*x;
				drawy+=TEX_SPACE_Y/2 + (TEX_HEIGHT+TEX_SPACE_Y)*y;
 				dc.Blit(drawx, drawy, draww, drawh, &bitmap_dc, 0, 0); 
				bitmap_dc.SelectObject(wxNullBitmap);
			}

			long tx, ty;
			wxString str = wxString::Format("%.8s", tex->GetName());
			dc.GetTextExtent(str, &tx, &ty);
			dc.SetTextForeground(wxColour(255,255,255));
			dc.SetBackgroundMode(wxTRANSPARENT);
			dc.DrawText(str, TEX_SPACE_X/2 + (TEX_WIDTH+TEX_SPACE_X)*x + TEX_WIDTH/2 - tx/2,
							 TEX_SPACE_Y/2 + (TEX_HEIGHT+TEX_SPACE_Y)*y + TEX_HEIGHT + 2);

			str = wxString::Format("%s, (%dx%d)",
				TextureTypeNames[tex->GetTextureType()], tex->TextureWidth(), tex->TextureHeight());
			dc.GetTextExtent(str, &tx, &ty);
			dc.DrawText(str, TEX_SPACE_X/2 + (TEX_WIDTH+TEX_SPACE_X)*x + TEX_WIDTH/2 - tx/2,
							 TEX_SPACE_Y/2 + (TEX_HEIGHT+TEX_SPACE_Y)*y + TEX_HEIGHT + 2+ty);
		}
	}

	if (m_Selection!=-1)
	{
		dc.SetPen(*wxWHITE_PEN);
		dc.SetBrush(*wxTRANSPARENT_BRUSH);

		int selpos = m_Selection-m_topleft;
		int sel_y = selpos/tx;
		int sel_x = selpos%tx;

		dc.DrawRectangle(sel_x*(TEX_WIDTH+TEX_SPACE_X)+1, sel_y*(TEX_HEIGHT+TEX_SPACE_Y)+10,
						 TEX_WIDTH+TEX_SPACE_X,  TEX_HEIGHT+TEX_SPACE_Y);
	}	
}


//==========================================================================
//
//
//
//==========================================================================

void TextureList::OnScroll(wxScrollWinEvent & event)
{
	WXTYPE ev = event.GetEventType();
	wxSize size = GetClientSize();
	int tx=(size.x)/(TEX_WIDTH+TEX_SPACE_X);
	int ty=(size.y)/(TEX_HEIGHT+TEX_SPACE_Y);
	int maxtop = ((m_texturelist.Size()-tx*ty+tx-1)/tx)*tx;

	int oldtop=m_topleft;

	if (ev == wxEVT_SCROLLWIN_LINEDOWN)
	{
		m_topleft-=tx;
		if (m_topleft<0) m_topleft=0;
	}
	else if (ev == wxEVT_SCROLLWIN_LINEDOWN)
	{
		m_topleft+=tx;
		if (m_topleft>maxtop) m_topleft=maxtop;
	}
	else if (ev == wxEVT_SCROLLWIN_PAGEUP)
	{
		m_topleft-=tx*ty;
		if (m_topleft<0) m_topleft=0;
	}
	else if (ev == wxEVT_SCROLLWIN_PAGEDOWN)
	{
		m_topleft+=tx*ty;
		if (m_topleft>maxtop) m_topleft=maxtop;
	}
	else if (ev == wxEVT_SCROLLWIN_TOP)
	{
		m_topleft=0;
	}
	else if (ev == wxEVT_SCROLLWIN_BOTTOM)
	{
		m_topleft=maxtop;
	}
	if (oldtop!=m_topleft) 
	{
		SetScrollPos(wxVERTICAL, m_topleft/tx);
		Refresh();
	}
}

//==========================================================================
//
//
//
//==========================================================================

void TextureList::OnKeyDown(wxKeyEvent & event)
{
	WXTYPE scrollevent;
	int orientation=wxVERTICAL;

	switch (event.GetKeyCode())
	{
	default:
		event.Skip();
		return;

	case WXK_UP:
	case WXK_NUMPAD_UP:
		scrollevent = wxEVT_SCROLLWIN_LINEUP;
		break;

	case WXK_DOWN:
	case WXK_NUMPAD_DOWN:
		scrollevent = wxEVT_SCROLLWIN_LINEDOWN;
		break;

	case WXK_PRIOR:
	case WXK_NUMPAD_PRIOR:
		scrollevent = wxEVT_SCROLLWIN_PAGEUP;
		break;

	case WXK_NEXT:
	case WXK_NUMPAD_NEXT:
		scrollevent = wxEVT_SCROLLWIN_PAGEDOWN;
		break;

	case WXK_HOME:
	case WXK_NUMPAD_HOME:
		scrollevent = wxEVT_SCROLLWIN_TOP;
		break;

	case WXK_END:
	case WXK_NUMPAD_END:
		scrollevent = wxEVT_SCROLLWIN_BOTTOM;
		break;
	}
	OnScroll(wxScrollWinEvent(scrollevent, 0, orientation));
}



//==========================================================================
//
//
//
//==========================================================================

void TextureList::OnSize(wxSizeEvent& event)
{
	wxSize size = GetClientSize();

	int tx=(size.x)/(TEX_WIDTH+TEX_SPACE_X);
	int ty=(size.y)/(TEX_HEIGHT+TEX_SPACE_Y);

	if (tx>0 && ty>0)
	{
		int maxtop = ((m_texturelist.Size()-tx*ty+tx-1)/tx);

		m_topleft = ((m_topleft+tx-1)/tx)*tx;
		SetScrollbar(wxVERTICAL, m_topleft/tx, 1, maxtop);
		Refresh();
	}
}

//==========================================================================
//
//
//
//==========================================================================

void TextureList::OnLButtonDown(wxMouseEvent & event)
{
	wxSize size = GetClientSize();
	wxPoint point = event.GetPosition();

	int tx=(size.x)/(TEX_WIDTH+TEX_SPACE_X);
	int ty=(size.y)/(TEX_HEIGHT+TEX_SPACE_Y);
	int maxtop = ((m_texturelist.Size()-tx*ty+tx-1)/tx)*tx;

	point.x/=(TEX_WIDTH+TEX_SPACE_X);
	point.y/=(TEX_HEIGHT+TEX_SPACE_Y);

	wxClientDC dc(this);
	dc.SetPen(*wxWHITE_PEN);
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.SetLogicalFunction(wxXOR);

	if (m_Selection!=-1)
	{
		int selpos = m_Selection-m_topleft;
		int sel_y = selpos/tx;
		int sel_x = selpos%tx;

		dc.DrawRectangle(sel_x*(TEX_WIDTH+TEX_SPACE_X)+1, sel_y*(TEX_HEIGHT+TEX_SPACE_Y)+10,
					 TEX_WIDTH+TEX_SPACE_X, TEX_HEIGHT+TEX_SPACE_Y);
	}
	{
		m_Selection=m_topleft+point.x+point.y*tx;
		int selpos = m_Selection-m_topleft;
		int sel_y = selpos/tx;
		int sel_x = selpos%tx;

		dc.DrawRectangle(sel_x*(TEX_WIDTH+TEX_SPACE_X)+1, sel_y*(TEX_HEIGHT+TEX_SPACE_Y)+10,
			TEX_WIDTH+TEX_SPACE_X, TEX_HEIGHT+TEX_SPACE_Y);
	}	
}

void TextureList::OnLButtonDblClk(wxMouseEvent & event)
{
	OnLButtonDown(event);
	ProcessEvent(wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED, GetId()));
}

//==========================================================================
//
//
//
//==========================================================================

void TextureList::SetTextures(TArray<int> & newlist)
{
	m_texturelist = newlist;
	m_topleft=0;
	Refresh();
}

//==========================================================================
//
//
//
//==========================================================================

void TextureList::SetSelection(int newval)
{
	for(unsigned i=0;i<m_texturelist.Size();i++)
	{
		if (m_texturelist[i]==newval)
		{
			m_Selection=i;
			UpdateScrollPos();
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void TextureList::SetToName(const wxString & name)
{
	int len=0;
	int choice=-1;
	for(unsigned i=0;i<m_texturelist.Size();i++)
	{
		const char * texname = texman->GetTextureName(m_texturelist[i]);

		while (1)
		{
			if (!strnicmp(name.c_str(), texname, len+1))
			{
				len++;
				choice=i;
				continue;
			}
			break;
		}
		if (len==name.length()) break;
	}
	if (choice!=-1)
	{
		m_Selection=choice;
		UpdateScrollPos();
	}
}

//==========================================================================
//
//
//
//==========================================================================

void TextureList::UpdateScrollPos()
{
	wxSize size = GetClientSize();

	int tx=(size.x)/(TEX_WIDTH+TEX_SPACE_X);
	int ty=(size.y)/(TEX_HEIGHT+TEX_SPACE_Y);
	int maxtop = ((m_texturelist.Size()-tx*ty+tx-1)/tx)*tx;

	if (m_Selection>=maxtop)
	{
		if (m_topleft!=maxtop)
		{
			m_topleft=maxtop;
			Refresh();
			return;
		}
	}
	if (m_Selection<tx*ty)
	{
		if (m_topleft!=0)
		{
			m_topleft=0;
			Refresh();
			return;
		}
	}
	m_topleft=m_Selection/tx*tx;
	Refresh();
}


