#include "stdafx.h"
#include "tarray.h"
#include "ZEd.h"

struct CEditorCommandDescript
{
	int iCommandID;
	const char * strText;
};


/*
	CMD_START=200,

	CMD_NEW=CMD_START,
	CMD_OPEN,
	CMD_SELECTIWAD,
	CMD_PREFERENCES,

	CMD_CHANGECONFIG,
	CMD_SAVE,
	CMD_SAVEAS,
	CMD_ENABLESDCOMPRESSION,

	CMD_LISTTHINGSINPWAD,
	CMD_LISTLINETYPESINPWAD,
	CMD_LISTUNUSEDTEXTURES,

	CMD_UNDO,
	CMD_REDO,
	CMD_COPY,
	CMD_CUT,
	CMD_PASTE,
	CMD_DELETE,
	CMD_CLEAR,

	CMD_DELETEUNUSEDVERTICES,
	CMD_DELETEUNUSEDSECTORS,
	CMD_REMOVEZEROLINES,

	CMD_EDITACS,
	CMD_EDITFS,
	CMD_EDITCONV,
	CMD_EDITFSGLOBAL,

	CMD_ZOOMIN,
	CMD_ZOOMOUT,
	CMD_CENTERVIEW,

	CMD_GOTOOBJECT,

	CMD_SNAPTOGRID,
	CMD_AUTOGRID,
	CMD_SHOWGRID,
	CMD_HIGHLIGHTGRID,
	CMD_SETGRID,
	CMD_CHANGEGRID,

	CMD_SIDEMARKERS,

	CMD_LISTTEXTURES,
	CMD_LISTLINETYPES,
	CMD_LISTTHINGS,
	CMD_ITEMSTATISTICS,
	CMD_LISTTAGS,

	CMD_INSERTRECT,
	CMD_INSERTPOLY,
	CMD_LINEDRAW,
	CMD_CURVE,

	CMD_FINDTHINGBYTYPE,
	CMD_FINDTHINGBYTID,
	CMD_FINDTHINGBYSPECIAL,
	CMD_CHANGETHINGTYPE,

	CMD_FINDLDBYTEXTURE,
	CMD_FINDLDBYTYPE,
	CMD_FINDLDBYTAG,

	CMD_FINDSECBYTEXTURE,
	CMD_FINDSECBYTYPE,
	CMD_FINDSECBYTAG,

	CMD_FINDNEXT,

	CMD_RENAMETEX,

	CMD_MODE_THINGS,
	CMD_MODE_LINEDEFS,
	CMD_MODE_SECTORS,
	CMD_MODE_VERTICES,

	CMD_STATISTICS,

	CMD_SECTORCLOSED,
	CMD_SECTORCLOSED2,
	CMD_XREF,
	CMD_MISSINGTEX,
	CMD_TEXNAMES,

	CMD_CHECKALL,
	CMD_CHECKALL2,

	CMD_CHECKALLPWAD,
	CMD_CHECKALLPWADP,
	CMD_CHECKTEXPWAD,

static CEditorCommandDescript edname[]={

	{ ID_MODE_THINGS,					"mode_things"		},
	{ ID_MODE_LINEDEFS,					"mode_linedefs"		},
	{ ID_MODE_SECTORS,					"mode_sectors"		},
	{ ID_MODE_VERTICES,					"mode_vertices"		},
	{ ID_VIEW_ZOOMIN,					"view_zoomin"		},
	{ ID_VIEW_ZOOMOUT,					"view_zoomout"		},
	{ ID_CMD_INSERT,					"edit_insert"		},
	{ ID_CMD_INSERT_M,					"menu_insert"		},
	{ ID_CMD_DELETE,					"edit_delete"		},
	{ IDM_PROP,							"edit_properties"	},	
	{ IDM_ROT,							"edit_rotate"		},
	{ IDM_SPLITLD,						"edit_splitld"		},
	{ IDM_SPLITLDSEC,					"edit_splitldsec"	},
	{ IDM_JOINSEC,						"edit_joinsec"		},
	{ IDM_FLIP,							"edit_flipld"		},
	{ IDM_SWAP,							"edit_swapsd"		},
	{ IDM_DEL2nd,						"edit_del2nd"		},
	{ IDM_ALIGNX1,						"edit_alignx1"		},
	{ IDM_ALIGNX2,						"edit_alignx2"		},
	{ IDM_ALIGNX3,						"edit_alignx3"		},
	{ IDM_ALIGNX4,						"edit_alignx4"		},
	{ IDM_ALIGNX5,						"edit_alignx5"		},
	{ IDM_ALIGNX6,						"edit_alignx6"		},
	{ IDM_ALIGNX7,						"edit_alignx7"		},
	{ IDM_ALIGNX8,						"edit_alignx8"		},
	{ IDM_ALIGNY,						"edit_aligny"		},
	{ IDC_REDUCESELECT,					"edit_reducesel"		},
	{ IDM_MAKEDOOR,						"edit_makedoor"		},
	{ IDM_MAKELIFT,						"edit_makelift"		},
	{ IDM_DISTFLOOR,					"edit_distfloor"		},
	{ IDM_DISTCEIL,						"edit_distceil"		},
	{ IDM_DELJOIN,						"edit_joinld"		},
	{ IDM_MERGE,						"edit_mergevt"		},
	{ IDM_SPLITSECTOR,					"edit_splitsector"	},
	{ IDM_LINEDRAWATVERT,				"edit_linedrawatvt"	},
	{ IDM_TAG,							"tool_findtag"		},
	{ IDM_STAT,							"tool_statistics"	},
	{ IDM_CLEVELSEC,					"check_sectors"		},
	{ IDM_CLEVELSECP,					"check_sectors_para"	},
	{ IDM_CLEVEL,						"check_xref"			},
	{ IDM_CMISSING,						"check_missingtex"	},
	{ IDM_CTEXTURE,						"check_textures"		},
	{ IDM_CHKALL,						"check_all"			},
	{ IDM_CHKALLP,						"check_all_para"		},
	{ IDM_NEXT,							"tool_next"			},
	{ IDM_NEXTSPC,						"tool_nextspecial"	},
	{ IDM_PREV,							"tool_prev"			},
	{ IDM_PREVSPC,						"tool_prevspecial"	},
	{ IDM_NEXTJUMP,						"tool_nextj"			},
	{ IDM_NEXTSPCJUMP,					"tool_nextspecialj"	},
	{ IDM_PREVJUMP,						"tool_prevj"			},
	{ IDM_PREVSPCJUMP,					"tool_prevspecialj"	},
										
	{ ID_VIEW_GOTOOBJECT,				"tool_goto"			},
	{ ID_VIEW_CLEARSELECTION,			"edit_clearsel"		},
	{ ID_EDIT_INSERTRECTANGLE,			"edit_insertrect"	},
	{ ID_EDIT_INSERTPOLYGON,			"edit_insertpoly"	},
	{ ID_EDIT_TOGGLEGRID,				"grid_toggle"		},
	{ ID_EDIT_CHANGEGRID,				"grid_change"		},
	{ ID_EDIT_LISTALLUSEDTEXTURES,		"view_listtextures"	},
	{ ID_Menu32928,						"view_listlinedefs"	},
	{ ID_EDIT_LISTALLUSEDSECTORTAGS,	"view_listtags"		},
	{ ID_EDIT_LISTTHINGS,				"view_listthings"	},
	{ ID_EDIT_ITEMSTATISTICS,			"view_itemstats"		},
	{ ID_EDIT_RENAMETEXTURES,			"tool_renametex"		},
	{ ID_FILE_SAVEAS,					"file_saveas"		},
	{ ID_FILE_SAVEASEXTENDED,			"file_saveext"		},
	{ ID_FILE_ENABLESIDEDEFCOMPRESSION,	"file_compress"		},
	{ ID_EDIT_REMOVEZERO,				"edit_removezero"	},
										
	{ ID_EDIT_EDITACSSCRIPT,			"edit_editacs"		},
	{ ID_EDIT_EDITFRAGGLESCRIPT,		"edit_editfs"		},
	{ ID_EDIT_EDITCONVERSATIONSCRIPT,	"edit_editconv"		},
	{ ID_EDIT_UNDO32949,				"edit_undo"			},
	{ ID_1_ROTATESELECTION91,			"edit_rotate90"		},
	{ ID_1_ROTATESELECTION92,			"edit_rotate270"		},
	{ ID_1_ROTATESELECTION181,			"edit_rotate180"		},
	{ ID_1_MIRRORSELECTIONHORIZONTALLY,	"edit_mirrorhorz"	},
	{ ID_1_MIRRORSELECTIONVERTICALLY,	"edit_mirrorvert"	},
	{ ID_SNAPTOGRID,					"edit_snaptogrid"	},
	{ ID_DISTRIBUTELIGHTLEVEL,			"edit_distlight"		},
	{ ID_RAISEFLOORBY8MAPUNITS,			"edit_raisefloor"	},
	{ ID_LOWERFLOORBY8MAPUNITS,			"edit_lowerfloor"	},
	{ ID_RAISECEI,						"edit_raiseceil"		},
	{ ID_LOWERCEILINGBY8MAPUNITS,		"edit_lowerceil"		},
	{ ID_1_RAISELIGHTLEVEL,				"edit_raiselight"	},
	{ ID_1_LOWERLIGHTLEVEL,				"edit_lowerlight"	},
	{ ID_UNSELECTONE,					"edit_unselectone"	},
	{ ID_UNSELECTTWO,					"edit_unselecttwo"	},
	{ ID_DELETEUNUSEDVERTICES,			"delete_unusedvt"	},
	{ ID_DELETEUNUSEDSECTORS,			"delete_unusedsec"	},
	{ ID_EDIT_REDO_L,					"edit_redo"			},
	{ ID_VIEW_SHOWSIDEMARKERS,			"view_showmarkers"	},
	{ ID_VIEW_CENTERVIEW,				"view_center"		},
	{ ID_VIEW_SNAPTOGRID,				"opt_snaptogrid"		},
	{ ID_VIEW_HIGHLIGHT64UNITGRID,		"opt_highlight64"	},
	{ ID_VIEW_SETGRID,					"opt_setgrid"		},
	{ ID_COPYPROPERTIES,				"edit_copyprop"		},
	{ ID_PASTEPROPERTIES,				"edit_pasteprop"		},
	{ ID_VIEW_AUTO,						"opt_autosnap"		},
	{ ID_TOOLS_FINDTHINGBYTYPE,			"find_thingbytype"	},
	{ ID_TOOLS_FINDTHINGBYTID,			"find_thingbytid"	},
	{ ID_TOOLS_CHANGETHINGTYPES,		"find_changething"	},
	{ ID_TOOLS_FINDLINEDEFBYTEXTURE,	"find_linebytexture"	},
	{ ID_TOOLS_FINDLINEDEFBYSPECIAL,	"find_linebyspecial"	},
	{ ID_TOOLS_FINDLINEDEFBYTAG,		"find_linebytag"		},
	{ ID_TOOLS_FINDSECTORBYTEXTURE,		"find_secbytexture"	},
	{ ID_TOOLS_FINDSECTORBYTYPE,		"find_secbytype"		},
	{ ID_TOOLS_FINDSECTORBYTAG,			"find_secbytag"		},
	{ ID_TOOLS_FINDTHINGBYSPECIAL,		"find_thingbyspecial"},
	{ ID_TOOLS_FINDNEXT,				"find_repeat"		},
	{ ID_EDIT_EDITFSGLOBAL,				"edit_editfsglobal"	},
	{ ID_TOOLS_STARTLINEDRAWING,		"edit_startlinedraw"	},
										
	{ ID_1_SELECTTHINGSINSECTORS,		"select_thingsinsec"	},
	{ ID_1_SELECTLINESFROMSECTORS,		"select_linesfromsec"},
	{ ID_1_SELECTVERTICESFROMSECTORS,	"select_vertsfromsec"},
	{ ID_1_SELECTVERTICESFROMLINES,		"select_vertsfromlines"},
	{ ID_1_SELECTLINESFROMVERTICES,		"select_linesfromverts"},
	{ ID_1_JOINSECTORS,					"edit_mergesectors"	},
	{ ID_1_CURVELINEDEFS,				"edit_curve"			},
	{ ID_EDIT_COPY,						"edit_copy"			},
	{ ID_EDIT_CUT,						"edit_cut"			},
	{ ID_EDIT_PASTE,					"edit_paste"			}
};
*/

struct CCommandBinding
{
	int iCommandID;
	int	iVk;
	int iShift;
};

static TArray<CCommandBinding> bindings_2d;
static TArray<CCommandBinding> bindings_3d;

static bool changedbinding[CMD_NUMCOMMANDS-CMD_START];


//==========================================================================
//
//
//
//==========================================================================

static wxString GetKeyName(int keycode)
{
    wxString key;

	switch ( keycode )
    {
        case WXK_BACK: key = _T("Backspace"); break;
        case WXK_TAB: key = _T("Tab"); break;
        case WXK_RETURN: key = _T("Return"); break;
        case WXK_ESCAPE: key = _T("Esc"); break;
        case WXK_SPACE: key = _T("Space"); break;
        case WXK_DELETE: key = _T("Delete"); break;
        case WXK_START: key = _T("Start"); break;
        case WXK_LBUTTON: key = _T("LButton"); break;
        case WXK_RBUTTON: key = _T("RButton"); break;
        case WXK_CANCEL: key = _T("Cancel"); break;
        case WXK_MBUTTON: key = _T("MButton"); break;
        case WXK_CLEAR: key = _T("Clear"); break;
        case WXK_INSERT: key = _T("Insert"); break;
        case WXK_HELP: key = _T("Help"); break;
        case WXK_NUMPAD0: key = _T("Numpad0"); break;
        case WXK_NUMPAD1: key = _T("Numpad1"); break;
        case WXK_NUMPAD2: key = _T("Numpad2"); break;
        case WXK_NUMPAD3: key = _T("Numpad3"); break;
        case WXK_NUMPAD4: key = _T("Numpad4"); break;
        case WXK_NUMPAD5: key = _T("Numpad5"); break;
        case WXK_NUMPAD6: key = _T("Numpad6"); break;
        case WXK_NUMPAD7: key = _T("Numpad7"); break;
        case WXK_NUMPAD8: key = _T("Numpad8"); break;
        case WXK_NUMPAD9: key = _T("Numpad9"); break;
        case WXK_MULTIPLY: key = _T("Multiply"); break;
        case WXK_ADD: key = _T("Add"); break;
        case WXK_SUBTRACT: key = _T("Subtract"); break;
        case WXK_DECIMAL: key = _T("Decimal"); break;
        case WXK_DIVIDE: key = _T("Divide"); break;
        case WXK_F1: key = _T("F1"); break;
        case WXK_F2: key = _T("F2"); break;
        case WXK_F3: key = _T("F3"); break;
        case WXK_F4: key = _T("F4"); break;
        case WXK_F5: key = _T("F5"); break;
        case WXK_F6: key = _T("F6"); break;
        case WXK_F7: key = _T("F7"); break;
        case WXK_F8: key = _T("F8"); break;
        case WXK_F9: key = _T("F9"); break;
        case WXK_F10: key = _T("F10"); break;
        case WXK_F11: key = _T("F11"); break;
        case WXK_F12: key = _T("F12"); break;
        case WXK_F13: key = _T("F13"); break;
        case WXK_F14: key = _T("F14"); break;
        case WXK_F15: key = _T("F15"); break;
        case WXK_F16: key = _T("F16"); break;
        case WXK_F17: key = _T("F17"); break;
        case WXK_F18: key = _T("F18"); break;
        case WXK_F19: key = _T("F19"); break;
        case WXK_F20: key = _T("F20"); break;
        case WXK_F21: key = _T("F21"); break;
        case WXK_F22: key = _T("F22"); break;
        case WXK_F23: key = _T("F23"); break;
        case WXK_F24: key = _T("F24"); break;
        case WXK_NUMLOCK: key = _T("NumLock"); break;
        case WXK_SCROLL: key = _T("Scroll"); break;

        default:
        {
            if ( wxIsprint((int)keycode) )
                key.Printf(_T("%c"), (char)keycode);
        }
    }
	return key;
}


//==========================================================================
//
//
//
//==========================================================================
						 
void UpdateAccelerator(wxMenuItem * item, CCommandBinding & bind)
{
	wxString str = item->GetText();
	
	if (!!str)
	{
		int pos = str.Find('\t');
		if (pos>=0) str.Truncate(pos);

		str+="\t ";
		if (bind.iShift&S_SHIFT) str+="Shift - ";
		if (bind.iShift&S_CONTROL) str+="Ctrl - ";
		if (bind.iShift&S_ALT) str+="Alt - ";
		str+=GetKeyName(bind.iVk);
		item->SetText(str);
	}
}

//==========================================================================
//
//
//
//==========================================================================
						 
void UpdateAccelerator(wxMenuBar * menu, CCommandBinding & bind)
{
	wxMenuItem * item = menu->FindItem(bind.iCommandID);

	if (item) UpdateAccelerator(item, bind);
}

//==========================================================================
//
//
//
//==========================================================================

static void AddBinding(TArray<CCommandBinding> & table, int vk, int shift, int id)
{
	CCommandBinding bind = { id, vk, shift };

	table.Push(bind);

	ZEdApp & app = wxGetApp();
	ZEdFrame * frame = app.GetFrame();

	UpdateAccelerator(frame->m_MainMenu, bind);
}

//==========================================================================
//
//
//
//==========================================================================

void RefreshAccelerator(int command)
{
	TArray<CCommandBinding> & table = bindings_2d;

	for(unsigned i = 0; i<table.Size(); i++)
	{
		if (command==table[i].iCommandID) 
		{
			ZEdApp & app = wxGetApp();
			ZEdFrame * frame = app.GetFrame();
			UpdateAccelerator(frame->m_MainMenu, table[i]);
			return;
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void SetDefaultBindings()
{
	AddBinding(bindings_2d, WXK_DELETE, 0, CMD_DELETE);
	AddBinding(bindings_2d, WXK_INSERT, 0, CMD_INSERT);
	AddBinding(bindings_2d, WXK_INSERT, S_SHIFT, CMD_INSERTCLOSE);
	AddBinding(bindings_2d, 'G', 0, CMD_CHANGEGRID);
	AddBinding(bindings_2d, 'C', S_CONTROL, CMD_COPY);
	AddBinding(bindings_2d, 'X', S_CONTROL, CMD_CUT);
	AddBinding(bindings_2d, 'V', S_CONTROL, CMD_PASTE);
	AddBinding(bindings_2d, 'G', S_CONTROL, CMD_SHOWGRID);
	AddBinding(bindings_2d, WXK_BACK, S_ALT, CMD_UNDO);
	AddBinding(bindings_2d, WXK_BACK, S_CONTROL|S_ALT, CMD_REDO);
	AddBinding(bindings_2d, 'L', 0, CMD_MODE_LINEDEFS);
	AddBinding(bindings_2d, 'S', 0, CMD_MODE_SECTORS);
	AddBinding(bindings_2d, 'T', 0, CMD_MODE_THINGS);
	AddBinding(bindings_2d, 'V', 0, CMD_MODE_VERTICES);
	AddBinding(bindings_2d, WXK_SPACE, S_CONTROL, CMD_CENTERVIEW);
	AddBinding(bindings_2d, 'C', 0, CMD_CLEAR);
	AddBinding(bindings_2d, 'N', 0, CMD_NEXT);
	AddBinding(bindings_2d, 'N', S_CONTROL, CMD_NEXTJUMP);
	AddBinding(bindings_2d, 'N', S_SHIFT, CMD_NEXTSPC);
	AddBinding(bindings_2d, 'N', S_SHIFT|S_CONTROL, CMD_NEXTSPCJUMP);
	AddBinding(bindings_2d, 'P', 0, CMD_PREV);
	AddBinding(bindings_2d, 'P', S_CONTROL, CMD_PREVJUMP);
	AddBinding(bindings_2d, 'P', S_SHIFT, CMD_PREVSPC);
	AddBinding(bindings_2d, 'P', S_SHIFT|S_CONTROL, CMD_PREVSPCJUMP);
	AddBinding(bindings_2d, 'J', 0, CMD_GOTOOBJECT);
	AddBinding(bindings_2d, WXK_F11, 0, CMD_TAG);
	AddBinding(bindings_2d, WXK_F3, 0, CMD_FINDNEXT);
	AddBinding(bindings_2d, '+', 0, CMD_ZOOMIN);
	AddBinding(bindings_2d, '-', 0, CMD_ZOOMOUT);
	AddBinding(bindings_2d, WXK_RETURN, 0, CMD_PROP);
}

//==========================================================================
//
//
//
//==========================================================================

int GetCommandFromKey(int mode, int vk, int shift)
{
	TArray<CCommandBinding> & table = mode==MODE_2D? bindings_2d : bindings_3d;

	for(unsigned i = 0; i<table.Size(); i++)
	{
		if (vk==table[i].iVk && shift==table[i].iShift) return table[i].iCommandID;
	}
	return -1;
}


