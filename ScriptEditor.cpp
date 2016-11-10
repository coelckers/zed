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
// Implements the script editor
// This uses the Scintilla editing control to handle syntax highlighting
// and some other neat things.
//

#include "stdafx.h"
#include "ZEd.h"
#include "Level.h"
#include "sc_man.h"
#include "images.h"

BEGIN_EVENT_TABLE(CScriptEdit, wxFrame)
	EVT_CLOSE(CScriptEdit::OnClose)
	EVT_MENU(CMD_NEW , CScriptEdit::OnFileNew)
	EVT_MENU(CMD_OPEN , CScriptEdit::OnFileOpen)
	EVT_MENU(CMD_SAVE , CScriptEdit::OnFileSave)
	EVT_UPDATE_UI(CMD_SAVE , CScriptEdit::OnUpdateFileSave)
	EVT_MENU(CMD_SAVEAS , CScriptEdit::OnFileSaveAs)
	EVT_MENU(CMD_COMPILE , CScriptEdit::OnEditCompile)
	EVT_MENU(CMD_RELOAD , CScriptEdit::OnFileReload)
	EVT_MENU(CMD_TAB , CScriptEdit::OnEditSettabstops)
	EVT_MENU(CMD_UNDO , CScriptEdit::OnEditUndo)
	EVT_MENU(CMD_REDO , CScriptEdit::OnEditRedo)
	EVT_MENU(CMD_CUT , CScriptEdit::OnEditCut)
	EVT_MENU(CMD_COPY , CScriptEdit::OnEditCopy)
	EVT_MENU(CMD_PASTE , CScriptEdit::OnEditPaste)
	EVT_MENU(CMD_CLEAR , CScriptEdit::OnEditClear)
	EVT_MENU(CMD_FIND , CScriptEdit::OnEditFind)
	EVT_MENU(CMD_FINDNEXT , CScriptEdit::OnEditRepeat)
	EVT_MENU(CMD_REPLACE , CScriptEdit::OnEditReplace)
	EVT_MENU(CMD_SELECTALL , CScriptEdit::OnEditSelectAll)
	EVT_UPDATE_UI(CMD_UNDO , CScriptEdit::OnUpdateEditUndo)
	EVT_UPDATE_UI(CMD_REDO , CScriptEdit::OnUpdateEditRedo)
	EVT_UPDATE_UI(CMD_CUT , CScriptEdit::OnUpdateEditCut)
	EVT_UPDATE_UI(CMD_PASTE , CScriptEdit::OnUpdateEditPaste)
    EVT_FIND(wxID_ANY, CScriptEdit::OnFindDialogFind)
    EVT_FIND_NEXT(wxID_ANY, CScriptEdit::OnFindDialogFind)
    EVT_FIND_REPLACE(wxID_ANY, CScriptEdit::OnFindDialogReplace)
    EVT_FIND_REPLACE_ALL(wxID_ANY, CScriptEdit::OnFindDialogReplaceAll)
    EVT_FIND_CLOSE(wxID_ANY, CScriptEdit::OnFindDialogClose)
	EVT_MENU(CMD_PRINT , CScriptEdit::OnPrint)

END_EVENT_TABLE()


//==========================================================================
//
//
//
//==========================================================================

void CScriptEdit::SetKeywords(const char * filename)
{
	ScriptMan sc;
	wxString buildbuffer;

	sc.SC_OpenFile(filename);
	while (sc.SC_GetString())
	{
		int list;
		if (sc.SC_Compare("reserved")) list=0;
		else if (sc.SC_Compare("identifier")) list=1;
		else if (sc.SC_Compare("constant")) list=3;
		else list=-1;
		buildbuffer="";
		sc.SC_MustGetStringName("{");
		while (!sc.SC_CheckString("}"))
		{
			sc.SC_MustGetString();
			strlwr(sc.sc_String);
			buildbuffer += sc.sc_String;
			buildbuffer += " ";
		}
		if (list!=-1) m_view->SetKeyWords(list, buildbuffer);
	}
	sc.SC_Close();
}

//==========================================================================
//
//
//
//==========================================================================
static char * names[]={"ACS editor", "FraggleScript editor", "Conversation editor"};

CScriptEdit::CScriptEdit(wxWindow * parent, CLevel * level, int scripttype) 
	: wxFrame(parent, -1, names[scripttype], wxDefaultPosition, wxDefaultSize, 
				(wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR) & ~wxMINIMIZE_BOX)
{
	SetIcon(wxIcon("DOC",  wxBITMAP_TYPE_ICO_RESOURCE, -1, -1));


	m_view = new wxStyledTextCtrl(this, -1);
	m_view->SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
	SetExtraStyle(wxWS_EX_BLOCK_EVENTS);

	m_type = scripttype;
	m_Level = level;
	m_find=NULL;
	m_finddata.SetFlags(wxFR_DOWN);

	switch (m_type)
	{
	case Script_ACS:
		m_view->SetLexer(wxSTC_LEX_CPPNOCASE);
		SetKeywords("acs.kw");
		break;

	case Script_FS:
		m_view->SetLexer(wxSTC_LEX_CPP);
		SetKeywords("fs.kw");
		break;

	case Script_Conv:
		m_view->SetLexer(wxSTC_LEX_CPPNOCASE);
		SetKeywords("conv.kw");
		break;

	}

	// Todo: Make colors configurable!
	m_view->StyleSetFontAttr(wxSTC_STYLE_DEFAULT, 10, "Courier New", false, false, false);
	m_view->SetMarginWidth(0, 5*m_view->TextWidth(wxSTC_STYLE_LINENUMBER, "9"));
	m_view->StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColour(0,0,0));
	m_view->StyleSetForeground(1, wxColour(0,100,0));
	m_view->StyleSetForeground(2, wxColour(0,100,0));
	m_view->StyleSetForeground(4, wxColour(0,0,128));
	m_view->StyleSetForeground(5, wxColour(0,0,255));
	m_view->StyleSetForeground(6, wxColour(128,128,128));
	m_view->StyleSetForeground(7, wxColour(0,0,255));
	m_view->StyleSetForeground(9, wxColour(255,0,0));
	m_view->StyleSetForeground(10, wxColour(160,0,0));
	m_view->StyleSetForeground(11, wxColour(0,0,0));
	m_view->StyleSetForeground(16, wxColour(0,128,128));
	m_view->StyleSetForeground(19, wxColour(128,0,128));

	wxMenu *fileMenu = new wxMenu;

    fileMenu->Append(CMD_NEW, "&New\tCtrl-N");
	fileMenu->Append(CMD_RELOAD, "&Reload");
    fileMenu->Append(CMD_OPEN, "&Import...\tCtrl-I");
    fileMenu->Append(CMD_SAVE, "&Save\tCtrl-S");
    fileMenu->Append(CMD_SAVEAS, "Export...");
    fileMenu->AppendSeparator();
    fileMenu->Append(CMD_PRINT, "Print...\tCtrl-P");
    fileMenu->AppendSeparator();
	fileMenu->Append(CMD_CLOSE, "Close");

	wxMenu * editmenu = new wxMenu;

    editmenu->Append(CMD_UNDO, "Undo\tAlt-Backspace");
    editmenu->Append(CMD_REDO, "Redo\tCtrl-Alt-Backspace");
    editmenu->AppendSeparator();
    editmenu->Append(CMD_COPY, "Copy\tCtrl-C");
    editmenu->Append(CMD_CUT, "Cut\tCtrl-X");
    editmenu->Append(CMD_PASTE, "Paste\tCtrl-P");
    editmenu->AppendSeparator();
    editmenu->Append(CMD_FIND, "Find...\tCtrl-F");
    editmenu->Append(CMD_FINDNEXT, "Find next...\tF3");
    editmenu->Append(CMD_REPLACE, "Replace\tCtrl-R");
    editmenu->AppendSeparator();
    editmenu->Append(CMD_SELECTALL, "Select all");
	editmenu->AppendSeparator();
    editmenu->Append(CMD_TAB, "Set Tab stops");
	if (m_type!=Script_FS)
	{
		editmenu->Append(CMD_COMPILE, "Compile\tF5");
	}

    m_MainMenu = new wxMenuBar();
	
    m_MainMenu->Append(fileMenu, _T("&File"));
	m_MainMenu->Append(editmenu, _T("&Edit"));
    SetMenuBar(m_MainMenu);

	m_statusbar = new wxStatusBar(this, -1);
	SetStatusBar(m_statusbar);

    static const int widths[] = { -1, 150, 60, 60 };

    m_statusbar->SetFieldsCount(4);
    m_statusbar->SetStatusWidths(4, widths);

	m_toolbar = CreateToolBar(wxNO_BORDER | wxTB_FLAT | wxTB_HORIZONTAL | wxTB_DOCKABLE);
	m_toolbar->AddTool(CMD_NEW, "New", bitmap(new), "New");
	m_toolbar->AddTool(CMD_OPEN, "Open", bitmap(open), "Open");
	m_toolbar->AddTool(CMD_SAVE, "Save", bitmap(save), "Save");
	m_toolbar->AddSeparator();
	m_toolbar->AddTool(CMD_PRINT, "Print", bitmap(print), "Print");
	m_toolbar->AddSeparator();
	m_toolbar->AddTool(CMD_CUT, "Cut", bitmap(cut), "Cut");
	m_toolbar->AddTool(CMD_COPY, "Copy", bitmap(copy), "Copy");
	m_toolbar->AddTool(CMD_PASTE, "Paste", bitmap(paste), "Paste");
	//m_toolbar->AddTool(CMD_DELETE, "Delete", bitmap(delete), "Delete");
	if (m_type!=Script_FS)
	{
		m_toolbar->AddSeparator();
		m_toolbar->AddTool(CMD_COMPILE, "Compile", bitmap(compile), "Compile");
	}
	m_toolbar->Realize();

	OnFileReload(wxCommandEvent());

	if (config.SetSection("Settings"))
	{
		int tab = config.GetIntValueForKey("TabSize");
		if (tab==0) tab=4;
		m_view->SetTabWidth(tab*4);
	}
	SetSize(640, 480);
	m_view->SetFocus();
}

//==========================================================================
//
//
//
//==========================================================================

int CScriptEdit::Confirm()
{
	if (m_view && m_view->GetModify())
	{
		int res = wxMessageBox("The script has not been saved.\nDo you want to save it now?", ZED_CAPTION, wxYES_NO|wxICON_QUESTION);
		if (res==wxYES) OnFileSave(wxCommandEvent());
	}
	return IDYES;
}


//==========================================================================
//
//
//
//==========================================================================

void CScriptEdit::OnClose(wxCloseEvent & event)
{
	if (Confirm()!=wxCANCEL)
	{
		m_Level->m_DrawWindow->SetFocus();
		m_Level->m_ScriptEdit[m_type] = NULL;
		Destroy();
		event.Skip();
	}
	else event.Veto();
}


//==========================================================================
//
//
//
//==========================================================================

void CScriptEdit::OnFileNew(wxCommandEvent & event)
{
	if (Confirm()!=wxCANCEL) 
	{
		m_view->SetText("");
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CScriptEdit::OnFileOpen(wxCommandEvent & event)
{
	static const char * extensions[]={".acs",".scr",".txt"};
	if (Confirm()!=wxCANCEL)
	{
		DirectorySaver ds("ScriptImport");

		wxFileDialog fdlg (this, "Import script file", ds.Dir(), "", "", wxOPEN|wxFILE_MUST_EXIST|wxCHANGE_DIR);
		if (fdlg.ShowModal()==wxID_OK)
		{
			FILE * f = fopen(fdlg.GetPath(), "rb");
			if (f!=NULL)
			{
				fseek(f, 0, SEEK_END);
				int len = ftell(f);
				char * text = new char[len+1];
				fseek(f, 0, SEEK_SET);
				fread(text, len, 1, f);
				fclose(f);
				text[len]=0;
				if (m_type==Script_Conv)	// Convert KSSC scripts to a more natural syntax
				{
					for(int i=0;i<len;i++)
					{
						if (text[i]=='&') text[i]='"';
					}
				}
				m_view->SetText(text);
				delete[] text;
			}
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CScriptEdit::OnFileSave(wxCommandEvent & event)
{
	wxString str = m_view->GetText();
	int textlen = (int)str.length();

	switch (m_type)
	{
	case Script_ACS:
		m_Level->m_ACSScript.Resize(textlen);
		memcpy(&m_Level->m_ACSScript[0], str.c_str(), textlen);
		break;

	case Script_FS:
		m_Level->m_FraggleScript.Resize(textlen);
		memcpy(&m_Level->m_FraggleScript[0], str.c_str(), textlen);
		break;

	case Script_Conv:
		m_Level->m_StrifeConvSrc.Resize(textlen);
		memcpy(&m_Level->m_StrifeConvSrc[0], str.c_str(), textlen);
		break;
	}
	m_view->SetSavePoint();
	m_Level->m_changed=true;
}

void CScriptEdit::OnUpdateFileSave(wxUpdateUIEvent & event)
{
	event.Enable(m_view->GetModify());
}

//==========================================================================
//
//
//
//==========================================================================

void CScriptEdit::OnFileSaveAs(wxCommandEvent & event)
{
	static const char * extensions[]={".acs",".scr",".txt"};
	wxString str = m_view->GetText();
	int textlen = (int)str.length();

	DirectorySaver ds("ScriptExport");

	wxFileDialog fdlg (this, "Export script file", ds.Dir(), "", "", wxSAVE|wxOVERWRITE_PROMPT|wxCHANGE_DIR);
	if (fdlg.ShowModal()==wxID_OK)
	{
		wxBusyCursor wait;
		FILE * f = fopen(fdlg.GetPath(), "wb");
		if (f!=NULL)
		{
			fwrite(str.c_str(), 1, textlen, f);
			fclose(f);
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CScriptEdit::OnEditCompile(wxCommandEvent & event)
{
	wxString str = m_view->GetText();
	int textlen = (int)str.length();


	if (m_type==Script_ACS)
	{
		// TODO: Handle libraries here!
		chdir("acc");

		FILE * f = fopen("acstemp.acs", "wb");
		fwrite(str.c_str(), 1, textlen, f);
		fclose(f);

		// Compile it
		wxExecute("acc acstemp.acs", wxEXEC_SYNC);

		// Open the output
		f = fopen("acstemp.o", "rb");

		// If output doesn't exist some kind of error occurred
		if (!f)
		{
			f = fopen("acs.err", "rb");

			if (!f)
			{
				wxMessageBox("Compile failed", ZED_CAPTION);
			}
			else
			{
				// Open a dialog with the contents of acs.err
				fseek(f, 0, SEEK_END);
				int len = ftell(f);
				char * text = new char[len+1];
				fseek(f, 0, SEEK_SET);
				fread(text, len, 1, f);
				fclose(f);
				text[len]=0;

				DisplayText (this, "Compile failed", text);
				delete[] text;
			}
		}
		else
		{
			fseek(f, 0, SEEK_END);
			int len = ftell(f);

			m_Level->m_Behavior.Resize(len);

			fseek(f, 0, SEEK_SET);
			fread(&m_Level->m_Behavior[0], len, 1, f);
			fclose(f);
			remove("acstemp.o");
		}
		chdir("..");
	}
	else if (m_type==Script_Conv)
	{
		CompileConversationScript(m_Level, (char*)str.c_str());
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CScriptEdit::OnFileReload(wxCommandEvent & event)
{
	if (Confirm()!=wxCANCEL)
	{
		switch (m_type)
		{
		case Script_FS:
			m_view->SetText(wxString(&m_Level->m_FraggleScript[0], m_Level->m_FraggleScript.Size()));
			break;

		case Script_ACS:
			m_view->SetText(wxString(&m_Level->m_ACSScript[0], m_Level->m_ACSScript.Size()));
			break;

		case Script_Conv:
			m_view->SetText(wxString(&m_Level->m_StrifeConvSrc[0], m_Level->m_StrifeConvSrc.Size()));
			break;

		}
		m_view->EmptyUndoBuffer();
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CScriptEdit::OnEditSettabstops(wxCommandEvent & event)
{
	int tab;
	if (config.SetSection("Settings")) tab = config.GetIntValueForKey("TabSize");
	else tab = 4;

	tab = wxGetNumberFromUser("", "Tab size:", "Set Tab size", tab, 1, 16, this);
	if (tab>0)
	{
		if (config.SetSection("Settings", true))
		{
			config.SetIntValueForKey("TabSize", tab);
		}
		m_view->SetTabWidth(tab*4);
	}
}



//==========================================================================
//
// basic editing commands
//
//==========================================================================

void CScriptEdit::OnEditUndo(wxCommandEvent & event)
{
	m_view->Undo();
}

void CScriptEdit::OnEditRedo(wxCommandEvent & event)
{
	m_view->Redo();
}

void CScriptEdit::OnEditCut(wxCommandEvent & event)
{
	m_view->Cut();
}

void CScriptEdit::OnEditCopy(wxCommandEvent & event)
{
	m_view->Copy();
}

void CScriptEdit::OnEditPaste(wxCommandEvent & event)
{
	m_view->Paste();
}

void CScriptEdit::OnEditClear(wxCommandEvent & event)
{
	m_view->ClearAll();
}

void CScriptEdit::OnEditSelectAll(wxCommandEvent & event)
{
	m_view->SelectAll();
}

void CScriptEdit::OnUpdateEditUndo(wxUpdateUIEvent & event)
{
	event.Enable(m_view->CanUndo());
}

void CScriptEdit::OnUpdateEditRedo(wxUpdateUIEvent & event)
{
	event.Enable(m_view->CanRedo());
}

void CScriptEdit::OnUpdateEditCut(wxUpdateUIEvent & event)
{
	int len = m_view->GetSelectionEnd() - m_view->GetSelectionStart();
	event.Enable(!!len);
}

void CScriptEdit::OnUpdateEditPaste(wxUpdateUIEvent & event)
{
	event.Enable(m_view->CanPaste());
}

//==========================================================================
//
//
//
//==========================================================================

void CScriptEdit::OnEditFind(wxCommandEvent & event)
{
	OnEditFindReplace(true);
}

void CScriptEdit::OnEditRepeat(wxCommandEvent & event)
{
	FindText();
}

void CScriptEdit::OnEditReplace(wxCommandEvent & event)
{
	OnEditFindReplace(false);
}

void CScriptEdit::OnFindDialogFind(wxFindDialogEvent & event)
{
	FindText();
}

void CScriptEdit::OnFindDialogClose(wxFindDialogEvent & event)
{
	m_find->Destroy();
	m_find=NULL;
}

//==========================================================================
//
//
//
//==========================================================================

void CScriptEdit::OnEditFindReplace(bool bFindOnly)
{
	if (m_find) 
	{
		if (bFindOnly!=m_replacing)
		{
			m_find->Destroy();
			m_find=NULL;
		}
		m_replacing=bFindOnly;
	}


	int len = m_view->GetSelectionEnd() - m_view->GetSelectionStart();
	if (len>0) 
	{
		m_finddata.SetFindString(m_view->GetSelectedText());
	}
	if (!m_find)
	{
		m_find = new wxFindReplaceDialog(this, &m_finddata, wxString(bFindOnly? "Find":"Replace"), bFindOnly? 0: wxFR_REPLACEDIALOG);
	}
	m_find->Show();
}


//==========================================================================
//
//
//
//==========================================================================

bool CScriptEdit::FindText()
{
	int fmin = m_view->GetSelectionStart();
	int fmax = m_view->GetSelectionEnd();
	int len = m_view->GetTextLength();
	bool down = !!(m_finddata.GetFlags()&wxFR_DOWN);
	bool casesensitive = !!(m_finddata.GetFlags()&wxFR_MATCHCASE);
	bool wholeword = !!(m_finddata.GetFlags()&wxFR_WHOLEWORD);

	if (fmin!=fmax)
	{
		if (down) fmin++;
		else  fmax=fmin-1;
	}

	if (down) fmax = len;
	else fmin = 0;

	if (!down)
	{
		int v=fmin;
		fmin=fmax;
		fmax=v;
	}

	long index = m_view->FindText(fmin, fmax, m_finddata.GetFindString(), 
		casesensitive * wxSTC_FIND_MATCHCASE + wholeword *wxSTC_FIND_WHOLEWORD);

	if (index!=-1)
	{
		m_view->SetSelection(index, index + (int)m_finddata.GetFindString().length());
	}
	return index!=-1;
}

//==========================================================================
//
//
//
//==========================================================================

bool CScriptEdit::SameAsSelected(const wxString & str)
{
	int start = m_view->GetSelectionStart();
	int end = m_view->GetSelectionEnd();
	if (str.length() != end - start) return false;

	// length is the same, check contents
	wxString buffer = m_view->GetSelectedText();
	return !(m_finddata.GetFlags()&wxFR_MATCHCASE? str.Cmp(buffer) : str.CmpNoCase(buffer));
}


//==========================================================================
//
//
//
//==========================================================================

void CScriptEdit::OnFindDialogReplace(wxFindDialogEvent & event)
{
	if (SameAsSelected(m_finddata.GetFindString()))
	{
		m_view->ReplaceSelection(m_finddata.GetReplaceString());
	}
	FindText();
}

//==========================================================================
//
//
//
//==========================================================================

void CScriptEdit::OnFindDialogReplaceAll(wxFindDialogEvent & event)
{
	// no selection or different than what looking for
	if (!SameAsSelected(m_finddata.GetFindString()) && !FindText()) return;

	wxBusyCursor wait;

	m_view->HideSelection(true);
	do
	{
		m_view->ReplaceSelection(m_finddata.GetReplaceString());
	} 
	while (FindText());
	m_view->HideSelection(false);
}


//==========================================================================
//
//
//
//==========================================================================

class ScriptPrintOut : public wxPrintout
{
	wxStyledTextCtrl * m_view;
	wxPrintDialogData * m_data;
	TArray<int> m_pageStart;

	bool DoPrintPage(int pageNum, bool print);

public:
	ScriptPrintOut(wxStyledTextCtrl * editctrl)
	{
		m_view = editctrl;
		m_pageStart.Push(0);
	}

	bool HasPage(int pageNum);
	bool OnPrintPage(int pageNum);
};


//==========================================================================
//
//
//
//==========================================================================

bool ScriptPrintOut::DoPrintPage(int pagenum, bool print)
{
	int w, h;
	int ww, hh;
	int nIndex = m_pageStart[pagenum-1];
	int nEndPrint = m_view->GetTextLength();

	if (nIndex >= nEndPrint)
	{
		return false;
	}

	GetPageSizePixels(&w, &h);
	GetPageSizeMM(&ww, &hh);

	wxRect rc(0,0,w-1,h-1);

	GetDC()->SetUserScale((float)w/ww/5, (float)h/hh/5);
	nIndex = m_view->FormatRange(print, nIndex, nEndPrint, GetDC(), GetDC(), rc, rc);

	//update pagination information for page just printed
	if (pagenum==m_pageStart.Size())
	{
		if (nIndex < nEndPrint)
			m_pageStart.Push(nIndex);
	}
	return true;
}

//==========================================================================
//
//
//
//==========================================================================

bool ScriptPrintOut::HasPage(int pagenum)
{
	while (pagenum>(int)m_pageStart.Size())
	{
		int page = m_pageStart.Size();
		DoPrintPage(page, false);
		if (page==m_pageStart.Size()) return false;
	}
	return true;
}

//==========================================================================
//
//
//
//==========================================================================

bool ScriptPrintOut::OnPrintPage(int pagenum)
{
	if (HasPage(pagenum)) return DoPrintPage(pagenum, true);
	else return false;
}

//==========================================================================
//
//
//
//==========================================================================

void CScriptEdit::OnPrint(wxCommandEvent & event)
{
	ScriptPrintOut printout(m_view);
	wxPrinter printer;

	printer.Print(this, &printout);
}

