#ifndef __SCRIPTED_H
#define __SCRIPTED_H

// CScriptEditor-Dialogfeld

enum
{
	Script_ACS,
	Script_FS,
	Script_Conv,
};

class CLevel;

class CScriptEdit : public wxFrame
{
	wxStatusBar * m_statusbar;
	wxToolBar * m_toolbar;
	wxStyledTextCtrl * m_view;
	wxFindReplaceDialog * m_find;
	wxFindReplaceData m_finddata;
	bool m_replacing;

	wxMenuBar * m_MainMenu;

	CLevel *	m_Level;
	int			m_type;
	int			m_external;

	int Confirm();
	void SetKeywords(const char * filename);
	void SetLevel(CLevel * lev, int resource) { m_Level=lev; m_type=resource; }
	void OnEditFindReplace(bool bFindOnly);
	bool FindText();
	bool SameAsSelected(const wxString & str);

	void OnClose(wxCloseEvent& event);
	void OnFileNew(wxCommandEvent & event);
	void OnFileOpen(wxCommandEvent & event);
	void OnFileSave(wxCommandEvent & event);
	void OnUpdateFileSave(wxUpdateUIEvent & event);
	void OnFileSaveAs(wxCommandEvent & event);
	void OnFileClose(wxCommandEvent & event);
	void OnEditCompile(wxCommandEvent & event);
	void OnFileReload(wxCommandEvent & event);
	void OnEditSettabstops(wxCommandEvent & event);
	void OnEditUndo(wxCommandEvent & event);
	void OnEditRedo(wxCommandEvent & event);
	void OnEditCut(wxCommandEvent & event);
	void OnEditCopy(wxCommandEvent & event);
	void OnEditPaste(wxCommandEvent & event);
	void OnEditClear(wxCommandEvent & event);
	void OnEditSelectAll(wxCommandEvent & event);
	void OnUpdateEditUndo(wxUpdateUIEvent & event);
	void OnUpdateEditRedo(wxUpdateUIEvent & event);
	void OnUpdateEditCut(wxUpdateUIEvent & event);
	void OnUpdateEditPaste(wxUpdateUIEvent & event);
	void OnEditFind(wxCommandEvent & event);
	void OnEditRepeat(wxCommandEvent & event);
	void OnEditReplace(wxCommandEvent & event);
    void OnFindDialogFind(wxFindDialogEvent & event);
    void OnFindDialogNext(wxFindDialogEvent & event);
    void OnFindDialogReplace(wxFindDialogEvent & event);
    void OnFindDialogReplaceAll(wxFindDialogEvent & event);
    void OnFindDialogClose(wxFindDialogEvent & event);
	void OnPrint(wxCommandEvent & event);
	void OnKeyDown(wxKeyEvent & event);


public:
	CScriptEdit(wxWindow * parent, CLevel * level, int scripttype);

	DECLARE_EVENT_TABLE()

};

#endif