

//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2000-2005 Christoph Oelckers
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
// DisplayText.cpp : Display a text in a dialog box.
//

#include "stdafx.h"
#include "ZEd.h"


enum
{

	DT_SAVE=100,
	DT_PRINT,
};

class CDisplayText : public wxDialog
{
	wxTextCtrl * m_Text;
	wxFont * m_Font;

public:
	CDisplayText(wxWindow * parent, const char * caption, wxString text);
	~CDisplayText();
	void OnSave(wxCommandEvent & event);
#ifdef PRINTTEXT
	void OnPrint(wxCommandEvent & event);
	wxString TextToHTML();
#endif

	DECLARE_EVENT_TABLE()

};

BEGIN_EVENT_TABLE(CDisplayText, wxDialog)
	EVT_BUTTON(DT_SAVE, CDisplayText::OnSave)
#ifdef PRINTTEXT
	EVT_BUTTON(DT_PRINT, CDisplayText::OnPrint)
#endif
END_EVENT_TABLE()


CDisplayText::CDisplayText(wxWindow * parent, const char * caption, wxString text)
	: wxDialog(parent, -1, wxString(caption), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxCLOSE_BOX|wxRESIZE_BORDER|wxMAXIMIZE_BOX|wxSYSTEM_MENU)
{
	wxBoxSizer * sizer1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer * sizer2 = new wxBoxSizer(wxVERTICAL);


	wxButton * button;

	button = new wxButton(this, wxID_OK, "OK", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	sizer2->Add(button, 0, wxEXPAND|wxALL, 4);
	button = new wxButton(this, DT_SAVE, "Save", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	sizer2->Add(button, 0, wxEXPAND|wxALL, 4);
#ifdef PRINTTEXT
	button = new wxButton(this, DT_PRINT, "Print", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	sizer2->Add(button, 0, wxEXPAND|wxALL, 4);
#endif

	m_Text = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_RICH2|wxTE_NOHIDESEL|wxHSCROLL);
	sizer1->Add(m_Text, 1, wxEXPAND|wxALL, 10);
	sizer1->Add(sizer2, 0, wxEXPAND|wxALL, 10);
	SetSizerAndFit(sizer1);
	Layout();
	SetSize(800,550);
	Center();
	m_Text->SetFocus();

	m_Font = wxFont::New(wxSize(6,10), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false,
		"Courier New", wxFONTENCODING_SYSTEM);

	wxTextAttr attr;
	attr.SetFont(*m_Font);
	m_Text->SetDefaultStyle(attr);
	m_Text->AppendText(text);
	m_Text->SetInsertionPoint(0);
}

CDisplayText::~CDisplayText()
{
	delete m_Font;
}

void CDisplayText::OnSave(wxCommandEvent & event)
{
	DirectorySaver ds("SaveText");

	wxFileDialog fdlg (this, "Save text as", ds.Dir(), "", "Text Files|*.txt|all files|*.*||", 
						wxSAVE|wxOVERWRITE_PROMPT|wxCHANGE_DIR);
	if (fdlg.ShowModal()==wxID_OK)
	{
		wxBusyCursor wait;
		m_Text->SaveFile(fdlg.GetPath());
	}
}

#ifdef PRINTTEXT
wxString CDisplayText::TextToHTML()
{
    wxString htmlString;

    htmlString << "<html><head><title></title></head><body><tt><FONT SIZE=-2>";

    wxChar ch;
	wxString text = m_Text->GetValue();
    int max = (int)text.length();

    for(int pos = 0; pos < max; pos++)
    {
        ch = text[pos];

        switch(ch)
        {
            case '\n':
            {
                htmlString << "<br />";
            }
            break;

            case '<':
            {
                htmlString << "&lt;";
            }
            break;

            case '&':
            {
                htmlString << "&amp;";
            }
            break;

            case ' ':
            {
                htmlString << "&nbsp;";
            }
            break;

            default:
            {
                htmlString << ch;
            }
            break;
        }
    }

    htmlString << "</font></tt></body></html>";

    return htmlString;
}

void CDisplayText::OnPrint(wxCommandEvent & event)
{
	wxString text = TextToHTML();
	wxHtmlEasyPrinting print("ZEd", this);
	print.SetHeader(GetTitle()+ " - (@PAGENUM@/@PAGESCNT@)<hr>");
	print.PrintText(text);
}
#endif


void DisplayText(wxWindow * parent, const char * caption, wxString text)
{
	CDisplayText dt(parent, caption, text);

	dt.ShowModal();
}

