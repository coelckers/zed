//
//-----------------------------------------------------------------------------
//
// Copyright (C) 1998-2005 Christoph Oelckers
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
// Texture Rename Dialog
//

#include "stdafx.h"
#include "ZEd.h"
#include "Level.h"

enum
{
	RT_TEXBUTTON1=100,
	RT_TEXBUTTON2,
	RT_ADDBUTTON,
	RT_SAVE,
	RT_LOAD,
	RT_REMOVE,
	RT_CLEAR
};

static wxArrayString texrename;
static wxArrayString thingrename;

//==========================================================================
//
//
//
//==========================================================================

class CTexRenameDlg : public wxDialog
{
	bool m_renameThings;

	wxButton * m_texbutton[2];
	wxTextCtrl * m_entertex[2];
	wxButton * m_AddButton;
	wxCheckBox * m_Place[2];

	wxListCtrl * m_List;
	wxButton * m_Buttons[4];


	wxSizer * CreateUpperBox();
	wxSizer * CreateLowerBox();
	void OnAddToList(wxCommandEvent & event);
	void OnSaveList(wxCommandEvent & event);
	void OnLoadList(wxCommandEvent & event);
	void OnRemove(wxCommandEvent & event);
	void OnClear(wxCommandEvent & event);

public:
	CTexRenameDlg(wxWindow * parent, const wxArrayString & strings, bool renamethings);
	wxListCtrl * GetList() { return m_List; }

	DECLARE_EVENT_TABLE()

	/*
	afx_msg void OnBnClickedBrowsefrom();
	afx_msg void OnBnClickedBrowseto();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	*/
};


BEGIN_EVENT_TABLE(CTexRenameDlg, wxDialog)

	//EVT_BUTTON(RT_TEXBUTTON1, CTexRenameDlg::OnBrowseTextures)
	//EVT_BUTTON(RT_TEXBUTTON2, CTexRenameDlg::OnBrowseTextures)
	EVT_BUTTON(RT_ADDBUTTON, CTexRenameDlg::OnAddToList)
	EVT_BUTTON(RT_SAVE, CTexRenameDlg::OnSaveList)
	EVT_BUTTON(RT_LOAD, CTexRenameDlg::OnLoadList)
	EVT_BUTTON(RT_REMOVE, CTexRenameDlg::OnRemove)
	EVT_BUTTON(RT_CLEAR, CTexRenameDlg::OnClear)

END_EVENT_TABLE()

//==========================================================================
//
//
//
//==========================================================================

wxSizer * CTexRenameDlg::CreateUpperBox()
{
	wxStaticBox *frame = new wxStaticBox(this, -1, "");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);
	wxBoxSizer * line = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer * row = new wxBoxSizer(wxVERTICAL);

	wxFont * Font = wxFont::New(wxSize(8,16), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false,
		"Courier New", wxFONTENCODING_SYSTEM);

	// These are created outside the main loop to get the correct tab order
	for(int i=0;i<2;i++)
	{
		m_entertex[i]=new wxTextCtrl(this, -1, "");
		m_entertex[i]->SetMaxLength(8);
		m_entertex[i]->SetFont(*Font);
	}
	delete Font;

	for(int i=0;i<2;i++)
	{
		wxBoxSizer * iline = new wxBoxSizer(wxHORIZONTAL);

		iline->Add(new wxStaticText(this, -1, wxString(i? "to":"from")), 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 4);
		iline->Add(m_entertex[i], 0, wxALL, 4);
		m_texbutton[i]=new wxButton(this, RT_TEXBUTTON1+i, "Browse...");
		iline->Add(m_texbutton[i], 0, wxALL, 4);
		row->Add(iline, 0, wxEXPAND|wxALL, 4);
	}
	line->Add(row, 0, wxEXPAND);
	m_AddButton=new wxButton(this, RT_ADDBUTTON, "Add to list");
	line->Add(m_AddButton, 0, wxALL|wxEXPAND, 4);
	box->Add(line, 0, wxALIGN_CENTER_HORIZONTAL);

	if (!m_renameThings)
	{
		line = new wxBoxSizer(wxHORIZONTAL);
		m_Place[0]=new wxCheckBox(this, -1, "as wall");
		m_Place[0]->SetValue(true);
		line->Add(m_Place[0], 0, wxALL, 4);
		m_Place[1]=new wxCheckBox(this, -1, "as flat");
		m_Place[1]->SetValue(true);
		line->Add(m_Place[1], 0, wxALL, 4);
		box->Add(line, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 4);
	}
	return box;
}

//==========================================================================
//
//
//
//==========================================================================

wxSizer * CTexRenameDlg::CreateLowerBox()
{
	wxStaticBox *frame = new wxStaticBox(this, -1, "");
	wxStaticBoxSizer *line = new wxStaticBoxSizer(frame, wxHORIZONTAL);
	wxBoxSizer * row = new wxBoxSizer(wxVERTICAL);

	m_List = new wxListCtrl(this, -1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT/*|wxLC_SINGLE_SEL*/ );
	line->Add(m_List, 1, wxALL|wxEXPAND, 4);
	m_List->SetMinSize(wxSize(234,120));

	m_List->InsertColumn(0, "", wxLIST_FORMAT_LEFT, 30);
	m_List->InsertColumn(1, "from", wxLIST_FORMAT_LEFT, 100);
	m_List->InsertColumn(2, "->to", wxLIST_FORMAT_LEFT, 100);

	for(int i=0;i<4;i++)
	{
		const char * labels[]={"Save list", "Load list", "Remove from list", "Clear list"};

		m_Buttons[i]=new wxButton(this, RT_SAVE+i, labels[i]);
		row->Add(m_Buttons[i], 0, wxEXPAND|wxALL, 8);
	}
	line->Add(row, 0, wxEXPAND);
	return line;
}


//==========================================================================
//
//
//
//==========================================================================

CTexRenameDlg::CTexRenameDlg(wxWindow * parent, const wxArrayString & strings, bool renamethings)
	: wxDialog(parent, -1, wxString(renamethings? "Replace things" : "Rename textures"))
{
	wxBoxSizer * mainsizer = new wxBoxSizer(wxVERTICAL);

	m_renameThings=renamethings;
	mainsizer->Add(CreateUpperBox(), 0, wxEXPAND|wxALL, 4);
	mainsizer->Add(CreateLowerBox(), 0, wxEXPAND|wxALL, 4);
	mainsizer->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 4);
	m_entertex[0]->SetFocus();
	m_AddButton->SetDefault();

	for(unsigned i=0;i< strings.GetCount();i+=3)
	{
		m_List->InsertItem(i/3, strings[i]);
		m_List->SetItem(i/3, 1, strings[i+1]);
		m_List->SetItem(i/3, 2, strings[i+2]);
	}

	SetSizerAndFit(mainsizer);
	Layout();
	Center();
}

//==========================================================================
//
//
//
//==========================================================================

void CTexRenameDlg::OnAddToList(wxCommandEvent & event)
{
	wxString from = m_entertex[0]->GetValue().Upper();
	int count = m_List->GetItemCount();
	int i;

	for (i=0;i<count;i++)
	{
		wxListItem li;

		li.SetColumn(1);
		li.SetId(i);
		li.SetMask(wxLIST_MASK_TEXT);
		m_List->GetItem(li);

		if (from == li.GetText())
		{
			wxString place;
			
			if (!m_renameThings) place = wxString::Format("%c%c", m_Place[0]->GetValue()? 'W':'_',m_Place[1]->GetValue()? 'F':'_');
			else place = "T";

			m_List->SetItem(i, 0, place);
			m_List->SetItem(i, 2, m_entertex[1]->GetValue().Upper());
			break;
		}
	}
	if (i==count)
	{
		wxString place;
		
		if (!m_renameThings) place = wxString::Format("%c%c", m_Place[0]->GetValue()? 'W':'_',m_Place[1]->GetValue()? 'F':'_');
		else place = "T";

		m_List->InsertItem(i, place);
		m_List->SetItem(i, 1, from);
		m_List->SetItem(i, 2, m_entertex[1]->GetValue().Upper());
	}
	m_entertex[0]->SetFocus();
	m_entertex[0]->SetSelection(0, -1);
}


//==========================================================================
//
//
//
//==========================================================================

void CTexRenameDlg::OnSaveList(wxCommandEvent & event)
{
	DirectorySaver ds(m_renameThings? "ThingRename" : "TexRename");

	wxFileDialog fdlg (this, 
						wxString(m_renameThings? "Save thing renumber list" : "Save texture rename list"), 
						ds.Dir(), "", 
						wxString(m_renameThings? "Thing renumber info|*.thr||" : "Texture rename info|*.trn||"), 
						wxSAVE|wxOVERWRITE_PROMPT|wxCHANGE_DIR);
	if (fdlg.ShowModal()==wxID_OK)
	{
		wxBusyCursor wait;

		FILE * fp=fopen(fdlg.GetPath(), "wt");
		int count = m_List->GetItemCount();

		for (int i=0;i<count;i++)
		{
			wxListItem li;

			li.SetColumn(1);
			li.SetId(i);
			li.SetMask(wxLIST_MASK_TEXT);
			m_List->GetItem(li);
			fprintf(fp, "%.8s,", li.GetText().c_str());

			li.SetColumn(2);
			li.SetId(i);
			li.SetMask(wxLIST_MASK_TEXT);
			m_List->GetItem(li);
			fprintf(fp, "%.8s,", li.GetText().c_str());

			li.SetColumn(0);
			li.SetId(i);
			li.SetMask(wxLIST_MASK_TEXT);
			m_List->GetItem(li);
			fprintf(fp, "%s\n", li.GetText().c_str());

		}
		fclose(fp);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CTexRenameDlg::OnLoadList(wxCommandEvent & event)
{
	DirectorySaver ds(m_renameThings? "ThingRename" : "TexRename");

	wxFileDialog fdlg (this, 
						wxString(m_renameThings? "Load thing renumber list" : "Load texture rename list"), 
						ds.Dir(), "", 
						wxString(m_renameThings? "Thing renumber info|*.thr||" : "Texture rename info|*.trn||"), 
						wxOPEN|wxFILE_MUST_EXIST|wxCHANGE_DIR);
	if (fdlg.ShowModal()==wxID_OK)
	{

		char readbuf[80];
		FILE * fp=fopen(fdlg.GetPath(),"rt");
		int i=0;
		m_List->DeleteAllItems();
		while (fgets(readbuf,79,fp))
		{
			CharUpper(readbuf);
			readbuf[79]=0;

			char * from = strtok(readbuf, ", \t");
			char * to = strtok(NULL, ", \t");
			char * type = strtok(NULL, ", \t\n");

			m_List->InsertItem(i, type);
			m_List->SetItem(i, 1, from);
			m_List->SetItem(i, 2, to);
			i++;
		}
		fclose(fp);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CTexRenameDlg::OnRemove(wxCommandEvent & event)
{
	int i;
	int c = m_List->GetItemCount();
	for (i=c-1;i>=0;i--)
	{
		if (m_List->GetItemState(i, wxLIST_STATE_SELECTED))
		{
			m_List->DeleteItem(i);
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CTexRenameDlg::OnClear(wxCommandEvent & event)
{
	m_List->DeleteAllItems();
}


/*
void CTexRenameDlg::OnBnClickedBrowsefrom()
{
	m_WhichTexture=0;
	m_TextureWindow = new CTextureWindow(this,this,false);

	if (m_TextureWindow->Create(NULL, "Select texture", WS_OVERLAPPEDWINDOW, CRect(0,0,640,480), NULL))
	{
		m_TextureWindow->ShowWindow(SW_SHOW);
	}
}

void CTexRenameDlg::OnBnClickedBrowseto()
{
	m_WhichTexture=1;
	m_TextureWindow = new CTextureWindow(this,this,false);

	if (m_TextureWindow->Create(NULL, "Select texture", WS_OVERLAPPEDWINDOW, CRect(0,0,640,480), NULL))
	{
		m_TextureWindow->ShowWindow(SW_SHOW);
	}
}

BOOL CTexRenameDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int cmd=LOWORD(wParam);
	const char * texname;
	CEdit * texctrl;

	if (cmd>=1999 && cmd<32768)
	{
		cmd-=2000;

		if (cmd==-1) texname="-";
		else texname = texman->GetTextureName(cmd);

		switch(m_WhichTexture)
		{
		case 0: texctrl=&m_editFrom; break;
		case 1: texctrl=&m_editTo; break;
		}
		texctrl->SetWindowText(texname);
		return true;
	}
	else return CDialog::OnCommand(wParam, lParam);
}
*/



//==========================================================================
//
//
//
//==========================================================================

void CLevel::rename_x(const wxArrayString & rename, char * buf, int type)
{
	for (size_t i = 0; i < rename.GetCount(); i+=3)
	{
		if ((type==1 && rename[i].c_str()[0]=='W') ||
			(type==2 && rename[i].c_str()[1]=='F'))
		{
			if (!strnicmp(rename[i+1].c_str(), buf, 8))
				strncpy(buf, rename[i+2].c_str(),8);
			m_changed=true;
			return;
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnEditRenametextures(wxCommandEvent & event)
{
	if (LineDrawNotAllowed()) return;

	CTexRenameDlg trd(m_DrawWindow, texrename, false);

	if (trd.ShowModal()==wxID_OK)
	{
		wxBusyCursor wait;
		wxListItem li;

		int count = trd.GetList()->GetItemCount();
		int i, j;

		for(i=0;i<count;i++)
		{
			for(j=0;j<3;j++)
			{
				li.SetColumn(j);
				li.SetId(i);
				li.SetMask(wxLIST_MASK_TEXT);
				trd.GetList()->GetItem(li);
				texrename.Add(li.GetText());
			}
		}

		for (i=0;i<NumLines();i++) for(j=0;j<2;j++)
		{
			CSideDef * psd=&GetLine(i)->sides[j];
			rename_x(texrename, psd->texUpper,1);
			rename_x(texrename, psd->texLower,1);
			rename_x(texrename, psd->texNormal,1);
		}
		for(i=0;i<NumSectors();i++)
		{
			CSector * pse=GetSector(i);
			rename_x(texrename, pse->floort,2);
			rename_x(texrename, pse->ceilt,2);
		}
		UpdateStatusBar();
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnToolsChangethingtypes(wxCommandEvent & event)
{
	if (LineDrawNotAllowed()) return;

	CTexRenameDlg trd(m_DrawWindow, thingrename, true);

	if (trd.ShowModal()==wxID_OK)
	{
		wxBusyCursor wait;
		wxListItem li;

		int count = trd.GetList()->GetItemCount();
		int i;
		unsigned j;
		TArray<int> renum;

		for(i=0;i<count;i++)
		{
			for(j=0;j<3;j++)
			{
				li.SetColumn(j);
				li.SetId(i);
				li.SetMask(wxLIST_MASK_TEXT);
				trd.GetList()->GetItem(li);
				thingrename.Add(li.GetText());
				if (j!=0) 
				{
					long v;
					li.GetText().ToLong(&v);
					renum.Push(v);
				}
			}
		}

		for (i=0;i<NumThings();i++)
		{
			CThing * t=GetThing(i);
			for(j=0;j<renum.Size();j+=2)
			{
				if (t->type==renum[j]) 
				{
					t->type=renum[j+1];
					m_changed=true;
				}
			}
		}
		m_DrawWindow->Refresh();
		UpdateStatusBar();
	}
}

