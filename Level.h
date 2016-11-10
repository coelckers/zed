#ifndef __LEVEL_H
#define __LEVEL_H

#include "Mapdata.h"
#include "doomtype.h"
#include "View2D.h"
#include "linedraw.h"
#include "ScriptEditor.h"
#include "tarray.h"
#include "name.h"
#include "zip/zlib/zlib.h"

extern char DEF_WALLTEX[];
extern char DEF_LIFTTEX[];
extern char DEF_DOORTRAK[];
extern char DEF_DOORTEX[];
extern int DEF_FLOORHEIGHT;
extern int DEF_CEILINGHEIGHT;
extern int DEF_LIGHT;
extern char DEF_FLOORTEX[];
extern char DEF_CEILTEX[];
class CResourceFile;
class ScriptMan;

enum
{
	THF_Type=1,
	THF_Angle=2,
	THF_X=4,
	THF_Y=8,
	THF_Z=16,
	THF_Special=32,
	THF_TID=64,
	THF_Arg1=128,
	THF_Arg2=256,
	THF_Arg3=512,
	THF_Arg4=1024,
	THF_Arg5=2048,

	THF_Guard = 0x80000000
};

enum
{
	LDF_Special=1,
	LDF_Sector=2,
	LDF_Sector1=16,
	LDF_Sector2=32,
	LDF_Tag=64,
	LDF_Arg1=128,
	LDF_Arg2=256,
	LDF_Arg3=512,
	LDF_Arg4=1024,
	LDF_Arg5=2048,
	LDF_Texture1U=4096,
	LDF_Texture1M=8192,
	LDF_Texture1L=16384,
	LDF_Texture2U=32768,
	LDF_Texture2M=0x10000,
	LDF_Texture2L=0x20000,

	LDF_LiteF=0x40000,
	LDF_LiteB=0x80000,
	LDF_Alpha=0x100000,

	LDF_Guard = 0x80000000
};

enum
{
	LDF2_Scale1Ux=2,
	LDF2_Scale1Uy=4,
	LDF2_Scale1Mx=8,
	LDF2_Scale1My=0x10,
	LDF2_Scale1Lx=0x20,
	LDF2_Scale1Ly=0x40,
	LDF2_Scale2Ux=0x80,
	LDF2_Scale2Uy=0x100,
	LDF2_Scale2Mx=0x200,
	LDF2_Scale2My=0x400,
	LDF2_Scale2Lx=0x800,
	LDF2_Scale2Ly=0x1000,

	LDF2_Guard = 0x80000000
};

enum
{
	SF_Type=1,
	SF_CeilH=2,
	SF_FloorH=4,
	SF_Light=8,
	SF_Damage=16,
	SF_Secret=32,
	SF_Wind=64,
	SF_Friction=128,
	SF_CeilT=256,
	SF_FloorT=512,
	SF_Tag=1024,

	SF_FScaleX=8192,
	SF_CScaleX=16384,
	SF_FScaleY=8192,
	SF_CScaleY=16384,
	SF_CRot=32768,
	SF_FRot=65536,
	SF_CLite=0x20000,
	SF_FLite=0x40000,
	SF_Gravity=0x80000,
	SF_LiteCol=0x100000,
	SF_FadeCol=0x200000,
	SF_Desat=0x400000,

	SF_Guard = 0x80000000

};




class CClipboard
{
	TArray<CThing> m_Things;
	TArray<CVertex> m_Vertices;
	TArray<CLine> m_Lines;
	TArray<CSector> m_Sectors;

public:
	CClipboard(void);
	~CClipboard(void);

	void Clear();
	void Copy(CLevel * l);
	void Paste(CLevel * l, int x, int y);
	void DrawSelection(wxDC & DC, CLevel * l, int x, int y);
	bool ContainsData() { return !!(m_Things.Size() + m_Lines.Size()); }

};

class CCheckArray
{
	TArray<bool> array;

public:

	CCheckArray ()
	{
	}
	CCheckArray (const CCheckArray &other) : array(other.array)
	{
	}
	CCheckArray &operator= (const CCheckArray &other)
	{
		array = other.array;
		return *+this;
	}
	// Return a reference to an element
	bool &operator[] (unsigned int index)
	{
		if (index >= array.Size()) 
		{
			unsigned now = array.Size();
			array.Resize(index+1);
			for(;now<array.Size();now++)
			{
				array[now]=false;
			}
		}
		return array[index];
	}
	forceinline void Delete (unsigned int index)
	{
		array.Delete(index);
	}
	forceinline void Clear ()
	{
		array.Clear();
	}
	void Resize(int amount)
	{
		array.Resize(amount);
	}
};


enum
{
};

class CLevel : public wxEvtHandler
{
	friend class C2DMapview;
	friend class CLevelDC;
	friend class CClipboard;
	friend class CUndoBuffer;
	friend class CScriptEdit;
	friend class CCurveDialog;
	friend int CompileConversationScript(CLevel * level_dest, char * script);

	enum
	{
		modeThings=1,
		modeVertexes=2,
		modeSectors=4,
		modeLineDefs=16,
		modeLineDraw=32,
		mode3D = 64,

		modeEdit = modeThings+modeVertexes+modeSectors+modeLineDefs,
		modeNotLinedraw = modeEdit+mode3D,
		modeNot3D = modeEdit+modeLineDraw,
	};

	enum MMODE_T
	{
		MMODE_NONE=0,
		MMODE_MOVE=1,
		MMODE_MARK=2
	};


	// The global state
	static int m_Mode;
	static bool m_Compress;
	static bool m_AsText;
	static bool m_showSideMarkers;
	static bool m_GridToggled;
	static bool m_SnapToGrid;
	static bool m_GridHighlight;
	static bool m_SnapAutoToggle;
	static int m_GridSize;
	static int m_GridSizeX;
	static int m_GridSizeY;
	static int m_GridOfsX;
	static int m_GridOfsY;
	static CClipboard clipboard;



	Map map;

	TArray<char> m_Segs;
	TArray<char> m_SSectors;
	TArray<char> m_Nodes;
	TArray<char> m_Reject;
	TArray<char> m_Blockmap;


	TArray<char> m_FraggleScript;
	TArray<char> m_Behavior;
	TArray<char> m_ACSScript;
	TArray<char> m_StrifeConv;
	TArray<char> m_StrifeConvSrc;

	bool m_changed;
	bool m_Extended;
	bool m_TextMap;
	bool m_NeedNodeBuild;
	char m_Mission[9];
	CCheckArray checked, checked_save;
	TArray<int> Selection;
	int m_Selection;
	wxPoint m_ContextPoint;
	int m_temp1, m_temp2;
	QWORD m_MapLump;

	CThing m_ClipThing;
	CLine m_ClipLine;
	CSector m_ClipSector;


	TArray<CUndoBuffer*> m_undolist;
	TArray<CUndoBuffer*> m_redolist;

	bool m_drawClipboard;
	int m_ScreenCenterX, m_ScreenCenterY;
	int m_CenterX, m_CenterY;
	int m_MouseMode;
	wxRealPoint m_MousePoint, m_LastMousePoint;
	CRect m_Bounds;
	C2DMapview * m_DrawWindow;
	CScriptEdit * m_ScriptEdit[3];
	int m_drawclipx, m_drawclipy;

	float m_ZoomFactor;

	wxColour hpW,hpG;
	wxColour hpGr,hpRe;
	wxColour hpCy,hpYe;
	wxColour hpLr;

	void LoadTextMap(const char *mapname, CResourceFile *mapfile, int index);
	void ParseThing(ScriptMan &sc, CThing *th);
	void ParseVertex(ScriptMan &sc, CVertex *vt);
	void ParseLinedef(ScriptMan &sc, CLineDef *ld);
	void ParseSidedef(ScriptMan &sc, CSideDef *sd);
	void ParseSector(ScriptMan &sc, CSector *se);

	void LoadDoomHexenMap(const char *mapname, CResourceFile *mapfile, int index);
	void LoadThings(const char *lumpdata, int size);
	void LoadLines(const char *lumpdata, int size);
	void LoadSides(const char *lumpdata, int size);
	void LoadVertexes(const char *lumpdata, int size);
	void LoadSectors(const char *lumpdata, int size);

	void InitSideDef(CSideDef * me, CSideDef * copyfrom, bool side=false);

	void OnCmdStatistics(wxCommandEvent &);
	void GoToObject(int objnum);
	void CenterCursor();
	void SetPosition(int x,int y);
	void GetObjectCoords( int objnum, int *xpos, int *ypos) ;
	int GetCurObject( CRectFloat * rr);
	void OnModeThings();
	void OnModeLinedefs();
	void OnModeSectors();
	void OnModeVertices();
	void OnModeThings(wxCommandEvent & event);
	void OnModeLinedefs(wxCommandEvent & event);
	void OnModeSectors(wxCommandEvent & event);
	void OnModeVertices(wxCommandEvent & event);
	void OnUpdateModeThings(wxUpdateUIEvent & event);
	void OnUpdateModeLinedefs(wxUpdateUIEvent & event);
	void OnUpdateModeSectors(wxUpdateUIEvent & event);
	void OnUpdateModeVertices(wxUpdateUIEvent & event);
	void Select(int sel, bool what);
	void UncheckAll();
	void PushChecked();
	void PopChecked();
	void CheckLineDefsFromSectors();
	void CheckVertexesFromLineDefs();
	void OnViewZoomin(wxCommandEvent & event);
	void OnViewZoomout(wxCommandEvent & event);
	void OnCmdNext(bool shift, bool control);
	void OnNext(wxCommandEvent & event);
	void OnNextspc(wxCommandEvent & event);
	void OnNextjump(wxCommandEvent & event);
	void OnNextspcjump(wxCommandEvent & event);
	void OnCmdPrev(bool shift, bool control);
	void OnPrev(wxCommandEvent & event);
	void OnPrevspc(wxCommandEvent & event);
	void OnPrevjump(wxCommandEvent & event);
	void OnPrevspcjump(wxCommandEvent & event);
	void OnViewGoto(wxCommandEvent & event);
	void OnCmdReduceSelection(wxCommandEvent & event);
	void OnViewClearselection(wxCommandEvent & event);
	void InsertRectangle( int x0, int y0, int xpos, int ypos); 
	void OnEditInsertrectangle(wxCommandEvent & event);
	void InsertPolygon( int x0, int y0, int xpos, int ypos);
	void OnEditInsertpolygon(wxCommandEvent & event);
	int GetLDRef(int sec);
	int GetSecRef(int ld);
	void OnEditRemovezero(wxCommandEvent & event);
	void OnViewShowsidemarkers(wxCommandEvent & event);
	void OnUpdateViewShowsidemarkers(wxUpdateUIEvent & event);
	void OnViewCenterview(wxCommandEvent & event);
	void OnSnaptogrid(wxCommandEvent & event);
	void OnDistributelightlevel(wxCommandEvent & event);
	void OnRaisefloorby8mapunits(wxCommandEvent & event);
	void OnLowerfloorby8mapunits(wxCommandEvent & event);
	void OnRaiseceilingby8mapunits(wxCommandEvent & event);
	void OnLowerceilingby8mapunits(wxCommandEvent & event);
	void OnRaiselightlevel(wxCommandEvent & event);
	void OnLowerlightlevel(wxCommandEvent & event);
	void OnUnselectone(wxCommandEvent & event);
	void OnUnselecttwo(wxCommandEvent & event);
	int Deleteunusedvertices();
	void OnDeleteunusedvertices(wxCommandEvent & event);
	void OnDeleteunusedsectors(wxCommandEvent & event);
	void OnSelectthingsinsectors(wxCommandEvent & event);
	void OnSelectlinesfromsectors(wxCommandEvent & event);
	void OnSelectverticesfromsectors(wxCommandEvent & event);
	void OnSelectverticesfromlines(wxCommandEvent & event);
	void OnSelectlinesfromvertices(wxCommandEvent & event);
	void OnJoinsectors(wxCommandEvent & event);
	void OnEditCopy(wxCommandEvent & event);
	void OnEditCut(wxCommandEvent & event);
	void OnEditPaste(wxCommandEvent & event);
	void OnUpdateEditCopyCut(wxUpdateUIEvent & event);
	void OnUpdateEditPaste(wxUpdateUIEvent & event);

	void OnEditEditacsscript(wxCommandEvent & event);
	void OnEditEditfragglescript(wxCommandEvent & event);
	void OnEditEditconversationscript(wxCommandEvent & event);
	void OnUpdateEditEditacsscript(wxUpdateUIEvent & event);
	void OnUpdateEditEditfragglescript(wxUpdateUIEvent & event);
	void OnUpdateEditEditconversationscript(wxUpdateUIEvent & event);

	// Level-Save
	void CompressSidedefs(TArray<CSideDef> & sds);
	void SaveAsWad(const char * fn);
	void OnFileSaveas(wxCommandEvent & event);
	void OnFileEnablesidedefcompression(wxCommandEvent & event);
	void OnUpdateFileEnablesidedefcompression(wxUpdateUIEvent & event);
	void OnFileSaveAsText(wxCommandEvent & event);
	void OnUpdateFileSaveAsText(wxUpdateUIEvent & event);

	void SaveAsText(const char * fn);

	wxString SaveVerticesAsText();
	wxString SaveThingsAsText();
	wxString SaveLinesAsText();
	wxString SaveSidesAsText(TArray<CSideDef> &compressed_sds);
	wxString SaveSectorsAsText();

	// Level-Coord
	int SX(double x);
	int SY(double y);
	double MX(int x);
	double MY(int y);
	bool IsLineDefInside( int cel, double x0, double y0, double x1, double y1);
	bool IsSectorInside(int ces,double x0,double y0,double x1,double y1);
	int GetThingFromPos(CRectFloat * lpr);
	int GetVertexFromPos(CRectFloat * lpr);
	int	GetVertexAtPoint(wxPoint point);
	int GetVertexFromPoint(wxPoint lpt);
	int GetLineDefFromPos(CRectFloat * r);
	int GetSectorFromPos(wxRealPoint p);
	void GetAttachedLines(int vt, TArray<int> & list);

	// Level-Draw
	void MapLine(wxDC & DC, wxColour col, CVertex * v1, CVertex * v2, int pensize=1);
	void MapLine(wxDC & DC, wxColour col, wxPoint * v1,wxPoint * v2, int pensize=1);
	void MapLine(wxDC & DC, wxColour col, double x1,double y1,double x2,double y2, int pensize=1);
	void DrawLinedef(wxDC & DC, wxColour col, CVertex * v1,CVertex * v2, int pensize=1);
	void MapEllipse(wxDC & DC, wxColour col, double x1,double y1,double x2,double y2);
	void MapRect(wxDC & DC, wxColour col, double x1,double y1,double x2,double y2);
	void DrawMapArrow(wxDC & hDC, wxColour col, double scrXstart, double scrYstart, unsigned angle);
	void DrawMapVector(wxDC & hDC, wxColour col, int scrXstart, int scrYstart, int scrXend, int scrYend);
	void HighlightLineDef(wxDC & DC, int line, wxColour col, bool hilighttags);
	void HighlightSector(wxDC & DC, int s_index, wxColour col, bool hilighttags);
	void MarkThing(wxDC & DC,CThing * pth, wxColour col, bool hilighttags);
	void MarkVertex(wxDC & DC,CVertex * pv,bool CheckOrSelect);
	void FillSector(wxDC & DC,int s_index,bool swapxy, bool toprinter, wxColour color);
	void DrawMarkRect(wxDC & DC,wxRealPoint from,wxRealPoint to);
	void DrawThing(wxDC & DC, CThing * pth, double x, double y, bool halfbright=false);
	void OnDrawThings(wxDC & DC,CRectFloat rcClip);
	void OnDrawVertexes(wxDC & DC,CRectFloat rcClip);
	void OnDrawSectors(wxDC & DC,CRectFloat rcClip);
	void OnDrawLineDefs(wxDC & DC,CRectFloat rcClip);
	void OnDraw(wxDC &, CRectFloat * r);

	// Level-Mouse
	void ContextMenu(wxMouseEvent & event);
	void OnLButtonUp(wxMouseEvent & event);
	void OnLButtonDown(wxMouseEvent & event);
	void OnLButtonThings(wxDC & DC,const wxRealPoint & p);
	void OnLButtonVertexes(wxDC & DC,const wxRealPoint & p);
	void OnLButtonSectors(wxDC & DC,const wxRealPoint & p);
	void OnLButtonLineDefs(wxDC & DC,const wxRealPoint & p);
	bool OnMouseMove(wxMouseEvent & event);
	void OnMMoveThings(wxDC & DC, const wxRealPoint & p);
	void OnMMoveVertexes(wxDC & DC, const wxRealPoint & p);
	void OnMMoveSectors(wxDC & DC, const wxRealPoint & p);
	void OnMMoveLineDefs(wxDC & DC,const wxRealPoint & p);
	void OnMMoveMarkThings(wxDC & DC);
	void OnMMoveMarkVertexes(wxDC & DC);
	void OnMMoveMarkSectors(wxDC & DC);
	void OnMMoveMarkLineDefs(wxDC & DC);
	void OnMMoveMoveThings(wxDC & DC,int xoff,int yoff);
	void OnMMoveMoveVertexes(wxDC & DC,int xoff,int yoff);

	//Level-Undo
	void ChangeUndoMenu();
	void MakeBackup(const char * action, bool vertices, bool lines, bool sectors, bool things);
	void RemoveBackup();
	void OnEditUndo(wxCommandEvent & event);
	void OnUpdateEditUndo(wxUpdateUIEvent & event);
	void OnEditRedo(wxCommandEvent & event);
	void OnUpdateEditRedo(wxUpdateUIEvent & event);
	void Revert();

	// Level-Insert
	void OnCmdDoInsert(bool shift, wxPoint pt);
	void OnCmdInsert(wxCommandEvent & event);
	void OnCmdInsertClose(wxCommandEvent & event);
	void OnCmdInsertM(wxCommandEvent & event);
	void InsertLinesFromVertices(bool shift);
	void InsertSectorsFromLines();
	int InsertThing(CThing * copything, double xpos, double ypos); 
	int InsertVertex(double xpos, double ypos, bool nosnap=false); 
	int InsertLineDef(CLine * copyfrom);
	int InsertSector(CSector * copyfrom);
	void InsertPolygonVertices( int centerx, int centery, int sides, int radius);
	int SplitLine(int l, int vertex);
	void CheckForLineSplit(int refv, bool force);

	// Level-Delete
	void OnCmdDelete(wxCommandEvent & event);
	void OnCmdDeleteThings();
	void OnCmdDeleteVertexes();
	void DeleteOneVertex(int i);
	void OnCmdDeleteLineDefs();
	void DeleteOneLineDef(int no);
	void OnCmdDeleteSectors();
	void DeleteOneSector(int no);

	// CurveDialog
	void SetCurve(int numverts, float distance, int angle, bool circle);
	void OnCurvelinedefs(wxCommandEvent & event);
	void OnUpdateCurvelinedefs(wxUpdateUIEvent & event);

	// Level-Obj
	int GetCurSector(int x0, int y0, int x1, int y1);
	bool IsLDInSector(int ld, bool paranoid);
	int GetOppositeSector(int ld1, bool firstside,int * pBest=NULL, bool paranoid=false);
	void OnCmdTag(wxCommandEvent & event);
	void OnCmdJoinLD(wxCommandEvent & event);
	void OnCmdJoinLD2(wxCommandEvent & event);
	void DeleteVerticesJoinLineDefs(int vert);
	void OnCmdMergeVertices(wxCommandEvent & event);
	void MergeVertices(int vtlist[],int cvtlist); 
	void OnCmdSplitSector(wxCommandEvent & event);
	void SplitSector(int v1,int v2); 
	void OnCmdSplitLinedefs(wxCommandEvent & event);
	void SplitLineDefs( int ld);
	void OnCmdSplitLinedefsAndSector(wxCommandEvent & event);
	void SplitLineDefsAndSector(int L1,int L2);
	void OnCmdDeleteLinedefsJoinSectors(wxCommandEvent & event);
	void DeleteLineDefsJoinSectors(int ld); 
	void OnCmdFlipLinedefs(wxCommandEvent & event);
	void OnCmdSwapSidedefs(wxCommandEvent & event);
	void FlipLineDefs(int lineno, bool swapvertices);
	void OnCmdDel2nd(wxCommandEvent & event);
	void DeleteSecondSideDef(int ln);
	void OnCmdAlignX(int type_sd,int type_tex,int type_off);
	void OnAlignSide1Tex(wxCommandEvent & event);
	void OnAlignSide1TexOfs(wxCommandEvent & event);
	void OnAlignSide1(wxCommandEvent & event);
	void OnAlignSide1Ofs(wxCommandEvent & event);
	void OnAlignSide2Tex(wxCommandEvent & event);
	void OnAlignSide2TexOfs(wxCommandEvent & event);
	void OnAlignSide2(wxCommandEvent & event);
	void OnAlignSide2Ofs(wxCommandEvent & event);
	void AlignTexturesX(int * sdlist,int csdlist,int type_sd,int type_tex,int type_off);
	void OnAlignY(wxCommandEvent & event);
	int GetTextureRefHeight(CLine * line, int side);
	void OnCmdMakeDoor(wxCommandEvent & event);
	void MakeDoor(int sectorno); 
	void OnCmdMakeLift(wxCommandEvent & event);
	void MakeLift(int secno);
	void OnCmdDistFloor(wxCommandEvent & event);
	void OnCmdDistCeil(wxCommandEvent & event);
	void OnCmdRotate(wxCommandEvent & event);
	void OnRotateselection90L(wxCommandEvent & event);
	void OnRotateselection90R(wxCommandEvent & event);
	void OnRotateselection180(wxCommandEvent & event);
	void OnMirrorselectionhorizontally(wxCommandEvent & event);
	void OnMirrorselectionvertically(wxCommandEvent & event);
	void RotateAndScaleObjects(double angle, double scalex, double scaley);

	// Level-Statistics
	void OnEditListallusedtextures(wxCommandEvent & event);
	void OnEditListLDT(wxCommandEvent & event);
	void OnEditListallusedsectortags(wxCommandEvent & event);
	void OnEditListthings(wxCommandEvent & event);
	void DoExtendedMapStatistics(unsigned short * statP,int mode);
	void OnEditItemstatistics(wxCommandEvent & event);

	// Level-Check
	void MarkPoly(int vertex);
	void CheckSectors(bool paranoid);
	void OnCmdSectorsClosed(wxCommandEvent & event);
	void OnCmdSectorsClosedParanoid(wxCommandEvent & event);
	void CheckCrossReferences();
	void OnCmdXRef(wxCommandEvent & event);
	void CheckTextures( void); 
	void OnCmdMissingTextures(wxCommandEvent & event);
	void CheckTextureNames( void); 
	void OnCmdTextureNames(wxCommandEvent & event);
	void CheckAll(bool paranoid);
	void OnCmdCheckAll(wxCommandEvent & event);
	void OnCmdCheckAllParanoid(wxCommandEvent & event);
	bool DoLevelCheck(wxProgressDialog & prog, int checkmode, int currentlev, int maxlev);
	void CheckPWAD(int mode);
	void OnCmdCheckAllPWAD(wxCommandEvent & event);
	void OnCmdCheckTexturesPWAD(wxCommandEvent & event);
	void OnCmdCheckAllPWADParanoid(wxCommandEvent & event);

	// Level-Properties

	void OnCmdProperties(wxCommandEvent & event);
	void OnCmdPropertiesThings();
	void OnCmdPropertiesVertexes();
	void OnCmdPropertiesSectors();
	void OnCmdPropertiesLineDefs();
	void OnCopyproperties(wxCommandEvent & event);
	void OnPasteproperties(wxCommandEvent & event);
	void OnUpdateCopyproperties(wxUpdateUIEvent & event);
	void OnUpdatePasteproperties(wxUpdateUIEvent & event);

	// Level-Find
	void OnToolsFindlinedefbytexture(wxCommandEvent & event);
	void OnToolsFindsectorbytexture(wxCommandEvent & event);
	void OnToolsFindthingbyspecial(wxCommandEvent & event);
	void OnToolsFindlinedefbyspecial(wxCommandEvent & event);
	void OnToolsFindsectorbytype(wxCommandEvent & event);
	void OnToolsFindthingbytype(wxCommandEvent & event);

	void OnToolsFindthingbytid(wxCommandEvent & event);
	void OnToolsFindlinedefbytag(wxCommandEvent & event);
	void OnToolsFindlinedefbytid(wxCommandEvent & event);
	void OnToolsFindlinedefbylineid(wxCommandEvent & event);
	void OnToolsFindsectorbytag(wxCommandEvent & event);
	void OnFindRepeat(wxCommandEvent & event);

	void OnUpdateToolsFindthingbytype(wxUpdateUIEvent & event);
	void OnUpdateToolsFindthingbytid(wxUpdateUIEvent & event);
	void OnUpdateToolsFindlinedefextended(wxUpdateUIEvent & event);
	void OnUpdateToolsFindlinedef(wxUpdateUIEvent & event);
	void OnUpdateToolsFindsector(wxUpdateUIEvent & event);

	void CLevel::OnToolsFindthingbycurrenttid(wxCommandEvent & event);
	void CLevel::OnToolsFindlinedefbycurrenttag(wxCommandEvent & event);
	void CLevel::OnToolsFindlinedefbycurrenttid(wxCommandEvent & event);
	void CLevel::OnToolsFindlinedefbycurrentlineid(wxCommandEvent & event);
	void CLevel::OnToolsFindsectorbycurrenttag(wxCommandEvent & event);

	void rename_x(const wxArrayString & rename, char * buf, int type);
	void OnEditRenametextures(wxCommandEvent & event);
	void OnToolsChangethingtypes(wxCommandEvent & event);
	void OnToolsMarkThingsWithTag(wxCommandEvent & event);


	// Level-Linedraw

	rect_t l_getrect(int l);
	int	v_getvertatpoint(wxPoint point);
	void v_getattachedlines(int v, numlist_t* list);
	bool lines_clockwise(numlist_t *lines);
	wxPoint v_getpoint(int v)	;
	int l_split(int l, int vertex);
	int add_line(int v1, int v2);
	bool v_isattached_sector(int v);
	void l_setsector(int l, int side, int sector);
	bool check_split(int split_sector);
	int determine_sector(double x, double y);
	int get_nearest_line_2(double x, double y);
	bool determine_line_side(int line, float x, float y);
	int check_vertex_split(DWORD vertex);
	int check_vertex_split(wxPoint p);
	void v_merge(int v1, int v2);
	void v_mergespot(int x, int y);
	void merge_verts();
	bool v_checkspot(int x, int y);
	int get_side_sector(int line, int side);
	bool l_needsuptex(int l, int side);
	bool l_needsmidtex(int l, int side);
	bool l_needslotex(int l, int side);
	void l_setdeftextures(int l);

	void CancelLineDraw();
	bool LineDrawNotAllowed();
	void AbortLineDraw();
	void DeleteLineDrawVertex();
	void DrawLineDraw(wxDC & DC,CRectFloat * rcClip);
	void OnMMoveLineDraw(wxDC & DC, wxRealPoint pt);
	void OnLButtonLineDraw(wxDC & DC, wxRealPoint pt);
	void OnLinedrawStartAtVertex(wxCommandEvent & event);
	void OnToolsStartlinedrawing(wxCommandEvent & event);
	void OnUpdateNoLinedraw(wxUpdateUIEvent & event);

	void OnWadListallusedlinedefandsectortypesinpwad(wxCommandEvent & event);
	void OnWadListallusedthingsincurrentpwad(wxCommandEvent & event);
	void OnUnusedTextures(wxCommandEvent & event);
	void OnUsedTextures(wxCommandEvent & event);


	void SaveGrid();
	void OnEditTogglegrid(wxCommandEvent & event);
	void OnUpdateEditTogglegrid(wxUpdateUIEvent & event);
	void OnViewSnaptogrid(wxCommandEvent & event);
	void OnUpdateViewSnaptogrid(wxUpdateUIEvent & event);
	void OnViewHighlight64unitgrid(wxCommandEvent & event);
	void OnUpdateViewHighlight64unitgrid(wxUpdateUIEvent & event);
	void OnViewAuto(wxCommandEvent & event);
	void OnUpdateViewAuto(wxUpdateUIEvent & event);
	void OnEditChangegrid(wxCommandEvent & event);
	void OnViewSetgrid(wxCommandEvent & event);
public:
	void SnapToGrid(CVertex *v);
	void SnapToGrid(CThing *v);
	void SnapToGrid(double * x, double * y);
	void SnapToGrid(short * x, short * y);
	void SnapToGrid(int * x, int * y);
	void DrawGrid(wxDC & DC,CRectFloat * lpr);

	int GetGridSizeX()
	{
		return m_GridToggled ? m_GridSizeX:1;
	}
	int GetGridSizeY()
	{
		return m_GridToggled ? m_GridSizeY:1;
	}

public:

	int m_currentWAD;
	
	static void LoadSettings();

	CLevel(bool extended);
	~CLevel(void);
	void Load(QWORD lump, const char *);
	forceinline bool Changed() { return m_changed; }
	forceinline bool Extended() { return m_Extended; }
	forceinline bool TextMap() { return m_TextMap; }

	int FindFreeTid();
	int FindFreeTag(); 
	int FindFreeLineId() ;

	void OnViewCenterview() {}
	void UpdateStatusBar();


	forceinline CVertex * StVt(CLine * ln)
	{
		return GetVertex(ln->line.Start);
	}

	forceinline CVertex * EnVt(CLine * ln)
	{
		return GetVertex(ln->line.End);
	}

	forceinline CVertex * StVt(CLine & ln)
	{
		return GetVertex(ln.line.Start);
	}

	forceinline CVertex * EnVt(CLine & ln)
	{
		return GetVertex(ln.line.End);
	}

	forceinline CVertex * StVt(int li)
	{
		return GetVertex(GetLine(li)->line.Start);
	}

	forceinline CVertex * EnVt(int li)
	{
		return GetVertex(GetLine(li)->line.End);
	}

	forceinline int StVtNo(CLine * li)
	{
		return li->line.Start;
	}

	forceinline int EnVtNo(CLine * li)
	{
		return li->line.End;
	}

	forceinline int StVtNo(int li)
	{
		return GetLine(li)->line.Start;
	}

	forceinline int EnVtNo(int li)
	{
		return GetLine(li)->line.End;
	}

	forceinline CRectFloat LineRect(CLine & ln)
	{
		CRectFloat r;
		r.Set(StVt(&ln)->X(), StVt(&ln)->Y(),EnVt(&ln)->X(), EnVt(&ln)->Y());
		return r;
	}
	forceinline CRectFloat LineRect(int num)
	{
		return LineRect(*GetLine(num));
	}

	forceinline CSector * FrontSec(CLine * ln)
	{
		if (!ln || ln->sides[0].sector==-1) return NULL;
		else return GetSector(ln->sides[0].sector);
	}

	forceinline CSector * BackSec(CLine * ln)
	{
		if (!ln || ln->sides[1].sector==-1) return NULL;
		else return GetSector(ln->sides[1].sector);
	}

	forceinline CSector * FrontSec(int ln)
	{
		return FrontSec(GetLine(ln));
	}

	forceinline CSector * BackSec(int ln)
	{
		return BackSec(GetLine(ln));
	}

	forceinline int FrontSecNo(CLine * ln)
	{
		return ln->sides[0].sector;
	}

	forceinline int BackSecNo(CLine * ln)
	{
		return ln->sides[1].sector;
	}

	forceinline int FrontSecNo(int ln)
	{
		return GetLine(ln)->sides[0].sector;
	}

	forceinline int BackSecNo(int ln)
	{
		return GetLine(ln)->sides[1].sector;
	}

	forceinline CThing * GetThing(int index)
	{
		return map.GetThing(index);
	}
		
	forceinline CSector * GetSector(int index)
	{
		return map.GetSector(index);
	}

	forceinline CVertex * GetVertex(int index)
	{
		return map.GetVertex(index);
	}

	forceinline CLine * GetLine(int index)
	{
		return map.GetLine(index);
	}

	public:

	forceinline int NumThings()
	{
		return map.NumThings();
	}

	forceinline int NumSectors()
	{
		return map.NumSectors();
	}

	forceinline int NumVertices()
	{
		return map.NumVertices();
	}

	forceinline int NumLines()
	{
		return map.NumLines();
	}


	DECLARE_EVENT_TABLE()
};



class CLevelDC : public wxClientDC
{
	wxBrush br;
public:
	CLevelDC(CLevel * level) : wxClientDC(level->m_DrawWindow),
							   br(wxColour(255,255,255), wxTRANSPARENT)
	{
		wxSize p = level->m_DrawWindow->GetClientSize();
		level->m_ScreenCenterX=p.x>>1;
		level->m_ScreenCenterY=p.y>>1;
		SetBrush(br);
	}
};


class CUndoBuffer
{
	friend class CLevel;

	CLevel * target;
	const char * m_descript;

	void * m_Vertices;
	void * m_Things;
	void * m_Lines;
	void * m_Sectors;
	void * m_Properties;

	unsigned int m_nVertices;
	unsigned int m_nThings;
	unsigned int m_nLines;
	unsigned int m_nSectors;
	unsigned int m_nProperties;
	unsigned int m_cVertices;
	unsigned int m_cThings;
	unsigned int m_cLines;
	unsigned int m_cSectors;
	unsigned int m_cProperties;

	int ProcessBuffer(void * s_data, void ** bufpAddr, int buflen);

public:
	CUndoBuffer(CLevel * t, const char * description, bool vertices, bool lines, bool sectors, bool things);
	~CUndoBuffer();
	void Apply();

	bool ChangeVertices() { return m_Vertices!=NULL; }
	bool ChangeLines() { return m_Lines!=NULL; }
	bool ChangeSectors() { return m_Sectors!=NULL; }
	bool ChangeThings() { return m_Things!=NULL; }
};


#endif