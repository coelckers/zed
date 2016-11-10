//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2000-2005 Christoph Oelckers
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
// GenericDoor.cpp : Handler for Boom's generic linedef type dialogs
//

#include "stdafx.h"
#include "GenericTriggers.h"

//==========================================================================
//
//
//
//==========================================================================

static const wxChar * Tx_Trigger[]={"W1","WR","S1","SR","G1","GR","D1","DR"};
static const wxChar * Tx_Speed[]={"Slow","Normal","Fast","Turbo"};
static const wxChar * Tx_Doordelay[]={"1 sec","4 sec","9 sec","30 sec"};
static const wxChar * Tx_Liftdelay[]={"1 sec","3 sec","5 sec","10 sec"};
static const wxChar * Tx_Doortype[]={"Open/Wait/Close", "Open (stays open)", "Close/Wait/Open", "Close (stays closed)"};
static const wxChar * Tx_Lifttype[]={"Lowest neighboring floor", "Next neighboring floor", "Lowest neighboring ceiling", "Moving platform"};
static const wxChar * Tx_YesNo[]={"No", "Yes"};
static const wxChar * Tx_Lock[]={ "any key", "Red keycard", "Blue keycard", "Yellow keycard", "Red skull key", "Blue skull key", "Yellow skull key", "all keys"};
static const wxChar * Tx_Step[]={"4 units","8 units","16 units","24 units"};
static const wxChar * Tx_UpDown[]={"Up", "Down"};
static const wxChar * Tx_Floor[]={"Highest neighboring floor", "Lowest neighboring floor", "Next neighboring floor", "Lowest neighboring ceiling", "Ceiling", "Shortest lower texture", "24 Units", "32 Units"};
static const wxChar * Tx_Ceil[]={"Highest neighboring ceiling", "Lowest neighboring ceiling", "Next neighboring ceiling", "Lowest neighboring floor", "Floor", "Shortest upper texture", "24 Units", "32 Units"};
static const wxChar * Tx_Change[]={"No changes", "Change texture and reset type", "Change texture only", "Change texture and type"};

const int GD_CHANGE=100;


class GenericDoorDialog : public wxDialog
{
	wxRadioBox * m_Trigger;
	wxRadioBox * m_Speed;
	wxRadioBox * m_Delay;
	wxRadioBox * m_Type;
	wxRadioBox * m_Monster;

	void OnOK(wxCommandEvent&);

public:

	int m_Special;

	GenericDoorDialog(wxWindow * parent, int startval);

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(GenericDoorDialog, wxDialog)
    EVT_BUTTON(wxID_OK, GenericDoorDialog::OnOK)
END_EVENT_TABLE()


//==========================================================================
//
//
//
//==========================================================================

GenericDoorDialog::GenericDoorDialog(wxWindow * parent, int startval)
	: wxDialog(parent, -1, wxString("Generic door"))
{
	wxBoxSizer * mainsizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * buttonsizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer * row2 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * row3 = new wxBoxSizer(wxVERTICAL);

	mainsizer->Add(buttonsizer, 0, wxTOP|wxBOTTOM, 4);

	m_Trigger = new wxRadioBox(this, -1, "Trigger", wxDefaultPosition, wxDefaultSize, wxArrayString(8, Tx_Trigger), 1, wxRA_SPECIFY_COLS);
	buttonsizer->Add(m_Trigger, 0, wxALL, 4);

	m_Speed = new wxRadioBox(this, -1, "Speed", wxDefaultPosition, wxDefaultSize, wxArrayString(4, Tx_Speed), 1, wxRA_SPECIFY_COLS);
	row2->Add(m_Speed, 0, wxALL, 4);

	m_Delay = new wxRadioBox(this, -1, "Delay", wxDefaultPosition, wxDefaultSize, wxArrayString(4, Tx_Doordelay), 1, wxRA_SPECIFY_COLS);
	row2->Add(m_Delay, 0, wxALL, 4);

	buttonsizer->Add(row2, 0, wxLEFT|wxRIGHT, 8);

	m_Type = new wxRadioBox(this, -1, "Type", wxDefaultPosition, wxDefaultSize, wxArrayString(4, Tx_Doortype), 1, wxRA_SPECIFY_COLS);
	row3->Add(m_Type, 0, wxALL, 4);

	m_Monster = new wxRadioBox(this, -1, "Monsters can open", wxDefaultPosition, wxDefaultSize, wxArrayString(2, Tx_YesNo), 1, wxRA_SPECIFY_COLS);
	row3->Add(m_Monster, 0, wxALL, 4);

	buttonsizer->Add(row3, 0, 0);

	mainsizer->Add(new wxStaticLine(this), 0, wxALL, 4);
	mainsizer->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxALIGN_CENTER);

	m_Trigger->SetSelection(GET(startval,TRIGGER));
	m_Speed->SetSelection(GET(startval,SPEED));
	m_Delay->SetSelection(GET(startval,DOORDELAY));
	m_Type->SetSelection(GET(startval,DOORTYPE));
	m_Monster->SetSelection(GET(startval,DOORMONSTER));

	SetSizerAndFit(mainsizer);
	Layout();
	Center();
}

//==========================================================================
//
//
//
//==========================================================================

void GenericDoorDialog::OnOK(wxCommandEvent & event)
{
	m_Special =	DOOR_BASE |
		SET(m_Trigger->GetSelection(),TRIGGER  ) |
		SET(m_Speed->GetSelection()  ,SPEED    ) |
		SET(m_Type->GetSelection()   ,DOORTYPE ) |
		SET(m_Delay->GetSelection()  ,DOORDELAY) |
		SET(m_Monster->GetSelection(),DOORMONSTER)
		;

	EndModal(wxID_OK);
}

//==========================================================================
//
//
//
//==========================================================================

int GetGenericDoorType(wxWindow * parent, int initial)
{
	GenericDoorDialog dlg(parent, initial);
	if (dlg.ShowModal()==wxID_OK) return dlg.m_Special;
	return -1;
}

//==========================================================================
//
//
//
//
//
//
//==========================================================================

class GenericLockedDoorDialog : public wxDialog
{
	wxRadioBox * m_Trigger;
	wxRadioBox * m_Speed;
	wxRadioBox * m_Type;
	wxRadioBox * m_Lock;
	wxCheckBox * m_Skull;

	void OnOK(wxCommandEvent&);

public:

	int m_Special;

	GenericLockedDoorDialog(wxWindow * parent, int startval);

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(GenericLockedDoorDialog, wxDialog)
    EVT_BUTTON(wxID_OK, GenericLockedDoorDialog::OnOK)
END_EVENT_TABLE()


//==========================================================================
//
//
//
//==========================================================================

GenericLockedDoorDialog::GenericLockedDoorDialog(wxWindow * parent, int startval)
	: wxDialog(parent, -1, wxString("Generic locked door"))
{
	wxBoxSizer * mainsizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * buttonsizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer * row2 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * row3 = new wxBoxSizer(wxVERTICAL);

	mainsizer->Add(buttonsizer, 0, wxTOP|wxBOTTOM, 4);

	m_Trigger = new wxRadioBox(this, -1, "Trigger", wxDefaultPosition, wxDefaultSize, wxArrayString(8, Tx_Trigger), 1, wxRA_SPECIFY_COLS);
	buttonsizer->Add(m_Trigger, 0, wxALL, 4);

	m_Speed = new wxRadioBox(this, -1, "Speed", wxDefaultPosition, wxDefaultSize, wxArrayString(4, Tx_Speed), 1, wxRA_SPECIFY_COLS);
	row2->Add(m_Speed, 0, wxALL, 4);

	m_Type = new wxRadioBox(this, -1, "Type", wxDefaultPosition, wxDefaultSize, wxArrayString(2, Tx_Doortype), 1, wxRA_SPECIFY_COLS);
	row2->Add(m_Type, 0, wxALL, 4);

	buttonsizer->Add(row2, 0, wxLEFT|wxRIGHT, 8);

	m_Lock = new wxRadioBox(this, -1, "Lock", wxDefaultPosition, wxDefaultSize, wxArrayString(8, Tx_Lock), 1, wxRA_SPECIFY_COLS);
	row3->Add(m_Lock, 0, wxALL, 4);

	m_Skull = new wxCheckBox(this, -1, "Skull=Card");
	row3->Add(m_Skull, 0, wxALL, 4);

	buttonsizer->Add(row3, 0, 0);

	mainsizer->Add(new wxStaticLine(this), 0, wxALL, 4);
	mainsizer->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxALIGN_CENTER);

	m_Trigger->SetSelection(GET(startval,TRIGGER));
	m_Speed->SetSelection(GET(startval,SPEED));
	m_Type->SetSelection(GET(startval,LOCKTYPE));
	m_Lock->SetSelection(GET(startval,LOCKLOCK));
	m_Skull->SetValue(!!GET(startval,LOCKSISK));

	SetSizerAndFit(mainsizer);
	Layout();
	Center();
}

//==========================================================================
//
//
//
//==========================================================================

void GenericLockedDoorDialog::OnOK(wxCommandEvent & event)
{
	m_Special =	LOCK_BASE |
		SET(m_Trigger->GetSelection(),TRIGGER  ) |
		SET(m_Speed->GetSelection()  ,SPEED    ) |
		SET(m_Type->GetSelection()   ,LOCKTYPE ) |
		SET(m_Lock->GetSelection()  ,LOCKLOCK) |
		SET(m_Skull->GetValue(),LOCKSISK)
		;

	EndModal(wxID_OK);
}

//==========================================================================
//
//
//
//==========================================================================

int GetGenericLockedDoorType(wxWindow * parent, int initial)
{
	GenericLockedDoorDialog dlg(parent, initial);
	if (dlg.ShowModal()==wxID_OK) return dlg.m_Special;
	return -1;
}

//==========================================================================
//
//
//
//
//
//
//==========================================================================

class GenericLiftDialog : public wxDialog
{
	wxRadioBox * m_Trigger;
	wxRadioBox * m_Speed;
	wxRadioBox * m_Delay;
	wxRadioBox * m_Type;
	wxRadioBox * m_Monster;

	void OnOK(wxCommandEvent&);

public:

	int m_Special;

	GenericLiftDialog(wxWindow * parent, int startval);

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(GenericLiftDialog, wxDialog)
    EVT_BUTTON(wxID_OK, GenericLiftDialog::OnOK)
END_EVENT_TABLE()


//==========================================================================
//
//
//
//==========================================================================

GenericLiftDialog::GenericLiftDialog(wxWindow * parent, int startval)
	: wxDialog(parent, -1, wxString("Generic lift"))
{
	wxBoxSizer * mainsizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * buttonsizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer * row2 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * row3 = new wxBoxSizer(wxVERTICAL);

	mainsizer->Add(buttonsizer, 0, wxTOP|wxBOTTOM, 4);

	m_Trigger = new wxRadioBox(this, -1, "Trigger", wxDefaultPosition, wxDefaultSize, wxArrayString(8, Tx_Trigger), 1, wxRA_SPECIFY_COLS);
	buttonsizer->Add(m_Trigger, 0, wxALL, 4);

	m_Speed = new wxRadioBox(this, -1, "Speed", wxDefaultPosition, wxDefaultSize, wxArrayString(4, Tx_Speed), 1, wxRA_SPECIFY_COLS);
	row2->Add(m_Speed, 0, wxALL, 4);

	m_Delay = new wxRadioBox(this, -1, "Delay", wxDefaultPosition, wxDefaultSize, wxArrayString(4, Tx_Liftdelay), 1, wxRA_SPECIFY_COLS);
	row2->Add(m_Delay, 0, wxALL, 4);

	buttonsizer->Add(row2, 0, wxLEFT|wxRIGHT, 8);

	m_Type = new wxRadioBox(this, -1, "Type", wxDefaultPosition, wxDefaultSize, wxArrayString(4, Tx_Lifttype), 1, wxRA_SPECIFY_COLS);
	row3->Add(m_Type, 0, wxALL, 4);

	m_Monster = new wxRadioBox(this, -1, "Monsters can trigger", wxDefaultPosition, wxDefaultSize, wxArrayString(2, Tx_YesNo), 1, wxRA_SPECIFY_COLS);
	row3->Add(m_Monster, 0, wxALL, 4);

	buttonsizer->Add(row3, 0, 0);

	mainsizer->Add(new wxStaticLine(this), 0, wxALL, 4);
	mainsizer->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxALIGN_CENTER);

	m_Trigger->SetSelection(GET(startval,TRIGGER));
	m_Speed->SetSelection(GET(startval,SPEED));
	m_Delay->SetSelection(GET(startval,LIFTDELAY));
	m_Type->SetSelection(GET(startval,LIFTTARGET));
	m_Monster->SetSelection(GET(startval,MONSTER));

	SetSizerAndFit(mainsizer);
	Layout();
	Center();
}

//==========================================================================
//
//
//
//==========================================================================

void GenericLiftDialog::OnOK(wxCommandEvent & event)
{
	m_Special =	LIFT_BASE |
		SET(m_Trigger->GetSelection(),TRIGGER  ) |
		SET(m_Speed->GetSelection()  ,SPEED    ) |
		SET(m_Type->GetSelection()   ,LIFTDELAY ) |
		SET(m_Delay->GetSelection()  ,LIFTTARGET) |
		SET(m_Monster->GetSelection(),MONSTER)
		;

	EndModal(wxID_OK);
}

//==========================================================================
//
//
//
//==========================================================================

int GetGenericLiftType(wxWindow * parent, int initial)
{
	GenericLiftDialog dlg(parent, initial);
	if (dlg.ShowModal()==wxID_OK) return dlg.m_Special;
	return -1;
}

//==========================================================================
//
//
//
//
//
//
//==========================================================================

class GenericStairsDialog : public wxDialog
{
	wxRadioBox * m_Trigger;
	wxRadioBox * m_Speed;
	wxRadioBox * m_Step;
	wxRadioBox * m_Direction;
	wxRadioBox * m_Monster;
	wxCheckBox * m_Ignore;

	void OnOK(wxCommandEvent&);

public:

	int m_Special;

	GenericStairsDialog(wxWindow * parent, int startval);

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(GenericStairsDialog, wxDialog)
    EVT_BUTTON(wxID_OK, GenericStairsDialog::OnOK)
END_EVENT_TABLE()


//==========================================================================
//
//
//
//==========================================================================

GenericStairsDialog::GenericStairsDialog(wxWindow * parent, int startval)
	: wxDialog(parent, -1, wxString("Generic Stairs"))
{
	wxBoxSizer * mainsizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * buttonsizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer * row2 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * row3 = new wxBoxSizer(wxVERTICAL);

	mainsizer->Add(buttonsizer, 0, wxTOP|wxBOTTOM, 4);

	m_Trigger = new wxRadioBox(this, -1, "Trigger", wxDefaultPosition, wxDefaultSize, wxArrayString(8, Tx_Trigger), 1, wxRA_SPECIFY_COLS);
	buttonsizer->Add(m_Trigger, 0, wxALL, 4);

	m_Speed = new wxRadioBox(this, -1, "Speed", wxDefaultPosition, wxDefaultSize, wxArrayString(4, Tx_Speed), 1, wxRA_SPECIFY_COLS);
	row2->Add(m_Speed, 0, wxALL, 4);

	m_Step = new wxRadioBox(this, -1, "Step height", wxDefaultPosition, wxDefaultSize, wxArrayString(4, Tx_Step), 1, wxRA_SPECIFY_COLS);
	row2->Add(m_Step, 0, wxALL, 4);

	buttonsizer->Add(row2, 0, wxLEFT|wxRIGHT, 8);

	m_Direction = new wxRadioBox(this, -1, "Direction", wxDefaultPosition, wxDefaultSize, wxArrayString(2, Tx_UpDown), 1, wxRA_SPECIFY_COLS);
	row3->Add(m_Direction, 0, wxALL, 4);

	m_Monster = new wxRadioBox(this, -1, "Monsters can trigger", wxDefaultPosition, wxDefaultSize, wxArrayString(2, Tx_YesNo), 1, wxRA_SPECIFY_COLS);
	row3->Add(m_Monster, 0, wxALL, 4);

	m_Ignore = new wxCheckBox(this, -1, "Ignore textures");
	row3->Add(m_Ignore, 0, wxALL, 4);


	buttonsizer->Add(row3, 0, 0);

	mainsizer->Add(new wxStaticLine(this), 0, wxALL, 4);
	mainsizer->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxALIGN_CENTER);

	m_Trigger->SetSelection(GET(startval,TRIGGER));
	m_Speed->SetSelection(GET(startval,SPEED));
	m_Step->SetSelection(GET(startval,STAIRSTEP));
	m_Direction->SetSelection(GET(startval,STAIRDIR));
	m_Monster->SetSelection(GET(startval,MONSTER));
	m_Ignore->SetValue(!!GET(startval,STAIRTEX));

	SetSizerAndFit(mainsizer);
	Layout();
	Center();
}


//==========================================================================
//
//
//
//==========================================================================

void GenericStairsDialog::OnOK(wxCommandEvent & event)
{
	m_Special =	STAIR_BASE |
		SET(m_Trigger->GetSelection(),TRIGGER  ) |
		SET(m_Speed->GetSelection()  ,SPEED    ) |
		SET(m_Step->GetSelection()   ,STAIRSTEP ) |
		SET(m_Direction->GetSelection()  ,STAIRDIR) |
		SET(m_Monster->GetSelection(),MONSTER) |
		SET(m_Ignore->GetValue(), STAIRTEX)
		;

	EndModal(wxID_OK);
}

//==========================================================================
//
//
//
//==========================================================================

int GetGenericStairsType(wxWindow * parent, int initial)
{
	GenericStairsDialog dlg(parent, initial);
	if (dlg.ShowModal()==wxID_OK) return dlg.m_Special;
	return -1;
}
//==========================================================================
//
//
//
//
//
//
//==========================================================================

class GenericCrusherDialog : public wxDialog
{
	wxRadioBox * m_Trigger;
	wxRadioBox * m_Speed;
	wxRadioBox * m_Monster;
	wxCheckBox * m_Silent;

	void OnOK(wxCommandEvent&);

public:

	int m_Special;

	GenericCrusherDialog(wxWindow * parent, int startval);

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(GenericCrusherDialog, wxDialog)
    EVT_BUTTON(wxID_OK, GenericCrusherDialog::OnOK)
END_EVENT_TABLE()


//==========================================================================
//
//
//
//==========================================================================

GenericCrusherDialog::GenericCrusherDialog(wxWindow * parent, int startval)
	: wxDialog(parent, -1, wxString("Generic Crusher"))
{
	wxBoxSizer * mainsizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * buttonsizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer * row2 = new wxBoxSizer(wxVERTICAL);

	mainsizer->Add(buttonsizer, 0, wxTOP|wxBOTTOM, 4);

	m_Trigger = new wxRadioBox(this, -1, "Trigger", wxDefaultPosition, wxDefaultSize, wxArrayString(8, Tx_Trigger), 1, wxRA_SPECIFY_COLS);
	buttonsizer->Add(m_Trigger, 0, wxALL, 4);

	m_Speed = new wxRadioBox(this, -1, "Speed", wxDefaultPosition, wxDefaultSize, wxArrayString(4, Tx_Speed), 1, wxRA_SPECIFY_COLS);
	row2->Add(m_Speed, 0, wxALL, 4);

	m_Monster = new wxRadioBox(this, -1, "Monsters can trigger", wxDefaultPosition, wxDefaultSize, wxArrayString(2, Tx_YesNo), 1, wxRA_SPECIFY_COLS);
	row2->Add(m_Monster, 0, wxALL, 4);

	m_Silent = new wxCheckBox(this, -1, "Silent");
	row2->Add(m_Silent, 0, wxALL, 4);

	buttonsizer->Add(row2, 0, 0);

	mainsizer->Add(new wxStaticLine(this), 0, wxALL, 4);
	mainsizer->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxALIGN_CENTER);

	m_Trigger->SetSelection(GET(startval,TRIGGER));
	m_Speed->SetSelection(GET(startval,SPEED));
	m_Monster->SetSelection(GET(startval,MONSTER));
	m_Silent->SetValue(!!GET(startval,CRUSHSILENT));

	SetSizerAndFit(mainsizer);
	Layout();
	Center();
}

//==========================================================================
//
//
//
//==========================================================================

void GenericCrusherDialog::OnOK(wxCommandEvent & event)
{
	m_Special =	CRUSHER_BASE |
		SET(m_Trigger->GetSelection(),TRIGGER  ) |
		SET(m_Speed->GetSelection()  ,SPEED    ) |
		SET(m_Monster->GetSelection(),MONSTER)	 |
		SET(m_Silent->GetValue(), CRUSHSILENT)
		;

	EndModal(wxID_OK);
}

//==========================================================================
//
//
//
//==========================================================================

int GetGenericCrusherType(wxWindow * parent, int initial)
{
	GenericCrusherDialog dlg(parent, initial);
	if (dlg.ShowModal()==wxID_OK) return dlg.m_Special;
	return -1;
}


//==========================================================================
//
//
//
//
//
//
//==========================================================================

class GenericFloorDialog : public wxDialog
{
	wxRadioBox * m_Trigger;
	wxRadioBox * m_Speed;
	wxRadioBox * m_Direction;
	wxRadioBox * m_Type;
	wxRadioBox * m_Monster;
	wxRadioBox * m_Change;
	wxCheckBox * m_Crush;
	bool m_ceiling;

	void OnOK(wxCommandEvent&);
	void OnChange(wxCommandEvent&);

public:

	int m_Special;

	GenericFloorDialog(wxWindow * parent, int startval, bool ceiling);

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(GenericFloorDialog, wxDialog)
    EVT_BUTTON(wxID_OK, GenericFloorDialog::OnOK)
	EVT_RADIOBOX(GD_CHANGE, GenericFloorDialog::OnChange)
END_EVENT_TABLE()


//==========================================================================
//
//
//
//==========================================================================

GenericFloorDialog::GenericFloorDialog(wxWindow * parent, int startval, bool ceiling)
	: wxDialog(parent, -1, wxString(ceiling? "Generic Ceiling" : "Generic Floor"))
{
	wxBoxSizer * mainsizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * buttonsizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer * buttonsizer2= new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer * row2 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * row3 = new wxBoxSizer(wxVERTICAL);
	const wxChar ** Tx_type = ceiling? Tx_Ceil : Tx_Floor;

	mainsizer->Add(buttonsizer, 0, wxTOP|wxBOTTOM, 4);
	mainsizer->Add(buttonsizer2, 0, wxTOP|wxBOTTOM, 4);

	m_Trigger = new wxRadioBox(this, -1, "Trigger", wxDefaultPosition, wxDefaultSize, wxArrayString(8, Tx_Trigger), 1, wxRA_SPECIFY_COLS);
	buttonsizer->Add(m_Trigger, 0, wxALL, 4);

	m_Speed = new wxRadioBox(this, -1, "Speed", wxDefaultPosition, wxDefaultSize, wxArrayString(4, Tx_Speed), 1, wxRA_SPECIFY_COLS);
	row2->Add(m_Speed, 0, wxALL, 4);

	m_Direction = new wxRadioBox(this, -1, "Direction", wxDefaultPosition, wxDefaultSize, wxArrayString(2, Tx_UpDown), 1, wxRA_SPECIFY_COLS);
	row2->Add(m_Direction, 0, wxALL, 4);

	buttonsizer->Add(row2, 0, wxLEFT|wxRIGHT, 8);

	m_Type = new wxRadioBox(this, -1, "Type", wxDefaultPosition, wxDefaultSize, wxArrayString(8, Tx_type), 1, wxRA_SPECIFY_COLS);
	buttonsizer->Add(m_Type, 0, wxALL, 4);

	m_Change = new wxRadioBox(this, GD_CHANGE, "Change sector attributes", wxDefaultPosition, wxDefaultSize, wxArrayString(4, Tx_Change), 1, wxRA_SPECIFY_COLS);
	buttonsizer2->Add(m_Change, 0, wxALL, 4);

	m_Monster = new wxRadioBox(this, -1, "Monsters can trigger", wxDefaultPosition, wxDefaultSize, wxArrayString(2, Tx_YesNo), 1, wxRA_SPECIFY_COLS);
	row3->Add(m_Monster, 0, wxALL, 4);

	m_Crush = new wxCheckBox(this, -1, "Crusher");
	row3->Add(m_Crush, 0, wxALL, 4);

	buttonsizer2->Add(row3, 0, 0);

	mainsizer->Add(new wxStaticLine(this), 0, wxALL, 4);
	mainsizer->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxALIGN_CENTER);

	m_Trigger->SetSelection(GET(startval,TRIGGER));
	m_Speed->SetSelection(GET(startval,SPEED));
	m_Direction->SetSelection(GET(startval,DIRECTION));
	m_Type->SetSelection(GET(startval,TARGET));
	m_Monster->SetSelection(GET(startval,MODEL));
	m_Change->SetSelection(GET(startval,CHANGE));
	m_Crush->SetValue(!!GET(startval,CRUSH));

	SetSizerAndFit(mainsizer);
	Layout();
	Center();
}

//==========================================================================
//
//
//
//==========================================================================

void GenericFloorDialog::OnChange(wxCommandEvent & event)
{
	if (m_Change->GetSelection()==0)
	{
		m_Monster->SetLabel("Monsters can trigger");
		m_Monster->SetString(0, "No");
		m_Monster->SetString(1, "Yes");
	}
	else
	{
		m_Monster->SetLabel("Model");
		m_Monster->SetString(0, "Trigger");
		m_Monster->SetString(1, "Sector");
	}
}

//==========================================================================
//
//
//
//==========================================================================

void GenericFloorDialog::OnOK(wxCommandEvent & event)
{
	m_Special =	(m_ceiling? CEILING_BASE : FLOOR_BASE) |
		SET(m_Trigger->GetSelection(),TRIGGER  ) |
		SET(m_Speed->GetSelection()  ,SPEED    ) |
		SET(m_Type->GetSelection()   ,TARGET ) |
		SET(m_Direction->GetSelection()  ,DIRECTION) |
		SET(m_Monster->GetSelection(),MODEL) |
		SET(m_Change->GetSelection(),CHANGE) |
		SET(m_Crush->GetValue(),CRUSH)
		;

	EndModal(wxID_OK);
}

//==========================================================================
//
//
//
//==========================================================================

int GetGenericFloorType(wxWindow * parent, int initial)
{
	GenericFloorDialog dlg(parent, initial, false);
	if (dlg.ShowModal()==wxID_OK) return dlg.m_Special;
	return -1;
}

//==========================================================================
//
//
//
//==========================================================================

int GetGenericCeilingType(wxWindow * parent, int initial)
{
	GenericFloorDialog dlg(parent, initial, true);
	if (dlg.ShowModal()==wxID_OK) return dlg.m_Special;
	return -1;
}


int GetGenericType(wxWindow * parent, int initial, int which)
{
	switch(which)
	{
	case 0:
		if (initial<DOOR_BASE || initial>=DOOR_BASE+DOOR_COUNT) initial=0;
		return GetGenericDoorType(parent, initial);

	case 1:
		if (initial<LOCK_BASE || initial>=LOCK_BASE+LOCK_COUNT) initial=0;
		return GetGenericLockedDoorType(parent, initial);

	case 2:
		if (initial<FLOOR_BASE || initial>=FLOOR_BASE+FLOOR_COUNT) initial=0;
		return GetGenericFloorType(parent, initial);

	case 3:
		if (initial<CEILING_BASE || initial>=CEILING_BASE+CEILING_COUNT) initial=0;
		return GetGenericCeilingType(parent, initial);

	case 4:
		if (initial<LIFT_BASE || initial>=LIFT_BASE+LIFT_COUNT) initial=0;
		return GetGenericLiftType(parent, initial);

	case 5:
		if (initial<STAIR_BASE || initial>=STAIR_BASE+STAIR_COUNT) initial=0;
		return GetGenericStairsType(parent, initial);

	case 6:
		if (initial<CRUSHER_BASE || initial>=CRUSHER_BASE+CRUSHER_COUNT) initial=0;
		return GetGenericCrusherType(parent, initial);

	default:
		return -1;
	}
}