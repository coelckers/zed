#ifndef __ZED_H
#define __ZED_H


#include "configfile.h"	
#include "GameConfig.h"
#include "Level.h"
#include "View2D.h"
#include "Infobars.h"
#include "doomerrors.h"

#define ZED_CAPTION "ZEd 2.2"

wxString GetConfigDir();


// Global commands
enum
{
	CMD_START=300,
	CMD_NEW=CMD_START,
	CMD_OPEN,
	CMD_SELECTIWAD,
	CMD_PREFERENCES,

	CMD_CHANGECONFIG,
	CMD_SAVE,
	CMD_SAVEAS,
	CMD_ENABLESDCOMPRESSION,
	CMD_ALWAYSSAVEASTEXT,

	CMD_LISTTHINGSINPWAD,
	CMD_LISTLINETYPESINPWAD,
	CMD_LISTUNUSEDTEXTURES,
	CMD_LISTUSEDTEXTURES,

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

	CMD_DOSNAPTOGRID,
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
	CMD_MARKTHINGSWITHTAG,

	CMD_FINDLDBYTEXTURE,
	CMD_FINDLDBYTYPE,
	CMD_FINDLDBYTAG,
	CMD_FINDLDBYTID,
	CMD_FINDLDBYLINEID,

	CMD_FINDSECBYTEXTURE,
	CMD_FINDSECBYTYPE,
	CMD_FINDSECBYTAG,
	CMD_FINDTHINGBYCURRENTTID, 
	CMD_FINDLINEBYCURRENTTAG,
	CMD_FINDLINEBYCURRENTTID,
	CMD_FINDLINEBYCURRENTLINEID,
	CMD_FINDSECTORBYCURRENTTAG,

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

	CMD_TAG,
	CMD_INSERT,
	CMD_INSERTCLOSE,
	CMD_INSERT_MENU,
	CMD_NEXT,
	CMD_NEXTJUMP,
	CMD_NEXTSPC,
	CMD_NEXTSPCJUMP,
	CMD_PREV,
	CMD_PREVJUMP,
	CMD_PREVSPC,
	CMD_PREVSPCJUMP,
	CMD_PROP,
	CMD_REDUCE,

	CMD_COPYPROP,
	CMD_PASTEPROP,
	CMD_ROTATE,
	CMD_ROTATE90L,
	CMD_ROTATE90R,
	CMD_ROTATE180,
	CMD_MIRROR_H,
	CMD_MIRROR_V,

	CMD_DELJOIN,
	CMD_MERGE,
	CMD_JOIN,
	CMD_SPLITSECTOR,
	CMD_LINEDRAWATVERT,
	CMD_LINESFROMVERTS,
	CMD_DELETEUNUSED,

	CMD_MAKEDOOR,
	CMD_MAKELIFT,
	CMD_DISTFLOOR,
	CMD_DISTCEIL,
	CMD_DISTLIGHT,
	CMD_RAISEFLOOR8,
	CMD_RAISECEIL8,
	CMD_LOWERFLOOR8,
	CMD_LOWERCEIL8,
	CMD_RAISELIGHT,
	CMD_LOWERLIGHT,
	CMD_SELECTTHINGSINSECTORS,
	CMD_SELECTLINESFROMSECTORS,
	CMD_SELECTVERTFROMSECTORS,

	CMD_SPLITLD,
	CMD_SPLITLDSEC,
	CMD_JOINSEC,
	CMD_DELSD_JOINSEC,
	CMD_FLIP,
	CMD_SWAP,
	CMD_DEL2ND,

	CMD_ALIGNX1,
	CMD_ALIGNX2,
	CMD_ALIGNX3,
	CMD_ALIGNX4,
	CMD_ALIGNX5,
	CMD_ALIGNX6,
	CMD_ALIGNX7,
	CMD_ALIGNX8,
	CMD_ALIGNY, 
	CMD_UNSELECTONE,
	CMD_UNSELECTTWO,
	CMD_SELECTVERTICESFROMLINES,

	CMD_RELOAD,
    CMD_PRINT,
	CMD_CLOSE,
    CMD_FIND,
    CMD_REPLACE,
    CMD_SELECTALL,
	CMD_COMPILE,
	CMD_TAB,

	CMD_NUMCOMMANDS
};

extern wxString CurrentConfig;
extern ConfigManager confman;
extern FConfigFile config;
extern CLevel * CurrentLevel;

class DirectorySaver
{
private:
	wxString m_olddir;
	wxString m_configkey;
	const char * m_thisdir;
public:
	const char * Dir() const { return m_thisdir; }
	DirectorySaver(const char * key);
	~DirectorySaver();
};



// Define a new frame
class ZEdFrame : public wxFrame
{
public:
	wxStatusBar * m_statusbar;
	wxToolBar * m_toolbar;
	C2DMapview * m_view;

	wxMenuBar * m_MainMenu;
	wxMenu * m_ThingMenu;
	wxMenu * m_VertexMenu;
	wxMenu * m_SectorMenu;
	wxMenu * m_LineMenu;

	ThingBar * m_ThingBar;
	LineBar * m_LineBar;
	SectorBar * m_SectorBar;
	VertexBar * m_VertexBar;
	wxWindow * m_CurrentBar;

    ZEdFrame(wxWindow *parent, const wxWindowID id, const wxString& title,
            const wxPoint& pos, const wxSize& size, const long style);

	~ZEdFrame();

    void InitToolBar(wxToolBar* toolBar);
	bool ProcessEvent(wxEvent & event);

    void OnSize(wxSizeEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnSelectIWAD(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);

	void SetStatusBarText(const char * text);
	void SetCoordinates(int x, int y);
	void SetInfoBar(wxWindow * bar);

    DECLARE_EVENT_TABLE()
};

class ZEdApp : public wxApp
{
	ZEdFrame * frame;
public:

	bool OnInit();
	int OnExit();

	void LoadConfig();
	void SaveConfig();

	ZEdFrame * GetFrame() const { return frame; }
};

DECLARE_APP(ZEdApp)


class SpinCtrlWithModify : public wxSpinCtrl
{
	bool ismodified;

public:
	SpinCtrlWithModify(wxWindow* parent, wxWindowID id = -1, const wxString& value = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxSP_ARROW_KEYS, int _min = 0, int _max = 100, int initial = 0, const wxString& name = _T("wxSpinCtrl"))
		: wxSpinCtrl(parent, id, value, pos, size, style, _min, _max, initial, name), ismodified(0)
	{
	}

 	bool IsModified() const { return ismodified; }
	virtual bool ProcessEvent(wxEvent& event);
};



enum
{
	S_CONTROL=1,
	S_SHIFT=2,
	S_ALT=4
};

enum
{
	MODE_2D,
	MODE_3D
};

int GetCommandFromKey(int mode, int vk, int shift);
void SetDefaultBindings();
void RefreshAccelerator(int command);

// Global functions to invoke the various dialogs.
// This way I can keep the class definitions local to the implementation
QWORD RunWADManager(wxWindow * parent);
wxString RunWADLibrary(wxWindow * parent);
void RunIWADSelect(wxWindow * parent);
bool GetRotation(wxWindow * parent, wxRealPoint * res);
bool Get2Numbers(wxWindow * parent, const char * caption, const char * t1, const char * t2, int min1, int max1, int min2, int max2, wxPoint * res);
bool Get2Floats(wxWindow * parent, const char * caption, const char * t1, const char * t2, double min1, double max1, double min2, double max2, wxRealPoint * res);
bool GetGrid(wxWindow * parent, wxPoint * res);
void DisplayText(wxWindow * parent, const char * caption, wxString text);
int GetGenericType(wxWindow * parent, int initial, int which);
int EditThing(wxWindow * parent, CLevel * level, CThing * thing);
int EditLine(wxWindow * parent, CLevel * level,  CLine * line);
int EditSector(wxWindow * parent, CLevel * level,  CSector * sector);
int SelectTexture(wxWindow * parent, tx_namespace nspace);
int GetSpecialArg(wxWindow * parent, int value, const wxString & name);

inline wxPen * GetPen(wxColour & color, int width = 1, int style = wxSOLID)
{
	return wxThePenList->FindOrCreatePen(color, width, style);
}

inline unsigned ComputeDist( int dx, int dy)
{
	return (unsigned) (hypot( (double) dx, (double) dy) + 0.5);
}

inline double ComputeDist(double dx, double dy)
{
	return hypot( dx, dy);
}

double distance(double x1, double y1, double x2, double y2);
double distance_to_line(double x1, double y1, double x2, double y2, double vx, double vy);
bool determine_line_side(float x1, float y1, float x2, float y2, float x, float y);



#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif


#endif
