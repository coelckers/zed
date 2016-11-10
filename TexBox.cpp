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
// Texture control for dialogs
//

#include "stdafx.h"
#include "ZEd.h"
#include "texture.h"

//==========================================================================
//
//
//
//==========================================================================

TextureBox::TextureBox(wxWindow *parent, int id, int width, int height)
:	wxWindow(parent, id, wxDefaultPosition, wxSize(width, height), wxSUNKEN_BORDER|wxFULL_REPAINT_ON_RESIZE)
{
	texnum=-1;
	width=height=0;

	if (id==-1) Enable(false);
	Show(true);
	Raise();
}

//==========================================================================
//
//
//
//==========================================================================

TextureBox::~TextureBox()
{
}

BEGIN_EVENT_TABLE(TextureBox, wxWindow)
	EVT_PAINT(TextureBox::OnPaint)
	EVT_LEFT_DOWN(TextureBox::OnClick)
END_EVENT_TABLE()

//==========================================================================
//
//
//
//==========================================================================

void TextureBox::OnPaint(wxPaintEvent &event)
{
	wxPaintDC dc(this);
	if (texman==NULL)
	{
		// Texture is invalid
		texnum=-1;
		return;
	}

	wxSize sz = GetClientSize();
	int drawx;
	int drawy;

	wxPen p1(wxColour(0,0,0), 1);
	wxBrush b1(wxColour(0,0,0));

	dc.SetPen(p1);
	dc.SetBrush(b1);
	dc.DrawRectangle(0,0, sz.x, sz.y);

	wxBitmap * bits = texman->CreateTexture(texnum);
	if (bits)
	{
		
		wxMemoryDC bitmap_dc;
		bitmap_dc.SelectObject(*bits);

		float drawratio = (float)sz.x/sz.y;
		float picratio = (float)width/height;

		drawx=drawy=0;
		if (drawratio>picratio)
		{
			int newdraw = int(sz.x * picratio / drawratio); 
			drawx = (sz.x-newdraw)/2;
			sz.x = newdraw;
		}
		else
		{
			int newdraw = int(sz.y * drawratio / picratio); 
			drawy = (sz.y-newdraw)/2;
			sz.y = newdraw;
		}

		float scalex = (float)width/sz.x;
		float scaley = (float)height/sz.y;
		float scale = max(scalex, scaley);
		bitmap_dc.SetUserScale(scale, scale);

 		dc.Blit(drawx, drawy, sz.x, sz.y, &bitmap_dc, 0, 0); 
		bitmap_dc.SelectObject(wxNullBitmap);
	}
	else
	{
		wxPen p2(wxColour(127,127,127), 2);
		dc.SetPen(p1);
		dc.DrawLine(0, 0, sz.x, sz.y);
		dc.DrawLine(sz.x, 0, 0, sz.y);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void TextureBox::OnClick(wxMouseEvent &event)
{
	if (GetId()!=-1)
	{
		wxCommandEvent ev(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
		GetParent()->ProcessEvent(ev);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void TextureBox::SetTexture(const char * tn, tx_namespace txn)
{
	int t;

	if (tn)
	{
		ValidateTextureManager();
		if (tn) t=texman->CheckTextureNumForName(tn, txn);
		else t=-1;

		if (t!=texnum)
		{
			texnum=t;
			if (t>=0)
			{
				width=texman->TextureWidth(t);
				height=texman->TextureHeight(t);
				type=texman->GetTextureType(t);
			}
			else
			{
				width=height=0;
			}
		}
	}
	Refresh(false);
}


bool SpinCtrlWithModify::ProcessEvent(wxEvent& event)
{
	return wxSpinCtrl::ProcessEvent(event);
}

