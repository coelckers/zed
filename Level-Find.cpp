//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2004-2005 Christoph Oelckers
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
// Find object implementation
//

#include "StdAfx.h"
#include "ZEd.h"
#include "GameConfig.h"
#include "Level.h"

enum
{
	FIND_None,
	FIND_WallTexture,
	FIND_SectorTexture,
	FIND_LineSpecial,
	FIND_ThingSpecial,
	FIND_SectorType,

	FIND_ThingType,
	FIND_Tid,
	FIND_LineTag,
	FIND_LineTid,
	FIND_LineID,
	FIND_SectorTag,

	FIND_Max
};

static const char * diaghead[]=
{
	NULL,
	"Search for wall textures",
	"Search for flat textures",
	"Find Line by Special",
	"Find Thing by Special",

	"Find Thing by Type",
	"Find Thing by Thing ID",
	"Find Line by Tag",
	"Find Line by Tid",
	"Find Line by Line ID",
	"Find Sector by Special",
	"Find Sector by Tag",
};

wxString WallTextureFind;
static int  WallTextureMask;
wxString FlatTextureFind;
static int  FlatTextureMask;
static int  FindIndexTab[FIND_Max];
static int ThingSpecialFind;
static int LineSpecialFind;
static int SectorSpecialFind;
static int ThingTypeFind;
static int FindLineTag;
static int FindLineTid;
static int FindLineID;
static int FindSectorTag;
static int FindTid;

static int findmode;
static int findstart;

enum
{
	FD_TEXPIX,
	FD_TEXEDIT,
	FD_CHANGESPECIAL,
	FD_SPECIALTEXT,
	FD_EDITSPECIAL,
	FD_TYPEBROWSE,

};

//==========================================================================
//
//
//
//==========================================================================

class CTextureFind : public wxDialog
{
	bool m_isSector;
	wxFont * m_Font;
	TextureBox * m_Texture;
	wxTextCtrl * m_TexName;
	wxStaticText * m_TexInfo;

	wxCheckBox * m_Check[6];

	wxSizer * CreateTextureBox();
	wxSizer * CreateWallSelectBox(int side);
	wxSizer * CreateSectorSelectBox();
	void UpdateTexture();
	void OnOK(wxCommandEvent & event);
	void OnTextureChange(wxCommandEvent & event);
	void OnTextureSelect(wxCommandEvent & event);

public:

	CTextureFind(wxWindow * parent, bool sector);
	~CTextureFind()
	{
		if (m_Font) delete m_Font;
	}

	DECLARE_EVENT_TABLE()
};


BEGIN_EVENT_TABLE(CTextureFind, wxDialog)

	EVT_BUTTON(wxID_OK, CTextureFind::OnOK)
	EVT_TEXT(FD_TEXEDIT, CTextureFind::OnTextureChange)
	EVT_BUTTON(FD_TEXPIX, CTextureFind::OnTextureSelect)

END_EVENT_TABLE()


//==========================================================================
//
//
//
//==========================================================================

wxSizer * CTextureFind::CreateTextureBox()
{
	wxStaticBox *frame = new wxStaticBox(this, -1, "");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);

	wxString texbuf = m_isSector? FlatTextureFind : WallTextureFind;
	m_Texture = new TextureBox(this, FD_TEXPIX, 128, 128);
	box->Add(m_Texture, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	m_TexInfo = new wxStaticText(this, -1, "", wxDefaultPosition, wxSize(128,-1));
	box->Add(m_TexInfo, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	m_TexName = new wxTextCtrl(this, FD_TEXEDIT);
	box->Add(m_TexName, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);

	m_TexName->SetFont(*m_Font);
	m_TexName->SetInsertionPoint(0);
	m_TexName->SetMaxLength(8);
	m_TexName->AppendText(wxString(texbuf, 8).Upper());
	UpdateTexture();

	return box;
}


//==========================================================================
//
//
//
//==========================================================================

wxSizer * CTextureFind::CreateWallSelectBox(int side)
{
	const char * names[]={"Upper texture", "Normal texture", "Lower texture"};
	wxStaticBox *frame = new wxStaticBox(this, -1, side? "Back" : "Front");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);

	for(int i=0;i<3;i++)
	{
		m_Check[i+side*3] = new wxCheckBox(this, -1, names[i]);
		box->Add(m_Check[i+side*3], 0, wxALL, 2);
		m_Check[i+side*3]->SetValue(true);
	}
	return box;
}


//==========================================================================
//
//
//
//==========================================================================

wxSizer * CTextureFind::CreateSectorSelectBox()
{
	const char * names[]={"Ceiling", "Floor"};
	wxStaticBox *frame = new wxStaticBox(this, -1, "");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);

	for(int i=0;i<2;i++)
	{
		m_Check[i] = new wxCheckBox(this, -1, names[i]);
		box->Add(m_Check[i], 0, wxALL, 2);
		m_Check[i]->SetValue(true);
	}
	return box;
}


//==========================================================================
//
//
//
//==========================================================================

void CTextureFind::UpdateTexture()
{
	wxString str = m_TexName->GetValue();
	m_Texture->SetTexture(str.c_str());
	if (m_Texture->isValidTexture())
	{
		m_TexInfo->SetLabel(wxString::Format("%d x %d, %s",
			m_Texture->GetTextureWidth(), m_Texture->GetTextureHeight(),
			TextureTypeNames[m_Texture->GetTextureType()]));
	}
	else
		m_TexInfo->SetLabel(wxEmptyString);
}

void CTextureFind::OnTextureChange(wxCommandEvent & event)
{
	UpdateTexture();
}

void CTextureFind::OnTextureSelect(wxCommandEvent & event)
{
	int texnum = SelectTexture(this, m_isSector? tx_flat : tx_texture);
	if (texnum>=0)
	{
		m_TexName->SetValue(texman->GetTextureName(texnum));
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CTextureFind::OnOK(wxCommandEvent & event)
{
	int mask=0;
	for(int i=0;i<6;i++) if (m_Check[i]->GetValue()) mask|=(1<<i);

	if (!m_isSector)
	{
		WallTextureFind = m_TexName->GetValue();
		WallTextureMask = mask;
	}
	else
	{
		FlatTextureFind = m_TexName->GetValue();
		FlatTextureMask=mask;
	}
	EndModal(wxID_OK);
}


//==========================================================================
//
//
//
//==========================================================================

CTextureFind::CTextureFind(wxWindow * parent, bool sector) 
	: wxDialog(parent, -1, wxString(diaghead[sector? FIND_SectorTexture : FIND_WallTexture]))
{
	m_Font = wxFont::New(wxSize(8,16), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false,
		"Courier New", wxFONTENCODING_SYSTEM);

	wxBoxSizer * diag = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * hbox = new wxBoxSizer(wxHORIZONTAL);

	hbox->Add(CreateTextureBox(), 0, wxALL, 4);
	if (!sector)
	{
		wxBoxSizer * vbox = new wxBoxSizer(wxVERTICAL);
		for(int i=0;i<2;i++) vbox->Add(CreateWallSelectBox(i), 0, wxALL, 4);
		hbox->Add(vbox);
	}
	else hbox->Add(CreateSectorSelectBox(), 0, wxALL, 4);

	diag->Add(hbox, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	diag->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	SetSizerAndFit(diag);
	Layout();
	Center();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnToolsFindlinedefbytexture(wxCommandEvent & event)
{
	if (LineDrawNotAllowed() || NumLines()==0) return;
	CTextureFind ft(m_DrawWindow, false);
	if (ft.ShowModal()==wxID_OK)
	{
		findmode = FIND_WallTexture;
		findstart=-1;
		OnFindRepeat(event);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnToolsFindsectorbytexture(wxCommandEvent & event)
{
	if (LineDrawNotAllowed() || NumSectors()==0) return;
	CTextureFind ft(m_DrawWindow, true);
	if (ft.ShowModal()==wxID_OK)
	{
		findmode = FIND_SectorTexture;
		findstart=-1;
		OnFindRepeat(event);
	}
}

//==========================================================================
//
//
//
//
//
//
//==========================================================================

class CSpecialFind : public wxDialog
{
	bool m_Changing;
	int m_Mode;
	bool m_isThing;

	wxButton * m_ChangeSpecial;
	wxStaticText * m_SpecialText;
	wxTextCtrl * m_Special;

	wxSizer * CreateSpecialBox();

	void ChangeSpecial(int special, bool fromedit);
	void OnEditSpecial(wxCommandEvent & event);
	void OnChangeSpecial(wxCommandEvent & event);
	void OnMenuSelect(wxCommandEvent & event);
	void OnOK(wxCommandEvent & event);

public:
	CSpecialFind(wxWindow * parent, int mode);

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(CSpecialFind, wxDialog)

	EVT_BUTTON(FD_CHANGESPECIAL, CSpecialFind::OnChangeSpecial)
	EVT_MENU_RANGE(2000, 55000, CSpecialFind::OnMenuSelect)
	EVT_TEXT(FD_EDITSPECIAL, CSpecialFind::OnEditSpecial)
	EVT_BUTTON(wxID_OK, CSpecialFind::OnOK)

END_EVENT_TABLE()

//==========================================================================
//
//
//
//==========================================================================

wxSizer * CSpecialFind::CreateSpecialBox()
{
	wxStaticBox *frame = new wxStaticBox(this, -1, "");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);

	m_SpecialText = new wxStaticText(this, FD_SPECIALTEXT, 
		m_Mode<2? "This is a very long text line to format the dialog properly but it must be longer" :
				  "This is a very long text line to format the dialog");
	box->Add(m_SpecialText, 0, wxALL, 6);

	m_ChangeSpecial = new wxButton(this, FD_CHANGESPECIAL, "Select");
	box->Add(m_ChangeSpecial, 0, wxALL, 6);

	m_Special = new wxTextCtrl(this, FD_EDITSPECIAL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	box->Add(m_Special, 0, wxALL, 6);

	// special cannot be set here because it would interfere with the dialog box formatting

	return box;
}


//==========================================================================
//
//
//
//==========================================================================

void CSpecialFind::ChangeSpecial(int special, bool fromedit)
{
	if (!fromedit) 
	{
		m_Changing=true;
		m_Special->SetValue(wxString::Format("%d", special));
		m_Changing=false;
	}
	if (m_Mode==2)
		m_SpecialText->SetLabel( cgc->GetSectorName(special) );
	else if (cgc->IsExtended)
		m_SpecialText->SetLabel( cgc->GetLineDefDescription(special, -1, NULL, 2) );
	else
		m_SpecialText->SetLabel( cgc->GetLDName(special) );
}


//==========================================================================
//
//
//
//==========================================================================

void CSpecialFind::OnEditSpecial(wxCommandEvent & event)
{
	long l;
	if (!m_Changing && m_Special->GetValue().ToLong(&l))
	{
		ChangeSpecial(l, true);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CSpecialFind::OnChangeSpecial(wxCommandEvent & event)
{
	PopupMenu(m_Mode<2? cgc->GetSpecialMenu() : cgc->GetSectorMenu() );
}

void CSpecialFind::OnMenuSelect(wxCommandEvent & event)
{
	int id = event.GetId();
	
	if (m_Mode==0 && id>=50000) id = GetGenericType(this, 0, id-50000);
	else if (m_Mode==2 && id>=2500) id+=32768-2500;
	else id-=2000;
	if (id==-1) return;
	ChangeSpecial(id, false);
}


//==========================================================================
//
//
//
//==========================================================================

void CSpecialFind::OnOK(wxCommandEvent & event)
{
	long v=0;
	m_Special->GetValue().ToLong(&v);
	if (m_Mode==0) LineSpecialFind = v;
	else if (m_Mode==1) ThingSpecialFind = v;
	else SectorSpecialFind = v;
	EndModal(wxID_OK);
}

//==========================================================================
//
//
//
//==========================================================================

CSpecialFind::CSpecialFind(wxWindow * parent, int mode) 
	: wxDialog(parent, -1, wxString(diaghead[mode==2? FIND_SectorType : mode==1? FIND_ThingSpecial : FIND_LineSpecial]))
{
	m_Mode=mode;
	m_Changing=false;

	wxBoxSizer * diag = new wxBoxSizer(wxVERTICAL);

	diag->Add(CreateSpecialBox(), 1, wxALL|wxEXPAND, 4);
	diag->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	SetSizerAndFit(diag);
	Layout();
	Center();
	ChangeSpecial(mode==2? SectorSpecialFind : mode==1? ThingSpecialFind : LineSpecialFind, false);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnToolsFindthingbyspecial(wxCommandEvent & event)
{
	if (LineDrawNotAllowed() || NumThings()==0) return;
	if (m_Extended)
	{
		CSpecialFind fd(m_DrawWindow, 1);
		if (fd.ShowModal()==wxID_OK)
		{
			findmode = FIND_ThingSpecial;
			findstart=-1;
			OnFindRepeat(event);
		}
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnToolsFindlinedefbyspecial(wxCommandEvent & event)
{
	if (LineDrawNotAllowed() || NumLines()==0) return;
	CSpecialFind fd(m_DrawWindow, 0);
	if (fd.ShowModal()==wxID_OK)
	{
		findmode = FIND_LineSpecial;
		findstart=-1;
		OnFindRepeat(event);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnToolsFindsectorbytype(wxCommandEvent & event)
{
	if (LineDrawNotAllowed() || NumSectors()==0) return;
	CSpecialFind fd(m_DrawWindow, 2);
	if (fd.ShowModal()==wxID_OK)
	{
		findmode = FIND_SectorType;
		findstart=-1;
		OnFindRepeat(event);
	}
}



//==========================================================================
//
//
//
//
//
//
//==========================================================================

class CThingFind : public wxDialog
{
	bool m_Changing;

	TextureBox * m_TypeBrowse;
	wxButton * m_ChangeType;
	wxStaticText * m_TypeText;
	wxTextCtrl * m_Type;

	wxSizer * CreateTypeBox();

	void ChangeType(int special, bool fromedit);
	void OnEditType(wxCommandEvent & event);
	void OnChangeType(wxCommandEvent & event);
	void OnMenuSelect(wxCommandEvent & event);
	void OnOK(wxCommandEvent & event);

public:
	CThingFind(wxWindow * parent);

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(CThingFind, wxDialog)

	EVT_BUTTON(FD_CHANGESPECIAL, CThingFind::OnChangeType)
	EVT_MENU_RANGE(2000, 35000, CThingFind::OnMenuSelect)
	EVT_TEXT(FD_EDITSPECIAL, CThingFind::OnEditType)
	EVT_BUTTON(wxID_OK, CThingFind::OnOK)

END_EVENT_TABLE()



//==========================================================================
//
//
//
//==========================================================================

wxSizer * CThingFind::CreateTypeBox()
{
	wxStaticBox *frame = new wxStaticBox(this, -1, "Type");
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxHORIZONTAL);
	wxBoxSizer * line = new wxBoxSizer(wxVERTICAL);

	m_TypeBrowse = new TextureBox(this, FD_TYPEBROWSE, 96, 96);
	box->Add(m_TypeBrowse, 0, wxALL, 4);

	m_TypeText = new wxStaticText(this, FD_SPECIALTEXT, "Execute Sector Action when Hitting Sector's Fake Floor");
	line->Add(m_TypeText, 0, wxALL, 6);

	m_ChangeType = new wxButton(this, FD_CHANGESPECIAL, "Change Type");
	line->Add(m_ChangeType, 0, wxALL, 6);

	m_Type = new wxTextCtrl(this, FD_EDITSPECIAL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	line->Add(m_Type, 0, wxALL, 6);

	box->Add(line, 0, wxALL, 4);

	// Thing type cannot be set here because it would interfere with the dialog box formatting

	return box;
}

//==========================================================================
//
//
//
//==========================================================================

void CThingFind::ChangeType(int newtype, bool fromedit)
{
	if (!fromedit) 
	{
		m_Changing=true;
		m_Type->SetValue(wxString::Format("%d",newtype));
		m_Changing=false;
	}
	m_TypeText->SetLabel(cgc->GetThingName(newtype));
	m_TypeBrowse->SetTexture(cgc->GetThingSprite(newtype), tx_sprite);
}

//==========================================================================
//
//
//
//==========================================================================

void CThingFind::OnChangeType(wxCommandEvent & event)
{
	PopupMenu(cgc->GetThingMenu());
}

void CThingFind::OnMenuSelect(wxCommandEvent & event)
{
	ChangeType(event.GetId()-2000, false);
}

//==========================================================================
//
//
//
//==========================================================================

void CThingFind::OnEditType(wxCommandEvent & event)
{
	long l;
	if (!m_Changing && m_Type->GetValue().ToLong(&l))
	{
		ChangeType(l, true);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CThingFind::OnOK(wxCommandEvent & event)
{
	long v=0;
	m_Type->GetValue().ToLong(&v);
	ThingTypeFind = v;
	EndModal(wxID_OK);
}


//==========================================================================
//
//
//
//==========================================================================

CThingFind::CThingFind(wxWindow * parent) 
	: wxDialog(parent, -1, wxString(diaghead[FIND_ThingType]))
{
	m_Changing=false;

	wxBoxSizer * diag = new wxBoxSizer(wxVERTICAL);

	diag->Add(CreateTypeBox(), 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 4);
	diag->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 4);
	SetSizerAndFit(diag);
	Layout();
	Center();
	ChangeType(ThingTypeFind, false);
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnToolsFindthingbytype(wxCommandEvent & event)
{
	if (LineDrawNotAllowed()) return;
	CThingFind fd(m_DrawWindow);
	if (fd.ShowModal())
	{
		findmode = FIND_ThingType;
		findstart=-1;
		OnFindRepeat(event);
	}
}



//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnToolsFindthingbytid(wxCommandEvent & event)
{
	if (LineDrawNotAllowed() || NumThings()==0) return;
	if (m_Extended)
	{
		int num = wxGetNumberFromUser("", "", diaghead[FIND_Tid], FindTid, 0, 32767, m_DrawWindow);
		if (num>=0)
		{
			findmode = FIND_Tid;
			FindTid=num;
			findstart=-1;
			OnFindRepeat(event);
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnToolsFindlinedefbytag(wxCommandEvent & event)
{
	if (LineDrawNotAllowed() || NumLines()==0) return;
	int num = wxGetNumberFromUser("", "", diaghead[FIND_LineTag], FindLineTag, 0, 32767, m_DrawWindow);
	if (num>=0)
	{
		FindLineTag=num;
		findmode = FIND_LineTag;
		findstart=-1;
		OnFindRepeat(event);
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnToolsFindlinedefbytid(wxCommandEvent & event)
{
	if (LineDrawNotAllowed() || NumLines()==0) return;
	int num = wxGetNumberFromUser("", "", diaghead[FIND_LineTid], FindLineTid, 0, 32767, m_DrawWindow);
	if (num>=0)
	{
		FindLineTid=num;
		findmode = FIND_LineTid;
		findstart=-1;
		OnFindRepeat(event);
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnToolsFindlinedefbylineid(wxCommandEvent & event)
{
	if (LineDrawNotAllowed() || NumLines()==0) return;
	int num = wxGetNumberFromUser("", "", diaghead[FIND_LineID], FindLineID, 0, 32767, m_DrawWindow);
	if (num>=0)
	{
		FindLineID=num;
		findmode = FIND_LineID;
		findstart=-1;
		OnFindRepeat(event);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnToolsFindsectorbytag(wxCommandEvent & event)
{
	if (LineDrawNotAllowed() || NumSectors()==0) return;
	int num = wxGetNumberFromUser("", "", diaghead[FIND_SectorTag], FindSectorTag, 0, 32767, m_DrawWindow);
	if (num>=0)
	{
		FindSectorTag=num;
		findmode = FIND_SectorTag;
		findstart=-1;
		OnFindRepeat(event);
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnUpdateToolsFindthingbytype(wxUpdateUIEvent & event)
{
	event.Enable(NumThings()>0 && m_Mode!=modeLineDraw);
}

void CLevel::OnUpdateToolsFindthingbytid(wxUpdateUIEvent & event)
{
	event.Enable(m_Extended && NumThings()>0 && m_Mode!=modeLineDraw);
}

void CLevel::OnUpdateToolsFindlinedefextended(wxUpdateUIEvent & event)
{
	event.Enable(m_Extended && NumLines()>0 && m_Mode!=modeLineDraw);
}

void CLevel::OnUpdateToolsFindlinedef(wxUpdateUIEvent & event)
{
	event.Enable(NumLines()>0 && m_Mode!=modeLineDraw);
}

void CLevel::OnUpdateToolsFindsector(wxUpdateUIEvent & event)
{
	event.Enable(NumSectors()>0 && m_Mode!=modeLineDraw);
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnFindRepeat(wxCommandEvent & event)
{
	if (LineDrawNotAllowed()) return;

again:
	int i;
	bool found=false;

	switch (findmode)
	{
	case FIND_WallTexture:
		//if (m_Mode==modeLineDefs && findstart!=-1) findstart=m_Selection;
		for(i=findstart+1;i<NumLines();i++)
		{
			CLine * ln = GetLine(i);
			if (ln->sides[0].sector!=-1)
			{
				if (WallTextureMask&1 && !strnicmp(WallTextureFind, ln->sides[0].texUpper, 8)) found=true;
				if (WallTextureMask&2 && !strnicmp(WallTextureFind, ln->sides[0].texNormal,8)) found=true;
				if (WallTextureMask&4 && !strnicmp(WallTextureFind, ln->sides[0].texLower, 8)) found=true;
			}
			if (ln->sides[1].sector!=-1)
			{
				if (WallTextureMask&8  && !strnicmp(WallTextureFind, ln->sides[1].texUpper, 8)) found=true;
				if (WallTextureMask&16 && !strnicmp(WallTextureFind, ln->sides[1].texNormal,8)) found=true;
				if (WallTextureMask&32 && !strnicmp(WallTextureFind, ln->sides[1].texLower, 8)) found=true;
			}
			if (found)
			{
				OnModeLinedefs();
				findstart=i;
				GoToObject(findstart);
				return;
			}
		}
		break;

	case FIND_SectorTexture:
		//if (m_Mode==modeSectors && findstart!=-1) findstart=m_Selection;
		for(i=findstart+1;i<NumSectors();i++)
		{
			CSector * sec = GetSector(i);
			if (FlatTextureMask&2 && !strnicmp(FlatTextureFind, sec->ceilt, 8)) found=true;
			if (FlatTextureMask&1 && !strnicmp(FlatTextureFind, sec->floort,8)) found=true;
			if (found)
			{
				OnModeSectors();
				findstart=i;
				GoToObject(findstart);
				return;
			}
		}
		break;

	case FIND_ThingType:
		for(i=findstart+1;i<NumThings();i++)
		{
			if (GetThing(i)->type == ThingTypeFind)
			{
				OnModeThings();
				findstart=i;
				GoToObject(findstart);
				return;
			}
		}
		break;

	case FIND_ThingSpecial:
		for(i=findstart+1;i<NumThings();i++)
		{
			if (GetThing(i)->special == ThingSpecialFind)
			{
				OnModeThings();
				findstart=i;
				GoToObject(findstart);
				return;
			}
		}
		break;

	case FIND_Tid:
		for(i=findstart+1;i<NumThings();i++)
		{
			if (GetThing(i)->thingid == FindTid)
			{
				OnModeThings();
				findstart=i;
				GoToObject(findstart);
				return;
			}
		}
		break;

	case FIND_LineSpecial:
		for(i=findstart+1;i<NumLines();i++)
		{
			if (GetLine(i)->line.type == LineSpecialFind)
			{
				OnModeLinedefs();
				findstart=i;
				GoToObject(findstart);
				return;
			}
		}
		break;

	case FIND_LineTag:
		for(i=findstart+1;i<NumLines();i++)
		{
			if (!m_Extended)
			{
				if (GetLine(i)->line.tag == FindLineTag)
				{
					OnModeLinedefs();
					findstart=i;
					GoToObject(findstart);
					return;
				}
			}
			else
			{
				CLine * ln = GetLine(i);
				int mask = cgc->GetTagArg(ln->line.type);
				for(int j=0;j<5;j++)
				{
					if (mask&(1<<j) && ln->line.args[j] == FindLineTag)
					{
						OnModeLinedefs();
						findstart=i;
						GoToObject(findstart);
						return;
					}
				}
			}
		}
		break;

	case FIND_LineTid:
		for(i=findstart+1;i<NumLines();i++)
		{
			if (m_Extended)
			{
				CLine * ln = GetLine(i);
				int mask = cgc->GetTidArg(ln->line.type);
				for(int j=0;j<5;j++)
				{
					if (mask&(1<<j) && ln->line.args[j] == FindLineTid)
					{
						OnModeLinedefs();
						findstart=i;
						GoToObject(findstart);
						return;
					}
				}
			}
		}
		break;

	case FIND_LineID:
		for(i=findstart+1;i<NumLines();i++)
		{
			if (m_Extended)
			{
				CLine * ln = GetLine(i);
				int mask = cgc->GetLineIDArg(ln->line.type);
				for(int j=0;j<5;j++)
				{
					if (mask&(1<<j) && ln->line.args[j] == FindLineID)
					{
						OnModeLinedefs();
						findstart=i;
						GoToObject(findstart);
						return;
					}
				}
			}
		}
		break;

	case FIND_SectorType:
		for(i=findstart+1;i<NumSectors();i++)
		{
			if (GetSector(i)->special == SectorSpecialFind)
			{
				OnModeSectors();
				findstart=i;
				GoToObject(findstart);
				return;
			}
		}
		break;

	case FIND_SectorTag:
		for(i=findstart+1;i<NumSectors();i++)
		{
			if (GetSector(i)->tag == FindSectorTag)
			{
				OnModeSectors();
				findstart=i;
				GoToObject(findstart);
				return;
			}
		}
		break;

	default:
		return;
	}

	// If the code reaches this point the search has failed.
	if (findstart!=-1)
	{
		findstart=-1;
		goto again;
	}
	wxMessageBox("Search failed", ZED_CAPTION);
}



//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnToolsFindthingbycurrenttid(wxCommandEvent & event)
{
	if (m_Extended && NumThings()>0 && m_Mode==modeLineDefs && m_Selection>=0)
	{
		CLine * line = GetLine(m_Selection);

		int mask = cgc->GetTidArg(line->line.type);
		for(int j=0;j<5;j++)
		{
			if (mask&(1<<j))
			{
				int num = line->line.args[j];
				if (num>0) 
				{
					findmode = FIND_Tid;
					FindTid=num;
					findstart=-1;
					OnFindRepeat(event);
				}
			}
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnToolsFindlinedefbycurrenttag(wxCommandEvent & event)
{
	if (m_Extended && NumLines()>0 && m_Mode==modeSectors && m_Selection>=0)
	{
		int num = GetSector(m_Selection)->tag;
		if (num>=0)
		{
			FindLineTag=num;
			findmode = FIND_LineTag;
			findstart=-1;
			OnFindRepeat(event);
		}
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnToolsFindlinedefbycurrenttid(wxCommandEvent & event)
{
	if (m_Extended && NumLines()>0 && m_Mode==modeThings && m_Selection>=0)
	{
		int num = GetThing(m_Selection)->thingid;

		if (num>=0)
		{
			FindLineTid=num;
			findmode = FIND_LineTid;
			findstart=-1;
			OnFindRepeat(event);
		}
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnToolsFindlinedefbycurrentlineid(wxCommandEvent & event)
{
	if (m_Extended && NumLines()>0 && m_Mode==modeLineDefs && m_Selection>=0)
	{
		CLine * line = GetLine(m_Selection);

		int mask = cgc->GetLineIDArg(line->line.type);
		for(int j=0;j<5;j++)
		{
			if (mask&(1<<j))
			{
				int num = line->line.args[j];
				if (num>0) 
				{
					FindLineID=num;
					findmode = FIND_LineID;
					findstart=-1;
					OnFindRepeat(event);
				}
			}
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnToolsFindsectorbycurrenttag(wxCommandEvent & event)
{
	int num=0;
	if (NumSectors()>0 && m_Mode==modeLineDefs && m_Selection>=0)
	{
		if (m_Extended)
		{
			CLine * line = GetLine(m_Selection);

			int mask = cgc->GetLineIDArg(line->line.type);
			for(int j=0;j<5;j++)
			{
				if (mask&(1<<j))
				{
					num = line->line.args[j];
					if (num>0) 
					{
						break;
					}
				}
			}
		}
	}
	else
	{
		num = GetLine(m_Selection)->line.tag;
	}
	if (num>=0)
	{
		FindSectorTag=num;
		findmode = FIND_SectorTag;
		findstart=-1;
		OnFindRepeat(event);
	}
}

//==========================================================================
//
//
//
//==========================================================================

int MarkTid;

void CLevel::OnToolsMarkThingsWithTag(wxCommandEvent & event)
{
	if (LineDrawNotAllowed()) return;

	int num = wxGetNumberFromUser("", "", "Mark things with tid", MarkTid, 0, 32767, m_DrawWindow);

	if (num>=0)
	{
		MarkTid = num;

		OnModeThings(event);
		for(int i=0;i<this->NumThings();i++)
		{
			CThing * th = GetThing(i);

			if (th && th->thingid == num)
			{
				Select(i, true);
			}
		}
	}
	m_DrawWindow->Refresh();
	UpdateStatusBar();
}
