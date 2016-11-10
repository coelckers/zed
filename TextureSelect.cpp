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
// Texture browser - Frame window class
//

#include "stdafx.h"
#include "ZEd.h"
#include "configfile.h"
#include "Texture.h"

enum
{
	TS_WALLTEX=100,
	TS_FLATS,
	TS_TX,
	TS_PATCHES,
	TS_SPRITES,
	TS_QUICKFIND,
	TS_LIST
};

//==========================================================================
//
//
//
//==========================================================================

class TextureSelect : public wxDialog
{
public:
	int m_result;

	TextureSelect(wxWindow * parent, tx_namespace which);

private:

	tx_namespace m_namespace;
	int m_buttonmask;

	wxToggleButton * m_switches[5];
	wxTextCtrl * m_quickfind;
	TextureList * m_texturelist;

	void SetTextures();
	void OnSwitch(wxCommandEvent & event);
	void OnOK(wxCommandEvent & event);
	void OnFindChange(wxCommandEvent & event);

	DECLARE_EVENT_TABLE()
};


//==========================================================================
//
//
//
//==========================================================================

TextureSelect::TextureSelect(wxWindow * parent, tx_namespace which)
	: wxDialog(parent, -1, wxString("Select texture"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxCLOSE_BOX|wxRESIZE_BORDER|wxMAXIMIZE_BOX|wxSYSTEM_MENU)
{
	m_namespace = which;
	ValidateTextureManager();
	if (config.SetSection("TextureMask"))
	{
		m_buttonmask=config.GetIntValueForKey(m_namespace==tx_flat ? "Flat":"Wall");
		if (m_buttonmask==0) m_buttonmask= m_namespace==tx_flat? 2:1;
	}														  

	wxBoxSizer * box = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * sizer = new wxBoxSizer(wxHORIZONTAL);

	for(int i=0;i<5;i++)
	{
		static const char * btn[]={"Wall textures", "Flats", "TX textures", "Patches", "Sprites"};
		m_switches[i]=new wxToggleButton(this, TS_WALLTEX+i, btn[i]);
		sizer->Add(m_switches[i], 0, wxLEFT|wxRIGHT, 16);
		m_switches[i]->SetValue(!!(m_buttonmask&(1<<i)));
	}
	m_quickfind=new wxTextCtrl(this, TS_QUICKFIND);
	m_quickfind->SetMaxLength(8);
	sizer->Add(m_quickfind, 0, wxLEFT|wxRIGHT, 16);
	box->Add(sizer, 0, wxTOP|wxBOTTOM, 5);

	m_texturelist = new TextureList(this, TS_LIST);
	box->Add(m_texturelist, 1, wxEXPAND);
	box->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);

	SetSizerAndFit(box);
	SetTextures();
	Layout();
}


BEGIN_EVENT_TABLE(TextureSelect, wxDialog)
	EVT_COMMAND_RANGE(TS_WALLTEX, TS_SPRITES, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, TextureSelect::OnSwitch)
	EVT_TEXT(TS_QUICKFIND, TextureSelect::OnFindChange)
	EVT_BUTTON(TS_LIST, TextureSelect::OnOK)
	EVT_BUTTON(wxID_OK, TextureSelect::OnOK)
END_EVENT_TABLE()


//==========================================================================
//
//
//
//==========================================================================

void TextureSelect::OnSwitch(wxCommandEvent & event)
{
	int button = event.GetId()-TS_WALLTEX;

	if (m_switches[button]->GetValue()) m_buttonmask|=(1<<button);
	else m_buttonmask&=~(1<<button);

	if (config.SetSection("TextureMask", true))
	{
		config.SetIntValueForKey(m_namespace==tx_flat ? "Flat":"Wall", m_buttonmask);
	}
	SetTextures();
}

//==========================================================================
//
//
//
//==========================================================================

void TextureSelect::OnFindChange(wxCommandEvent & event)
{
	m_texturelist->SetToName(m_quickfind->GetValue());
}

//==========================================================================
//
//
//
//==========================================================================

void TextureSelect::OnOK(wxCommandEvent & event)
{
	m_result = m_texturelist->GetSelection();
	EndModal(wxID_OK);
}



//==========================================================================
//
//
//
//==========================================================================

void TextureSelect::SetTextures()
{
	TArray<int> texturearray;
	int oldselect = m_texturelist->GetSelection();
	texman->CollectTextures(texturearray, m_namespace, m_buttonmask);
	m_texturelist->SetTextures(texturearray);
	m_texturelist->SetSelection(oldselect);
}

//==========================================================================
//
//
//
//==========================================================================

int SelectTexture(wxWindow * parent, tx_namespace nspace)
{
	TextureSelect sel(parent, nspace);

	if (sel.ShowModal()) return sel.m_result;
	return -1;
}
