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
#include "GameConfig.h"
#include "texture.h"


enum
{
	TE_ANGLE_E=100, 
	TE_ANGLE_SE,
	TE_ANGLE_S, 
	TE_ANGLE_SW,
	TE_ANGLE_W, 
	TE_ANGLE_NW,
	TE_ANGLE_N, 
	TE_ANGLE_NE,
	TE_ANGLE_EDIT,
	TE_CHANGETYPE,
	TE_TYPETEXT,
	TE_TYPEBROWSE,
	TE_EDITTYPE,
	TE_CHANGESPECIAL,
	TE_SPECIALTEXT,
	TE_EDITSPECIAL,
	TE_TIDBTN,
	TE_ARG1BTN,
	//TE_ARG1BTN=TE_ARG1+5,
};


//==========================================================================
//
//
//
//==========================================================================

class ThingEditDialog : public wxDialog
{
	wxCheckBox * m_Flags[MAX_FLAGS];
	wxRadioButton * m_Angles[9];
	wxSpinCtrl * m_AngleEdit;
	wxSpinCtrl * m_X, * m_Y, * m_Z;
	wxTextCtrl * m_Xf, * m_Yf, * m_Zf;
	wxButton * m_ChangeType;
	wxTextCtrl * m_Type;
	wxStaticText * m_TypeText;
	TextureBox * m_TypeBrowse;

	wxButton * m_ChangeSpecial;
	wxStaticText * m_SpecialText;
	wxTextCtrl * m_Special;
	wxSpinCtrl * m_Tid;
	wxButton * m_TidButton;
	wxSpinCtrl * m_Args[5];
	wxStaticText * m_ArgText[5];
	wxButton * m_ArgButton[5];

	wxPanel * m_pages[2];

	bool m_Extended;
	bool m_TextMap;
	bool specialmenu;

	wxSizer * CreateAngleBox(wxWindow * parent);
	wxSizer * CreateFlagBox(wxWindow * parent, const wxString &title, int start, int end);
	wxSizer * CreateTypeBox(wxWindow * parent);
	wxSizer * CreateCoordBox(wxWindow * parent);
	wxSizer * CreateSpecialBox(wxWindow * parent);
	wxSizer * CreateArgBox(wxWindow * parent);
	wxSizer * CreateTidBox(wxWindow * parent);
	void CreatePage1(wxPanel * page);
	void CreatePage2(wxPanel * page);

	void ChangeType(int newtype, bool fromedit=false);
	void ChangeSpecial(int special, bool fromedit=false);
	void ChangeArgs();
	void OnChangeAngle(int mask, int value);

	void OnOK(wxCommandEvent & event);
	void OnChangeType(wxCommandEvent & event);
	void OnChangeSpecial(wxCommandEvent & event);
	void OnMenuSelect(wxCommandEvent & event);
	void OnEditType(wxCommandEvent & event);
	void OnEditSpecial(wxCommandEvent & event);
	void OnRadioAngle(wxCommandEvent & event);
	void OnEditAngle(wxCommandEvent & event);
	void OnSpinAngle(wxSpinEvent & event);
	void OnTidButton(wxCommandEvent & event);
	void OnArgButton(wxCommandEvent & event);

public:
	CLevel * m_Level;
	CThing * m_Thing;
	int m_Changes;

	ThingEditDialog(wxWindow * parent, CLevel * level, CThing * thing);

	DECLARE_EVENT_TABLE()
};


BEGIN_EVENT_TABLE(ThingEditDialog, wxDialog)

	EVT_BUTTON(TE_CHANGETYPE, ThingEditDialog::OnChangeType)
	EVT_BUTTON(TE_CHANGESPECIAL, ThingEditDialog::OnChangeSpecial)
	EVT_BUTTON(TE_TIDBTN, ThingEditDialog::OnTidButton)
	EVT_MENU_RANGE(2000, 35000, ThingEditDialog::OnMenuSelect)
	EVT_TEXT(TE_EDITTYPE, ThingEditDialog::OnEditType)
	EVT_TEXT(TE_EDITSPECIAL, ThingEditDialog::OnEditSpecial)
	EVT_COMMAND_RANGE(TE_ANGLE_E, TE_ANGLE_NE, wxEVT_COMMAND_RADIOBUTTON_SELECTED , ThingEditDialog::OnRadioAngle) 
	EVT_TEXT(TE_ANGLE_EDIT, ThingEditDialog::OnEditAngle)
	EVT_SPIN(TE_ANGLE_EDIT, ThingEditDialog::OnSpinAngle)
	EVT_BUTTON(wxID_OK, ThingEditDialog::OnOK)
	EVT_COMMAND_RANGE(TE_ARG1BTN, TE_ARG1BTN+4, wxEVT_COMMAND_BUTTON_CLICKED, ThingEditDialog::OnArgButton)

END_EVENT_TABLE()

//==========================================================================
//
//
//
//==========================================================================

wxSizer * ThingEditDialog::CreateAngleBox(wxWindow * parent)
{
	// Taken from Slade.

	wxStaticBox * frame = new wxStaticBox(parent, -1, "Angle");
	wxStaticBoxSizer * box = new wxStaticBoxSizer(frame, wxVERTICAL);

	m_Angles[0] = new wxRadioButton(parent, TE_ANGLE_N, "", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_Angles[1] = new wxRadioButton(parent, TE_ANGLE_NE, "");
	m_Angles[2] = new wxRadioButton(parent, TE_ANGLE_E, "");
	m_Angles[3] = new wxRadioButton(parent, TE_ANGLE_SE, "");
	m_Angles[4] = new wxRadioButton(parent, TE_ANGLE_S, "");
	m_Angles[5] = new wxRadioButton(parent, TE_ANGLE_SW, "");
	m_Angles[6] = new wxRadioButton(parent, TE_ANGLE_W, "");
	m_Angles[7] = new wxRadioButton(parent, TE_ANGLE_NW, "");
	m_Angles[8] = new wxRadioButton(parent, -1, "");
	m_Angles[8]->Hide();

	wxGridSizer *grid = new wxGridSizer(5, 5, 4, 4);
	box->Add(grid, 1, wxEXPAND|wxALL, 4);

	// Yeah, I know its ugly. I can't find a better way to do this
	grid->AddSpacer(0);
	grid->AddSpacer(0);
	grid->Add(m_Angles[6]);
	grid->AddSpacer(0);
	grid->AddSpacer(0);
	grid->AddSpacer(0);
	grid->Add(m_Angles[5]);
	grid->AddSpacer(0);
	grid->Add(m_Angles[7]);
	grid->AddSpacer(0);
	grid->Add(m_Angles[4]);
	grid->AddSpacer(0);
	grid->AddSpacer(0);
	grid->AddSpacer(0);
	grid->Add(m_Angles[0]);
	grid->AddSpacer(0);
	grid->Add(m_Angles[3]);
	grid->AddSpacer(0);
	grid->Add(m_Angles[1]);
	grid->AddSpacer(0);
	grid->AddSpacer(0);
	grid->AddSpacer(0);
	grid->Add(m_Angles[2]);

	m_AngleEdit = new wxSpinCtrl(parent, TE_ANGLE_EDIT, "");
	box->Add(m_AngleEdit, 0, wxEXPAND|wxALL, 4);
	m_AngleEdit->SetRange(0,359);

	m_Changes|=THF_Guard;
	OnChangeAngle(3, m_Thing->angle);
	m_Changes&=~THF_Guard;

	return box;
}

//==========================================================================
//
//
//
//==========================================================================

wxSizer * ThingEditDialog::CreateFlagBox(wxWindow * parent, const wxString &title, int start, int end)
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
		wxString txt = cgc->GetThingFlagText(i, false, m_Level->TextMap());
		if (!!txt) flagc++;
		m_Flags[i]=NULL;
	}
	flagc=(flagc+2)/3*3;

	for(int i=start;i<=end;i++)
	{
		wxString txt = cgc->GetThingFlagText(i, false, m_Level->TextMap());
		if (!!txt)
		{
			int check;
			int bit = 1<<i;
			if (m_Thing->multi)	// multiple selection
			{
				int set = m_Thing->Flags.GetBit(i);
				int not = m_Thing->notflags.GetBit(i);

				if (!set && !not) check = 0;
				else if (set && not) check = 1;
				else check = 2;
			}
			else check = !!m_Thing->Flags.GetBit(i);

			long style = check==2? wxCHK_3STATE | wxCHK_ALLOW_3RD_STATE_FOR_USER : 0;

			m_Flags[i]=new wxCheckBox(parent, -1, txt, wxDefaultPosition, wxDefaultSize, style);
			if (flagc1<flagc/3) left->Add(m_Flags[i], 1, wxEXPAND|wxTOP|wxBOTTOM, 1);
			else if (flagc1<flagc*2/3) mid->Add(m_Flags[i], 1, wxEXPAND|wxTOP|wxBOTTOM, 1);
			else right->Add(m_Flags[i], 1, wxEXPAND|wxTOP|wxBOTTOM, 1);

			if (check==2) m_Flags[i]->Set3StateValue(wxCHK_UNDETERMINED);
			else m_Flags[i]->SetValue(!!check);

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

wxSizer * ThingEditDialog::CreateTypeBox(wxWindow * parent)
{
	wxStaticBox *frame = new wxStaticBox(parent, -1, "Type");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxHORIZONTAL);
	wxBoxSizer * line = new wxBoxSizer(wxVERTICAL);

	m_TypeBrowse = new TextureBox(parent, TE_TYPEBROWSE, 96, 96);
	box->Add(m_TypeBrowse, 0, wxALL, 4);

	m_TypeText = new wxStaticText(parent, TE_TYPETEXT, "Execute Sector Action when Hitting Sector's Fake Floor");
	line->Add(m_TypeText, 0, wxALL, 6);

	m_ChangeType = new wxButton(parent, TE_CHANGETYPE, "Change Type");
	line->Add(m_ChangeType, 0, wxALL, 6);

	m_Type = new wxTextCtrl(parent, TE_EDITTYPE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	line->Add(m_Type, 0, wxALL, 6);

	box->Add(line, 0, wxALL, 4);

	// Thing type cannot be set here because it would interfere with the dialog box formatting

	return box;
}

//==========================================================================
//
//
//
//==========================================================================

wxSizer * ThingEditDialog::CreateCoordBox(wxWindow * parent)
{
	wxStaticBox *frame = new wxStaticBox(parent, -1, "Coordinates");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);
	wxBoxSizer * line;

	if (!m_TextMap)
	{
		line = new wxBoxSizer(wxHORIZONTAL);
		line->Add(new wxStaticText(parent, -1, "X-position"), 0, wxRIGHT, 5);
		m_X = new wxSpinCtrl(parent, -1);
		m_X->SetRange(-32767,32767);
		m_X->SetValue(int(m_Thing->X()));
		line->Add(m_X, 0, wxALIGN_RIGHT);
		box->Add(line, 0, wxALL, 4);

		line = new wxBoxSizer(wxHORIZONTAL);
		line->Add(new wxStaticText(parent, -1, "Y-position"), 0, wxRIGHT, 5);
		m_Y = new wxSpinCtrl(parent, -1);
		m_Y->SetRange(-32767,32767);
		m_Y->SetValue(int(m_Thing->Y()));
		line->Add(m_Y, 0, wxALIGN_RIGHT);
		box->Add(line, 0, wxALL, 4);

		if (m_Extended)
		{
			line = new wxBoxSizer(wxHORIZONTAL);
			line->Add(new wxStaticText(parent, -1, "Z-position"), 0, wxRIGHT, 5);
			m_Z = new wxSpinCtrl(parent, -1);
			m_Z->SetRange(-32767,32767);
			m_Z->SetValue(int(m_Thing->Z()));
			line->Add(m_Z, 0, wxALIGN_RIGHT);
			box->Add(line, 0, wxALL, 4);
		}
	}
	else
	{
		line = new wxBoxSizer(wxHORIZONTAL);
		line->Add(new wxStaticText(parent, -1, "X-position"), 0, wxRIGHT, 5);
		m_Xf = new wxTextCtrl(parent, -1);
		m_Xf->SetValue(wxString::Format("%1.3f", m_Thing->X()+0.0001));
		line->Add(m_Xf, 0, wxALIGN_RIGHT);
		box->Add(line, 0, wxALL, 4);

		line = new wxBoxSizer(wxHORIZONTAL);
		line->Add(new wxStaticText(parent, -1, "Y-position"), 0, wxRIGHT, 5);
		m_Yf = new wxTextCtrl(parent, -1);
		m_Yf->SetValue(wxString::Format("%1.3f", m_Thing->Y()+0.0001));
		line->Add(m_Yf, 0, wxALIGN_RIGHT);
		box->Add(line, 0, wxALL, 4);

		line = new wxBoxSizer(wxHORIZONTAL);
		line->Add(new wxStaticText(parent, -1, "Z-position"), 0, wxRIGHT, 5);
		m_Zf = new wxTextCtrl(parent, -1);
		m_Zf->SetValue(wxString::Format("%1.3f", m_Thing->Z()+0.0001));
		line->Add(m_Zf, 0, wxALIGN_RIGHT);
		box->Add(line, 0, wxALL, 4);
	}

	return box;
}

//==========================================================================
//
//
//
//==========================================================================

void ThingEditDialog::CreatePage1(wxPanel * page)
{
	wxBoxSizer * box = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * box2 = new wxBoxSizer(wxHORIZONTAL);

	box->Add(CreateTypeBox(page), 1, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	if (!m_TextMap)
	{
		box->Add(CreateFlagBox(page, "Flags", 0, MAX_FLAGS-1), 0, wxALL|wxEXPAND, 4);
	}
	else
	{
		memset(m_Flags, 0, sizeof(m_Flags));
		for(int i=0; i < cgc->ThingGroupCount; i++)
		{
			FlagGroupDesc * fd = &cgc->TextMapThingFlagGroups[i];
			if (m_Extended && fd->modeflag == 1) continue;
			if (!m_Extended && fd->modeflag == 2) continue;
			box->Add(CreateFlagBox(page, fd->name, fd->firstindex, fd->lastindex), 0, wxALL|wxEXPAND, 4);
		}
	}
	box2->Add(CreateAngleBox(page), 0, wxRIGHT, 4);
	box2->Add(CreateCoordBox(page), 0, wxLEFT, 4);
	box->Add(box2, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	page->SetSizerAndFit(box);
}

//==========================================================================
//
//
//
//==========================================================================

wxSizer * ThingEditDialog::CreateSpecialBox(wxWindow * parent)
{
	wxStaticBox *frame = new wxStaticBox(parent, -1, "Special");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);

	m_SpecialText = new wxStaticText(parent, TE_SPECIALTEXT, "This is a very long text line to format the dialog properly but it must be longer");
	box->Add(m_SpecialText, 0, wxALL, 6);

	m_ChangeSpecial = new wxButton(parent, TE_CHANGESPECIAL, "Change Special");
	box->Add(m_ChangeSpecial, 0, wxALL, 6);

	m_Special = new wxTextCtrl(parent, TE_EDITSPECIAL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	box->Add(m_Special, 0, wxALL, 6);

	// special cannot be set here because it would interfere with the dialog box formatting

	return box;
}

//==========================================================================
//
//
//
//==========================================================================

wxSizer * ThingEditDialog::CreateArgBox(wxWindow * parent)
{
	wxStaticBox *frame = new wxStaticBox(parent, -1, "Arguments");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);

	for(int i=0;i<5;i++)
	{
		wxBoxSizer * line = new wxBoxSizer(wxHORIZONTAL);
		int size = m_TextMap? 100:60;

		m_Args[i] = new wxSpinCtrl(parent, -1, wxEmptyString, wxDefaultPosition, wxSize(size,-1));
		if (!m_TextMap) m_Args[i]->SetRange(0,255);
		m_Args[i]->SetValue(m_Thing->args[i]);
		line->Add(m_Args[i], 0, wxLEFT, 4);

		m_ArgButton[i] = new wxButton(parent, TE_ARG1BTN+i, "Change", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
		line->Add(m_ArgButton[i], 0, wxLEFT, 4);

		m_ArgText[i] = new wxStaticText(parent, -1, "This is a very long text line to format the dialog");
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

wxSizer * ThingEditDialog::CreateTidBox(wxWindow * parent)
{
	wxStaticBox *frame = new wxStaticBox(parent, -1, "Thing ID");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxHORIZONTAL);

	m_Tid = new wxSpinCtrl(parent, -1, wxEmptyString);
	m_Tid->SetRange(0,32767);
	m_Tid->SetValue(m_Thing->thingid);
	box->Add(m_Tid, 0, wxALL, 4);

	m_TidButton = new wxButton(parent, TE_TIDBTN, "Find free", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	box->Add(m_TidButton, 0, wxALL, 4);
	return box;
}

//==========================================================================
//
//
//
//==========================================================================

void ThingEditDialog::CreatePage2(wxPanel * page)
{
	wxBoxSizer * box = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * box2 = new wxBoxSizer(wxHORIZONTAL);

	box->Add(CreateSpecialBox(page), 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	box->Add(CreateArgBox(page), 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	box->Add(CreateTidBox(page), 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	box->Add(box2, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	page->SetSizerAndFit(box);
}

//==========================================================================
//
//
//
//==========================================================================

void ThingEditDialog::ChangeType(int newtype, bool fromedit)
{
	if (!fromedit) 
	{
		m_Changes|=THF_Guard;
		m_Type->SetValue(wxString::Format("%d",newtype));
		m_Changes&=~THF_Guard;
	}
	m_TypeText->SetLabel(cgc->GetThingName(newtype));
	m_TypeBrowse->SetTexture(cgc->GetThingSprite(newtype), tx_sprite);
	m_Thing->type = newtype;
	ChangeArgs();
}

//==========================================================================
//
//
//
//==========================================================================

void ThingEditDialog::ChangeSpecial(int special, bool fromedit)
{
	if (!fromedit) 
	{
		m_Changes|=THF_Guard;
		m_Special->SetValue(wxString::Format("%d", special));
		m_Changes&=~THF_Guard;
	}
	// This is always Hexen mode!
	m_SpecialText->SetLabel( cgc->GetLineDefDescription(special, -1, NULL, 2) );
	m_Thing->special = special;
	ChangeArgs();
}

void ThingEditDialog::ChangeArgs()
{
	if (m_Extended)
	{
		int type = m_Thing->type;
		int special = m_Thing->special;
		bool thingargs = !!cgc->GetThingArg(type, 0);

		for(int i=0;i<5;i++)
		{
			wxString str;

			if (thingargs) str = cgc->GetThingArg(type, i);
			else if (cgc->LineMap[special]) str = cgc->LineMap[special]->args[i];
			if (!str) str = wxString::Format("Arg %d",i+1);

			int pos=str.Find(':');
			if (pos>=0)
			{
				// Special arg type - to be implemented
				
				m_ArgButton[i]->Enable();

				str = str.Mid(pos+1); 
			}
			else m_ArgButton[i]->Enable(false);

			m_ArgText[i]->SetLabel(str);
		}
	}
}


//==========================================================================
//
//
//
//==========================================================================

ThingEditDialog::ThingEditDialog(wxWindow * parent, CLevel * level, CThing * thing)
	: wxDialog(parent, -1, wxString("Edit Thing"))
{
	wxBoxSizer * box = new wxBoxSizer(wxVERTICAL);
	wxNotebook * tabctrl = new wxNotebook(this, -1);

	m_Thing = thing;
	m_Level = level;
	m_Extended = level->Extended();
	m_TextMap = level->TextMap();
	m_Changes=0;

	m_pages[0] = new wxPanel(tabctrl);
	tabctrl->AddPage(m_pages[0], "Properties");
	CreatePage1(m_pages[0]);

	if (m_Extended)
	{
		m_pages[1] = new wxPanel(tabctrl);
		tabctrl->AddPage(m_pages[1], "Special");
		CreatePage2(m_pages[1]);
	}
	box->Add(tabctrl, 1, wxEXPAND|wxALL, 4);

	box->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);

	SetSizerAndFit(box);
	Layout();
	Center();
	// Do this after the layout is complete
	ChangeType(m_Thing->type);
	if (m_Extended) ChangeSpecial(m_Thing->special);
}


//==========================================================================
//
//
//
//==========================================================================

void ThingEditDialog::OnChangeType(wxCommandEvent & event)
{
	specialmenu=false;
	PopupMenu(cgc->GetThingMenu());
}

void ThingEditDialog::OnChangeSpecial(wxCommandEvent & event)
{
	specialmenu=true;
	PopupMenu(cgc->GetSpecialMenu());
}

void ThingEditDialog::OnMenuSelect(wxCommandEvent & event)
{
	if (!specialmenu)
	{
		m_Thing->type = event.GetId()-2000;
		ChangeType(m_Thing->type);
		m_Changes|=THF_Type;
	}
	else
	{
		m_Thing->special = event.GetId()-2000;
		ChangeSpecial(m_Thing->special);
		m_Changes|=THF_Special;
	}
}

//==========================================================================
//
//
//
//==========================================================================

void ThingEditDialog::OnEditType(wxCommandEvent & event)
{
	long l;
	if (!(m_Changes&THF_Guard) && m_Type->GetValue().ToLong(&l))
	{
		m_Thing->type = l;
		ChangeType(l, true);
		m_Changes|=THF_Type;
	}
}

//==========================================================================
//
//
//
//==========================================================================

void ThingEditDialog::OnEditSpecial(wxCommandEvent & event)
{
	long l;
	if (!(m_Changes&THF_Guard) && m_Special->GetValue().ToLong(&l))
	{
		m_Thing->special = l;
		ChangeSpecial(l, true);
		m_Changes|=THF_Special;
	}
}

//==========================================================================
//
//
//
//==========================================================================

void ThingEditDialog::OnChangeAngle(int mask, int value)
{
	if (!(m_Changes&THF_Guard))
	{
		m_Changes|=THF_Guard;
		if (mask&1)
		{
			m_AngleEdit->SetValue(value);
		}
		if (mask&2)
		{
			for(int i=0;i<8;i++) m_Angles[i]->SetValue(false);
			if (value%45==0)
			{
				m_Angles[(8-value/45)&7]->SetValue(true);
			}
		}
		m_Changes&=~THF_Guard;
		m_Changes|=THF_Angle;
		m_Thing->angle=value;
	}
}

void ThingEditDialog::OnRadioAngle(wxCommandEvent & event)
{
	// Yes, this looks retarded but it was the only way to make it work
	// The radio button notifications apparently don't work as you would expect...
	for(int i=0;i<8;i++) if (m_Angles[i]->GetValue())
	{
		OnChangeAngle(1, ((8-i)&7)*45);
		return;
	}
}

void ThingEditDialog::OnEditAngle(wxCommandEvent & event)
{
	OnChangeAngle(2, m_AngleEdit->GetValue());
}

void ThingEditDialog::OnSpinAngle(wxSpinEvent & event)
{
	OnChangeAngle(2, m_AngleEdit->GetValue());
}


//==========================================================================
//
//
//
//==========================================================================

void ThingEditDialog::OnTidButton(wxCommandEvent & event)
{
	m_Thing->thingid = m_Level->FindFreeTid();
	m_Tid->SetValue(m_Thing->thingid);
	m_Changes|=THF_TID;
}

//==========================================================================
//
//
//
//==========================================================================

void ThingEditDialog::OnArgButton(wxCommandEvent & event)
{
	int btn = event.GetId()-TE_ARG1BTN;

	if (m_Extended)
	{
		if (cgc->LineMap[m_Thing->special])
		{
			wxString str = cgc->LineMap[m_Thing->special]->args[btn];
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

void ThingEditDialog::OnOK(wxCommandEvent & event)
{
	m_Thing->Flags.Clear();
	m_Thing->notflags.Clear();
	for(int i=0;i<MAX_FLAGS;i++) if (m_Flags[i])
	{
		// Stupid wxWidgets limitation...
		int state = m_Flags[i]->Is3State()? m_Flags[i]->Get3StateValue() : m_Flags[i]->GetValue()? wxCHK_CHECKED : wxCHK_UNCHECKED;

		switch(state)
		{
		case wxCHK_CHECKED:
			m_Thing->Flags.SetBit(i);
		case wxCHK_UNCHECKED:
			m_Thing->notflags.SetBit(i);
		default:
			break;
		}
	}
	if (!m_TextMap)
	{
		m_Thing->SetX(m_X->GetValue(), false);
		m_Thing->SetY(m_Y->GetValue(), false);
	}
	if (m_Extended)
	{
		if (!m_TextMap) m_Thing->SetZ(m_Z->GetValue(), false);
		for(int i=0;i<5;i++)
		{
			if (m_Args[i]->GetValue()!=m_Thing->args[i])
			{
				m_Thing->args[i] = m_Args[i]->GetValue();
				m_Changes |= THF_Arg1<<i;
			}
		}
		if (m_Tid->GetValue()!=m_Thing->thingid)
		{
			m_Thing->thingid = m_Tid->GetValue();
			m_Changes |= THF_TID;
		}
	}
	if (m_TextMap)
	{
		double v;

		v = strtod(m_Xf->GetValue(), NULL);
		if (v < -32767) v = -32767;
		if (v > 32767) v = 32767;
		m_Thing->SetX(v, true);

		v = strtod(m_Yf->GetValue(), NULL);
		if (v < -32767) v = -32767;
		if (v > 32767) v = 32767;
		m_Thing->SetY(v, true);

		v = strtod(m_Zf->GetValue(), NULL);
		if (v < -32767) v = -32767;
		if (v > 32767) v = 32767;
		m_Thing->SetZ(v, true);
	}
	EndModal(wxID_OK);
}

//==========================================================================
//
//
//
//==========================================================================

int EditThing(wxWindow * parent, CLevel * level,  CThing * thing)
{
	ThingEditDialog dlg(parent, level, thing);
	int res = dlg.ShowModal();
	return res==wxID_OK? dlg.m_Changes : -1;
}
