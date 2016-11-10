//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2006 Christoph Oelckers
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
// CurveDialog.cpp : implementation file
//
// This file implements the Curve Linedef function from DoomBuilder
// The Curving algorithm was taken directly from the DoomBuilder
// source code.

#include "stdafx.h"
#include "doomtype.h"
#include "level.h"


//==========================================================================
//
//
//
//==========================================================================

class CCurveDialog : public wxDialog
{
	CLevel * m_level;

	enum
	{
		SPIN_Vert,
		SPIN_Dist,
		SPIN_Angle
	};

	wxSpinCtrl * m_spins[3];
	wxCheckBox * m_segment;
	bool m_initializing;

	void ApplySettings();
	void OnEditChange(wxCommandEvent & event);
	void OnOK(wxCommandEvent & event);
	void OnCancel(wxCommandEvent & event);

public:
	CCurveDialog(wxWindow * parent, CLevel * lev);

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(CCurveDialog, wxDialog)

	EVT_TEXT(wxID_ANY, CCurveDialog::OnEditChange)
	EVT_CHECKBOX(wxID_ANY, CCurveDialog::OnEditChange)
	EVT_BUTTON(wxID_OK, CCurveDialog::OnOK)
	EVT_BUTTON(wxID_CANCEL, CCurveDialog::OnCancel)

END_EVENT_TABLE()


//==========================================================================
//
//
//
//==========================================================================

CCurveDialog::CCurveDialog(wxWindow * parent, CLevel * lev)
	: wxDialog(parent, -1, wxString("Curve Linedefs"))
{
	wxBoxSizer * sizer = new wxBoxSizer(wxVERTICAL);
	wxStaticBox *frame = new wxStaticBox(this, -1, "");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);

	m_initializing=true;
	m_level=lev;
	m_level->MakeBackup("Curve linedefs", true, true, false, false);
	for(int i=0;i<3;i++)
	{
		static const char * spintitles[]={ "Number of vertices", "Curve distance", "Delta Angle"};
		static int minmax[]={1,99, -9999, 9999, -360, 360};
		static int spininit[]={  8, 20, 180 };

		wxBoxSizer * line = new wxBoxSizer(wxHORIZONTAL);
		m_spins[i]=new wxSpinCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxSize(66,-1));
		line->Add(new wxStaticText(this, -1, spintitles[i]), 1, wxEXPAND|wxALL|wxALIGN_CENTER_VERTICAL, 4);
		line->Add(m_spins[i], 0, wxALL, 4);
		m_spins[i]->SetRange(minmax[i*2], minmax[i*2+1]);
		m_spins[i]->SetValue(spininit[i]);
		box->Add(line, 0, wxEXPAND);
	}
	m_segment = new wxCheckBox(this, -1, "Force circular segment");
	box->Add(m_segment, 0, wxALL, 4);
	sizer->Add(box, 0, wxALL, 4);
	sizer->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);


	SetSizerAndFit(sizer);
	Layout();
	Center();
	m_initializing=false;
	ApplySettings();
}


//==========================================================================
//
//
//
//==========================================================================

void CCurveDialog::ApplySettings()
{
	if (!m_initializing)
	{
		int verts = m_spins[SPIN_Vert]->GetValue();
		int dist  = m_spins[SPIN_Dist]->GetValue();
		int angle = m_spins[SPIN_Angle]->GetValue();
		bool circle = m_segment->GetValue();
		m_level->Revert();
		m_level->SetCurve(verts, dist, angle, circle);
	}
}

void CCurveDialog::OnEditChange(wxCommandEvent & event)
{
	ApplySettings();
}

void CCurveDialog::OnOK(wxCommandEvent & event)
{
	ApplySettings();
	EndModal(wxID_OK);
}

void CCurveDialog::OnCancel(wxCommandEvent & event)
{
	m_level->Revert();
	m_level->RemoveBackup();
	EndModal(wxID_CANCEL);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::SetCurve(int numverts, float distance, int angle, bool circle)
{
	int oldlinecount=m_temp1;

	if (numverts<1) numverts=1;
	if (numverts>99) numverts=99;
	if (distance<-9999) distance=-9999;
	if (distance>9999) distance=9999;
	if (angle<-360) angle=-360;
	if (angle>360) angle=360;

	Selection.Resize(m_temp2);
	checked.Resize(oldlinecount);
	if (distance!=0 && angle!=0) for(int i=0;i<m_temp2;i++)
	{
		CLine * ln = GetLine(Selection[i]);

		double x1 = StVt(ln)->X();
		double y1 = StVt(ln)->Y();
		double x2 = EnVt(ln)->X();
		double y2 = EnVt(ln)->Y();

		double dx = x2-x1;
		double dy = y2-y1;

		double lineangle = atan2(dx, dy);

		double fangle = angle * 3.14159265358979323846/180;
		double c = sqrt(dx*dx+dy*dy);
		double d = (c / tan(fangle/2.0))/2;
		double r = d / cos(fangle/2.0);
		double h = r-d;

		if (circle) distance = distance<0? -h:h;


		CLine * newline = ln;

		for(int v=1;v<=numverts;v++)
		{
			double a = (v * fangle / (numverts + 1)) + (3.14159265358979323846 - fangle) / 2;

			double fx = cos(a) * r;
			double fy = (sin(a) * r - d) * distance / h;

			double fa = atan2(fx, fy) + lineangle;
			double fd = sqrt(fx * fx + fy * fy);

			double vx = (x1 + dx * 0.5 + cos(fa) * fd);
			double vy = (y1 + dy * 0.5 - sin(fa) * fd);

			// No grid snapping here. It would defeat the purpose of this function!
			int v_index = InsertVertex(vx, vy, true);
			int l_index = InsertLineDef(newline);
			CLine * nln=GetLine(l_index);
			newline->line.End = nln->line.Start = v_index;
			newline = nln;
			Selection.Push(l_index);
			checked[l_index]=true;
		}
	}
	m_DrawWindow->Refresh();
	m_DrawWindow->Update();
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCurvelinedefs(wxCommandEvent & event)
{
	if (Selection.Size()==0) 
	{
		if (m_Selection==-1) return;
		Select(m_Selection, true);
	}
	m_temp1=NumLines();
	m_temp2=Selection.Size();
	if (m_Mode==modeLineDefs)
	{
		CCurveDialog cd(m_DrawWindow, this);
		cd.ShowModal();
		m_DrawWindow->Refresh();
		UpdateStatusBar();
	}
}

void CLevel::OnUpdateCurvelinedefs(wxUpdateUIEvent & event)
{
	event.Enable(m_Mode==modeLineDefs && NumLines()>0);
}
