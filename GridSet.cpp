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
// CGridSet.cpp : Implements the input grid dialog
//

#include "stdafx.h"
#include "ZEd.h"


//==========================================================================
//
//
//
//==========================================================================

class CGridSet : public wxDialog
{
	wxSpinCtrl * m_Ctrl[4];

	void OnOK(wxCommandEvent&);
	void AddControl(const char * text, wxStaticBoxSizer * box, int num, int min, int max, int set);

public:

	wxPoint m_result[2];

	CGridSet(wxWindow * parent);
	~CGridSet();

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(CGridSet, wxDialog)
    EVT_BUTTON(wxID_OK, CGridSet::OnOK)
END_EVENT_TABLE()


//==========================================================================
//
//
//
//==========================================================================

void CGridSet::AddControl(const char * text, wxStaticBoxSizer * box, int num, int min, int max, int set)
{
	wxBoxSizer * dlgline = new wxBoxSizer(wxHORIZONTAL);
	box->Add(dlgline, 1, wxEXPAND|wxALL, 4);
	m_Ctrl[num] = new wxSpinCtrl(this, -1, wxString::Format("%d", set), wxDefaultPosition, wxSize(56, -1));
	m_Ctrl[num]->SetRange(min, max);
	dlgline->Add(new wxStaticText(this, -1, wxString(text)), 0, wxEXPAND|wxALIGN_CENTER_VERTICAL);
	dlgline->AddStretchSpacer();
	dlgline->Add(m_Ctrl[num], 0, wxEXPAND|wxALIGN_RIGHT);
}

//==========================================================================
//
//
//
//==========================================================================

CGridSet::CGridSet(wxWindow * parent) 
	: wxDialog(parent, -1, wxString("Set Grid"))
{
	wxBoxSizer * dlgbox;
	wxStaticBoxSizer * box;
	wxStaticBox * frame;

	dlgbox = new wxBoxSizer(wxVERTICAL);

	frame = new wxStaticBox(this, -1, "");
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	dlgbox->Add(box, 0, wxEXPAND|wxALL, 4);

	AddControl("Grid X size: ", box, 0, 2, 256, 128);
	AddControl("Grid X offset: ", box, 1, -256, 256, 0);

	frame = new wxStaticBox(this, -1, "");
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	dlgbox->Add(box, 0, wxEXPAND|wxALL, 4);

	AddControl("Grid Y size: ", box, 2, 2, 256, 128);
	AddControl("Grid Y offset: ", box, 3, -256, 256, 0);

	dlgbox->Add(CreateButtonSizer(wxOK|wxCANCEL), 1, wxALL, 4);
	SetSizerAndFit(dlgbox);
	Layout();
	Center();
	m_Ctrl[0]->SetFocus();
}

CGridSet::~CGridSet()
{
}


//==========================================================================
//
//
//
//==========================================================================

void CGridSet::OnOK(wxCommandEvent& WXUNUSED(event))
{
	m_result[0].x = m_Ctrl[0]->GetValue();
	m_result[0].y = m_Ctrl[2]->GetValue();
	m_result[1].x = m_Ctrl[1]->GetValue();
	m_result[2].y = m_Ctrl[3]->GetValue();
	EndModal(wxID_OK);
}


//==========================================================================
//
//
//
//==========================================================================

bool GetGrid(wxWindow * parent, wxPoint * res)
{
	CGridSet rr(parent);

	if (rr.ShowModal()==wxID_OK)
	{
		res[0]=rr.m_result[0];
		res[1]=rr.m_result[1];
		return true;
	}
	return false;
}
