//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2005-2006 Christoph Oelckers
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
// IWADSelect.cpp : Implements dialog for IWAD selection
//

#include "stdafx.h"
#include "sc_man.h"
#include "cmdlib.h"
#include "ZEd.h"


const int WL_WADLIST=100;


//==========================================================================
//
//
//
//==========================================================================

class CIWADSelect : public wxDialog
{
	wxListCtrl	* m_GameList;

	void OnDblclkListctrl(wxListEvent & event);
	void GetWADs();

public:
	CIWADSelect(wxWindow *parent);
	~CIWADSelect();

    DECLARE_EVENT_TABLE()
};


//==========================================================================
//
//
//
//==========================================================================

BEGIN_EVENT_TABLE(CIWADSelect, wxDialog)

	EVT_LIST_ITEM_ACTIVATED(WL_WADLIST, CIWADSelect::OnDblclkListctrl)

END_EVENT_TABLE()


//==========================================================================
//
//
//
//==========================================================================

CIWADSelect::CIWADSelect(wxWindow *parent) : wxDialog(parent, -1, wxString(_T("Select IWADs")))
{
	//wxPanel * Panel = new wxPanel(this, -1);
	wxDialog * Panel=this;

	wxBoxSizer * Sizer_Vert = new wxBoxSizer(wxVERTICAL);

	Panel->SetSizer(Sizer_Vert);

	m_GameList = new wxListCtrl(Panel, WL_WADLIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL );
	Sizer_Vert->Add(m_GameList, 1, wxEXPAND|wxALL, 4);

	m_GameList->InsertColumn(0, "Game Configuration", wxLIST_FORMAT_LEFT, 250);
	m_GameList->InsertColumn(1, "Selected IWAD", wxLIST_FORMAT_LEFT, 250);
	GetWADs();

	wxBoxSizer * ButtonSizer = new wxBoxSizer(wxHORIZONTAL);

	Sizer_Vert->Add(ButtonSizer, 0, wxALIGN_CENTRE);

	wxButton * ButtonClose = new wxButton(Panel, wxID_CANCEL, "Close", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	ButtonSizer->Add(ButtonClose, 0, wxALL|wxALIGN_CENTRE, 4);

	Layout();
	SetSize(600, 400);
	Center();
}

//==========================================================================
//
//
//
//==========================================================================

CIWADSelect::~CIWADSelect()
{
}

//==========================================================================
//
//
//
//==========================================================================

void CIWADSelect::GetWADs()
{
	wxDir dir;
	int i=0;
	bool isdir = false;
	wxString fn = GetConfigDir();


	if (dir.Open(fn))
	{
		wxString filename;

		config.SetSection("IWADs", true);
		for(bool cont = dir.GetFirst(&filename, "*.cfg"); cont = dir.GetNext(&filename); )
		{
			wxString path = fn + filename;

			ScriptMan sc;

			sc.SC_OpenFile(path.c_str());
			if (sc.SC_GetString())
			{
				if (sc.SC_Compare("MAPFORMAT"))
				{
					wxString configname = wxFileName(path).GetName();
					m_GameList->InsertItem(i, configname);

					const char * iwad = config.GetValueForKey(configname.c_str());
					if (iwad!=NULL)
					{
						m_GameList->SetItem(i, 1, iwad);
					}
					i++;
				}
			}
			sc.SC_Close();
		}
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CIWADSelect::OnDblclkListctrl(wxListEvent & event)
{
	int index = m_GameList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (index>=0)
	{
		DirectorySaver ds("IWADSelect");

		wxFileDialog fdlg (this, "Select an IWAD", ds.Dir(), "", "WAD Files|*.wad||", 
							wxOPEN|wxFILE_MUST_EXIST|wxCHANGE_DIR);
		if (fdlg.ShowModal()==wxID_OK)
		{
			config.SetSection("IWADs", true);
			config.SetValueForKey(m_GameList->GetItemText(index).c_str(), fdlg.GetPath().c_str());
			m_GameList->SetItem(index, 1, fdlg.GetPath());
		}
	}
}


//==========================================================================
//
//
//
//==========================================================================

void RunIWADSelect(wxWindow * parent)
{
	CIWADSelect is(parent);
	is.ShowModal();
}