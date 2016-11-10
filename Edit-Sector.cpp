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
	SE_CHANGETYPE,
	SE_TYPETEXT,
	SE_EDITTYPE,
	SE_TAGBTN,
	SE_TEXEDITC,
	SE_TEXEDITF,
	SE_TEXPIXC,
	SE_TEXPIXF,
	SE_CHANGECOLOR,
	SE_CHANGEFADE,
};

enum
{
	R,G,B
};


//==========================================================================
//
//
//
//==========================================================================

class SectorEditDialog : public wxDialog
{
	wxCheckBox * m_Flags[3];

	wxButton * m_ChangeType;
	wxStaticText * m_TypeText;
	wxCheckBox * m_ExtendedType;
	//wxTextCtrl * m_Type;
	wxComboBox * m_Damage;
	wxSpinCtrl * m_Tag;
	wxButton * m_TagButton;
	wxSpinCtrl * m_Height[3];

	wxFont * m_Font;
	wxCheckBox * m_XFlags[MAX_FLAGS];

	struct STexture
	{
		TextureBox * m_Texture;
		wxTextCtrl * m_TexName;
		wxStaticText * m_TexInfo;
	};

	STexture m_Textures[2];
	wxString m_TexNames[2];

	struct SPlane
	{
		wxTextCtrl * m_rotation[5];
		wxSpinCtrl * m_planelight;
	};

	SPlane m_planes[2];

	wxTextCtrl *m_Color[3];
	wxTextCtrl *m_Fade[3];
	wxTextCtrl *m_Desaturation;
	wxTextCtrl *m_Gravity;

	wxPanel * m_pages[4];

	bool m_Extended;
	bool m_TextMap;
	bool m_Boom;
	bool specialmenu;

	wxSizer * CreateFlagBox(wxWindow * parent);
	wxSizer * CreateXFlagBox(wxWindow * parent, const wxString &title, int start, int end);
	wxSizer * CreateDamageBox(wxWindow * parent);
	wxSizer * CreateTypeBox(wxWindow * parent);
	wxSizer * CreateTagBox(wxWindow * parent);
	wxSizer * CreateHeightBox(wxWindow * parent);
	wxSizer * CreateTextureBox(wxWindow * parent, int num);
	wxSizer * CreatePlaneBox(wxWindow * parent, int num);
	wxSizer * CreateColorBox(wxPanel *page);
	wxSizer * CreateOtherBox(wxPanel *parent);
	void CreatePage1(wxPanel * page);
	void CreatePage2(wxPanel * page);
	void CreatePage3(wxPanel * page);
	void CreatePage4(wxPanel * page);

	void ChangeType(int special, bool fromedit=false);
	void UpdateTexture(int pos);

	void OnOK(wxCommandEvent & event);
	void OnChangeType(wxCommandEvent & event);
	void OnChangeColor(wxCommandEvent & event);
	void OnChangeFade(wxCommandEvent & event);
	void OnMenuSelect(wxCommandEvent & event);
	void OnTagButton(wxCommandEvent & event);
	void OnTextureChange(wxCommandEvent & event);
	void OnTextureSelect(wxCommandEvent & event);

public:
	CLevel * m_Level;
	CSector * m_Sector;
	int m_Changes;

	SectorEditDialog(wxWindow * parent, CLevel * level, CSector * sector);
	~SectorEditDialog()
	{
		if (m_Font) delete m_Font;
	}

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(SectorEditDialog, wxDialog)

	EVT_BUTTON(SE_CHANGETYPE, SectorEditDialog::OnChangeType)
	EVT_BUTTON(SE_TAGBTN, SectorEditDialog::OnTagButton)
	EVT_MENU_RANGE(2000, 35000, SectorEditDialog::OnMenuSelect)
	EVT_BUTTON(wxID_OK, SectorEditDialog::OnOK)
	EVT_COMMAND_RANGE(SE_TEXEDITC, SE_TEXEDITF, wxEVT_COMMAND_TEXT_UPDATED, SectorEditDialog::OnTextureChange)
	EVT_COMMAND_RANGE(SE_TEXPIXC, SE_TEXPIXF, wxEVT_COMMAND_BUTTON_CLICKED, SectorEditDialog::OnTextureSelect)
	EVT_BUTTON(SE_CHANGECOLOR, SectorEditDialog::OnChangeColor)
	EVT_BUTTON(SE_CHANGEFADE, SectorEditDialog::OnChangeFade)

END_EVENT_TABLE()


//==========================================================================
//
//
//
//==========================================================================

wxSizer * SectorEditDialog::CreateFlagBox(wxWindow * parent)
{
	static const char * names[]={"Secret", "Friction", "Wind"};

	wxStaticBox *frame = new wxStaticBox(parent, -1, "Flags");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);

	for(int i=0;i<3;i++)
	{
		int check;
		int bit = m_Extended? (1024<<i) : (128<<i);
		if (m_Sector->multi)	// multiple selection
		{
			if (!(m_Sector->flags & bit) && !(m_Sector->notflags & bit)) check = 0;
			else if ((m_Sector->flags & bit) && (m_Sector->notflags & bit)) check = 1;
			else check = 2;
		}
		else check = !! (m_Sector->flags & bit);

		long style = check==2? wxCHK_3STATE | wxCHK_ALLOW_3RD_STATE_FOR_USER : 0;

		m_Flags[i]=new wxCheckBox(parent, -1, names[i], wxDefaultPosition, wxDefaultSize, style);
		box->Add(m_Flags[i], 1, wxEXPAND|wxTOP|wxBOTTOM, 1);

		if (check==2) m_Flags[i]->Set3StateValue(wxCHK_UNDETERMINED);
		else m_Flags[i]->SetValue(!!check);
	}
	return box;
}

//==========================================================================
//
//
//
//==========================================================================

wxSizer * SectorEditDialog::CreateDamageBox(wxWindow * parent)
{
	static const char * damagenames[]={ "No damage", "-2/5 %", "-5/10 %", "-10/20%"};

	wxStaticBox *frame = new wxStaticBox(parent, -1, "Damage");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);

	m_Damage = new wxComboBox(parent, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize,
				wxArrayString(4, damagenames), wxCB_READONLY);

	box->Add(m_Damage, 0, wxALL, 4);
	int val = m_Extended? (m_Sector->special&768)>>8 : (m_Sector->special&96)>>5;
	m_Damage->SetSelection( val );

	/*
	if (cgc->)
	{
		m_DamageVal = new wxSpinCtrl(parent, -1, wxEmptyString, wxDefaultPosition, wxSize(60,-1));
		m_DamageVal->SetRange(0,255);
		m_DamageVal->SetValue(m_Sector->damage);
		box->Add(m_DamageVal, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

		m_Mod = new wxSpinCtrl(parent, -1, wxEmptyString, wxDefaultPosition, wxSize(60,-1));
		m_Mod->SetRange(0,255);
		m_Mod->SetValue(m_Sector->meansofdeath);
		box->Add(m_Mod, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

	}
	*/

	return box;
}

//==========================================================================
//
//
//
//==========================================================================

wxSizer * SectorEditDialog::CreateTypeBox(wxWindow * parent)
{
	wxStaticBox *frame = new wxStaticBox(parent, -1, "Type");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);

	m_TypeText = new wxStaticText(parent, SE_TYPETEXT, "This is a very long text line to format the dialog");
	box->Add(m_TypeText, 0, wxALL, 4);

	wxBoxSizer * box2 = new wxBoxSizer(wxHORIZONTAL);

	m_ChangeType = new wxButton(parent, SE_CHANGETYPE, "Set Type");
	box2->Add(m_ChangeType, 0, wxALL, 4);

	box->Add(box2);

	return box;
}

//==========================================================================
//
//
//
//==========================================================================

wxSizer * SectorEditDialog::CreateTagBox(wxWindow * parent)
{
	wxStaticBox *frame = new wxStaticBox(parent, -1, "Tag");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxHORIZONTAL);

	m_Tag = new wxSpinCtrl(parent, -1, wxEmptyString, wxDefaultPosition, wxSize(66,-1));
	m_Tag->SetRange(0,32767);
	m_Tag->SetValue(m_Sector->tag);
	box->Add(m_Tag, 0, wxALL, 4);

	m_TagButton = new wxButton(parent, SE_TAGBTN, "Find free", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	box->Add(m_TagButton, 0, wxALL, 4);
	return box;
}


//==========================================================================
//
//
//
//==========================================================================

wxSizer * SectorEditDialog::CreateHeightBox(wxWindow * parent)
{
	const char * names[]={"Ceiling height", "Floor height", "Light level"};

	wxStaticBox *frame = new wxStaticBox(parent, -1, "");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);
	wxBoxSizer * line;

	for(int i=0;i<3;i++)
	{
		line = new wxBoxSizer(wxHORIZONTAL);
		line->Add(new wxStaticText(parent, -1, wxString(names[i])), 1, wxEXPAND|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5);
		//line->AddStretchSpacer(1);

		m_Height[i] = new wxSpinCtrl(parent, -1, wxEmptyString, wxDefaultPosition, wxSize(66,-1));
		if (i!=2) m_Height[i]->SetRange(-32767,32767);
		else m_Height[i]->SetRange(0,255);
		m_Height[i]->SetValue(i==0? m_Sector->ceilh : i==1? m_Sector->floorh : m_Sector->light);
		line->Add(m_Height[i], 0, wxEXPAND);
		box->Add(line, 1, wxEXPAND|wxALL, 4);
	}
	return box;
}

//==========================================================================
//
//
//
//==========================================================================

wxSizer * SectorEditDialog::CreateXFlagBox(wxWindow * parent, const wxString &title, int start, int end)
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
		wxString txt = cgc->GetSectorFlagText(i, false);
		if (!!txt) flagc++;
		m_XFlags[i]=NULL;
	}
	flagc=(flagc+2)/3*3;
	for(int i=start;i<=end;i++)
	{
		wxString txt = cgc->GetSectorFlagText(i, false);
		if (!!txt)
		{
			int check;
			int bit = 1<<i;
			if (m_Sector->multi)	// multiple selection
			{
				int set = m_Sector->Flags.GetBit(i);
				int not = m_Sector->notFlags.GetBit(i);

				if (!set && !not) check = 0;
				else if (set && not) check = 1;
				else check = 2;
			}
			else check = !!m_Sector->Flags.GetBit(i);

			long style = check==2? wxCHK_3STATE | wxCHK_ALLOW_3RD_STATE_FOR_USER : 0;

			m_XFlags[i]=new wxCheckBox(parent, -1, txt, wxDefaultPosition, wxDefaultSize, style);
			if (flagc1<flagc/3) left->Add(m_XFlags[i], 1, wxEXPAND|wxTOP|wxBOTTOM, 1);
			else if (flagc1<flagc*2/3) mid->Add(m_XFlags[i], 1, wxEXPAND|wxTOP|wxBOTTOM, 1);
			else right->Add(m_XFlags[i], 1, wxEXPAND|wxTOP|wxBOTTOM, 1);

			if (check==2) m_XFlags[i]->Set3StateValue(wxCHK_UNDETERMINED);
			else m_XFlags[i]->SetValue(!!check);

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

void SectorEditDialog::CreatePage1(wxPanel * page)
{
	wxBoxSizer * box = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * box1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer * box2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer * box3 = new wxBoxSizer(wxVERTICAL);

	box->Add(box1, 0, wxCENTER);
	box1->Add(CreateTypeBox(page), 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	box3->Add(CreateTagBox(page), 0, wxEXPAND);
	box2->Add(box3, 1, wxEXPAND|wxCENTER);
	box2->Add(CreateHeightBox(page), 0, wxALL|wxALIGN_RIGHT, 4);
	if (m_Boom)
	{
		box1->Add(CreateFlagBox(page), 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
		box3->Add(CreateDamageBox(page), 0, wxEXPAND);
	}
	box->Add(box2, 1, wxEXPAND|wxCENTER);

	memset(m_XFlags, 0, sizeof(m_XFlags));
	if (m_TextMap)
	{
		for(int i=0; i < cgc->SectorGroupCount; i++)
		{
			FlagGroupDesc * fd = &cgc->TextMapSectorFlagGroups[i];
			box->Add(CreateXFlagBox(page, fd->name, fd->firstindex, fd->lastindex), 0, wxALL|wxEXPAND, 4);
		}
	}

	page->SetSizerAndFit(box);
}


//==========================================================================
//
//
//
//==========================================================================

wxSizer * SectorEditDialog::CreateTextureBox(wxWindow * parent, int num)
{
	static const char * tex[]={"Ceiling texture", "Floor texture" };
	wxStaticBox *frame = new wxStaticBox(parent, -1, tex[num]);
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);

	STexture * texinf = &m_Textures[num];

	char * texbuf = num==0? m_Sector->ceilt : m_Sector->floort;

	texinf->m_Texture = new TextureBox(parent, SE_TEXPIXC+num, 128, 128);
	box->Add(texinf->m_Texture, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	texinf->m_TexInfo = new wxStaticText(parent, -1, "", wxDefaultPosition, wxSize(128,-1));
	box->Add(texinf->m_TexInfo, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	texinf->m_TexName = new wxTextCtrl(parent, SE_TEXEDITC+num);
	box->Add(texinf->m_TexName, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);

	texinf->m_TexName->SetFont(*m_Font);
	texinf->m_TexName->SetInsertionPoint(0);
	texinf->m_TexName->SetMaxLength(8);
	texinf->m_TexName->AppendText(wxString(texbuf, 8).Upper());
	m_TexNames[num]=texinf->m_TexName->GetValue();
	UpdateTexture(num);

	return box;
}


//==========================================================================
//
//
//
//==========================================================================

void SectorEditDialog::CreatePage2(wxPanel * page)
{
	wxBoxSizer * box = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * box1 = new wxBoxSizer(wxHORIZONTAL);

	for(int i=0;i<2;i++) box1->Add(CreateTextureBox(page, i));
	box->Add(box1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	page->SetSizerAndFit(box);
}


//==========================================================================
//
//
//
//==========================================================================

wxSizer * SectorEditDialog::CreatePlaneBox(wxWindow * parent, int num)
{
	static const char * tex[]={"Ceiling", "Floor" };
	static const char * labels[]={ "X offset", "Y offset", "X scale", "Y scale", "Rotation", "Light level"};
	double values[]= { m_Sector->offset[num][X], m_Sector->offset[num][Y], 
					  m_Sector->scale[num][X], m_Sector->scale[num][Y], m_Sector->rotation[num]};

	wxStaticBox *frame = new wxStaticBox(parent, -1, tex[num]);
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);

	SPlane * plane = &m_planes[num];


	for(int i=0;i<6;i++)
	{
		wxBoxSizer * line = new wxBoxSizer(wxHORIZONTAL);
		line->Add(new wxStaticText(parent, -1, labels[i]), 1, wxEXPAND|wxLEFT, 4);

		if (i!=5)
		{
			plane->m_rotation[i]= new wxTextCtrl(parent, -1, wxString::Format("%1.3f", values[i]), 
												 wxDefaultPosition, wxSize(60,-1));
			line->Add(plane->m_rotation[i], 0, wxEXPAND|wxLEFT|wxRIGHT, 4);
		}
		else
		{
			plane->m_planelight= new wxSpinCtrl(parent, -1, wxEmptyString, wxDefaultPosition, wxSize(60,-1));
			plane->m_planelight->SetRange(-255,255);
			plane->m_planelight->SetValue(m_Sector->planelight[num]);
			line->Add(plane->m_planelight, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);
		}
		box->Add(line, 0, wxEXPAND|wxTOP|wxBOTTOM, 4);
	}

	return box;
}

//==========================================================================
//
//
//
//==========================================================================

void SectorEditDialog::CreatePage3(wxPanel * page)
{
	wxBoxSizer * box = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * box1 = new wxBoxSizer(wxHORIZONTAL);

	for(int i=0;i<2;i++) box1->Add(CreatePlaneBox(page, i));
	box->Add(box1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	page->SetSizerAndFit(box);
}


//==========================================================================
//
//
//
//==========================================================================

wxSizer * SectorEditDialog::CreateColorBox(wxPanel *parent)
{
	static int shift[]={16,8,0};
	static char RGB[][2]={"R", "G", "B"};
	wxStaticBox *frame = new wxStaticBox(parent, -1, "Colors");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);

	wxBoxSizer * box1;


	unsigned int color = m_Sector->color;

	box->Add(new wxStaticText(parent, -1, "Light Color"), 1, wxALL|wxALIGN_LEFT, 4);
	box1 = new wxBoxSizer(wxHORIZONTAL);
	for(int i=R; i<=B; i++)
	{
		m_Color[i] = new wxTextCtrl(parent, -1, wxString::Format("%d", (color>>shift[i])&255), 
												 wxDefaultPosition, wxSize(30,-1));
		box1->Add(m_Color[i], 0, wxALL, 2);
		box1->Add(new wxStaticText(parent, -1, RGB[i]), 0, wxALL, 4);
	}
	box1->Add(new wxButton(parent, SE_CHANGECOLOR, "Change"), 0, wxALL);
	box->Add(box1, 0, wxALL, 4);

	color = m_Sector->fade;

	box->Add(new wxStaticText(parent, -1, "Fade Color"), 1, wxALL|wxALIGN_LEFT, 4);
	box1 = new wxBoxSizer(wxHORIZONTAL);
	for(int i=R; i<=B; i++)
	{
		m_Fade[i] = new wxTextCtrl(parent, -1, wxString::Format("%d", (color>>shift[i])&255), 
												 wxDefaultPosition, wxSize(30,-1));
		box1->Add(m_Fade[i], 0, wxALL, 2);
		box1->Add(new wxStaticText(parent, -1, RGB[i]), 0, wxALL, 4);
	}
	box1->Add(new wxButton(parent, SE_CHANGEFADE, "Change"), 0, wxALL);
	box->Add(box1, 0, wxALL|wxALIGN_LEFT, 4);

	box->Add(new wxStaticText(parent, -1, "Desaturation"), 1, wxALL|wxALIGN_LEFT, 4);
	m_Desaturation = new wxTextCtrl(parent, -1, wxString::Format("%1.3f", m_Sector->desaturation), 
											 wxDefaultPosition, wxSize(60,-1));
	box->Add(m_Desaturation, 1, wxALL|wxALIGN_LEFT, 2);

	return box;

}

wxSizer * SectorEditDialog::CreateOtherBox(wxPanel *parent)
{
	wxStaticBox *frame = new wxStaticBox(parent, -1, "Gravity");// wxEmptyString);
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_Gravity = new wxTextCtrl(parent, -1, wxString::Format("%1.3f", m_Sector->gravity), 
											 wxDefaultPosition, wxSize(60,-1));
	box->Add(m_Gravity, 1, wxALL|wxALIGN_LEFT, 2);
	return box;
}

//==========================================================================
//
//
//
//==========================================================================

void SectorEditDialog::CreatePage4(wxPanel * page)
{
	wxBoxSizer * box = new wxBoxSizer(wxVERTICAL);
	wxSizer * box1;
		
	box1 = CreateColorBox(page);
	box->Add(box1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	box1 = CreateOtherBox(page);
	box->Add(box1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	page->SetSizerAndFit(box);
}


//==========================================================================
//
//
//
//==========================================================================

void SectorEditDialog::ChangeType(int special, bool fromedit)
{
	if (!m_Extended && special&0x8000)
	{
		special&=0x80ff;
	}
	else if (m_Extended) 
	{
		special&=0xff;
	}
	else if (m_Boom)
	{
		special&=0x1f;
	}
	m_TypeText->SetLabel( cgc->GetSectorName(special) );
}


//==========================================================================
//
//
//
//==========================================================================

SectorEditDialog::SectorEditDialog(wxWindow * parent, CLevel * level, CSector * sector)
	: wxDialog(parent, -1, wxString("Edit Sector"))
{
	wxBoxSizer * box = new wxBoxSizer(wxVERTICAL);
	wxNotebook * tabctrl = new wxNotebook(this, -1);

	m_Font = wxFont::New(wxSize(8,16), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false,
		"Courier New", wxFONTENCODING_SYSTEM);

	m_Sector = sector;
	m_Level = level;
	m_Extended = level->Extended();
	m_TextMap = level->TextMap();
	m_Boom = cgc->genericsectors;
	m_Changes=0;

	m_pages[0] = new wxPanel(tabctrl);
	tabctrl->AddPage(m_pages[0], "Sector");
	CreatePage1(m_pages[0]);

	m_pages[1] = new wxPanel(tabctrl);
	tabctrl->AddPage(m_pages[1], "Textures");
	CreatePage2(m_pages[1]);

	if (this->m_TextMap)
	{
		m_pages[2] = new wxPanel(tabctrl);
		tabctrl->AddPage(m_pages[2], "Planes");
		CreatePage3(m_pages[2]);

		m_pages[3] = new wxPanel(tabctrl);
		tabctrl->AddPage(m_pages[3], "Advanced");
		CreatePage4(m_pages[3]);

	}

	box->Add(tabctrl, 1, wxEXPAND|wxALL, 4);

	box->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);

	SetSizerAndFit(box);
	Layout();
	Center();

	ChangeType( m_Sector->special);
}


//==========================================================================
//
//
//
//==========================================================================

void SectorEditDialog::OnChangeType(wxCommandEvent & event)
{
	PopupMenu(cgc->GetSectorMenu());
}

void SectorEditDialog::OnMenuSelect(wxCommandEvent & event)
{
	m_Sector->special = event.GetId()-2000;
	if (m_Sector->special>=500) m_Sector->special+=32768-500;
	ChangeType(m_Sector->special);
	m_Changes|=SF_Type;
}

//==========================================================================
//
//
//
//==========================================================================

void SectorEditDialog::OnChangeColor(wxCommandEvent & event)
{
	int color = m_Sector->color;
	wxColour col((color>>16)&255,(color>>8)&255,(color)&255);
	wxColourData cold;
	cold.SetColour(col);
	wxColourDialog diag(this, &cold);
	if (diag.ShowModal() == wxID_OK)
	{
		cold = diag.GetColourData();
		col = cold.GetColour();
		m_Color[0]->SetValue(wxString::Format("%d", col.Red()));
		m_Color[1]->SetValue(wxString::Format("%d", col.Green()));
		m_Color[2]->SetValue(wxString::Format("%d", col.Blue()));
	}
}

//==========================================================================
//
//
//
//==========================================================================

void SectorEditDialog::OnChangeFade(wxCommandEvent & event)
{
	int fade = m_Sector->fade;
	wxColour col((fade>>16)&255,(fade>>8)&255,(fade)&255);
	wxColourData cold;
	cold.SetColour(col);
	wxColourDialog diag(this, &cold);
	if (diag.ShowModal() == wxID_OK)
	{
		cold = diag.GetColourData();
		col = cold.GetColour();
		m_Fade[0]->SetValue(wxString::Format("%d", col.Red()));
		m_Fade[1]->SetValue(wxString::Format("%d", col.Green()));
		m_Fade[2]->SetValue(wxString::Format("%d", col.Blue()));
	}
}

//==========================================================================
//
//
//
//==========================================================================

void SectorEditDialog::OnTagButton(wxCommandEvent & event)
{
	m_Sector->tag = m_Level->FindFreeTag();
	m_Tag->SetValue(m_Sector->tag);
	m_Changes|=SF_Tag;
}

//==========================================================================
//
//
//
//==========================================================================
void SectorEditDialog::UpdateTexture(int pos)
{
	wxString str = m_Textures[pos].m_TexName->GetValue();
	m_Textures[pos].m_Texture->SetTexture(str.c_str());
	if (m_Textures[pos].m_Texture->isValidTexture())
	{
		m_Textures[pos].m_TexInfo->SetLabel(wxString::Format("%d x %d, %s",
			m_Textures[pos].m_Texture->GetTextureWidth(),
			m_Textures[pos].m_Texture->GetTextureHeight(),
			TextureTypeNames[m_Textures[pos].m_Texture->GetTextureType()]));
	}
	else
		m_Textures[pos].m_TexInfo->SetLabel(wxEmptyString);
}

void SectorEditDialog::OnTextureChange(wxCommandEvent & event)
{
	int num = event.GetId()-SE_TEXEDITC;
	UpdateTexture(num);
}

void SectorEditDialog::OnTextureSelect(wxCommandEvent & event)
{
	int texnum = SelectTexture(this, tx_flat);
	if (texnum>=0)
	{
		int num = event.GetId()-SE_TEXPIXC;
		m_Textures[num].m_TexName->SetValue(texman->GetTextureName(texnum));
	}
}


//==========================================================================
//
// All information that isn't handled explicitly is read out here
//
//==========================================================================

void SectorEditDialog::OnOK(wxCommandEvent & event)
{
	if (m_Boom)
	{
		m_Sector->flags = m_Sector->notflags=0;

		int damage = m_Damage->GetSelection();
		if (damage!=wxNOT_FOUND)
		{
			m_Sector->flags = damage * 32;
			m_Sector->notflags = 0x60;
		}
		for(int i=0;i<3;i++) if (m_Flags[i])
		{
			int bit = 7+i;
			// Stupid wxWidgets limitation...
			int state = m_Flags[i]->Is3State()? m_Flags[i]->Get3StateValue() : m_Flags[i]->GetValue()? wxCHK_CHECKED : wxCHK_UNCHECKED;

			switch(state)
			{
			case wxCHK_CHECKED:
				m_Sector->flags |= 1<<bit;
			case wxCHK_UNCHECKED:
				m_Sector->notflags |= 1<<bit;
			default:
				break;
			}
		}
		if (m_Extended || m_Sector->special&0x8000)
		{
			m_Sector->flags<<=3;
			m_Sector->notflags<<=3;
		}
	}

	if (m_TextMap)
	{
		m_Sector->Flags.Clear();
		m_Sector->notFlags.Clear();
		for(int i=0;i<MAX_FLAGS;i++) if (m_XFlags[i])
		{
			// Stupid wxWidgets limitation...
			int state = m_XFlags[i]->Is3State()? m_XFlags[i]->Get3StateValue() : m_XFlags[i]->GetValue()? wxCHK_CHECKED : wxCHK_UNCHECKED;

			switch(state)
			{
			case wxCHK_CHECKED:
				m_Sector->Flags.SetBit(i);
			case wxCHK_UNCHECKED:
				m_Sector->notFlags.SetBit(i);
			default:
				break;
			}
		}
	}

	m_Sector->ceilh = m_Height[0]->GetValue();
	m_Sector->floorh= m_Height[1]->GetValue();

	int light = m_Height[2]->GetValue();
	if (light!=m_Sector->light)
	{
		m_Sector->light=light;
		m_Changes|=SF_Light;
	}

	if (m_Tag->GetValue()!=m_Sector->tag)
	{
		m_Sector->tag = m_Tag->GetValue();
		m_Changes |= SF_Tag;
	}


	for(int i=0;i<2;i++)
	{
		char * texbuf = i==0? m_Sector->ceilt : m_Sector->floort;

		if (m_Textures[i].m_TexName->GetValue().CmpNoCase(m_TexNames[i]))
		{
			wxString tex = m_Textures[i].m_TexName->GetValue().Upper();
			strncpy(texbuf, tex.c_str(), 8);
			m_Changes |= SF_CeilT << i;
		}
	}

	if (m_TextMap)
	{
		static int bits[2][5]={{0,0,SF_CScaleX,SF_CScaleY,SF_CRot},{0,0,SF_FScaleX,SF_FScaleY,SF_FRot}};
		for(int i=0;i<2;i++) 
		{
			double *values[]= { &m_Sector->offset[i][X], &m_Sector->offset[i][Y], 
							  &m_Sector->scale[i][X], &m_Sector->scale[i][Y], &m_Sector->rotation[i]};

			for(int j=0;j<5;j++)
			{
				double v = strtod(m_planes[i].m_rotation[j]->GetValue(), NULL);
				if (int(v*1000) != int(*values[j]*1000))
				{
					m_Changes |= bits[i][j];
					*values[j]=v;
				}
			}
			if (m_planes[i].m_planelight->GetValue() != m_Sector->planelight[i])
			{
				m_Sector->planelight[i] = m_planes[i].m_planelight->GetValue();
				m_Changes |= (i? SF_CLite:SF_FLite);
			}
		}

		int r,g,b;

		r = strtol(m_Color[R]->GetValue(), NULL, 10);
		g = strtol(m_Color[G]->GetValue(), NULL, 10);
		b = strtol(m_Color[B]->GetValue(), NULL, 10);
		int val = PalEntry(r,g,b);
		if (val != m_Sector->color)
		{
			m_Sector->color = val;
			m_Changes |= SF_LiteCol;
		}

		r = strtol(m_Fade[R]->GetValue(), NULL, 10);
		g = strtol(m_Fade[G]->GetValue(), NULL, 10);
		b = strtol(m_Fade[B]->GetValue(), NULL, 10);
		val = PalEntry(r,g,b);
		if (val != m_Sector->fade)
		{
			m_Sector->fade = val;
			m_Changes |= SF_FadeCol;
		}

		double v = strtod(m_Gravity->GetValue(), NULL);
		if (int(v*1000) != int(m_Sector->gravity*1000))
		{
			m_Changes |= SF_Gravity;
			m_Sector->gravity=v;
		}

		v = strtod(m_Desaturation->GetValue(), NULL);
		if (int(v*1000) != int(m_Sector->desaturation*1000))
		{
			m_Changes |= SF_Desat;
			m_Sector->desaturation=v;
		}
	}

	EndModal(wxID_OK);
}


//==========================================================================
//
//
//
//==========================================================================

int EditSector(wxWindow * parent, CLevel * level,  CSector * sector)
{
	SectorEditDialog dlg(parent, level, sector);
	int res = dlg.ShowModal();
	return res==wxID_OK? dlg.m_Changes : -1;
}
