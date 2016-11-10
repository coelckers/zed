//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2005 Christoph Oelckers
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
// Wad library dialog
//

#include "stdafx.h"
#include "ZEd.h"
#include "ResourceFile.h"
#include "configfile.h"
#include "sc_man.h"
#include "GameConfig.h"



//==========================================================================
//
//
//
//==========================================================================

enum
{
	WL_WADLIST=100,
	WL_ADD,
	WL_DEL,
	WL_DELALL,
	WL_DESC,
	WL_UP,
	WL_DOWN
};


//==========================================================================
//
//
//
//==========================================================================

class CWadLibrary : public wxDialog
{
private:

	wxListCtrl	* m_WadList;

	wxButton	* m_ButtonAdd;
	wxButton	* m_ButtonDelete;
	wxButton	* m_ButtonDeleteAll;
	wxButton	* m_ButtonDescription;
	wxButton	* m_ButtonUp;
	wxButton	* m_ButtonDown;
	wxButton	* m_ButtonClose;


	void GetWADs();
	void SaveWADs();
	void OnButtonAddwad(wxCommandEvent & event);
	void OnButtonDeletewad(wxCommandEvent & event);
	void OnButtonDeleteall(wxCommandEvent & event);
	void OnButtonMovedown(wxCommandEvent & event);
	void OnButtonMoveup(wxCommandEvent & event);
	void OnDblclkListctrl(wxListEvent & event);
	void OnButtonDescription(wxCommandEvent & event);

public:

	CWadLibrary(wxWindow *parent);
	~CWadLibrary();

	wxString	m_result;

    DECLARE_EVENT_TABLE()

};

//==========================================================================
//
//
//
//==========================================================================

BEGIN_EVENT_TABLE(CWadLibrary, wxDialog)

	EVT_BUTTON(WL_ADD, CWadLibrary::OnButtonAddwad)
	EVT_BUTTON(WL_DEL, CWadLibrary::OnButtonDeletewad)
	EVT_BUTTON(WL_DELALL, CWadLibrary::OnButtonDeleteall)
	EVT_BUTTON(WL_UP, CWadLibrary::OnButtonMoveup)
	EVT_BUTTON(WL_DOWN, CWadLibrary::OnButtonMovedown)
	EVT_BUTTON(WL_DESC, CWadLibrary::OnButtonDescription)

	EVT_LIST_ITEM_ACTIVATED(WL_WADLIST, CWadLibrary::OnDblclkListctrl)

END_EVENT_TABLE()


//==========================================================================
//
//
//
//==========================================================================

CWadLibrary::CWadLibrary(wxWindow *parent) 
	: wxDialog(parent, -1, wxString(_T("Wad Library")), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxCLOSE_BOX|wxRESIZE_BORDER|wxMAXIMIZE_BOX|wxSYSTEM_MENU)
{
	//wxPanel * Panel = new wxPanel(this, -1);
	wxDialog * Panel = this;

	wxBoxSizer * Sizer_Horz = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer * Sizer_Vert = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * Sizer_Vert_Right = new wxBoxSizer(wxVERTICAL);

	Panel->SetSizer(Sizer_Vert);
	Sizer_Vert->Add(Sizer_Horz, 1, wxEXPAND);

	m_WadList = new wxListCtrl(Panel, WL_WADLIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL );
	Sizer_Horz->Add(m_WadList, 1, wxEXPAND|wxALL, 4);

	m_WadList->InsertColumn(0, "File Name", wxLIST_FORMAT_LEFT, 250);
	m_WadList->InsertColumn(1, "Description", wxLIST_FORMAT_LEFT, 250);
	GetWADs();

	Sizer_Horz->Add(Sizer_Vert_Right, 0, wxALL|wxALIGN_BOTTOM|wxALIGN_RIGHT, 4);

	wxBoxSizer * ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer * ButtonSizer1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer * ButtonSizer2 = new wxBoxSizer(wxHORIZONTAL);

	Sizer_Vert->Add(ButtonSizer, 0, 0);
	ButtonSizer->Add(ButtonSizer1, 0, 0);
	ButtonSizer->Add(ButtonSizer2, 0, wxALIGN_RIGHT);

	m_ButtonAdd = new wxButton(Panel, WL_ADD, "Add WAD", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	ButtonSizer1->Add(m_ButtonAdd, 0, wxALL, 4);

	m_ButtonDelete = new wxButton(Panel, WL_DEL, "Delete WAD", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	ButtonSizer1->Add(m_ButtonDelete, 0, wxALL, 4);

	m_ButtonDeleteAll = new wxButton(Panel, WL_DELALL, "Delete All", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	ButtonSizer1->Add(m_ButtonDeleteAll, 0, wxALL, 4);

	m_ButtonDescription = new wxButton(Panel, WL_DESC, "Add Description", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	ButtonSizer1->Add(m_ButtonDescription, 0, wxALL, 4);

	m_ButtonClose = new wxButton(Panel, wxID_CANCEL, "Close", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	ButtonSizer2->Add(m_ButtonClose, 0, wxALL, 4);

	m_ButtonUp = new wxButton(Panel, WL_UP, "Up", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	Sizer_Vert_Right->Add(m_ButtonUp, 0, wxEXPAND|wxALL, 4);

	m_ButtonDown = new wxButton(Panel, WL_DOWN, "Down", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	Sizer_Vert_Right->Add(m_ButtonDown, 0, wxEXPAND|wxALL, 4);

	Layout();
	SetSize(600, 400);
	Center();
}


//==========================================================================
//
//
//
//==========================================================================

CWadLibrary::~CWadLibrary()
{
}

//==========================================================================
//
//
//
//==========================================================================

void CWadLibrary::GetWADs()
{
	const char *key;
	const char *value;
	int i=0;

	if (config.SetSection("WadLibrary"))
	{
		while (config.NextInSection (key, value))
		{
			if (stricmp (key, "File") == 0)
			{
				m_WadList->InsertItem(i++, value);
			}
		}
	}

	if (config.SetSection("WADDescriptions", false))
	{
		int itemcount = m_WadList->GetItemCount();
		for(int i=0;i<itemcount;i++)
		{
			wxString item = m_WadList->GetItemText(i);
			const char * descr = config.GetValueForKey(item.c_str());
			m_WadList->SetItem(i, 1, descr);
		}
	}

	m_WadList->SetItemState(0, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED,
							   wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);

}

//==========================================================================
//
//
//
//==========================================================================

void CWadLibrary::SaveWADs()
{
	if (config.SetSection("WadLibrary", true))
	{
		config.ClearCurrentSection();
		int itemcount = m_WadList->GetItemCount();
		for(int i=0;i<itemcount;i++)
		{
			wxString buffer = m_WadList->GetItemText(i);
			config.SetValueForKey("File", buffer.c_str(), true);
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CWadLibrary::OnButtonAddwad(wxCommandEvent & event)
{
	DirectorySaver ds("OpenWADLibrary");

	wxFileDialog fdlg(this, "Add WAD to library", ds.Dir(), "", 
					  "WAD Files|*.wad|PK3 Files|*.pk3|ZIP Files|*.zip|All Files|*.*||", 
					  wxOPEN|wxFILE_MUST_EXIST|wxCHANGE_DIR|wxMULTIPLE);


	if (fdlg.ShowModal()==wxID_OK)
	{
		wxArrayString paths;
	
		fdlg.GetPaths(paths);

		for(size_t i=0;i<paths.GetCount();i++)
		{
			if (m_WadList->FindItem(-1, paths[i])<0)
			{
				m_WadList->InsertItem(m_WadList->GetItemCount(), paths[i]);
			}
			SaveWADs();
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CWadLibrary::OnButtonDeletewad(wxCommandEvent & event)
{
	int index = m_WadList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (index>=0) m_WadList->DeleteItem(index);
	SaveWADs();
}

//==========================================================================
//
//
//
//==========================================================================

void CWadLibrary::OnButtonDeleteall(wxCommandEvent & event)
{
	m_WadList->DeleteAllItems();
	SaveWADs();
}


//==========================================================================
//
//
//
//==========================================================================

void CWadLibrary::OnButtonMoveup(wxCommandEvent & event)
{
	int index = m_WadList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (index>0)
	{
		wxString label = m_WadList->GetItemText(index);
		m_WadList->DeleteItem(index);
		m_WadList->InsertItem(index-1, label);
		if (config.SetSection("WADDescriptions", false))
		{
			const char * descr = config.GetValueForKey(label.c_str());
			m_WadList->SetItem(index-1, 1, descr);
		}
		m_WadList->SetItemState(index-1, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED,
										 wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
		SaveWADs();
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CWadLibrary::OnButtonMovedown(wxCommandEvent & event)
{
	int index = m_WadList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (index>=0 && index<m_WadList->GetItemCount()-1)
	{
		wxString label = m_WadList->GetItemText(index);
		m_WadList->DeleteItem(index);
		m_WadList->InsertItem(index+1, label);
		if (config.SetSection("WADDescriptions", false))
		{
			const char * descr = config.GetValueForKey(label.c_str());
			m_WadList->SetItem(index+1, 1, descr);
		}
		m_WadList->SetItemState(index+1, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED,
										 wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
		SaveWADs();
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CWadLibrary::OnDblclkListctrl(wxListEvent & event)
{
	int index = m_WadList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (index>=0)
	{
		m_result = m_WadList->GetItemText(index);
		EndModal(wxID_OK);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CWadLibrary::OnButtonDescription(wxCommandEvent & event)
{
	int index = m_WadList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	if (index>=0)
	{
		wxListItem li;

		li.SetColumn(1);
		li.SetId(index);
		li.SetMask(wxLIST_MASK_TEXT);
		m_WadList->GetItem(li);

		wxString newdesc = wxGetTextFromUser("", "Enter WAD description", li.GetText(), this);

		if (!!newdesc)
		{
			m_WadList->SetItem(index, 1, newdesc);

			if (config.SetSection("WADDescriptions", true))
			{
				wxString label = m_WadList->GetItemText(index);
				config.SetValueForKey(label.c_str(), newdesc.c_str());
			}
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

wxString RunWADLibrary(wxWindow * parent)
{
	CWadLibrary wl(parent);
	wl.ShowModal();
	return wl.m_result;
}
