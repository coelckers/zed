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
// CNumberInput.cpp : Implements the input numbers dialog
//

#include "stdafx.h"
#include "ZEd.h"


//==========================================================================
//
//
//
//==========================================================================

class CNumberInput : public wxDialog
{
	wxSpinCtrl * m_Ctrl[2];
	int m_minmax[2][2];

	void OnOK(wxCommandEvent&);

public:

	wxPoint m_result;

	CNumberInput(wxWindow * parent, const char * caption, const char * t1, const char * t2, int min1, int max1, int min2, int max2, wxPoint * init);
	~CNumberInput();

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(CNumberInput, wxDialog)
    EVT_BUTTON(wxID_OK, CNumberInput::OnOK)
END_EVENT_TABLE()


//==========================================================================
//
//
//
//==========================================================================

CNumberInput::CNumberInput(wxWindow * parent, const char * caption, const char * t1, const char * t2, 
						   int min1, int max1, int min2, int max2, wxPoint * init) 
: wxDialog(parent, -1, wxString(caption))
{
	wxBoxSizer * dlgbox;
	wxBoxSizer * dlgline;
	wxStaticBoxSizer * box;

	m_minmax[0][0]=min1;
	m_minmax[0][1]=max1;
	m_minmax[1][0]=min2;
	m_minmax[1][1]=max2;

	dlgbox = new wxBoxSizer(wxVERTICAL);

	wxStaticBox * frame = new wxStaticBox(this, -1, "");
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	dlgbox->Add(box, 0, wxEXPAND|wxALL, 4);


	dlgline = new wxBoxSizer(wxHORIZONTAL);
	box->Add(dlgline, 1, wxEXPAND|wxALL, 4);
	m_Ctrl[0] = new wxSpinCtrl(this, -1, "0", wxDefaultPosition, wxSize(64, -1));
	m_Ctrl[0]->SetRange(min1, max1);
	m_Ctrl[0]->SetValue(init->x);
	dlgline->Add(new wxStaticText(this, -1, wxString(t1)+": "), 0, wxEXPAND|wxALIGN_CENTER_VERTICAL);
	dlgline->AddStretchSpacer();
	dlgline->Add(m_Ctrl[0], 0, wxEXPAND|wxALIGN_RIGHT);

	dlgline = new wxBoxSizer(wxHORIZONTAL);
	box->Add(dlgline, 1, wxEXPAND|wxALL, 4);
	m_Ctrl[1] = new wxSpinCtrl(this, -1, "1", wxDefaultPosition, wxSize(64, -1));
	m_Ctrl[1]->SetRange(min2, max2);
	m_Ctrl[1]->SetValue(init->y);
	dlgline->Add(new wxStaticText(this, -1, wxString(t2)+": "), 0, wxEXPAND|wxALIGN_CENTER_VERTICAL);
	dlgline->AddStretchSpacer();
	dlgline->Add(m_Ctrl[1], 0, wxEXPAND|wxALIGN_RIGHT);

	dlgbox->Add(CreateButtonSizer(wxOK|wxCANCEL), 1, wxALL, 4);
	SetSizerAndFit(dlgbox);
	Layout();
	Center();
	m_Ctrl[0]->SetFocus();
}

CNumberInput::~CNumberInput()
{
}


//==========================================================================
//
//
//
//==========================================================================

void CNumberInput::OnOK(wxCommandEvent& WXUNUSED(event))
{
	m_result.x = m_Ctrl[0]->GetValue();
	m_result.y = m_Ctrl[1]->GetValue();
	EndModal(wxID_OK);
}


//==========================================================================
//
//
//
//==========================================================================

bool Get2Numbers(wxWindow * parent, const char * caption, const char * t1, const char * t2, int min1, int max1, int min2, int max2, wxPoint * res)
{
	CNumberInput rr(parent, caption, t1, t2, min1, max1, min2, max2, res);

	if (rr.ShowModal()==wxID_OK)
	{
		*res=rr.m_result;
		return true;
	}
	return false;
}



//==========================================================================
//
//
//
//==========================================================================

class CFloatInput : public wxDialog
{
	wxTextCtrl * m_Ctrl[2];
	double m_minmax[2][2];

	void OnOK(wxCommandEvent&);

public:

	wxRealPoint m_result;

	CFloatInput(wxWindow * parent, const char * caption, const char * t1, const char * t2, double min1, double max1, double min2, double max2, wxRealPoint * init);
	~CFloatInput();

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(CFloatInput, wxDialog)
    EVT_BUTTON(wxID_OK, CFloatInput::OnOK)
END_EVENT_TABLE()


//==========================================================================
//
//
//
//==========================================================================

CFloatInput::CFloatInput(wxWindow * parent, const char * caption, const char * t1, const char * t2, 
						   double min1, double max1, double min2, double max2, wxRealPoint * init) 
: wxDialog(parent, -1, wxString(caption))
{
	wxBoxSizer * dlgbox;
	wxBoxSizer * dlgline;
	wxStaticBoxSizer * box;

	m_minmax[0][0]=min1;
	m_minmax[0][1]=max1;
	m_minmax[1][0]=min2;
	m_minmax[1][1]=max2;

	dlgbox = new wxBoxSizer(wxVERTICAL);

	wxStaticBox * frame = new wxStaticBox(this, -1, "");
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	dlgbox->Add(box, 0, wxEXPAND|wxALL, 4);


	dlgline = new wxBoxSizer(wxHORIZONTAL);
	box->Add(dlgline, 1, wxEXPAND|wxALL, 4);
	m_Ctrl[0] = new wxTextCtrl(this, -1, "0", wxDefaultPosition, wxSize(64, -1));
	m_Ctrl[0]->SetValue(wxString::Format("%1.3f", init->x+0.0001));
	dlgline->Add(new wxStaticText(this, -1, wxString(t1)+": "), 0, wxEXPAND|wxALIGN_CENTER_VERTICAL);
	dlgline->AddStretchSpacer();
	dlgline->Add(m_Ctrl[0], 0, wxEXPAND|wxALIGN_RIGHT);

	dlgline = new wxBoxSizer(wxHORIZONTAL);
	box->Add(dlgline, 1, wxEXPAND|wxALL, 4);
	m_Ctrl[1] = new wxTextCtrl(this, -1, "1", wxDefaultPosition, wxSize(64, -1));
	m_Ctrl[1]->SetValue(wxString::Format("%1.3f", init->y+0.0001));
	dlgline->Add(new wxStaticText(this, -1, wxString(t2)+": "), 0, wxEXPAND|wxALIGN_CENTER_VERTICAL);
	dlgline->AddStretchSpacer();
	dlgline->Add(m_Ctrl[1], 0, wxEXPAND|wxALIGN_RIGHT);

	dlgbox->Add(CreateButtonSizer(wxOK|wxCANCEL), 1, wxALL, 4);
	SetSizerAndFit(dlgbox);
	Layout();
	Center();
	m_Ctrl[0]->SetFocus();
}

CFloatInput::~CFloatInput()
{
}


//==========================================================================
//
//
//
//==========================================================================

void CFloatInput::OnOK(wxCommandEvent& WXUNUSED(event))
{
	double v;

	for(int i=0;i<2;i++)
	{
		v = strtod(m_Ctrl[i]->GetValue(), NULL);
		if (v < m_minmax[i][0]) v = m_minmax[i][0];
		if (v > m_minmax[i][1]) v = m_minmax[i][1];
		if (i==0) m_result.x = v;
		else m_result.y = v;
	}
	EndModal(wxID_OK);
}


//==========================================================================
//
//
//
//==========================================================================

bool Get2Floats(wxWindow * parent, const char * caption, const char * t1, const char * t2, double min1, double max1, double min2, double max2, wxRealPoint * res)
{
	CFloatInput rr(parent, caption, t1, t2, min1, max1, min2, max2, res);

	if (rr.ShowModal()==wxID_OK)
	{
		*res=rr.m_result;
		return true;
	}
	return false;
}
