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
// CRotateInput.cpp : Implements the rotate objects dialog
//

#include "stdafx.h"
#include "ZEd.h"


//==========================================================================
//
//
//
//==========================================================================

class CRotateInput : public wxDialog
{
	wxTextCtrl * m_Angle;
	wxTextCtrl * m_Scale;

	void OnOK(wxCommandEvent&);

public:

	wxRealPoint m_result;

	CRotateInput(wxWindow * parent);
	~CRotateInput();

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(CRotateInput, wxDialog)
    EVT_BUTTON(wxID_OK, CRotateInput::OnOK)
END_EVENT_TABLE()


//==========================================================================
//
//
//
//==========================================================================

CRotateInput::CRotateInput(wxWindow* parent) : wxDialog(parent, -1, wxString("Rotate and scale objects"))
{
	wxBoxSizer * dlgbox;
	wxBoxSizer * dlgline;
	wxStaticBoxSizer * box;

	dlgbox = new wxBoxSizer(wxVERTICAL);

	wxStaticBox * frame = new wxStaticBox(this, -1, "");
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	dlgbox->Add(box, 0, wxALL, 4);


	dlgline = new wxBoxSizer(wxHORIZONTAL);
	box->Add(dlgline, 0, wxEXPAND|wxALL, 4);
	m_Angle = new wxTextCtrl(this, -1, "0", wxDefaultPosition, wxSize(56, -1));
	dlgline->Add(new wxStaticText(this, -1, "Rotation angle in °:"), 0, wxALIGN_CENTER_VERTICAL);
	dlgline->AddStretchSpacer();
	dlgline->Add(m_Angle, 0, wxEXPAND|wxALIGN_RIGHT);

	// Y
	dlgline = new wxBoxSizer(wxHORIZONTAL);
	box->Add(dlgline, 0, wxEXPAND|wxALL, 4);
	m_Scale = new wxTextCtrl(this, -1, "1", wxDefaultPosition, wxSize(56, -1));
	dlgline->Add(new wxStaticText(this, -1, "Scaling factor:"), 0, wxALIGN_CENTER_VERTICAL);
	dlgline->AddStretchSpacer();
	dlgline->Add(m_Scale, 0, wxEXPAND|wxALIGN_RIGHT);

	dlgbox->Add(CreateButtonSizer(wxOK|wxCANCEL), 1, wxALL, 4);
	SetSizerAndFit(dlgbox);
	Layout();
	Center();
	m_Angle->SetFocus();
}

CRotateInput::~CRotateInput()
{
}


//==========================================================================
//
//
//
//==========================================================================

void CRotateInput::OnOK(wxCommandEvent& WXUNUSED(event))
{
	bool res = m_Angle->GetValue().ToDouble(&m_result.x);
	res &= m_Scale->GetValue().ToDouble(&m_result.y);
	EndModal(res? wxID_OK : wxID_CANCEL);
}


//==========================================================================
//
//
//
//==========================================================================

bool GetRotation(wxWindow * parent, wxRealPoint * res)
{
	CRotateInput rr(parent);

	if (rr.ShowModal()==wxID_OK)
	{
		*res=rr.m_result;
		return true;
	}
	return false;
}
