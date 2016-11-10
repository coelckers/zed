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
// Thing edit dialog
//


#include "StdAfx.h"
#include "Level.h"
#include "ZEd.h"
#include "texture.h"
#include "GameConfig.h"



enum
{
	LE_CHANGESPECIAL,
	LE_SPECIALTEXT,
	LE_EDITSPECIAL,
	LE_TAGBTN,
	LE_TRIGGER,
	LE_ARG1BTN,
	LE_ARG2BTN,
	LE_ARG3BTN,
	LE_ARG4BTN,
	LE_ARG5BTN,
	LE_TEXEDIT1U,
	LE_TEXEDIT1M,
	LE_TEXEDIT1L,
	LE_TEXEDIT2U,
	LE_TEXEDIT2M,
	LE_TEXEDIT2L,
	LE_TEXPIX1U,
	LE_TEXPIX1M,
	LE_TEXPIX1L,
	LE_TEXPIX2U,
	LE_TEXPIX2M,
	LE_TEXPIX2L,
};
//==========================================================================
//
//
//
//==========================================================================

class LineEditDialog : public wxDialog
{
	wxCheckBox * m_Flags[MAX_FLAGS];

	wxButton * m_ChangeSpecial;
	wxStaticText * m_SpecialText;
	wxTextCtrl * m_Special;
	wxComboBox * m_Trigger;
	wxSpinCtrl * m_Tag;
	wxButton * m_TagButton;
	wxSpinCtrl * m_Args[5];
	wxStaticText * m_ArgText[5];
	wxButton * m_ArgButton[5];
	wxSpinCtrl * m_Vertices[2];
	wxTextCtrl * m_Alpha;

	wxFont * m_Font;

	struct SDTexture
	{
		TextureBox * m_Texture;
		wxTextCtrl * m_TexName;
		wxStaticText * m_TexInfo;
		wxTextCtrl * m_Ofs[2];
		wxTextCtrl * m_Scale[2];
	};

	SDTexture m_Textures[2][3];
	wxTextCtrl * m_Sector[2];
	wxSpinCtrl * m_xOff[2], * m_yOff[2];
	wxSpinCtrl * m_Light[2];
	wxCheckBox * m_SideFlags[2][MAX_FLAGS];

	wxString m_TexNames[2][3];


	wxPanel * m_pages[3];

	bool m_Extended;
	bool m_TextMap;
	bool specialmenu;

	wxSizer * CreateFlagBox(wxWindow * parent, wxCheckBox **boxarray, const wxString &title, bool side,
							BitField &setflags, BitField &notflags, int start, int end);
	wxSizer * CreateTypeBox(wxWindow * parent);
	wxSizer * CreateSpecialBox(wxWindow * parent);
	wxSizer * CreateArgBox(wxWindow * parent);
	wxSizer * CreateTagBox(wxWindow * parent);
	wxSizer * CreateVertexBox(wxWindow * parent);
	wxSizer * CreateTextureBox(wxWindow * parent, int side, int num);
	wxSizer * CreateAlphaBox(wxWindow * parent);
	void CreateLDPage(wxPanel * page);
	void CreateSDPage(int num, wxPanel * page);

	void ChangeSpecial(int special, bool fromedit=false);
	void UpdateTexture(int side, int pos);

	void OnOK(wxCommandEvent & event);
	void OnChangeSpecial(wxCommandEvent & event);
	void OnMenuSelect(wxCommandEvent & event);
	void OnEditSpecial(wxCommandEvent & event);
	void OnTagButton(wxCommandEvent & event);
	void OnTextureChange(wxCommandEvent & event);
	void OnTextureSelect(wxCommandEvent & event);
	void OnArgButton(wxCommandEvent & event);

public:
	CLevel * m_Level;
	CLine * m_Line;
	int m_Changes;

	LineEditDialog(wxWindow * parent, CLevel * level, CLine * line);
	~LineEditDialog()
	{
		if (m_Font) delete m_Font;
	}

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(LineEditDialog, wxDialog)

	EVT_BUTTON(LE_CHANGESPECIAL, LineEditDialog::OnChangeSpecial)
	EVT_BUTTON(LE_TAGBTN, LineEditDialog::OnTagButton)
	EVT_MENU_RANGE(2000, 55000, LineEditDialog::OnMenuSelect)
	EVT_TEXT(LE_EDITSPECIAL, LineEditDialog::OnEditSpecial)
	EVT_BUTTON(wxID_OK, LineEditDialog::OnOK)
	EVT_COMMAND_RANGE(LE_TEXEDIT1U, LE_TEXEDIT2L, wxEVT_COMMAND_TEXT_UPDATED, LineEditDialog::OnTextureChange)
	EVT_COMMAND_RANGE(LE_TEXPIX1U, LE_TEXPIX2L, wxEVT_COMMAND_BUTTON_CLICKED, LineEditDialog::OnTextureSelect)
	EVT_COMMAND_RANGE(LE_ARG1BTN, LE_ARG5BTN, wxEVT_COMMAND_BUTTON_CLICKED, LineEditDialog::OnArgButton)

END_EVENT_TABLE()


//==========================================================================
//
//
//
//==========================================================================

wxSizer * LineEditDialog::CreateFlagBox(wxWindow * parent, wxCheckBox **boxarray, const wxString &title, bool side,
										BitField &setflags, BitField &notflags, int start, int end)
{
	int flagc=0,flagc1=0;

	wxStaticBox *frame = new wxStaticBox(parent, -1, title);
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxHORIZONTAL);

	wxBoxSizer * left = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * mid = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * right = new wxBoxSizer(wxVERTICAL);

	box->Add(left, 1, wxALL, 4);
	box->Add(mid, 1, wxALL, 4);
	box->Add(right, 1, wxALL, 4);

	for(int i=start;i<=end;i++)
	{
		wxString txt;
		
		if (!side) txt = cgc->GetLineFlagText(i, false, m_Level->TextMap());
		else txt = cgc->GetSideFlagText(i, false, m_Level->TextMap());

		if (!!txt) flagc++;
		boxarray[i]=NULL;
	}
	flagc=(flagc+2)/3*3;
	for(int i=start;i<=end;i++)
	{
		wxString txt;
		
		if (!side) txt = cgc->GetLineFlagText(i, false, m_Level->TextMap());
		else txt = cgc->GetSideFlagText(i, false, m_Level->TextMap());

		if (!!txt)
		{
			int check;
			int bit = 1<<i;
			if (m_Line->multi)	// multiple selection
			{
				int set = setflags.GetBit(i);
				int not = notflags.GetBit(i);

				if (!set && !not) check = 0;
				else if (set && not) check = 1;
				else check = 2;
			}
			else check = !!setflags.GetBit(i);

			long style = check==2? wxCHK_3STATE | wxCHK_ALLOW_3RD_STATE_FOR_USER : 0;

			boxarray[i]=new wxCheckBox(parent, -1, txt, wxDefaultPosition, wxDefaultSize, style);
			if (flagc1<flagc/3) left->Add(boxarray[i], 1, wxEXPAND|wxTOP|wxBOTTOM, 1);
			else if (flagc1<flagc*2/3) mid->Add(boxarray[i], 1, wxEXPAND|wxTOP|wxBOTTOM, 1);
			else right->Add(boxarray[i], 1, wxEXPAND|wxTOP|wxBOTTOM, 1);

			if (check==2) boxarray[i]->Set3StateValue(wxCHK_UNDETERMINED);
			else boxarray[i]->SetValue(!!check);

			flagc1++;
		}
	}
	return box;
}


//==========================================================================
//
//
//
//==========================================================================

static const char * triggernames[]={
	"W1 Player crosses",
	"WR Player crosses",
	"S1 Player activates",
	"SR Player activates",
	"M1 Monster crosses",
	"MR Monster crosses",
	"G1 Projectile hits",
	"GR Projectile hits",
	"P1 Player pushes",
	"PR Player pushes",
	"R1 Projectile crosses",
	"R2 Projectile crosses",
	"U1 Player activates / Usethrough",
	"UR Player activates / Usethrough",
	"I1 Projectile hits or crosses",
	"IR Projectile hits or crosses"
};

wxSizer * LineEditDialog::CreateSpecialBox(wxWindow * parent)
{
	wxStaticBox *frame = new wxStaticBox(parent, -1, "Special");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);

	m_SpecialText = new wxStaticText(parent, LE_SPECIALTEXT, "This is a very long text line to format the dialog properly but it must be longer");
	box->Add(m_SpecialText, 0, wxALL, 4);

	wxBoxSizer * box2 = new wxBoxSizer(wxHORIZONTAL);

	m_Special = new wxTextCtrl(parent, LE_EDITSPECIAL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	box2->Add(m_Special, 0, wxALL, 4);

	m_ChangeSpecial = new wxButton(parent, LE_CHANGESPECIAL, "Change Special");
	box2->Add(m_ChangeSpecial, 0, wxALL, 4);

	box->Add(box2);

	if (m_Extended && !m_TextMap)
	{
		m_Trigger = new wxComboBox(parent, LE_TRIGGER, wxEmptyString, wxDefaultPosition, wxDefaultSize,
					wxArrayString(16, triggernames), wxCB_READONLY);

		box->Add(m_Trigger, 0, wxALL, 4);
		m_Trigger->SetSelection((m_Line->line.Flags.GetShort()&0x1e00)/0x200);
	}

	// special cannot be set here because it would interfere with the dialog box formatting

	return box;
}

//==========================================================================
//
//
//
//==========================================================================

wxSizer * LineEditDialog::CreateArgBox(wxWindow * parent)
{
	wxStaticBox *frame = new wxStaticBox(parent, -1, "Arguments");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);

	int max;
	if (!m_Extended) max = 1; else max = 5;

	for(int i=0;i<max;i++)
	{
		wxBoxSizer * line = new wxBoxSizer(wxHORIZONTAL);
		int size = m_TextMap? 100:60;

		m_Args[i] = new wxSpinCtrl(parent, -1, wxEmptyString, wxDefaultPosition, wxSize(size,-1));
		if (!m_TextMap) m_Args[i]->SetRange(0,255);
		m_Args[i]->SetValue(m_Line->line.args[i]);
		line->Add(m_Args[i], 0, wxLEFT, 4);

		if (m_Extended)
		{
			m_ArgButton[i] = new wxButton(parent, LE_ARG1BTN+i, "Change", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
			line->Add(m_ArgButton[i], 0, wxLEFT, 4);
			m_ArgText[i] = new wxStaticText(parent, -1, "This is a very long text line to format the dialog");
		}
		else
		{
			m_ArgButton[i] = NULL;
			m_ArgText[i] = new wxStaticText(parent, -1, "Tag");
		}
		line->Add(m_ArgText[i], 1, wxEXPAND|wxLEFT|wxRIGHT, 6);


		box->Add(line, 0, wxALL, 4);
	}
	return box;
}


//==========================================================================
//
//
//
//==========================================================================

wxSizer * LineEditDialog::CreateVertexBox(wxWindow * parent)
{
	wxStaticBox *frame = new wxStaticBox(parent, -1, "Vertices");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, m_Extended? wxHORIZONTAL : wxVERTICAL);
	wxBoxSizer * line;

	for(int i=0;i<2;i++)
	{
		line = new wxBoxSizer(wxHORIZONTAL);
		line->Add(new wxStaticText(parent, -1, wxString(i? "End" : "Start")), 1, wxEXPAND|wxRIGHT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
		m_Vertices[i] = new wxSpinCtrl(parent, -1);
		m_Vertices[i]->SetRange(0, m_Level->NumVertices()-1);
		m_Vertices[i]->SetValue(i? m_Line->line.End : m_Line->line.Start);
		line->Add(m_Vertices[i], 0, wxALIGN_RIGHT);
		box->Add(line, 1, wxEXPAND|wxALL, 4);

	}
	return box;
}

//==========================================================================
//
//
//
//==========================================================================

wxSizer * LineEditDialog::CreateAlphaBox(wxWindow * parent)
{
	wxStaticBox *frame = new wxStaticBox(parent, -1, "Alpha");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxHORIZONTAL);

	m_Alpha = new wxTextCtrl(parent, -1, wxString::Format("%1.3f", m_Line->line.alpha));
	box->Add(m_Alpha, 1, wxEXPAND|wxALL, 4);
	return box;
}


//==========================================================================
//
//
//
//==========================================================================

wxSizer * LineEditDialog::CreateTagBox(wxWindow * parent)
{
	wxStaticBox *frame = new wxStaticBox(parent, -1, !m_TextMap? "Sector tag" : "Line ID");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxHORIZONTAL);

	m_Tag = new wxSpinCtrl(parent, -1, wxEmptyString);
	m_Tag->SetRange(0,32767);
	m_Tag->SetValue(m_Line->line.tag);
	box->Add(m_Tag, 0, wxALL, 4);

	m_TagButton = new wxButton(parent, LE_TAGBTN, "Find free", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	box->Add(m_TagButton, 0, wxALL, 4);
	return box;
}

//==========================================================================
//
//
//
//==========================================================================

void LineEditDialog::CreateLDPage(wxPanel * page)
{
	wxBoxSizer * box = new wxBoxSizer(wxVERTICAL);

	box->Add(CreateSpecialBox(page), 0, wxALL|wxEXPAND, 4);
	if (m_Extended)
	{
		box->Add(CreateArgBox(page), 0, wxALL|wxEXPAND, 4);
		if (!m_Line->multi) box->Add(CreateVertexBox(page), 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
		if (m_TextMap) 
		{
			box->Add(CreateTagBox(page), 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
		}
	}
	else
	{
		if (m_TextMap)
		{
			box->Add(CreateArgBox(page), 0, wxALL|wxEXPAND, 4);
		}
		wxBoxSizer * box2 = new wxBoxSizer(wxHORIZONTAL);
		box2->Add(CreateTagBox(page), 0, wxALL|wxALIGN_LEFT, 4);
		if (!m_Line->multi) box2->Add(CreateVertexBox(page), 0, wxALL|wxALIGN_RIGHT, 4);
		box->Add(box2, 0, wxCENTER);
	}

	if (!m_TextMap)
	{
		box->Add(CreateFlagBox(page, m_Flags, "Flags", false, m_Line->line.Flags, m_Line->line.notflags ,
								0, MAX_FLAGS-1), 0, wxALL|wxEXPAND, 4);
	}
	else
	{
		box->Add(CreateAlphaBox(page), 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
		memset(m_Flags, 0, sizeof(m_Flags));
		for(int i=0; i < cgc->LineGroupCount; i++)
		{
			FlagGroupDesc * fd = &cgc->TextMapLineFlagGroups[i];
			if (m_Extended && fd->modeflag == 1) continue;
			if (!m_Extended && fd->modeflag == 2) continue;
			box->Add(CreateFlagBox(page, m_Flags, fd->name, false, m_Line->line.Flags, m_Line->line.notflags, 
									fd->firstindex, fd->lastindex), 0, wxALL|wxEXPAND, 4);
		}
	}

	page->SetSizerAndFit(box);
}


//==========================================================================
//
//
//
//==========================================================================

wxSizer * LineEditDialog::CreateTextureBox(wxWindow * parent, int side, int num)
{
	static const char * tex[]={"Upper texture", "Normal texture", "Lower texture" };
	wxStaticBox *frame = new wxStaticBox(parent, -1, tex[num]);
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);

	SDTexture * texinf = &m_Textures[side][num];

	char * texbuf = num==0? m_Line->sides[side].texUpper :
					num==1? m_Line->sides[side].texNormal :
							m_Line->sides[side].texLower;


	texinf->m_Texture = new TextureBox(parent, LE_TEXPIX1U+side*3+num, 128, 128);
	box->Add(texinf->m_Texture, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	texinf->m_TexInfo = new wxStaticText(parent, -1, "", wxDefaultPosition, wxSize(128,-1));
	box->Add(texinf->m_TexInfo, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	texinf->m_TexName = new wxTextCtrl(parent, LE_TEXEDIT1U+side*3+num);
	box->Add(texinf->m_TexName, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);

	if (m_TextMap)
	{
		box->Add(new wxStaticText(parent, -1, "X offset"), 0, wxALL|wxLEFT, 4);
		texinf->m_Ofs[0] = new wxTextCtrl(parent, -1, wxString::Format("%1.3f", m_Line->sides[side].texOffset[num][0]+0.0001));
		box->Add(texinf->m_Ofs[0], 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
		box->Add(new wxStaticText(parent, -1, "Y offset"), 0, wxALL|wxLEFT, 4);
		texinf->m_Ofs[1] = new wxTextCtrl(parent, -1, wxString::Format("%1.3f", m_Line->sides[side].texOffset[num][1]+0.0001));
		box->Add(texinf->m_Ofs[1], 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);

		box->Add(new wxStaticText(parent, -1, "X scale"), 0, wxALL|wxLEFT, 4);
		texinf->m_Scale[0] = new wxTextCtrl(parent, -1, wxString::Format("%1.3f", m_Line->sides[side].texScale[num][0]+0.0001));
		box->Add(texinf->m_Scale[0], 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
		box->Add(new wxStaticText(parent, -1, "Y scale"), 0, wxALL|wxLEFT, 4);
		texinf->m_Scale[1] = new wxTextCtrl(parent, -1, wxString::Format("%1.3f", m_Line->sides[side].texScale[num][1]+0.0001));
		box->Add(texinf->m_Scale[1], 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	}


	texinf->m_TexName->SetFont(*m_Font);
	texinf->m_TexName->SetInsertionPoint(0);
	texinf->m_TexName->SetMaxLength(8);
	texinf->m_TexName->AppendText(wxString(texbuf, 8).Upper());
	m_TexNames[side][num] = texinf->m_TexName->GetValue();
	UpdateTexture(side, num);

	return box;
}


//==========================================================================
//
//
//
//==========================================================================

void LineEditDialog::CreateSDPage(int num, wxPanel * page)
{
	wxBoxSizer * box1 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * box2 = new wxBoxSizer(wxHORIZONTAL);

	for(int i=0;i<3;i++) box2->Add(CreateTextureBox(page, num, i));
	box1->Add(box2, 0);

	wxStaticBox *frame = new wxStaticBox(page, -1, wxEmptyString);
	wxStaticBoxSizer *box3 = new wxStaticBoxSizer(frame, wxHORIZONTAL);

	box3->Add(new wxStaticText(page, -1, "Sector Ref."), 0, wxALL, 4);
	m_Sector[num] = new wxTextCtrl(page, -1, wxEmptyString, wxDefaultPosition, wxSize(66,-1));
	m_Sector[num]->SetMaxLength(5);
	if (m_Line->sides[num].sector!=-2) m_Sector[num]->SetValue(wxString::Format("%d", m_Line->sides[num].sector));
	box3->Add(m_Sector[num], 0, wxALL, 4);

	box3->Add(new wxStaticText(page, -1, "X offset"), 0, wxALL, 4);
	m_xOff[num] = new wxSpinCtrl(page, -1, wxEmptyString, wxDefaultPosition, wxSize(66,-1));
	m_xOff[num]->SetRange(-32767,32767);
	m_xOff[num]->SetValue(m_Line->sides[num].xoff);
	box3->Add(m_xOff[num], 0, wxALL, 4);

	box3->Add(new wxStaticText(page, -1, "Y offset"), 0, wxALL, 4);
	m_yOff[num] = new wxSpinCtrl(page, -1, wxEmptyString, wxDefaultPosition, wxSize(66,-1));
	m_yOff[num]->SetRange(-32767,32767);
	m_yOff[num]->SetValue(m_Line->sides[num].yoff);
	box3->Add(m_yOff[num], 0, wxALL, 4);

	box1->Add(box3, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL);

	box1->Add(CreateFlagBox(page, m_SideFlags[num], "Flags", true, m_Line->sides[num].Flags, m_Line->sides[num].notflags, 
		0, MAX_FLAGS-1), 0, wxALL|wxEXPAND, 4);

	if (m_TextMap)
	{
		frame = new wxStaticBox(page, -1, wxEmptyString);
		box3 = new wxStaticBoxSizer(frame, wxHORIZONTAL);

		box3->Add(new wxStaticText(page, -1, "Light level"), 0, wxALL, 4);
		m_Light[num] = new wxSpinCtrl(page, -1, wxEmptyString, wxDefaultPosition, wxSize(66,-1));
		m_Light[num]->SetRange(-256,256);
		m_Light[num]->SetValue(m_Line->sides[num].light);
		box3->Add(m_Light[num], 0, wxALL, 4);
		box1->Add(box3, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL);
	}


	page->SetSizerAndFit(box1);
}

//==========================================================================
//
//
//
//==========================================================================

void LineEditDialog::ChangeSpecial(int special, bool fromedit)
{
	if (!fromedit) 
	{
		m_Changes|=LDF_Guard;
		m_Special->SetValue(wxString::Format("%d", special));
		m_Changes&=~LDF_Guard;
	}
	if (m_Extended)
		m_SpecialText->SetLabel( cgc->GetLineDefDescription(special, -1, NULL, 2) );
	else
		m_SpecialText->SetLabel( cgc->GetLDName(special) );

	if (m_Extended)
	{
		for(int i=0;i<5;i++)
		{
			wxString str;

			if (cgc->LineMap[special])
			{
				str = cgc->LineMap[special]->args[i];
				int pos=str.Find(':');
				if (pos>=0)
				{
					// Special arg type - to be implemented
					
					m_ArgButton[i]->Enable();

					str = str.Mid(pos+1); 
				}
				else m_ArgButton[i]->Enable(false);
			}

			m_ArgText[i]->SetLabel(str);
		}
	}
}


//==========================================================================
//
//
//
//==========================================================================

LineEditDialog::LineEditDialog(wxWindow * parent, CLevel * level, CLine * line)
	: wxDialog(parent, -1, wxString("Edit Line"))
{
	wxBoxSizer * box = new wxBoxSizer(wxVERTICAL);
	wxNotebook * tabctrl = new wxNotebook(this, -1);

	m_Font = wxFont::New(wxSize(8,16), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false,
		"Courier New", wxFONTENCODING_SYSTEM);

	m_Line = line;
	m_Level = level;
	m_Extended = level->Extended();
	m_TextMap = level->TextMap();
	m_Changes=0;

	m_pages[0] = new wxPanel(tabctrl);
	tabctrl->AddPage(m_pages[0], "Linedef");
	CreateLDPage(m_pages[0]);

	for(int i=0;i<2;i++)
	{
		m_pages[1+i] = new wxPanel(tabctrl);
		tabctrl->AddPage(m_pages[1+i], wxString(i? "Back" : "Front"));
		CreateSDPage(i, m_pages[1+i]);
	}

	box->Add(tabctrl, 1, wxEXPAND|wxALL, 4);

	box->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);

	SetSizerAndFit(box);
	Layout();
	Center();
	// Do this after the layout is complete

	ChangeSpecial(m_Line->line.type);
}

//==========================================================================
//
//
//
//==========================================================================

void LineEditDialog::OnChangeSpecial(wxCommandEvent & event)
{
	PopupMenu(cgc->GetSpecialMenu());
}

void LineEditDialog::OnMenuSelect(wxCommandEvent & event)
{
	int id = event.GetId();
	if (id<50000) m_Line->line.type = id - 2000;
	else m_Line->line.type = GetGenericType(this, m_Line->line.type, id-50000);
	if (m_Line->line.type==-1) return;
	ChangeSpecial(m_Line->line.type);
	m_Changes|=LDF_Special;
}

//==========================================================================
//
//
//
//==========================================================================

void LineEditDialog::OnEditSpecial(wxCommandEvent & event)
{
	long l;
	if (!(m_Changes&LDF_Guard) && m_Special->GetValue().ToLong(&l))
	{
		m_Line->line.type = l;
		ChangeSpecial(l, true);
		m_Changes|=LDF_Special;
	}
}


//==========================================================================
//
//
//
//==========================================================================

void LineEditDialog::OnTagButton(wxCommandEvent & event)
{
	m_Line->line.tag = cgc->IsExtended? m_Level->FindFreeTag()/*LineID*/ : m_Level->FindFreeTag();
	m_Tag->SetValue(m_Line->line.tag);
	m_Changes|=LDF_Tag;
}

//==========================================================================
//
//
//
//==========================================================================
void LineEditDialog::UpdateTexture(int side, int pos)
{
	wxString str = m_Textures[side][pos].m_TexName->GetValue();
	m_Textures[side][pos].m_Texture->SetTexture(str.c_str());
	if (m_Textures[side][pos].m_Texture->isValidTexture())
	{
		m_Textures[side][pos].m_TexInfo->SetLabel(wxString::Format("%d x %d, %s",
			m_Textures[side][pos].m_Texture->GetTextureWidth(),
			m_Textures[side][pos].m_Texture->GetTextureHeight(),
			TextureTypeNames[m_Textures[side][pos].m_Texture->GetTextureType()]));
	}
	else
		m_Textures[side][pos].m_TexInfo->SetLabel(wxEmptyString);
}

void LineEditDialog::OnTextureChange(wxCommandEvent & event)
{
	int num = event.GetId()-LE_TEXEDIT1U;
	UpdateTexture(num/3, num%3);
}

void LineEditDialog::OnTextureSelect(wxCommandEvent & event)
{
	int texnum = SelectTexture(this, tx_texture);
	if (texnum>=0)
	{
		int num = event.GetId()-LE_TEXPIX1U;
		m_Textures[num/3][num%3].m_TexName->SetValue(texman->GetTextureName(texnum));
	}
}

//==========================================================================
//
//
//
//==========================================================================

void LineEditDialog::OnArgButton(wxCommandEvent & event)
{
	int btn = event.GetId()-LE_ARG1BTN;

	if (m_Extended)
	{
		if (cgc->LineMap[m_Line->line.type])
		{
			wxString str = cgc->LineMap[m_Line->line.type]->args[btn];
			int pos=str.Find(':');
			if (pos>=0)
			{
				str.Truncate(pos);
				int v = GetSpecialArg(this, m_Args[btn]->GetValue(), str);
				m_Args[btn]->SetValue(v);
			}
		}
	}
}

//==========================================================================
//
// All information that isn't handled explicitly is read out here
//
//==========================================================================

void LineEditDialog::OnOK(wxCommandEvent & event)
{
	int oldtrigger = ((m_Line->line.Flags.GetShort()&0x1e00)/0x200);

	m_Line->line.Flags.Clear();
	m_Line->line.notflags.Clear();
	for(int i=0;i<MAX_FLAGS;i++) if (m_Flags[i])
	{
		// Stupid wxWidgets limitation...
		int state = m_Flags[i]->Is3State()? m_Flags[i]->Get3StateValue() : m_Flags[i]->GetValue()? wxCHK_CHECKED : wxCHK_UNCHECKED;

		switch(state)
		{
		case wxCHK_CHECKED:
			m_Line->line.Flags.SetBit(i);
		case wxCHK_UNCHECKED:
			m_Line->line.notflags.SetBit(i);
		default:
			break;
		}
	}
	
	if (m_TextMap) for(int s=0;s<2;s++)
	{
		for(int i=0;i<MAX_FLAGS;i++) if (m_SideFlags[s][i])
		{
			// Stupid wxWidgets limitation...
			int state = m_SideFlags[s][i]->Is3State()? m_SideFlags[s][i]->Get3StateValue() : m_SideFlags[s][i]->GetValue()? wxCHK_CHECKED : wxCHK_UNCHECKED;

			switch(state)
			{
			case wxCHK_CHECKED:
				m_Line->sides[s].Flags.SetBit(i);
			case wxCHK_UNCHECKED:
				m_Line->sides[s].notflags.SetBit(i);
			default:
				break;
			}
		}
	}

	if (!m_Line->multi)
	{
		m_Line->line.Start = m_Vertices[0]->GetValue();
		m_Line->line.End = m_Vertices[1]->GetValue();
	}
	if (m_Extended)
	{
		for(int i=0;i<5;i++)
		{
			if (m_Args[i]->GetValue()!=m_Line->line.args[i])
			{
				m_Line->line.args[i] = m_Args[i]->GetValue();
				m_Changes |= LDF_Arg1<<i;
			}
		}
		if (!m_TextMap)
		{

			int newtrigger = m_Trigger->GetSelection();

			if (oldtrigger!=newtrigger || !m_Line->multi)
			{
				m_Line->line.Flags.SetShort(m_Line->line.Flags.GetShort() | (newtrigger*0x200));
				m_Line->line.notflags.SetShort(m_Line->line.notflags.GetShort() | 0x1e00);
			}
		}
	}
	else
	{
		if (m_TextMap)
		{
			if (m_Args[0]->GetValue()!=m_Line->line.args[0])
			{
				m_Line->line.args[0] = m_Args[0]->GetValue();
				m_Changes |= LDF_Arg1;
			}
		}

		if (m_Tag->GetValue()!=m_Line->line.tag)
		{
			m_Line->line.tag = m_Tag->GetValue();
			m_Changes |= LDF_Tag;
		}
	}

	for(int i=0;i<2;i++)
	{
		wxString sec = m_Sector[i]->GetValue();
		if (!!sec)
		{
			long newsector;
			if (sec.ToLong(&newsector))
			{
				if (m_Line->sides[i].sector!=newsector)
				{
					m_Line->sides[i].sector=newsector;
					m_Changes |= LDF_Sector1<<i;
				}
			}
		}

		m_Line->sides[i].xoff = m_xOff[i]->GetValue();
		m_Line->sides[i].yoff = m_yOff[i]->GetValue();

		if (m_TextMap)
		{
			int v = m_Light[i]->GetValue();
			if (v != m_Line->sides[i].light)
			{
				m_Changes |= i? LDF_LiteB : LDF_LiteF;
				m_Line->sides[i].light = v;
			}

			double dv = strtod(m_Alpha->GetValue(), NULL);
			if (int(dv*1000) != int(m_Line->line.alpha*1000))
			{
				m_Changes |= LDF_Alpha;
				m_Line->line.alpha=dv;
			}
		}

		for(int j=0;j<3;j++)
		{
			char * texbuf = j==0? m_Line->sides[i].texUpper :
							j==1? m_Line->sides[i].texNormal :
								  m_Line->sides[i].texLower;

			if (m_Textures[i][j].m_TexName->GetValue().CmpNoCase(m_TexNames[i][j])!=0)
			{
				wxString tex = m_Textures[i][j].m_TexName->GetValue().Upper();
				strncpy(texbuf, tex.c_str(), 8);
				m_Changes |= LDF_Texture1U << (i*3+j);
			}

			if (m_TextMap)
			{
			for(int k=0;k<2;k++)
			{
				double v = strtod(m_Textures[i][j].m_Ofs[k]->GetValue(), NULL);
				m_Line->sides[i].texOffset[j][k] = v;

				v = strtod(m_Textures[i][j].m_Scale[k]->GetValue(), NULL);
				m_Line->sides[i].texScale[j][k] = v;
			}
			}
		}
	}

	EndModal(wxID_OK);
}


//==========================================================================
//
//
//
//==========================================================================

int EditLine(wxWindow * parent, CLevel * level,  CLine * line)
{
	LineEditDialog dlg(parent, level, line);
	int res = dlg.ShowModal();
	return res==wxID_OK? dlg.m_Changes : -1;
}
