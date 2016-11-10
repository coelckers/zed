//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2002-2005 Christoph Oelckers
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
// Editor configuration file parser
//
//
#include "stdafx.h"
#include "ZEd.h"
#include "sc_man.h"
#include "tarray.h"
#include "GameConfig.h"
#include "GenericTriggers.h"
#include "doomerrors.h"


class SpecialArgDialog : public wxDialog
{
	wxRadioBox * m_value;
	TArray<wxCheckBox*> m_flags;
	ArgType * m_what;

public:

	int m_Result;

	SpecialArgDialog(wxWindow * parent, int value, ArgType * what);
	void OnOK(wxCommandEvent & event);

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(SpecialArgDialog, wxDialog)
	EVT_BUTTON(wxID_OK, SpecialArgDialog::OnOK)
END_EVENT_TABLE()


SpecialArgDialog::SpecialArgDialog(wxWindow * parent, int value, ArgType * what)
: wxDialog(parent, -1, wxString("Set Argument"))
{
	wxBoxSizer * sizer = new wxBoxSizer(wxVERTICAL);

	m_what=what;
	if (what->values.GetCount()>0)
	{
		m_value = new wxRadioBox(this, -1, "", wxDefaultPosition, wxDefaultSize, what->values,
			(int)(what->values.GetCount()>10 ? (what->values.GetCount()+1)/2 : what->values.GetCount()), wxRA_SPECIFY_ROWS);
		sizer->Add(m_value, 0, wxALL|wxEXPAND, 4);
	}
	else m_value=NULL;

	if (what->flags.GetCount()>0)
	{
		wxStaticBox * frame = new wxStaticBox(this, -1, "Flags");
		wxStaticBoxSizer * box = new wxStaticBoxSizer(frame, wxVERTICAL);
		for(unsigned i=0;i<what->flags.GetCount();i++)
		{
			wxCheckBox * check = new wxCheckBox(this, -1, what->flags[i]);
			box->Add(check, 0, wxALL, 4);
			m_flags.Push(check);
			check->SetValue(!!(value & what->flag_values[i]));
			value &= ~what->flag_values[i];
		}
		sizer->Add(box, 0, wxALL|wxEXPAND, 4);
	}
	for(unsigned i=0;i<what->value_values.Size();i++)
	{
		if (value==what->value_values[i])
		{
			m_value->SetSelection(i);
			break;
		}
	}
	sizer->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	SetSizerAndFit(sizer);
	Layout();
	Center();
}


void SpecialArgDialog::OnOK(wxCommandEvent & event)
{
	int value = m_value? m_what->value_values[m_value->GetSelection()] : 0;

	for(unsigned i=0;i<m_what->flag_values.Size();i++)
	{
		if (m_flags[i]->GetValue()) value |= m_what->flag_values[i];
	}
	m_Result=value;
	EndModal(wxID_OK);
}





int GetSpecialArg(wxWindow * parent, int value, const wxString & name)
{
	for(unsigned i=0;i<cgc->ArgTypes.Size();i++)
	{
		if (!name.CmpNoCase(cgc->ArgTypes[i]->name))
		{
			SpecialArgDialog dlg(parent, value, cgc->ArgTypes[i]);
			if (dlg.ShowModal()==wxID_OK) return dlg.m_Result;
			else return value;
		}
	}
	// Handle special default dialogs here
	return value;
}