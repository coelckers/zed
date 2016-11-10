#include "stdafx.h"
#include "ZEd.h"
#include "ResourceFile.h"
#include "images.h"

//==========================================================================
//
//
//
//==========================================================================

BEGIN_EVENT_TABLE(ZEdFrame, wxFrame)
    EVT_MENU(wxID_ABOUT, ZEdFrame::OnAbout)
    EVT_MENU(wxID_EXIT, ZEdFrame::OnQuit)
    EVT_MENU(CMD_OPEN, ZEdFrame::OnOpen)
    EVT_MENU(CMD_SELECTIWAD, ZEdFrame::OnSelectIWAD)

	//EVT_MENU(MDI_NEW_WINDOW, ZEdFrame::OnNewWindow)

    EVT_CLOSE(ZEdFrame::OnClose)
    EVT_SIZE(ZEdFrame::OnSize)
END_EVENT_TABLE()

//==========================================================================
//
//
//
//==========================================================================

ZEdFrame::ZEdFrame(wxWindow *parent,
                 const wxWindowID id,
                 const wxString& title,
                 const wxPoint& pos,
                 const wxSize& size,
                 const long style)
       : wxFrame(parent, id, title, pos, size, style)
{

	SetIcon(wxIcon("AZED",  wxBITMAP_TYPE_ICO_RESOURCE, -1, -1));

	wxMenu *fileMenu = new wxMenu;

	m_view=NULL;
    fileMenu->Append(CMD_NEW, "&New map");
    fileMenu->Append(CMD_OPEN, "&WAD manager...");
    fileMenu->Append(CMD_SELECTIWAD, "Select IWADs...");
    fileMenu->Append(CMD_CHANGECONFIG, "Change configuration...");
    fileMenu->AppendSeparator();
    fileMenu->Append(CMD_SAVE, "Save");
    fileMenu->Append(CMD_SAVEAS, "Save as...");
    fileMenu->AppendSeparator();
    fileMenu->Append(CMD_PREFERENCES, "Preferences...");
    fileMenu->AppendSeparator();
    fileMenu->AppendCheckItem(CMD_ENABLESDCOMPRESSION, "Enable sidedef compression");
    fileMenu->AppendCheckItem(CMD_ALWAYSSAVEASTEXT, "Always save as text format");
    fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, "Quit\t Alt-F4");

	wxMenu * wadmenu = new wxMenu;

	wadmenu->Append(CMD_LISTTHINGSINPWAD, "List all used things in current PWAD");
	wadmenu->Append(CMD_LISTLINETYPESINPWAD, "List all used linedef and sector types in PWAD");
	wadmenu->Append(CMD_LISTUNUSEDTEXTURES, "List all unused textures in PWAD");
	wadmenu->Append(CMD_LISTUSEDTEXTURES, "List all used textures in PWAD");
                                                
	wxMenu * editmenu = new wxMenu;

    editmenu->Append(CMD_UNDO, "Undo");
    editmenu->Append(CMD_REDO, "Redo");
    editmenu->AppendSeparator();
    editmenu->Append(CMD_COPY, "Copy");
    editmenu->Append(CMD_CUT, "Cut");
    editmenu->Append(CMD_PASTE, "Paste");
    editmenu->Append(CMD_DELETE, "Delete");
    editmenu->Append(CMD_CLEAR, "Clear Selection");
    editmenu->AppendSeparator();
    editmenu->Append(CMD_DELETEUNUSEDVERTICES, "Delete unused vertices");
    editmenu->Append(CMD_DELETEUNUSEDSECTORS, "Delete unused sectors");
    editmenu->Append(CMD_REMOVEZEROLINES, "Delete zero-length linedefs");
    editmenu->AppendSeparator();
    editmenu->Append(CMD_EDITACS, "Edit ACS script");
    editmenu->Append(CMD_EDITFS, "Edit FraggleScript");
    editmenu->Append(CMD_EDITCONV, "Edit conversation script");
    editmenu->Append(CMD_EDITFSGLOBAL, "Edit FSGLOBAL");
    //MENUITEM "Add ACS library/include",     ID_EDIT_ADDACSLIBRARY
    //MENUITEM "Edit ACS library/include",    ID_Menu33018

	wxMenu * viewmenu = new wxMenu;
	
	viewmenu->Append(CMD_ZOOMIN, "Zoom in");
	viewmenu->Append(CMD_ZOOMOUT, "Zoom out");
	viewmenu->Append(CMD_CENTERVIEW, "Center view");
	viewmenu->AppendSeparator();
	viewmenu->Append(CMD_GOTOOBJECT, "Go to object...");
	viewmenu->AppendSeparator();
	viewmenu->AppendCheckItem(CMD_SNAPTOGRID, "Automatically snap to grid");
	viewmenu->AppendCheckItem(CMD_AUTOGRID, "Auto-toggle snap");
	viewmenu->AppendCheckItem(CMD_SHOWGRID, "Show grid");
	viewmenu->AppendCheckItem(CMD_HIGHLIGHTGRID, "Highlight 64 Unit Grid");
	viewmenu->Append(CMD_SETGRID, "Set grid...");
	viewmenu->Append(CMD_CHANGEGRID, "Change grid");
	viewmenu->AppendSeparator();
	viewmenu->AppendCheckItem(CMD_SIDEMARKERS, "Show side markers on Lines");
	viewmenu->AppendSeparator();
	viewmenu->Append(CMD_LISTTEXTURES, "List all used textures");
	viewmenu->Append(CMD_LISTLINETYPES, "List all used linedef and sector types");
	viewmenu->Append(CMD_LISTTHINGS, "List all used things");
	viewmenu->Append(CMD_ITEMSTATISTICS, "Item statistics");
	viewmenu->Append(CMD_LISTTAGS, "List all used tags");
	
	wxMenu * toolsmenu = new wxMenu;
	
	//toolsmenu->Append(CMD_LINEDRAW, "Start line drawing...");
	//toolsmenu->AppendSeparator();
	toolsmenu->Append(CMD_FINDTHINGBYTYPE, "Find thing by type...");
	toolsmenu->Append(CMD_FINDTHINGBYTID, "Find thing by tid...");
	toolsmenu->Append(CMD_FINDTHINGBYSPECIAL, "Find thing by special...");
	toolsmenu->Append(CMD_CHANGETHINGTYPE, "Change thing types...");
	toolsmenu->Append(CMD_MARKTHINGSWITHTAG, "Mark things with tid...");
	toolsmenu->AppendSeparator();
	toolsmenu->Append(CMD_FINDLDBYTEXTURE, "Find linedef by texture...");
	toolsmenu->Append(CMD_FINDLDBYTYPE, "Find linedef by type...");
	toolsmenu->Append(CMD_FINDLDBYTAG, "Find linedef by tag...");
	toolsmenu->Append(CMD_FINDLDBYTID, "Find linedef by tid...");
	toolsmenu->Append(CMD_FINDLDBYLINEID, "Find linedef by line ID...");
	toolsmenu->AppendSeparator();
	toolsmenu->Append(CMD_FINDSECBYTEXTURE, "Find sector by texture...");
	toolsmenu->Append(CMD_FINDSECBYTYPE, "Find sector by type...");
	toolsmenu->Append(CMD_FINDSECBYTAG, "Find sector by tag...");
	toolsmenu->AppendSeparator();
	toolsmenu->Append(CMD_FINDNEXT, "Find next");
	toolsmenu->AppendSeparator();
	toolsmenu->Append(CMD_RENAMETEX, "Rename textures");
	toolsmenu->Append(CMD_TAG, "Find next unused tag");

	wxMenu * modemenu = new wxMenu;

	modemenu->AppendRadioItem(CMD_MODE_THINGS, "Things");
	modemenu->AppendRadioItem(CMD_MODE_LINEDEFS, "Linedefs");
	modemenu->AppendRadioItem(CMD_MODE_SECTORS, "Sectors");
	modemenu->AppendRadioItem(CMD_MODE_VERTICES, "Vertices");

	wxMenu * checkmenu = new wxMenu;

	checkmenu->Append(CMD_STATISTICS, "Statistics");
	checkmenu->AppendSeparator();
	checkmenu->Append(CMD_SECTORCLOSED, "Check if all Sectors are closed");
	checkmenu->Append(CMD_SECTORCLOSED2, "Check if all Sectors are closed (also list non-critical sectors)");
	checkmenu->Append(CMD_XREF, "Cross references");
	checkmenu->Append(CMD_MISSINGTEX, "Check for missing textures");
	checkmenu->Append(CMD_TEXNAMES, "Check texture names");
	checkmenu->AppendSeparator();
	checkmenu->Append(CMD_CHECKALL, "Check all and output list");
	checkmenu->Append(CMD_CHECKALL2, "Check all and output list (also list non-critical sectors)");
	checkmenu->AppendSeparator();
	checkmenu->Append(CMD_CHECKALLPWAD, "Check all levels in current PWAD and output list");
	checkmenu->Append(CMD_CHECKALLPWADP, "Check all levels in current PWAD and output list (also list non-critical sectors)");
	checkmenu->Append(CMD_CHECKTEXPWAD, "Check all levels in current PWAD for texture errors and output list");

    wxMenu * helpMenu = new wxMenu;
    helpMenu->Append(wxID_ABOUT, _T("&About..."));

    wxMenuBar *menuBar = new wxMenuBar();
	
    menuBar->Append(fileMenu, _T("&File"));
	menuBar->Append(wadmenu, _T("&WAD"));
	menuBar->Append(editmenu, _T("&Edit"));
	menuBar->Append(viewmenu, _T("&View"));
	menuBar->Append(toolsmenu, _T("&Tools"));
	menuBar->Append(modemenu, _T("&Mode"));
	menuBar->Append(checkmenu, _T("&Check"));
    menuBar->Append(helpMenu, _T("&Help"));

    SetMenuBar(menuBar);
	m_MainMenu = menuBar;

	m_statusbar = new wxStatusBar(this, -1);
	SetStatusBar(m_statusbar);

    static const int widths[] = { -1, 150, 60, 60 };

    m_statusbar->SetFieldsCount(4);
    m_statusbar->SetStatusWidths(4, widths);
	m_statusbar->SetStatusText("No level loaded");

	m_toolbar = CreateToolBar(wxNO_BORDER | wxTB_FLAT | wxTB_HORIZONTAL | wxTB_DOCKABLE);
	m_toolbar->AddTool(CMD_NEW, "New map", bitmap(new), "New Map");
	m_toolbar->AddTool(CMD_OPEN, "Wad Manager", bitmap(open), "Wad Manager");
	m_toolbar->AddTool(CMD_SAVEAS, "Save As", bitmap(save), "Save As");
	m_toolbar->AddSeparator();
	m_toolbar->AddTool(CMD_CUT, "Cut", bitmap(cut), "Cut");
	m_toolbar->AddTool(CMD_COPY, "Copy", bitmap(copy), "Copy");
	m_toolbar->AddTool(CMD_PASTE, "Paste", bitmap(paste), "Paste");
	m_toolbar->AddTool(CMD_DELETE, "Delete", bitmap(delete), "Delete");
	m_toolbar->AddSeparator();
	m_toolbar->AddTool(CMD_EDITACS, "Edit ACS script", bitmap(acs), "Edit ACS script");
	m_toolbar->AddTool(CMD_EDITFS, "Edit FraggleScript", bitmap(fs), "Edit FraggleScript");
	m_toolbar->AddTool(CMD_EDITCONV, "Edit conversation script", bitmap(conv), "Edit conversation script");
	m_toolbar->AddSeparator();
	m_toolbar->AddTool(CMD_ZOOMOUT, "Zoom out", bitmap(minus), "Zoom out");
	m_toolbar->AddTool(CMD_ZOOMIN, "Zoom in", bitmap(plus), "Zoom in");
	m_toolbar->AddTool(CMD_CENTERVIEW, "Center view", bitmap(center), "Center view");
	m_toolbar->AddTool(CMD_GOTOOBJECT, "Go to object", bitmap(goto), "Go to object");
	m_toolbar->AddCheckTool(CMD_SHOWGRID, "Grid", bitmap(grid), wxNullBitmap, "Grid");
	m_toolbar->AddTool(CMD_DOSNAPTOGRID, "Snap to Grid", bitmap(snap), "Snap to Grid");
	m_toolbar->AddSeparator();
	m_toolbar->AddTool(CMD_LISTLINETYPES, "List linedef and sector types", bitmap(linetype), "List linedef and sector types");
	m_toolbar->AddTool(CMD_LISTTHINGS, "List all used things", bitmap(things), "List all used things");
	m_toolbar->AddTool(CMD_LISTTEXTURES, "List all used textures", bitmap(textures), "List all used textures");
	m_toolbar->AddTool(CMD_ITEMSTATISTICS, "Item statistics", bitmap(stat), "Item statistics");
	m_toolbar->AddSeparator();
	m_toolbar->AddTool(CMD_INSERTRECT, "Insert rectangle", bitmap(rect), "Insert rectangle");
	m_toolbar->AddTool(CMD_INSERTPOLY, "Insert polygon", bitmap(poly), "Insert polygon");
	m_toolbar->AddTool(CMD_CURVE, "Curve linedefs", bitmap(curve), "Curve linedefs");
	m_toolbar->AddSeparator();
	m_toolbar->AddRadioTool(CMD_MODE_THINGS, "Thing mode", bitmap(tt), wxNullBitmap, "Thing mode");
	m_toolbar->AddRadioTool(CMD_MODE_VERTICES, "Vertex mode", bitmap(v), wxNullBitmap, "Vertex mode");
	m_toolbar->AddRadioTool(CMD_MODE_SECTORS, "Sector mode", bitmap(s), wxNullBitmap, "Sector mode");
	m_toolbar->AddRadioTool(CMD_MODE_LINEDEFS, "Linedef mode", bitmap(l), wxNullBitmap, "Linedef mode");
	m_toolbar->AddSeparator();
	m_toolbar->AddTool(CMD_STATISTICS, "Map statistics", bitmap(map), "Map statistics");
	m_toolbar->AddTool(CMD_CHECKALL, "Check map", bitmap(check), "Check map");
	m_toolbar->Realize();


	m_ThingMenu = new wxMenu;

	m_ThingMenu->Append(CMD_PROP, "Properties...");
	m_ThingMenu->Append(CMD_COPYPROP, "Copy properties");
	m_ThingMenu->Append(CMD_PASTEPROP, "Paste properties");
	m_ThingMenu->Append(CMD_DOSNAPTOGRID, "Snap to grid");
	m_ThingMenu->AppendSeparator();
	m_ThingMenu->Append(CMD_INSERT_MENU, "Insert Thing");
	m_ThingMenu->Append(CMD_DELETE, "Delete Things");
	m_ThingMenu->AppendSeparator();
	m_ThingMenu->Append(CMD_ROTATE, "Rotate and scale Things...");
	m_ThingMenu->Append(CMD_ROTATE90L, "Rotate selection 90° left");
	m_ThingMenu->Append(CMD_ROTATE90R, "Rotate selection 90° right");
	m_ThingMenu->Append(CMD_ROTATE180, "Rotate selection 180°");
	m_ThingMenu->Append(CMD_MIRROR_H, "Mirror selection horizontally");
	m_ThingMenu->Append(CMD_MIRROR_V, "Mirror selection vertically");
	m_ThingMenu->AppendSeparator();
	m_ThingMenu->Append(CMD_FINDLINEBYCURRENTTID, "Find tagged lines");


	m_VertexMenu = new wxMenu;
	m_VertexMenu->Append(CMD_PROP, "Properties...");
	m_VertexMenu->Append(CMD_DOSNAPTOGRID, "Snap to grid");
	m_VertexMenu->AppendSeparator();
	m_VertexMenu->Append(CMD_INSERT_MENU, "Insert Vertex");
	m_VertexMenu->Append(CMD_DELETE, "Delete Vertices");
	m_VertexMenu->AppendSeparator();
	m_VertexMenu->Append(CMD_ROTATE, "Rotate and scale objects...");
	m_VertexMenu->Append(CMD_ROTATE90L, "Rotate selection 90° left");
	m_VertexMenu->Append(CMD_ROTATE90R, "Rotate selection 90° right");
	m_VertexMenu->Append(CMD_ROTATE180, "Rotate selection 180°");
	m_VertexMenu->Append(CMD_MIRROR_H, "Mirror selection horizontally");
	m_VertexMenu->Append(CMD_MIRROR_V, "Mirror selection vertically");
	m_VertexMenu->AppendSeparator();
	m_VertexMenu->Append(CMD_DELJOIN, "Delete Vertices and join LineDefs");
	m_VertexMenu->Append(CMD_MERGE, "Merge Vertices into one");
	m_VertexMenu->Append(CMD_SPLITSECTOR, "Split sector");
	m_VertexMenu->AppendSeparator();
	//m_VertexMenu->Append(CMD_LINEDRAWATVERT, "Start Line Drawing");
	//m_VertexMenu->AppendSeparator();
	m_VertexMenu->Append(CMD_LINESFROMVERTS, "Select Lines from Vertices");

	m_SectorMenu = new wxMenu;

	m_SectorMenu->Append(CMD_PROP, "Properties...");
	m_SectorMenu->Append(CMD_COPYPROP, "Copy properties");
	m_SectorMenu->Append(CMD_PASTEPROP, "Paste properties");
	m_SectorMenu->Append(CMD_DOSNAPTOGRID, "Snap to grid");
	m_SectorMenu->AppendSeparator();
	m_SectorMenu->Append(CMD_INSERT_MENU, "Insert Sector");
	m_SectorMenu->Append(CMD_DELETE, "Delete Sectors");
	m_SectorMenu->Append(CMD_DELETEUNUSED, "Delete unused Sectors");
	m_SectorMenu->AppendSeparator();
	m_SectorMenu->Append(CMD_ROTATE, "Rotate and scale objects...");
	m_SectorMenu->Append(CMD_ROTATE90L, "Rotate selection 90° left");
	m_SectorMenu->Append(CMD_ROTATE90R, "Rotate selection 90° right");
	m_SectorMenu->Append(CMD_ROTATE180, "Rotate selection 180°");
	m_SectorMenu->Append(CMD_MIRROR_H, "Mirror selection horizontally");
	m_SectorMenu->Append(CMD_MIRROR_V, "Mirror selection vertically");
	m_SectorMenu->AppendSeparator();
	m_SectorMenu->Append(CMD_JOINSEC, "Join sectors");
	m_SectorMenu->AppendSeparator();
	m_SectorMenu->Append(CMD_MAKEDOOR, "Make door from Sector");
	m_SectorMenu->Append(CMD_MAKELIFT, "Make lift from Sector");
	m_SectorMenu->Append(CMD_DISTFLOOR, "Distribute floor heights");
	m_SectorMenu->Append(CMD_DISTCEIL, "Distribute ceiling heights");
	m_SectorMenu->Append(CMD_DISTLIGHT, "Distribute light levels");
	m_SectorMenu->AppendSeparator();
	m_SectorMenu->Append(CMD_RAISEFLOOR8, "Raise floor by 8 map units");
	m_SectorMenu->Append(CMD_RAISECEIL8, "Raise ceiling by 8 map units");
	m_SectorMenu->Append(CMD_LOWERFLOOR8, "Lower floor by 8 map units");
	m_SectorMenu->Append(CMD_LOWERCEIL8, "Lower ceiling by 8 map units");
	m_SectorMenu->Append(CMD_RAISELIGHT, "Raise light level");
	m_SectorMenu->Append(CMD_LOWERLIGHT, "Lower Light Level");
	m_SectorMenu->AppendSeparator();
	m_SectorMenu->Append(CMD_SELECTTHINGSINSECTORS, "Select Things in Sectors");
	m_SectorMenu->Append(CMD_SELECTLINESFROMSECTORS, "Select Lines from Sectors");
	m_SectorMenu->Append(CMD_SELECTVERTFROMSECTORS, "Select Vertices from Sectors");
	m_SectorMenu->AppendSeparator();
	m_SectorMenu->Append(CMD_FINDLINEBYCURRENTTAG, "Find tagged lines");

	m_LineMenu = new wxMenu;

	m_LineMenu->Append(CMD_PROP, "Properties...");
	m_LineMenu->Append(CMD_COPYPROP, "Copy properties");
	m_LineMenu->Append(CMD_PASTEPROP, "Paste properties");
	m_LineMenu->Append(CMD_DOSNAPTOGRID, "Snap to grid");
	m_LineMenu->AppendSeparator();
	m_LineMenu->Append(CMD_INSERT_MENU, "Insert Linedef");
	m_LineMenu->Append(CMD_DELETE, "Delete Linedefs");
	m_LineMenu->Append(CMD_INSERTRECT, "Insert rectangle...");
	m_LineMenu->Append(CMD_INSERTPOLY, "Insert polygon...");
	m_LineMenu->AppendSeparator();
	m_LineMenu->Append(CMD_CURVE, "Curve linedefs...");
	m_LineMenu->Append(CMD_ROTATE, "Rotate and scale objects...");
	m_LineMenu->Append(CMD_ROTATE90L, "Rotate selection 90° left");
	m_LineMenu->Append(CMD_ROTATE90R, "Rotate selection 90° right");
	m_LineMenu->Append(CMD_ROTATE180, "Rotate selection 180°");
	m_LineMenu->Append(CMD_MIRROR_H, "Mirror selection horizontally");
	m_LineMenu->Append(CMD_MIRROR_V, "Mirror selection vertically");
	m_LineMenu->AppendSeparator();
	m_LineMenu->Append(CMD_JOIN, "Join LineDefs");
	m_LineMenu->Append(CMD_SPLITLD, "Split LineDefs");
	m_LineMenu->Append(CMD_SPLITLDSEC, "Split LineDefs and Sector");
	m_LineMenu->Append(CMD_DELSD_JOINSEC, "Delete LineDefs and join Sectors");
	m_LineMenu->Append(CMD_FLIP, "Flip LineDefs");
	m_LineMenu->Append(CMD_SWAP, "Swap SideDefs");
	m_LineMenu->Append(CMD_DEL2ND, "Delete second SideDef");
	m_LineMenu->AppendSeparator();
	m_LineMenu->Append(CMD_FINDSECTORBYCURRENTTAG, "Find tagged sectors");
	m_LineMenu->Append(CMD_FINDLINEBYCURRENTLINEID, "Find tagged lines");
	m_LineMenu->Append(CMD_FINDTHINGBYCURRENTTID, "Find tagged things");

	wxMenu * alignpopup = new wxMenu;
	m_LineMenu->Append(-1, "Align textures X-offset", alignpopup);

	alignpopup->Append(-1, "SideDef1:")->Enable(false);
	alignpopup->Append(CMD_ALIGNX1, "Check for identical textures");
	alignpopup->Append(CMD_ALIGNX2, "As above but with initial offset");
	alignpopup->Append(CMD_ALIGNX3, "No texture checking");
	alignpopup->Append(CMD_ALIGNX4, "As above but with initial offset");

	alignpopup->Append(-1, "SideDef2:")->Enable(false);
	alignpopup->Append(CMD_ALIGNX5, "Check for identical textures");
	alignpopup->Append(CMD_ALIGNX6, "As above but with initial offset");
	alignpopup->Append(CMD_ALIGNX7, "No texture checking");
	alignpopup->Append(CMD_ALIGNX8, "As above but with initial offset");

	m_LineMenu->Append(CMD_ALIGNY, "Align textures Y-offset");
	m_LineMenu->AppendSeparator();
	m_LineMenu->Append(CMD_UNSELECTONE, "Unselect one-sided lines");
	m_LineMenu->Append(CMD_UNSELECTTWO, "Unselect two-sided lines");
	m_LineMenu->Append(CMD_REDUCE, "Reduce Selection...");
	m_LineMenu->Append(CMD_SELECTVERTICESFROMLINES, "Select Vertices from Lines");

	m_view = new C2DMapview(this);
	m_view->Show();

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(vbox);
	vbox->Add(m_view, 1, wxEXPAND);

	m_ThingBar = new ThingBar(this);
	vbox->Add(m_ThingBar, 0, wxEXPAND);
	m_CurrentBar = m_ThingBar;

	m_VertexBar = new VertexBar(this);
	m_LineBar = new LineBar(this);
	m_SectorBar = new SectorBar(this);
	m_VertexBar->Show(false);
	m_LineBar->Show(false);
	m_SectorBar->Show(false);

	Maximize(true);
	Show();
	Layout();
}

//==========================================================================
//
//
//
//==========================================================================

ZEdFrame::~ZEdFrame()
{
	if (m_ThingMenu) delete m_ThingMenu;
	if (m_LineMenu) delete m_LineMenu;
	if (m_SectorMenu) delete m_SectorMenu;
	if (m_VertexMenu) delete m_VertexMenu;
}

//==========================================================================
//
//
//
//==========================================================================

void ZEdFrame::SetInfoBar(wxWindow * bar)
{
	if (bar != m_CurrentBar)
	{
		GetSizer()->Detach(m_CurrentBar);
		m_CurrentBar->Hide();
		GetSizer()->Add(bar, 0, wxEXPAND);
		m_CurrentBar=bar;
		bar->Show(true);
		bar->Layout();
		Layout();
	}
}
//==========================================================================
//
//
//
//==========================================================================

void ZEdFrame::OnSize(wxSizeEvent& event)
{
	wxFrame::OnSize(event);
}

//==========================================================================
//
//
//
//==========================================================================

void ZEdFrame::OnAbout(wxCommandEvent& event)
{
	wxMessageBox(_T("ZEd v2.1\n(c) 1995-2008 by Christoph Oelckers\n"));
}

//==========================================================================
//
//
//
//==========================================================================

void ZEdFrame::OnQuit(wxCommandEvent& event)
{
    Close();
}

//==========================================================================
//
//
//
//==========================================================================

void ZEdFrame::OnOpen(wxCommandEvent& event)
{
	QWORD maplump = RunWADManager(this);

	if (maplump!=(QWORD)-1)
	{
		if (m_view->Detach(false))
		{
			CLevel * level = new CLevel(false);
			try
			{
				level->Load(maplump, CResourceFile::GlobalGetLumpName(maplump));
			}
			catch (CRecoverableError * err)
			{
				wxMessageBox(err->GetMessage(), ZED_CAPTION, wxICON_ERROR);
				delete level;
				return;
			}
			m_view->Attach(level);
			level->m_currentWAD=int((maplump>>32)-1);
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void ZEdFrame::OnSelectIWAD(wxCommandEvent& event)
{
	RunIWADSelect(this);
}

//==========================================================================
//
//
//
//==========================================================================

void ZEdFrame::OnClose(wxCloseEvent& event)
{
	if (event.CanVeto() && m_view && !m_view->Detach(false))
    {
		event.Veto();
		return;
    }
	Destroy();
    event.Skip();
}


//==========================================================================
//
//
//
//==========================================================================

bool ZEdFrame::ProcessEvent(wxEvent & event)
{
	if (m_view && m_view->GetLevel() && event.IsCommandEvent())
	{
		// First try to let the level handle the event
		//int level = event.StopPropagation();
		bool res = m_view->GetLevel()->ProcessEvent(event);
		//event.ResumePropagation(level);
		if (res) return true;
	}
	return wxFrame::ProcessEvent(event);
}

//==========================================================================
//
//
//
//==========================================================================

void ZEdFrame::SetStatusBarText(const char * text)
{
	m_statusbar->SetStatusText(text);
}

void ZEdFrame::SetCoordinates(int x, int y)
{
	m_statusbar->SetStatusText(wxString::Format("x: %d", x), 2);
	m_statusbar->SetStatusText(wxString::Format("y: %d", y), 3);
}
