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
// The WAD manager class
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
	ID_GAMECONFIG=100,
	ID_RECENT,
	ID_OPENLIST,
	ID_MAPLIST,

	ID_OPENWAD,
	ID_CLOSE,
	ID_CLOSEALL,
	ID_WADLIBRARY,
};


//==========================================================================
//
//
//
//==========================================================================

class CWadManager : public wxDialog
{
private:

	wxChoice	* m_ComboGameConfig;

	wxListBox	* m_ListOpenWads;
	wxListBox	* m_ListRecentWads;
	wxListBox	* m_ListMaps;

	wxButton	* m_ButtonOpen;
	wxButton	* m_ButtonClose;
	wxButton	* m_ButtonCloseAll;
	wxButton	* m_ButtonWadLibrary;
	wxButton	* m_ButtonAddMap;
	wxButton	* m_ButtonCloseDlg;

	wxArrayString m_ConfigList;

	TArray<MapRecord> m_mapRecords;

	void MakeConfigList();
	void AddRecentWAD(const char * name);
	void GetRecentWADs();
	void SaveRecentWADs();
	void SetCurrentConfig(const char * buffer);
	void AddWAD(wxString filename, bool addtorecent);

public:

	void OnChangeConfig(wxCommandEvent & event);
	void OnChangeOpenWads(wxCommandEvent & event);
	void OnChangeMaps(wxCommandEvent & event);

	void OnDblClickRecentWads(wxCommandEvent & event);
	void OnDblClickMaps(wxCommandEvent & event);

	void OnOpenWad(wxCommandEvent & event);
	void OnCloseWad(wxCommandEvent & event);
	void OnCloseAll(wxCommandEvent & event);
	void OnWadLibrary(wxCommandEvent & event);


	CWadManager(wxWindow *parent);
	~CWadManager();

	QWORD m_MapLump;	// the result

    DECLARE_EVENT_TABLE()
};



//==========================================================================
//
//
//
//==========================================================================

BEGIN_EVENT_TABLE(CWadManager, wxDialog)

	// Game combo
	EVT_CHOICE(ID_GAMECONFIG, CWadManager::OnChangeConfig)

	// Open wads list
	EVT_LISTBOX(ID_OPENLIST, CWadManager::OnChangeOpenWads)
	EVT_LISTBOX(ID_MAPLIST, CWadManager::OnChangeMaps)

	EVT_LISTBOX_DCLICK(ID_RECENT, CWadManager::OnDblClickRecentWads)
	EVT_LISTBOX_DCLICK(ID_MAPLIST, CWadManager::OnDblClickMaps)

	// Buttons
	EVT_BUTTON(ID_OPENWAD, CWadManager::OnOpenWad)
	EVT_BUTTON(ID_CLOSE, CWadManager::OnCloseWad)
	EVT_BUTTON(ID_CLOSEALL, CWadManager::OnCloseAll)
	EVT_BUTTON(ID_WADLIBRARY, CWadManager::OnWadLibrary)
END_EVENT_TABLE()


//==========================================================================
//
//
//
//==========================================================================

CWadManager::CWadManager(wxWindow *parent) 
: wxDialog(parent, -1, wxString(_T("Wad Manager")), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxCLOSE_BOX|wxRESIZE_BORDER|wxMAXIMIZE_BOX|wxSYSTEM_MENU)
{
	//wxPanel * Panel = new wxPanel(this, -1);
	wxDialog * Panel = this;

	wxBoxSizer * Sizer_Horz = new wxBoxSizer(wxHORIZONTAL);
	Panel->SetSizer(Sizer_Horz);

	wxBoxSizer * Sizer_Vert_Left = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * Sizer_Vert_Right = new wxBoxSizer(wxVERTICAL);

	// Game configuration combo box
	wxStaticBox * GameConfigFrame = new wxStaticBox(Panel, -1, "Game configuration:");
	wxStaticBoxSizer * GameConfigSizer = new wxStaticBoxSizer(GameConfigFrame, wxVERTICAL);

	MakeConfigList();
	m_ComboGameConfig = new wxChoice(Panel, ID_GAMECONFIG, wxDefaultPosition, wxDefaultSize, m_ConfigList);
	m_ComboGameConfig->SetSelection(0);

	GameConfigSizer->Add(m_ComboGameConfig, 1, wxEXPAND);

	Sizer_Vert_Left->Add(GameConfigSizer, 0, wxEXPAND|wxALL, 4);

	// Open WADs list + buttons
	wxStaticBox * OpenWadsFrame = new wxStaticBox(Panel, -1, "Open WADs:");
	wxStaticBoxSizer * OpenWadsSizer = new wxStaticBoxSizer(OpenWadsFrame, wxVERTICAL);

	Sizer_Vert_Left->Add(OpenWadsSizer, 1, wxEXPAND|wxALL, 4);

	m_ListOpenWads = new wxListBox(Panel, ID_OPENLIST);

	OpenWadsSizer->Add(m_ListOpenWads, 1, wxEXPAND);

	wxBoxSizer * ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	OpenWadsSizer->Add(ButtonSizer, 0, wxEXPAND);

	m_ButtonOpen = new wxButton(Panel, ID_OPENWAD, "Open WAD", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	ButtonSizer->Add(m_ButtonOpen, 0, wxEXPAND|wxTOP|wxRIGHT, 4);

	m_ButtonClose = new wxButton(Panel, ID_CLOSE, "Close WAD", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	ButtonSizer->Add(m_ButtonClose, 0, wxEXPAND|wxTOP|wxRIGHT, 4);

	m_ButtonCloseAll = new wxButton(Panel, ID_CLOSEALL, "Close All", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	ButtonSizer->Add(m_ButtonCloseAll, 0, wxEXPAND|wxTOP|wxRIGHT, 4);

	m_ButtonWadLibrary = new wxButton(Panel, ID_WADLIBRARY, "WAD Library", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	ButtonSizer->Add(m_ButtonWadLibrary, 0, wxEXPAND|wxTOP|wxRIGHT, 4);

	// Recent WADs list + buttons
	wxStaticBox * RecentWadsFrame = new wxStaticBox(Panel, -1, "Recent WADs:");
	wxStaticBoxSizer * RecentWadsSizer = new wxStaticBoxSizer(RecentWadsFrame, wxVERTICAL);

	Sizer_Vert_Left->Add(RecentWadsSizer, 1, wxEXPAND|wxALL, 4);

	m_ListRecentWads = new wxListBox(Panel, ID_RECENT);

	RecentWadsSizer->Add(m_ListRecentWads, 1, wxEXPAND);

	// Recent WADs list + buttons
	wxStaticBox * MapsFrame = new wxStaticBox(Panel, -1, "Maps:");
	wxStaticBoxSizer * MapsSizer = new wxStaticBoxSizer(MapsFrame, wxHORIZONTAL);

	Sizer_Vert_Right->Add(MapsSizer, 1, wxEXPAND|wxALL, 4);

	m_ListMaps = new wxListBox(Panel, ID_MAPLIST);
	m_ListMaps->SetSizeHints(80, 0, 80, -1);

	MapsSizer->Add(m_ListMaps, 1, wxEXPAND);

	m_ButtonCloseDlg = new wxButton(Panel, wxID_CANCEL, "Close WAD Manager", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	Sizer_Vert_Right->Add(m_ButtonCloseDlg, 0, wxEXPAND|wxTOP|wxRIGHT, 4);

	Sizer_Horz->Add(Sizer_Vert_Left, 1, wxEXPAND|wxALL, 4);
	Sizer_Horz->Add(Sizer_Vert_Right, 0, wxEXPAND|wxALL, 4);

	GetRecentWADs();

	for(unsigned i=0;i<CResourceFile::OpenFiles.Size();i++)
	{
		m_ListOpenWads->Append(CResourceFile::OpenFiles[i]->GetName());
	}
	m_ListOpenWads->SetSelection(CResourceFile::OpenFiles.Size()-1);
	m_ComboGameConfig->SetStringSelection(CurrentConfig);

	OnChangeOpenWads(wxCommandEvent());

	Layout();
	SetSize(600, 400);
	Center();
}


//==========================================================================
//
//
//
//==========================================================================

CWadManager::~CWadManager()
{
}


//==========================================================================
//
//
//
//==========================================================================

void CWadManager::MakeConfigList()
{
	wxDir dir;

	if (dir.Open("./configs/"))
	{
		wxString filename;

		for(bool cont = dir.GetFirst(&filename, "*.cfg"); cont = dir.GetNext(&filename); )
		{
			wxString path = "configs/" + filename;

			ScriptMan sc;

			sc.SC_OpenFile(path.c_str());
			if (sc.SC_GetString())
			{
				if (sc.SC_Compare("MAPFORMAT"))
				{
					m_ConfigList.Add(wxFileName(path).GetName());
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

void CWadManager::AddRecentWAD(const char * name)
{
	if (m_ListRecentWads->FindString(name)==wxNOT_FOUND)
	{
		if (m_ListRecentWads->GetCount()>=10)
		{
			m_ListRecentWads->Delete(9);
		}
		m_ListRecentWads->Insert(name, 0);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CWadManager::GetRecentWADs()
{
	const char *key;
	const char *value;

	if (config.SetSection("Recent"))
	{
		while (config.NextInSection (key, value))
		{
			if (stricmp (key, "File") == 0)
			{
				m_ListRecentWads->Append(value);
			}
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CWadManager::SaveRecentWADs()
{
	if (config.SetSection("Recent", true))
	{
		config.ClearCurrentSection();
		for(int i=0;i<10 && i<m_ListRecentWads->GetCount();i++)
		{
			config.SetValueForKey("File", m_ListRecentWads->GetString(i).c_str(), true);
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CWadManager::SetCurrentConfig(const char * buffer)
{
	CurrentConfig = buffer;

	config.SetSection("IWADs", true);

	const char * iwad = config.GetValueForKey(buffer);

	if (!iwad)
	{
		DirectorySaver ds("IWADSelect");

		wxFileDialog fdlg (this, "Select an IWAD", ds.Dir(), "", "WAD Files|*.wad||", 
						   wxOPEN|wxFILE_MUST_EXIST|wxCHANGE_DIR);
		if (fdlg.ShowModal()==wxID_OK)
		{
			config.SetSection("IWADs", true);
			config.SetValueForKey(buffer, fdlg.GetPath().c_str());
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CWadManager::OnChangeConfig(wxCommandEvent & event)
{
	wxString buffer;
	wxArrayString wads;

	buffer = m_ComboGameConfig->GetStringSelection();
	SetCurrentConfig(buffer.c_str());

	if (config.SetSection("IWADs"))
	{
		const char * iwad = config.GetValueForKey(CurrentConfig.c_str());
		if (iwad!=NULL && *iwad!=0) 
		{
			wads.Add(iwad);

			int count = m_ListOpenWads->GetCount();
			int sel = m_ListOpenWads->GetSelection();

			for(int i=1; i<count;i++)
			{
				wads.Add(m_ListOpenWads->GetString(i));
			}

			CResourceFile::OpenResourceFiles(wads);

			m_ListOpenWads->Clear();
			for(unsigned i=0; i<wads.GetCount(); i++)
			{
				m_ListOpenWads->Append(wads[i]);
			}
			m_ListOpenWads->SetSelection(sel);
			OnChangeOpenWads(event);
		}
	}
	confman.RefreshConfigs();
}

//==========================================================================
//
//
//
//==========================================================================

void CWadManager::OnChangeOpenWads(wxCommandEvent & event)
{
	wxString wad = m_ListOpenWads->GetStringSelection();

	m_mapRecords.Clear();
	if (wad != "")
	{
		CResourceFile::GetMapList(wad.c_str(), m_mapRecords);
	}

	m_ListMaps->Clear();

	for(unsigned i=0;i< m_mapRecords.Size();i++)
	{
		m_ListMaps->Append(m_mapRecords[i].mapname);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CWadManager::OnChangeMaps(wxCommandEvent & event)
{
}


//==========================================================================
//
//
//
//==========================================================================

void CWadManager::AddWAD(wxString filename, bool addtorecent)
{
	if (m_ListOpenWads->FindString(filename)==wxNOT_FOUND)
	{
		CResourceFile::AddResourceFile(filename.c_str());
		m_ListOpenWads->Append(filename);
		if (addtorecent) AddRecentWAD(filename.c_str());
		SaveRecentWADs();
		confman.RefreshConfigs();
		m_ListOpenWads->SetSelection(m_ListOpenWads->GetCount()-1);
		OnChangeOpenWads(wxCommandEvent());
	}
	else
	{
		wxMessageBox("The WAD is already open!");
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CWadManager::OnDblClickRecentWads(wxCommandEvent & event)
{
	wxString sel = m_ListRecentWads->GetStringSelection();
	if (sel != "") AddWAD(sel, true);
}

//==========================================================================
//
//
//
//==========================================================================

void CWadManager::OnDblClickMaps(wxCommandEvent & event)
{
	int wadindex = m_ListOpenWads->GetSelection();
	int mapindex = m_ListMaps->GetSelection();

	CResourceFile * rf = CResourceFile::GetResourceFile(wadindex);

	if (rf!=NULL && mapindex>=0)
	{
		wxString buffer = m_ListMaps->GetString(mapindex);

		int lumpindex = m_mapRecords[mapindex].maplump;

		m_MapLump = ((QWORD)(wadindex+1)<<32) + lumpindex;

		EndModal(wxID_OK);
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CWadManager::OnOpenWad(wxCommandEvent & event)
{
	DirectorySaver * ds = new DirectorySaver("OpenWAD");

	wxFileDialog fdlg(this, "Open WAD", ds->Dir(), "", 
					  "WAD Files|*.wad|PK3 Files|*.pk3|ZIP Files|*.zip|All Files|*.*||", 
					  wxOPEN|wxFILE_MUST_EXIST|wxCHANGE_DIR);


	if (fdlg.ShowModal()==wxID_OK) 
	{
		delete ds;
		AddWAD(fdlg.GetPath(), true);
	}
	else delete ds;
}

//==========================================================================
//
//
//
//==========================================================================

void CWadManager::OnCloseWad(wxCommandEvent & event)
{
	int index = m_ListOpenWads->GetSelection();

	if (index==0)
	{
		if (config.SetSection("IWADs"))
		{
			const char * iwad = config.GetValueForKey(CurrentConfig.c_str());
			if (iwad!=NULL && !stricmp(CResourceFile::OpenFiles[0]->GetName(), iwad))
			{
				wxMessageBox("You cannot close the IWAD.");
				return;
			}
		}
	}

	if (index>=0 )
	{
		wxString buffer = m_ListOpenWads->GetStringSelection();
		CResourceFile::CloseResourceFile(buffer.c_str());
		m_ListOpenWads->Delete(index);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CWadManager::OnCloseAll(wxCommandEvent & event)
{
	TArray<const char * > iwads;

	if (config.SetSection("IWADs"))
	{
		const char * iwad = config.GetValueForKey(CurrentConfig.c_str());
		if (iwad!=NULL && *iwad!=0) iwads.Push(iwad);
	}
	CResourceFile::OpenResourceFiles(iwads);
	m_ListOpenWads->Clear();
	m_ListOpenWads->Append(iwads[0]);
	confman.RefreshConfigs();
}

//==========================================================================
//
//
//
//==========================================================================

void CWadManager::OnWadLibrary(wxCommandEvent & event)
{
	wxString newwad = RunWADLibrary(this);
	if (!!newwad) AddWAD(newwad, false);
}


//==========================================================================
//
//
//
//==========================================================================

QWORD RunWADManager(wxWindow * parent)
{
	CWadManager wadman(parent);

	if (wadman.ShowModal()==wxID_OK) return wadman.m_MapLump;
	else return (QWORD)-1;
}
