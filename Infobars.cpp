//
//-----------------------------------------------------------------------------
//
// Copyright (C) 1995-2006 Christoph Oelckers
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
// Info bars
//

#include "StdAfx.h"
#include "Level.h"
#include "ZEd.h"



//==========================================================================
//
//
//
//==========================================================================

ThingBar::ThingBar(wxWindow *parent) : wxPanel(parent)
{
	wxBoxSizer *barsizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(barsizer);

	m_frame = new wxStaticBox(this, -1, "No Thing selected");
	wxFont framefont = m_frame->GetFont();
	framefont.SetWeight(wxFONTWEIGHT_BOLD);
	m_frame->SetFont(framefont);

	wxStaticBoxSizer *box = new wxStaticBoxSizer(m_frame, wxHORIZONTAL);
	barsizer->Add(box, 1, wxEXPAND|wxRIGHT, 4);

	for(int i = 0; i < 2; i++)
	{
		wxBoxSizer *row = new wxBoxSizer(wxVERTICAL);
		row->SetMinSize(200, -1);
		if (i==0) box->Add(row, 0, wxRIGHT, 8);
		else box->Add(row);
		for(int j = 0; j < 7; j++)
		{
			m_Items[i][j] = new wxStaticText(this, -1, "");
			row->Add(m_Items[i][j]);
		}
	}

	// Sprite frame
	wxStaticBox * spriteframe = new wxStaticBox(this, -1, "");
	box = new wxStaticBoxSizer(spriteframe, wxVERTICAL);
	barsizer->Add(box, 0, wxEXPAND);

	m_sprite = new TextureBox(this, -1, 96, 96);
	box->Add(m_sprite, 1, wxALIGN_CENTER);

	SetSizeHints(-1, 128, -1, 128);
	SetSizer(barsizer);
	Layout();
}


//==========================================================================
//
//
//
//==========================================================================

void ThingBar::Update(int num, CThing * th, bool textmap)
{
	static_cast<ZEdFrame*>(GetParent())->SetInfoBar(this);

	if (num==-1 || th == NULL)
	{
		m_frame->SetLabel("No Thing selected");
		for(int i = 0; i < 2; i++)
		{
			for(int j = 0; j < 7; j++)
			{
				m_Items[i][j]->SetLabel("");
			}
		}
		m_sprite->SetTexture("", tx_sprite);
		return;
	}
	else
	{
		int line = 0;
		m_frame->SetLabel(wxString::Format("Thing #%d: %s", num, cgc->GetThingName(th->type)));
		if (textmap)
		{
			m_Items[0][line++]->SetLabel(wxString::Format("Position: (%1.3f, %1.3f, %1.3f)", th->X(), th->Y(), th->Z()));
		}
		else if (cgc->IsExtended)
		{
			m_Items[0][line++]->SetLabel(wxString::Format("Position: (%d, %d, %d)", int(th->X()), int(th->Y()), int(th->Z())));
		}
		else
		{
			m_Items[0][line++]->SetLabel(wxString::Format("Position: (%d, %d)", int(th->X()), int(th->Y())));
		}

		m_Items[0][line++]->SetLabel(wxString::Format("Angle: %d", th->angle));

		if (!textmap)
		{
			wxString flagstr = "Flags:";
			for(int i=0;i<MAX_FLAGS;i++)
			{
				if (th->Flags.GetBit(i))
				{
					wxString t = cgc->GetThingFlagText(i, true, textmap);
					if (!!t) 
					{
						flagstr+=" ";
						flagstr+=t;
					}
				}
			}
			m_Items[0][line++]->SetLabel(flagstr);
		}
		else
		{
			for(int i=0; i<cgc->ThingGroupCount;i++)
			{
				FlagGroupDesc *fg = &cgc->TextMapThingFlagGroups[i];
				wxString flagstr = fg->name;
				flagstr += ":";
				for(int j= fg->firstindex; j < fg->lastindex; j++)
				{
					if (th->Flags.GetBit(j))
					{
						wxString t = cgc->GetThingFlagText(j, true, textmap);
						if (!!t) 
						{
							flagstr+=" ";
							flagstr+=t;
						}
					}
				}
				if (line < 7) m_Items[0][line++]->SetLabel(flagstr);
			}
		}
		for(;line<7;line++) m_Items[0][line]->SetLabel("");
		line = 0;

		if (cgc->IsExtended)
		{
			bool thingargs = !!cgc->GetThingArg(th->type, 0);

			m_Items[1][line++]->SetLabel(wxString::Format("Tid: %d", th->thingid));
			if (!thingargs) m_Items[1][line++]->SetLabel(cgc->GetLineDefDescription(th->special, -1, th->args, 0));

			for(int i=0;i<5;i++)
			{
				wxString str;

				if (thingargs) str = cgc->GetThingArg(th->type, i);
				else if (cgc->LineMap[th->special]) str = cgc->LineMap[th->special]->args[i];
				if (!str) str = wxString::Format("Arg %d",i+1);

				int pos=str.Find(':');
				if (pos>=0)
				{
					str = str.Mid(pos+1); 
				}
				str += wxString::Format(": %d", th->args[i]);
				m_Items[1][line++]->SetLabel(str);
			}
		}
		for(;line<7;line++) m_Items[1][line]->SetLabel("");
		m_sprite->SetTexture(cgc->GetThingSprite(th->type), tx_sprite);
	}
	Layout();
}


//==========================================================================
//
//
//
//==========================================================================

VertexBar::VertexBar(wxWindow *parent) : wxPanel(parent)
{
	wxBoxSizer *barsizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(barsizer);

	m_frame = new wxStaticBox(this, -1, "No Vertex selected");
	wxFont framefont = m_frame->GetFont();
	framefont.SetWeight(wxFONTWEIGHT_BOLD);
	m_frame->SetFont(framefont);
	wxStaticBoxSizer *row = new wxStaticBoxSizer(m_frame, wxVERTICAL);
	barsizer->Add(row, 1, wxEXPAND|wxRIGHT, 4);

	m_x = new wxStaticText(this, -1, "");
	row->Add(m_x);
	m_y = new wxStaticText(this, -1, "");
	row->Add(m_y);

	SetSizeHints(-1, 128, -1, 128);
}

//==========================================================================
//
//
//
//==========================================================================

void VertexBar::Update(int num, CVertex * vt)
{
	static_cast<ZEdFrame*>(GetParent())->SetInfoBar(this);

	if (num == -1)
	{
		m_frame->SetLabel("No Vertex selected");
		m_x->SetLabel("");
		m_y->SetLabel("");
	}
	else
	{
		m_frame->SetLabel(wxString::Format("Vertex #%d", num));
		m_x->SetLabel(wxString::Format("X Position: %1.3f", vt->X()));
		m_y->SetLabel(wxString::Format("Y Position: %1.3f", vt->Y()));
	}

	Layout();
}


//==========================================================================
//
//
//
//==========================================================================

LineBar::LineBar(wxWindow *parent) : wxPanel(parent)
{
	wxBoxSizer *barsizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(barsizer);

	// Main frame
	m_lineframe = new wxStaticBox(this, -1, "No Line selected");
	wxFont framefont = m_lineframe->GetFont();
	framefont.SetWeight(wxFONTWEIGHT_BOLD);
	m_lineframe->SetFont(framefont);
	wxStaticBoxSizer *box = new wxStaticBoxSizer(m_lineframe, wxHORIZONTAL);
	barsizer->Add(box, 1, wxEXPAND|wxRIGHT, 4);


	for(int i = 0; i < 3; i++)
	{
		wxBoxSizer *row = new wxBoxSizer(wxVERTICAL);
		row->SetMinSize(160, -1);
		if (i==0) box->Add(row, 0, wxRIGHT, 8);
		else box->Add(row);
		for(int j = 0; j < 7; j++)
		{
			m_Items[i][j] = new wxStaticText(this, -1, "");
			row->Add(m_Items[i][j]);
		}
	}

	for(int i=0;i<2;i++)
	{
		m_sideframe[i] = new wxStaticBox(this, -1, "");
		m_sideframe[i]->SetFont(framefont);
		wxSizer * sidebox = new wxStaticBoxSizer(m_sideframe[i], wxHORIZONTAL);
		sidebox->SetMinSize(192, -1);
		barsizer->Add(sidebox, 0, wxEXPAND|wxRIGHT, 4);

		for(int j=0;j<3;j++)
		{
			static char * texnames[]={"Upper", "Normal", "Lower"};

			wxBoxSizer *row = new wxBoxSizer(wxVERTICAL);
			sidebox->Add(row);

			row->Add(new wxStaticText(this, -1, texnames[j], wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER), 0, wxEXPAND);

			m_texture[i][j] = new TextureBox(this, -1, 64, 64);
			row->Add(m_texture[i][j], 0, wxALIGN_CENTRE|wxEXPAND|wxTOP, 4);

			m_texname[i][j] = new wxStaticText(this, -1, "", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
			row->Add(m_texname[i][j], 0, wxEXPAND|wxTOP, 4);

		}
	}
	SetSizeHints(-1, 128, -1, 128);
}


//==========================================================================
//
//
//
//==========================================================================

void LineBar::Update(int num, CLevel * lev, CLine * ln)
{
	int Distance(CVertex * v1,CVertex * v2);

	static_cast<ZEdFrame*>(GetParent())->SetInfoBar(this);

	if (num == -1)
	{
		m_lineframe->SetLabel("No Line selected");
		for(int i = 0; i < 3; i++)
		{
			for(int j = 0; j < 7; j++)
			{
				m_Items[i][j]->SetLabel("");
			}
		}

		for(int i=0;i<2;i++)
		{
			m_sideframe[i]->SetLabel("");
			for(int j=0;j<3;j++)
			{
				m_texture[i][j]->SetTexture("", tx_texture);
				m_texname[i][j]->SetLabel("-");
			}
		}
		return;
	}
	else
	{
		int row = 0;
		int line = 0;
		m_lineframe->SetLabel(wxString::Format("Line #%d", num));
		m_Items[row][line++]->SetLabel(wxString::Format("Length: %d", Distance(lev->StVt(ln), lev->EnVt(ln))));
		m_Items[row][line++]->SetLabel(wxString::Format("Vertices: (%d, %d)", ln->line.Start, ln->line.End));
		for(int i=0;i<2;i++)
		{
			if (ln->sides[i].sector!=-1)
			{
				m_Items[row][line++]->SetLabel(wxString::Format("%s offsets: (%d, %d)", i? "Back":"Front", ln->sides[i].xoff, ln->sides[i].yoff));
			}
		}

		if (!lev->TextMap())
		{
			wxString flagstr = "Flags:";
			for(int i=0;i<MAX_FLAGS;i++)
			{
				if (ln->line.Flags.GetBit(i))
				{
					wxString t = cgc->GetLineFlagText(i, true, lev->TextMap());
					if (!!t) 
					{
						flagstr+=" ";
						flagstr+=t;
					}
				}
			}
			m_Items[row][line++]->SetLabel(flagstr);
		}
		else
		{
			for(int i=0; i<cgc->LineGroupCount;i++)
			{
				int flagcount = 0;
				FlagGroupDesc *fg = &cgc->TextMapLineFlagGroups[i];
				wxString flagstr = fg->name;
				flagstr += ":";
				for(int j= fg->firstindex; j < fg->lastindex; j++)
				{
					if (ln->line.Flags.GetBit(j))
					{
						wxString t = cgc->GetLineFlagText(j, true, lev->TextMap());
						if (!!t) 
						{
							flagstr+=" ";
							flagstr+=t;
							flagcount++;
						}
					}
					if (flagcount == 7)
					{
						if (line < 7) m_Items[row][line++]->SetLabel(flagstr);
						if (row == 0 && line == 7)
						{
							row = 1;
							line = 0;
						}
						flagcount = 0;
						flagstr = fg->name;
						flagstr += ":";
					}
				}
				if (line < 7) m_Items[row][line++]->SetLabel(flagstr);
				if (row == 0 && line == 7)
				{
					row = 1;
					line = 0;
				}
			}
		}
		for(; line < 7; line++) m_Items[row][line]->SetLabel("");
		row++;

		line = 0;

		if (lev->TextMap()) m_Items[row][line++]->SetLabel(wxString::Format("ID: %d", ln->line.tag));

		m_Items[row][line++]->SetLabel(cgc->GetLineDefDescription(&ln->line, false));
		if (!cgc->IsExtended)
		{
			if (lev->TextMap()) m_Items[row][line++]->SetLabel(wxString::Format("Tag: %d", ln->line.args[0]));
			else m_Items[row][line++]->SetLabel(wxString::Format("Tag: %d", ln->line.tag));
		}
		else
		{
			for(int i=0;i<5;i++)
			{
				wxString str;

				if (cgc->LineMap[ln->line.type])
				{
					str = cgc->LineMap[ln->line.type]->args[i];
					if (!!str)
					{
						int pos=str.Find(':');
						if (pos>=0)
						{
							str = str.Mid(pos+1); 
						}
					}
					else str = wxString::Format("Arg %d", i+1);
					str += wxString::Format(": %d", ln->line.args[i]);
					m_Items[row][line++]->SetLabel(str);
				}
			}
		}
		for(; line < 7; line++) m_Items[row][line]->SetLabel("");
		if (row == 1) for(line = 0; line < 7; line++) m_Items[2][line]->SetLabel("");

		for(int i=0;i<2;i++)
		{
			if (ln->sides[i].sector!=-1)
			{
				m_sideframe[i]->SetLabel(wxString::Format("%s side (sector #%d)", i? "Back":"Front", ln->sides[i].sector));
				for(int j=0;j<3;j++)
				{
					char * tex = j==0? ln->sides[i].texUpper : j==1? ln->sides[i].texNormal : ln->sides[i].texLower;
					wxString texstr = wxString(tex,8).Upper();

					m_texture[i][j]->SetTexture(texstr.c_str(), tx_texture);
					m_texname[i][j]->SetLabel(texstr);
				}
			}
			else
			{
				m_sideframe[i]->SetLabel(wxString::Format("No %s side", i? "back":"front"));
				for(int j=0;j<3;j++)
				{
					m_texture[i][j]->SetTexture("", tx_texture);
					m_texname[i][j]->SetLabel("");
				}
			}
		}
	}
	Layout();
}


//==========================================================================
//
//
//
//==========================================================================

SectorBar::SectorBar(wxWindow *parent) : wxPanel(parent)
{
	wxBoxSizer *barsizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(barsizer);

	// Main frame
	m_frame = new wxStaticBox(this, -1, "No Sector selected");
	wxFont framefont = m_frame->GetFont();
	framefont.SetWeight(wxFONTWEIGHT_BOLD);
	m_frame->SetFont(framefont);
	wxStaticBoxSizer *box = new wxStaticBoxSizer(m_frame, wxVERTICAL);
	barsizer->Add(box, 1, wxEXPAND|wxRIGHT, 4);

	m_floorheight = new wxStaticText(this, -1, "");
	box->Add(m_floorheight);
	m_ceilingheight = new wxStaticText(this, -1, "");
	box->Add(m_ceilingheight);
	m_height = new wxStaticText(this, -1, "");
	box->Add(m_height);
	m_lightlevel = new wxStaticText(this, -1, "");
	box->Add(m_lightlevel);
	m_type = new wxStaticText(this, -1, "");
	box->Add(m_type);
	m_tag = new wxStaticText(this, -1, "");
	box->Add(m_tag);

	wxStaticBox * texframe = new wxStaticBox(this, -1, "");
	wxSizer * texbox = new wxStaticBoxSizer(texframe, wxHORIZONTAL);
	texbox->SetMinSize(128, -1);
	barsizer->Add(texbox, 0, wxEXPAND|wxRIGHT, 4);

	for(int j=0;j<2;j++)
	{
		static char * texnames[]={"Ceiling", "Floor"};

		wxSizer * row = new wxBoxSizer(wxVERTICAL);
		texbox->Add(row);

		row->Add(new wxStaticText(this, -1, texnames[j], wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER), 0, wxEXPAND);

		m_texture[j] = new TextureBox(this, -1, 64, 64);
		row->Add(m_texture[j], 0, wxALIGN_CENTRE|wxEXPAND|wxTOP, 4);

		m_texname[j] = new wxStaticText(this, -1, "", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
		row->Add(m_texname[j], 0, wxEXPAND|wxTOP, 4);

	}
	SetSizeHints(-1, 128, -1, 128);
}

//==========================================================================
//
//
//
//==========================================================================

void SectorBar::Update(int num, CSector * sec)
{
	static_cast<ZEdFrame*>(GetParent())->SetInfoBar(this);

	if (num==-1)
	{
		m_frame->SetLabel("No Sector selected");
		m_floorheight->SetLabel("");
		m_ceilingheight->SetLabel("");
		m_height->SetLabel("");
		m_lightlevel->SetLabel("");
		m_type->SetLabel("");
		m_tag->SetLabel("");

		for(int j=0;j<2;j++)
		{
			m_texture[j]->SetTexture("", tx_flat);
			m_texname[j]->SetLabel("");
		}
	}
	else
	{
		m_frame->SetLabel(wxString::Format("Sector #%d", num));
		m_floorheight->SetLabel(wxString::Format("Floor Height: %d", sec->floorh));
		m_ceilingheight->SetLabel(wxString::Format("Ceiling Height: %d", sec->ceilh));
		m_height->SetLabel(wxString::Format("Height: %d", sec->ceilh-sec->floorh));
		m_lightlevel->SetLabel(wxString::Format("Light level: %d", sec->light));
		m_type->SetLabel(wxString::Format("Special: %d (%s)", sec->special, cgc->GetSectorName(sec->special, false)));
		m_tag->SetLabel(wxString::Format("Tag: %d", sec->tag));

		for(int j=0;j<2;j++)
		{
			char * tex = j==0? sec->ceilt : sec->floort;
			wxString texstr = wxString(tex,8).Upper();

			m_texture[j]->SetTexture(texstr.c_str(), tx_texture);
			m_texname[j]->SetLabel(texstr);
		}
	}
	Layout();
}
