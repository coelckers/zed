//
//-----------------------------------------------------------------------------
//
// Copyright (C) 1995-2005 Christoph Oelckers
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
// Main code file for level data maintenance
//


#include "StdAfx.h"
#include "ResourceFile.h"
#include "Level.h"
#include "ZEd.h"
#include "name.h"



//==========================================================================
//
//
//
//==========================================================================

static bool CheckName(const char * level,const char * section, const char * lump_name, bool printerror = true)
{
	if (!lump_name)
	{
		if (printerror)
			I_Error("%s: Unable to find %s.",level,section);
		else
			return false;
	}
	if (stricmp(section, lump_name))
	{
		if (printerror)
			I_Error("%s: %s is not correctly named. Unable to load level",level,section);
		else
			return false;
	}
	return true;
}


//==========================================================================
//
//
//
//==========================================================================

int CLevel::m_Mode=modeThings;
bool CLevel::m_showSideMarkers;
bool CLevel::m_Compress=false;
bool CLevel::m_AsText=false;
bool CLevel::m_GridToggled=true;
bool CLevel::m_SnapToGrid=true;
bool CLevel::m_GridHighlight=true;
bool CLevel::m_SnapAutoToggle=true;
int CLevel::m_GridSize=128;
int CLevel::m_GridSizeX=128;
int CLevel::m_GridSizeY=128;
int CLevel::m_GridOfsX=0;
int CLevel::m_GridOfsY=0;
CClipboard CLevel::clipboard;


//==========================================================================
//
//
//
//==========================================================================

BEGIN_EVENT_TABLE(CLevel, wxEvtHandler)
	EVT_MENU(CMD_UNDO, CLevel::OnEditUndo)
	EVT_MENU(CMD_REDO, CLevel::OnEditRedo)
	EVT_MENU(CMD_STATISTICS, CLevel::OnCmdStatistics)
	EVT_UPDATE_UI(CMD_UNDO, CLevel::OnUpdateEditUndo)
	EVT_UPDATE_UI(CMD_REDO, CLevel::OnUpdateEditRedo)
	EVT_MENU(CMD_MODE_LINEDEFS, CLevel::OnModeLinedefs)
	EVT_UPDATE_UI(CMD_MODE_LINEDEFS, CLevel::OnUpdateModeLinedefs)
	EVT_MENU(CMD_MODE_SECTORS, CLevel::OnModeSectors)
	EVT_UPDATE_UI(CMD_MODE_SECTORS, CLevel::OnUpdateModeSectors)
	EVT_MENU(CMD_MODE_VERTICES, CLevel::OnModeVertices)
	EVT_UPDATE_UI(CMD_MODE_VERTICES, CLevel::OnUpdateModeVertices)
	EVT_MENU(CMD_MODE_THINGS, CLevel::OnModeThings)
	EVT_UPDATE_UI(CMD_MODE_THINGS, CLevel::OnUpdateModeThings)
	EVT_MENU(CMD_ZOOMIN, CLevel::OnViewZoomin)
	EVT_MENU(CMD_ZOOMOUT, CLevel::OnViewZoomout)
	EVT_MENU(CMD_NEXT, CLevel::OnNext)
	EVT_MENU(CMD_NEXTSPC, CLevel::OnNextspc)
	EVT_MENU(CMD_NEXTJUMP, CLevel::OnNextjump)
	EVT_MENU(CMD_NEXTSPCJUMP, CLevel::OnNextspcjump)
	EVT_MENU(CMD_PREV, CLevel::OnPrev)
	EVT_MENU(CMD_PREVSPC, CLevel::OnPrevspc)
	EVT_MENU(CMD_PREVJUMP, CLevel::OnPrevjump)
	EVT_MENU(CMD_PREVSPCJUMP, CLevel::OnPrevspcjump)
	EVT_MENU(CMD_GOTOOBJECT, CLevel::OnViewGoto)
	EVT_MENU(CMD_REDUCE, CLevel::OnCmdReduceSelection)
	EVT_MENU(CMD_CLEAR, CLevel::OnViewClearselection)
	EVT_MENU(CMD_INSERT, CLevel::OnCmdInsert)
	EVT_MENU(CMD_INSERTCLOSE, CLevel::OnCmdInsertClose)
	EVT_MENU(CMD_INSERT_MENU, CLevel::OnCmdInsertM)
	EVT_MENU(CMD_DELETE, CLevel::OnCmdDelete)
	EVT_MENU(CMD_TAG , CLevel::OnCmdTag)
	EVT_MENU(CMD_DELJOIN , CLevel::OnCmdJoinLD)
	EVT_MENU(CMD_JOIN , CLevel::OnCmdJoinLD2)
	EVT_MENU(CMD_MERGE , CLevel::OnCmdMergeVertices)
	EVT_MENU(CMD_SPLITSECTOR , CLevel::OnCmdSplitSector)
	EVT_MENU(CMD_SPLITLD , CLevel::OnCmdSplitLinedefs)
	EVT_MENU(CMD_SPLITLDSEC , CLevel::OnCmdSplitLinedefsAndSector)
	EVT_MENU(CMD_DELSD_JOINSEC , CLevel::OnCmdDeleteLinedefsJoinSectors)
	EVT_MENU(CMD_FLIP , CLevel::OnCmdFlipLinedefs)
	EVT_MENU(CMD_SWAP , CLevel::OnCmdSwapSidedefs)
	EVT_MENU(CMD_DEL2ND , CLevel::OnCmdDel2nd)
	EVT_MENU(CMD_ALIGNX1 , CLevel::OnAlignSide1Tex)
	EVT_MENU(CMD_ALIGNX2 , CLevel::OnAlignSide1TexOfs)
	EVT_MENU(CMD_ALIGNX3 , CLevel::OnAlignSide1)
	EVT_MENU(CMD_ALIGNX4 , CLevel::OnAlignSide1Ofs)
	EVT_MENU(CMD_ALIGNX5 , CLevel::OnAlignSide2Tex)
	EVT_MENU(CMD_ALIGNX6 , CLevel::OnAlignSide2TexOfs)
	EVT_MENU(CMD_ALIGNX7 , CLevel::OnAlignSide2)
	EVT_MENU(CMD_ALIGNX8 , CLevel::OnAlignSide2Ofs)
	EVT_MENU(CMD_ALIGNY , CLevel::OnAlignY)
	EVT_MENU(CMD_MAKEDOOR , CLevel::OnCmdMakeDoor)
	EVT_MENU(CMD_MAKELIFT , CLevel::OnCmdMakeLift)
	EVT_MENU(CMD_DISTFLOOR , CLevel::OnCmdDistFloor)
	EVT_MENU(CMD_DISTCEIL , CLevel::OnCmdDistCeil)
	EVT_MENU(CMD_ROTATE , CLevel::OnCmdRotate)
	EVT_MENU(CMD_ROTATE90L , CLevel::OnRotateselection90L)
	EVT_MENU(CMD_ROTATE90R , CLevel::OnRotateselection90R)
	EVT_MENU(CMD_ROTATE180 , CLevel::OnRotateselection180)
	EVT_MENU(CMD_MIRROR_H , CLevel::OnMirrorselectionhorizontally)
	EVT_MENU(CMD_MIRROR_V , CLevel::OnMirrorselectionvertically)

	EVT_MENU(CMD_INSERTRECT, CLevel::OnEditInsertrectangle)
	EVT_MENU(CMD_INSERTPOLY, CLevel::OnEditInsertpolygon)
	EVT_MENU(CMD_REMOVEZEROLINES, CLevel::OnEditRemovezero)
	EVT_MENU(CMD_SIDEMARKERS, CLevel::OnViewShowsidemarkers)
	EVT_UPDATE_UI(CMD_SIDEMARKERS, CLevel::OnUpdateViewShowsidemarkers)
	EVT_MENU(CMD_CENTERVIEW, CLevel::OnViewCenterview)
	EVT_MENU(CMD_DOSNAPTOGRID, CLevel::OnSnaptogrid)
	EVT_MENU(CMD_DISTLIGHT, CLevel::OnDistributelightlevel)
	EVT_MENU(CMD_RAISEFLOOR8, CLevel::OnRaisefloorby8mapunits)
	EVT_MENU(CMD_LOWERFLOOR8, CLevel::OnLowerfloorby8mapunits)
	EVT_MENU(CMD_RAISECEIL8, CLevel::OnRaiseceilingby8mapunits)
	EVT_MENU(CMD_LOWERCEIL8, CLevel::OnLowerceilingby8mapunits)
	EVT_MENU(CMD_RAISELIGHT, CLevel::OnRaiselightlevel)
	EVT_MENU(CMD_LOWERLIGHT, CLevel::OnLowerlightlevel)
	EVT_MENU(CMD_UNSELECTONE, CLevel::OnUnselectone)
	EVT_MENU(CMD_UNSELECTTWO, CLevel::OnUnselecttwo)
	EVT_MENU(CMD_DELETEUNUSEDVERTICES, CLevel::OnDeleteunusedvertices)
	EVT_MENU(CMD_DELETEUNUSEDSECTORS, CLevel::OnDeleteunusedsectors)
	EVT_MENU(CMD_SELECTTHINGSINSECTORS, CLevel::OnSelectthingsinsectors)
	EVT_MENU(CMD_SELECTLINESFROMSECTORS, CLevel::OnSelectlinesfromsectors)
	EVT_MENU(CMD_SELECTVERTFROMSECTORS, CLevel::OnSelectverticesfromsectors)
	EVT_MENU(CMD_SELECTVERTICESFROMLINES, CLevel::OnSelectverticesfromlines)
	EVT_MENU(CMD_LINESFROMVERTS, CLevel::OnSelectlinesfromvertices)
	EVT_MENU(CMD_JOINSEC, CLevel::OnJoinsectors)
	EVT_MENU(CMD_COPY, CLevel::OnEditCopy)
	EVT_MENU(CMD_CUT, CLevel::OnEditCut)
	EVT_MENU(CMD_PASTE, CLevel::OnEditPaste)
	EVT_UPDATE_UI(CMD_COPY, CLevel::OnUpdateEditCopyCut)
	EVT_UPDATE_UI(CMD_CUT, CLevel::OnUpdateEditCopyCut)
	EVT_UPDATE_UI(CMD_PASTE, CLevel::OnUpdateEditPaste)
	EVT_MENU(CMD_LISTTEXTURES, CLevel::OnEditListallusedtextures)
	EVT_MENU(CMD_LISTLINETYPES, CLevel::OnEditListLDT)
	EVT_MENU(CMD_LISTTAGS, CLevel::OnEditListallusedsectortags)
	EVT_MENU(CMD_LISTTHINGS, CLevel::OnEditListthings)
	EVT_MENU(CMD_ITEMSTATISTICS, CLevel::OnEditItemstatistics)
	EVT_MENU(CMD_SAVEAS, CLevel::OnFileSaveas)
	EVT_MENU(CMD_ENABLESDCOMPRESSION, CLevel::OnFileEnablesidedefcompression)
	EVT_UPDATE_UI(CMD_ENABLESDCOMPRESSION, CLevel::OnUpdateFileEnablesidedefcompression)
	EVT_MENU(CMD_ALWAYSSAVEASTEXT, CLevel::OnFileSaveAsText)
	EVT_UPDATE_UI(CMD_ALWAYSSAVEASTEXT, CLevel::OnUpdateFileSaveAsText)
	EVT_MENU(CMD_SECTORCLOSED, CLevel::OnCmdSectorsClosed)
	EVT_MENU(CMD_SECTORCLOSED2, CLevel::OnCmdSectorsClosedParanoid)
	EVT_MENU(CMD_XREF, CLevel::OnCmdXRef)
	EVT_MENU(CMD_MISSINGTEX, CLevel::OnCmdMissingTextures)
	EVT_MENU(CMD_TEXNAMES, CLevel::OnCmdTextureNames)
	EVT_MENU(CMD_CHECKALL, CLevel::OnCmdCheckAll)
	EVT_MENU(CMD_CHECKALL2, CLevel::OnCmdCheckAllParanoid)
	EVT_MENU(CMD_CHECKALLPWAD, CLevel::OnCmdCheckAllPWAD)
	EVT_MENU(CMD_CHECKTEXPWAD, CLevel::OnCmdCheckTexturesPWAD)
	EVT_MENU(CMD_CHECKALLPWADP, CLevel::OnCmdCheckAllPWADParanoid)
	//EVT_MENU(CMD_LINEDRAW, CLevel::OnToolsStartlinedrawing)
	//EVT_MENU(CMD_LINEDRAWATVERT, CLevel::OnLinedrawStartAtVertex)
	/*
	EVT_UPDATE_UI(CMD_LINEDRAW, CLevel::OnUpdateNoLinedraw)
	EVT_UPDATE_UI(CMD_SAVEAS, CLevel::OnUpdateNoLinedraw)
	EVT_UPDATE_UI(CMD_DELETEUNUSEDVERTICES, CLevel::OnUpdateNoLinedraw)
	EVT_UPDATE_UI(CMD_DELETEUNUSEDSECTORS, CLevel::OnUpdateNoLinedraw)
	EVT_UPDATE_UI(CMD_REMOVEZEROLINES, CLevel::OnUpdateNoLinedraw)
	EVT_UPDATE_UI(CMD_INSERTRECT, CLevel::OnUpdateNoLinedraw)
	EVT_UPDATE_UI(CMD_INSERTPOLY, CLevel::OnUpdateNoLinedraw)
	EVT_UPDATE_UI(CMD_RENAMETEX , CLevel::OnUpdateNoLinedraw)
	EVT_UPDATE_UI(CMD_SECTORCLOSED , CLevel::OnUpdateNoLinedraw)
	EVT_UPDATE_UI(CMD_SECTORCLOSED2 , CLevel::OnUpdateNoLinedraw)
	EVT_UPDATE_UI(CMD_XREF , CLevel::OnUpdateNoLinedraw)
	EVT_UPDATE_UI(CMD_MISSINGTEX  , CLevel::OnUpdateNoLinedraw)
	EVT_UPDATE_UI(CMD_TEXNAMES , CLevel::OnUpdateNoLinedraw)
	EVT_UPDATE_UI(CMD_GOTOOBJECT , CLevel::OnUpdateNoLinedraw)
	*/

	EVT_MENU(CMD_LISTLINETYPESINPWAD, CLevel::OnWadListallusedlinedefandsectortypesinpwad)
	EVT_MENU(CMD_LISTTHINGSINPWAD, CLevel::OnWadListallusedthingsincurrentpwad)
	EVT_MENU(CMD_LISTUNUSEDTEXTURES, CLevel::OnUnusedTextures)
	EVT_MENU(CMD_LISTUSEDTEXTURES, CLevel::OnUsedTextures)

	EVT_MENU(CMD_SHOWGRID, CLevel::OnEditTogglegrid)
	EVT_UPDATE_UI(CMD_SHOWGRID, CLevel::OnUpdateEditTogglegrid)
	EVT_MENU(CMD_SNAPTOGRID, CLevel::OnViewSnaptogrid)
	EVT_UPDATE_UI(CMD_SNAPTOGRID, CLevel::OnUpdateViewSnaptogrid)
	EVT_MENU(CMD_HIGHLIGHTGRID, CLevel::OnViewHighlight64unitgrid)
	EVT_UPDATE_UI(CMD_HIGHLIGHTGRID, CLevel::OnUpdateViewHighlight64unitgrid)
	EVT_MENU(CMD_AUTOGRID, CLevel::OnViewAuto)
	EVT_UPDATE_UI(CMD_AUTOGRID, CLevel::OnUpdateViewAuto)
	EVT_MENU(CMD_CHANGEGRID, CLevel::OnEditChangegrid)
	EVT_MENU(CMD_SETGRID, CLevel::OnViewSetgrid)

	EVT_MENU(CMD_PROP, CLevel::OnCmdProperties)
	EVT_MENU(CMD_COPYPROP, CLevel::OnCopyproperties)
	EVT_MENU(CMD_PASTEPROP, CLevel::OnPasteproperties)
	EVT_UPDATE_UI(CMD_COPYPROP, CLevel::OnUpdateCopyproperties)
	EVT_UPDATE_UI(CMD_PASTEPROP, CLevel::OnUpdatePasteproperties)

	EVT_MENU(CMD_FINDLDBYTEXTURE, CLevel::OnToolsFindlinedefbytexture)
	EVT_MENU(CMD_FINDSECBYTEXTURE, CLevel::OnToolsFindsectorbytexture)
	EVT_MENU(CMD_FINDLDBYTYPE, CLevel::OnToolsFindlinedefbyspecial)
	EVT_MENU(CMD_FINDTHINGBYSPECIAL, CLevel::OnToolsFindthingbyspecial)
	EVT_MENU(CMD_FINDSECBYTYPE, CLevel::OnToolsFindsectorbytype)
	EVT_MENU(CMD_FINDTHINGBYTYPE, CLevel::OnToolsFindthingbytype)
	EVT_MENU(CMD_FINDTHINGBYTID, CLevel::OnToolsFindthingbytid)
	EVT_MENU(CMD_FINDLDBYTAG, CLevel::OnToolsFindlinedefbytag)
	EVT_MENU(CMD_FINDLDBYTID, CLevel::OnToolsFindlinedefbytid)
	EVT_MENU(CMD_FINDLDBYLINEID, CLevel::OnToolsFindlinedefbylineid)
	EVT_MENU(CMD_FINDSECBYTAG, CLevel::OnToolsFindsectorbytag)
	EVT_MENU(CMD_FINDNEXT, CLevel::OnFindRepeat)

	//EVT_UPDATE_UI(CMD_FINDNEXT            , CLevel::OnUpdateNoLinedraw)
	EVT_UPDATE_UI(CMD_FINDTHINGBYTYPE     , CLevel::OnUpdateToolsFindthingbytype)
	EVT_UPDATE_UI(CMD_FINDTHINGBYTID      , CLevel::OnUpdateToolsFindthingbytid)
	EVT_UPDATE_UI(CMD_FINDTHINGBYSPECIAL  , CLevel::OnUpdateToolsFindthingbytid)
	EVT_UPDATE_UI(CMD_FINDLDBYTEXTURE, CLevel::OnUpdateToolsFindlinedef)
	EVT_UPDATE_UI(CMD_FINDLDBYTYPE, CLevel::OnUpdateToolsFindlinedef)
	EVT_UPDATE_UI(CMD_FINDLDBYTAG    , CLevel::OnUpdateToolsFindlinedef)
	EVT_UPDATE_UI(CMD_FINDLDBYTID    , CLevel::OnUpdateToolsFindlinedefextended)
	EVT_UPDATE_UI(CMD_FINDLDBYLINEID    , CLevel::OnUpdateToolsFindlinedefextended)
	EVT_UPDATE_UI(CMD_FINDSECBYTEXTURE , CLevel::OnUpdateToolsFindsector)
	EVT_UPDATE_UI(CMD_FINDSECBYTYPE    , CLevel::OnUpdateToolsFindsector)
	EVT_UPDATE_UI(CMD_FINDSECBYTAG     , CLevel::OnUpdateToolsFindsector)

	EVT_MENU(CMD_FINDTHINGBYCURRENTTID, CLevel::OnToolsFindthingbycurrenttid)
	EVT_MENU(CMD_FINDLINEBYCURRENTTAG, CLevel::OnToolsFindlinedefbycurrenttag)
	EVT_MENU(CMD_FINDLINEBYCURRENTTID, CLevel::OnToolsFindlinedefbycurrenttid)
	EVT_MENU(CMD_FINDLINEBYCURRENTLINEID, CLevel::OnToolsFindlinedefbycurrentlineid)
	EVT_MENU(CMD_FINDSECTORBYCURRENTTAG, CLevel::OnToolsFindsectorbycurrenttag)

	//EVT_UPDATE_UI(CMD_CHANGETHINGTYPE    , CLevel::OnUpdateNoLinedraw)
	//EVT_UPDATE_UI(CMD_MARKTHINGSWITHTAG    , CLevel::OnUpdateNoLinedraw)

	EVT_MENU(CMD_RENAMETEX, CLevel::OnEditRenametextures)
	EVT_MENU(CMD_CHANGETHINGTYPE, CLevel::OnToolsChangethingtypes)
	EVT_MENU(CMD_MARKTHINGSWITHTAG, CLevel::OnToolsMarkThingsWithTag)

	EVT_MENU(CMD_EDITACS, CLevel::OnEditEditacsscript)
	EVT_MENU(CMD_EDITFS, CLevel::OnEditEditfragglescript)
	EVT_MENU(CMD_EDITCONV, CLevel::OnEditEditconversationscript)
	EVT_UPDATE_UI(CMD_EDITACS, CLevel::OnUpdateEditEditacsscript)
	EVT_UPDATE_UI(CMD_EDITFS, CLevel::OnUpdateEditEditfragglescript)
	EVT_UPDATE_UI(CMD_EDITCONV, CLevel::OnUpdateEditEditconversationscript)

	EVT_MENU(CMD_CURVE, CLevel::OnCurvelinedefs)
	EVT_UPDATE_UI(CMD_CURVE, CLevel::OnUpdateCurvelinedefs)

END_EVENT_TABLE()

//==========================================================================
//
//
//
//==========================================================================

void CLevel::LoadSettings()
{
	if (config.SetSection("Settings"))
	{
		m_showSideMarkers = !!config.GetIntValueForKey("Sidemarkers");
		m_GridToggled = !!config.GetIntValueForKey("GridOn");
		m_SnapToGrid = !!config.GetIntValueForKey("GridSnap");
		m_Compress = !!config.GetIntValueForKey("CompressSD");
		m_GridSize = config.GetIntValueForKey("GridSize");
		m_GridSizeX = config.GetIntValueForKey("GridSizeX");
		m_GridSizeY = config.GetIntValueForKey("GridSizeY");
		m_GridOfsX = config.GetIntValueForKey("GridOfsX");
		m_GridOfsY = config.GetIntValueForKey("GridOfsY");
		m_SnapAutoToggle = !!config.GetIntValueForKey("AutoSnap");
		if (m_GridSize==0) m_GridSize=128;
		if (m_GridSizeX==0) m_GridSizeX = m_GridSize;
		if (m_GridSizeY==0) m_GridSizeY = m_GridSize;
	}
}


//==========================================================================
//
//
//
//==========================================================================

CLevel::CLevel(bool extended) :

		hpW(255,255,255),
		hpG(128,128,128),
		hpGr(0,255,0),
		hpYe(255,255,0),
		hpCy(0,255,255),
		hpRe(255,128,192),
		hpLr(255,128,128)

{
	m_Mission[0]=0;
	m_Extended=extended;
	m_ScreenCenterX = m_ScreenCenterY = 0;
	m_CenterX = m_CenterY = 0;
	m_ZoomFactor=60;
	m_DrawWindow=NULL;
	m_Selection=0;
	m_MouseMode=MMODE_NONE;
	m_drawClipboard=false;
	m_NeedNodeBuild=false;
	m_drawclipx=m_drawclipy=0;
	m_currentWAD=0;
	m_TextMap = false;
	m_MapLump = 0;

	// Initialize the property clipboard
	memset(&m_ClipThing, 0, sizeof(CThing));
	m_ClipThing.type  = 1;
	m_ClipThing.Flags.SetShort(0x07);

	memset(&m_ClipLine, 0, sizeof(CLine));
	InitSideDef(&m_ClipLine.sides[0], NULL);
	InitSideDef(&m_ClipLine.sides[1], NULL);

	memset(&m_ClipSector,0,sizeof(CSector));
	m_ClipSector.floorh = DEF_FLOORHEIGHT;
	m_ClipSector.ceilh = DEF_CEILINGHEIGHT;
	strncpy( m_ClipSector.floort, DEF_FLOORTEX, 8);
	strncpy( m_ClipSector.ceilt, DEF_CEILTEX, 8);
	m_ClipSector.light = DEF_LIGHT;
	m_ScriptEdit[Script_ACS]=m_ScriptEdit[Script_FS]=m_ScriptEdit[Script_Conv]=NULL;

	checked.Clear();
	m_changed=false;

}

//==========================================================================
//
//
//
//==========================================================================

CLevel::~CLevel(void)
{
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::Load(QWORD lump, const char * mapname)
{
	int size;
	const char * lump_name;
	char * maplumpdata;
	CResourceFile *mapfile;
	int mapindex;
	bool closemapfile;

	m_MapLump = lump;
	strncpy(m_Mission, mapname, 8);
	strupr(m_Mission);
	m_Mission[8]=0;

	lump_name = CResourceFile::GlobalGetFullLumpName(lump);
	if (lump_name && !strnicmp(lump_name, "maps/", 5))
	{
		size = CResourceFile::GlobalGetLumpSize(lump);
		maplumpdata = (char*)CResourceFile::GlobalReadLump(lump);
		if (size < 16 || (memcmp(maplumpdata, "IWAD", 4) && memcmp(maplumpdata, "PWAD", 4)))
		{
			I_Error("%s: Not a valid map.", mapname);
		}

		mapfile = new CWADFile(mapname, maplumpdata);
		mapindex = 0;
		closemapfile = true;
		delete [] maplumpdata;
	}
	else
	{
		mapfile =CResourceFile::GetResourceFile(int(lump>>32)-1);
		mapindex = int(lump&0x7fffffff);
		closemapfile = false;
	}

	try
	{
		if (CheckName(mapname, "THINGS", mapfile->GetLumpName(mapindex+1), false))
		{
			LoadDoomHexenMap(mapname, mapfile, mapindex);
		}
		else if (CheckName(mapname, "TEXTMAP", mapfile->GetLumpName(mapindex+1), false))
		{
			cgc=confman.GetConfig(CurrentConfig+".cfg");
			m_TextMap = true;
			LoadTextMap(mapname, mapfile, mapindex);
		}
		else if (strstr(lump_name, ".map"))
		{
			cgc = confman.GetConfig(CurrentConfig + ".cfg");
			m_TextMap = true;
			LoadBuildMap(mapname, mapfile, mapindex);
		}
		else
		{
			I_Error("%s: Not a valid map.", mapname);
		}
	}
	catch (CRecoverableError &)
	{
		if (closemapfile) delete mapfile;
		throw;
	}
	if (closemapfile) delete mapfile;
	m_changed=false;
}

void CLevel::LoadTextMap(const char* mapname, CResourceFile* mapfile, int index)
{
	try
	{
		m_FraggleScript.Resize(mapfile->GetLumpSize(index));
		mapfile->ReadLump(index, &m_FraggleScript[0]);
		index++;

		char* textlump = (char*)mapfile->ReadLump(index);
		LoadTextMap(textlump, mapfile->GetLumpSize(index));
	}
	catch (...)
	{
	}
}

void ConvertBuildMap(FileReader& fr, FileWriter* fw);

void CLevel::LoadBuildMap(const char* mapname, CResourceFile* mapfile, int index)
{
	try
	{
		m_FraggleScript.Resize(0);

		char* textlump = (char*)mapfile->ReadLump(index);
		MemoryReader fr(textlump, mapfile->GetLumpSize(index));
		BufferWriter fw;
		ConvertBuildMap(fr, &fw);
		auto buf = fw.GetBuffer();
		LoadTextMap((char*)buf->Data(), buf->Size());
	}
	catch (...)
	{
	}
}

void CLevel::LoadTextMap(char* textlump, int size)
{
	TArray<CSideDef> sides;
	try
	{
		m_Bounds.bottom = m_Bounds.right = -32767;
		m_Bounds.top = m_Bounds.left = 32767;

		ScriptMan sc("{}=;");
		sc.SC_OpenMem("textmap", textlump, size);
		while (sc.SC_GetString())
		{
			if (sc.SC_Compare("namespace"))
			{
				sc.SC_MustGetStringName("=");
				sc.SC_MustGetString();
				if (sc.SC_Compare("ZDoom") || sc.SC_Compare("Hexen"))
				{
					m_Extended = true;
				}
				else if (sc.SC_Compare("ZDoomTranslated"))
				{
				}
				else
				{
					// unknown
				}
				sc.SC_MustGetStringName(";");
			}
			else if (sc.SC_Compare("thing"))
			{
				CThing *th = map.AddThing();
				ParseThing(sc, th);
			}
			else if (sc.SC_Compare("linedef"))
			{
				CLine *li = map.AddLine();
				li->line.Sidedef1 = li->line.Sidedef2 = NO_SIDE;
				ParseLinedef(sc, &li->line);
			}
			else if (sc.SC_Compare("sidedef"))
			{
				CSideDef sd;
				InitSideDef(&sd, NULL, true);
				ParseSidedef(sc, &sd);
				sides.Push(sd);
			}
			else if (sc.SC_Compare("sector"))
			{
				CSector *se = map.AddSector();
				ParseSector(sc, se);
			}
			else if (sc.SC_Compare("vertex"))
			{
				CVertex *vt = map.AddVertex();
				ParseVertex(sc, vt);
			}
			else
			{
				sc.SC_ScriptError("Unknown global key '%s'", sc.sc_String);
			}
		}
		for(int i=0;i<NumLines();i++)
		{
			CLineDef *l = &map.GetLine(i)->line;
			if (l->Sidedef1 != NO_SIDE) map.GetLine(i)->sides[0] = sides[l->Sidedef1];
			else map.GetLine(i)->sides[0].sector = -1;
			if (l->Sidedef2 != NO_SIDE) map.GetLine(i)->sides[1] = sides[l->Sidedef2];
			else map.GetLine(i)->sides[1].sector = -1;
		}
	}
	catch(...)
	{
		throw;
	}
}


void CLevel::ParseThing(ScriptMan &sc, CThing *th)
{
	int flagval;
	th->Flags.Clear();
	sc.SC_MustGetStringName("{");
	while (!sc.SC_CheckString("}"))
	{
		sc.SC_MustGetString();
		FName key = sc.sc_String;
		sc.SC_MustGetStringName("=");
		sc.SC_MustGetToken();
		switch(key)
		{
		case NAME_Id:
			th->thingid = sc.tkInt(key);
			break;
		case NAME_X:
			th->SetX(sc.tkFloat(key), true);
			break;
		case NAME_Y:
			th->SetY(sc.tkFloat(key), true);
			break;
		case NAME_Height:
			th->SetZ(sc.tkFloat(key), true);
			break;
		case NAME_Angle:
			th->angle = sc.tkInt(key);
			break;
		case NAME_Type:
			th->type = sc.tkInt(key);
			break;
		case NAME_Special:
			th->special = sc.tkInt(key);
			break;
		case NAME_Arg0:
		case NAME_Arg1:
		case NAME_Arg2:
		case NAME_Arg3:
		case NAME_Arg4:
			th->args[int(key)-int(NAME_Arg0)] = sc.tkInt(key);
			break;
		case NAME_Comment:
			th->comment = sc.tkString(key);
			break;

		default:
			if (cgc && (flagval = cgc->CheckTextMapThingFlag(key))>=0)
			{
				if (sc.tkBool(key))
					th->Flags.SetBit(flagval);
				else
					th->Flags.ClearBit(flagval);
			}
			else
			{
				// Unknown property
#ifdef PROPS
				th->props[key] = value;
#endif
			}
		}
		sc.SC_MustGetStringName(";");
	}
}

void CLevel::ParseVertex(ScriptMan &sc, CVertex *vt)
{
	sc.SC_MustGetStringName("{");
	while (!sc.SC_CheckString("}"))
	{
		sc.SC_MustGetString();
		FName key = sc.sc_String;
		sc.SC_MustGetStringName("=");
		sc.SC_MustGetToken();
		switch(key)
		{
		case NAME_X:
			vt->SetX(sc.tkFloat(key), true);
			if (vt->X()<m_Bounds.left) m_Bounds.left=vt->X();
			if (vt->X()>m_Bounds.right) m_Bounds.right=vt->X();
			break;
		case NAME_Y:
			vt->SetY(sc.tkFloat(key), true);
			if (vt->Y()<m_Bounds.top) m_Bounds.top=vt->Y();
			if (vt->Y()>m_Bounds.bottom) m_Bounds.bottom=vt->Y();
			break;

		default:
			// I will only implement generic vertex properties if something needs them!
			//vt->props[key] = value;
			;
		}
		sc.SC_MustGetStringName(";");
	}
}

void CLevel::ParseLinedef(ScriptMan &sc, CLineDef *ld)
{
	int flagval;
	ld->Flags.Clear();
	ld->tag=-1;
	sc.SC_MustGetStringName("{");
	while (!sc.SC_CheckString("}"))
	{
		sc.SC_MustGetString();
		FName key = sc.sc_String;
		sc.SC_MustGetStringName("=");
		sc.SC_MustGetToken();
		switch(key)
		{
		case NAME_V1:
			ld->Start = sc.tkInt(key);
			break;

		case NAME_V2:
			ld->End = sc.tkInt(key);
			break;

		case NAME_Special:
			ld->type = sc.tkInt(key);
			break;

		case NAME_Id:
			ld->tag = sc.tkInt(key);
			break;

		case NAME_Sidefront:
			ld->Sidedef1 = sc.tkInt(key);
			break;

		case NAME_Sideback:
			ld->Sidedef2 = sc.tkInt(key);
			break;

		case NAME_Arg0:
		case NAME_Arg1:
		case NAME_Arg2:
		case NAME_Arg3:
		case NAME_Arg4:
			ld->args[int(key)-int(NAME_Arg0)] = sc.tkInt(key);
			break;

		case NAME_Comment:
			ld->comment = sc.tkString(key);
			break;

		case NAME_Alpha:
			ld->alpha = sc.tkFloat(key);
			break;

		case NAME_Renderstyle:
			if (sc.sc_Token == Tk_String)
			{
				if (sc.SC_Compare("add"))
				{
					int bit = cgc->CheckTextMapLineFlag("!renderstyle");
					if (bit != -1) ld->Flags.SetBit(bit);
				}
			}
			break;

		default:
			if (cgc && (flagval = cgc->CheckTextMapLineFlag(key))>=0)
			{
				if (sc.tkBool(key))
					ld->Flags.SetBit(flagval);
				else
					ld->Flags.ClearBit(flagval);
			}
			else
			{
				// Unknown property
#ifdef PROPS
				ld->props[key] = value;
#endif
			}
		}
		sc.SC_MustGetStringName(";");
	}
}

void CLevel::ParseSidedef(ScriptMan &sc, CSideDef *sd)
{
	int flagval;

	sc.SC_MustGetStringName("{");
	while (!sc.SC_CheckString("}"))
	{
		sc.SC_MustGetString();
		FName key = sc.sc_String;
		sc.SC_MustGetStringName("=");
		sc.SC_MustGetToken();
		switch(key)
		{
		case NAME_Offsetx:
			sd->xoff = sc.tkInt(key);
			break;

		case NAME_Offsety:
			sd->yoff = sc.tkInt(key);
			break;

		case NAME_Texturetop:
			sc.tkCopyString(key, sd->texUpper, 8);
			break;

		case NAME_Texturebottom:
			sc.tkCopyString(key, sd->texLower, 8);
			break;

		case NAME_Texturemiddle:
			sc.tkCopyString(key, sd->texNormal, 8);
			break;

		case NAME_Sector:
			sd->sector = sc.tkInt(key);
			break;

		case NAME_Comment:
			sd->comment = sc.tkString(key);
			break;

		case NAME_offsetx_top:
			sd->texOffset[0][X] = sc.tkFloat(key);
			break;

		case NAME_offsety_top:
			sd->texOffset[0][Y] = sc.tkFloat(key);
			break;

		case NAME_offsetx_mid:
			sd->texOffset[1][X] = sc.tkFloat(key);
			break;

		case NAME_offsety_mid:
			sd->texOffset[1][Y] = sc.tkFloat(key);
			break;

		case NAME_offsetx_bottom:
			sd->texOffset[2][X] = sc.tkFloat(key);
			break;

		case NAME_offsety_bottom:
			sd->texOffset[2][Y] = sc.tkFloat(key);
			break;

		case NAME_scalex_top:
			sd->texScale[0][X] = sc.tkFloat(key);
			break;

		case NAME_scaley_top:
			sd->texScale[0][Y] = sc.tkFloat(key);
			break;

		case NAME_scalex_mid:
			sd->texScale[1][X] = sc.tkFloat(key);
			break;

		case NAME_scaley_mid:
			sd->texScale[1][Y] = sc.tkFloat(key);
			break;

		case NAME_scalex_bottom:
			sd->texScale[2][X] = sc.tkFloat(key);
			break;

		case NAME_scaley_bottom:
			sd->texScale[2][Y] = sc.tkFloat(key);
			break;

		case NAME_light:
			sd->light = sc.tkInt(key);
			break;

		default:
			if ((flagval = cgc->CheckTextMapSideFlag(key))>=0)
			{
				if (sc.tkBool(key))
					sd->Flags.SetBit(flagval);
				else
					sd->Flags.ClearBit(flagval);
			}
			else
			{
				// Unknown property
#ifdef PROPS
				ld->props[key] = value;
#endif
			}
		}
		sc.SC_MustGetStringName(";");
	}
}

void CLevel::ParseSector(ScriptMan &sc, CSector *se)
{
	int flagval;
	se->Init();
	se->light = 160;
	sc.SC_MustGetStringName("{");
	while (!sc.SC_CheckString("}"))
	{
		sc.SC_MustGetString();
		FName key = sc.sc_String;
		sc.SC_MustGetStringName("=");
		sc.SC_MustGetToken();
		switch(key)
		{
		case NAME_Heightfloor:
			se->floorh = (int)sc.tkFloat(key);
			break;

		case NAME_Heightceiling:
			se->ceilh = (int)sc.tkFloat(key);
			break;

		case NAME_Texturefloor:
			sc.tkCopyString(key, se->floort, 8);
			break;

		case NAME_Textureceiling:
			sc.tkCopyString(key, se->ceilt, 8);
			break;

		case NAME_Lightlevel:
			se->light = sc.tkInt(key);
			break;

		case NAME_Special:
			se->special = sc.tkInt(key);
			break;

		case NAME_Id:
			se->tag = sc.tkInt(key);
			break;

		case NAME_Comment:
			se->comment = sc.tkString(key);
			break;

		case NAME_Xpanningfloor:
			se->offset[FLOOR][X] = sc.tkFloat(key);
			break;

		case NAME_Ypanningfloor:
			se->offset[FLOOR][Y] = sc.tkFloat(key);
			break;

		case NAME_Xpanningceiling:
			se->offset[CEILING][X] = sc.tkFloat(key);
			break;

		case NAME_Ypanningceiling:
			se->offset[CEILING][Y] = sc.tkFloat(key);
			break;

		case NAME_Xscalefloor:
			se->offset[FLOOR][X] = sc.tkFloat(key);
			break;

		case NAME_Yscalefloor:
			se->offset[FLOOR][Y] = sc.tkFloat(key);
			break;

		case NAME_Xscaleceiling:
			se->scale[CEILING][X] = sc.tkFloat(key);
			break;

		case NAME_Yscaleceiling:
			se->scale[CEILING][Y] = sc.tkFloat(key);
			break;

		case NAME_Rotationfloor:
			se->rotation[FLOOR] = sc.tkFloat(key);
			break;

		case NAME_Rotationceiling:
			se->rotation[CEILING] = sc.tkFloat(key);
			break;

		case NAME_Lightfloor:
			se->planelight[FLOOR] = sc.tkInt(key);
			break;

		case NAME_Lightceiling:
			se->planelight[CEILING] = sc.tkInt(key);
			break;

		case NAME_Gravity:
			se->gravity = sc.tkFloat(key);
			break;


		default:
			if ((flagval = cgc->CheckTextMapLineFlag(key))>=0)
			{
				if (sc.tkBool(key))
					se->Flags.SetBit(flagval);
				else
					se->Flags.ClearBit(flagval);
			}
			else
			{
				// Unknown property
#ifdef PROPS
				ld->props[key] = value;
#endif
			}
		}
		sc.SC_MustGetStringName(";");
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::LoadDoomHexenMap(const char *mapname, CResourceFile *mapfile, int index)
{
	char *thingslump = NULL;
	char *lineslump = NULL;
	char *sideslump = NULL;
	char *vertexeslump = NULL;
	char *sectorslump = NULL;
	int thingssize, linessize, sidessize, vertexessize, sectorssize;

	m_TextMap = false;
	try
	{
		m_FraggleScript.Resize(mapfile->GetLumpSize(index));
		mapfile->ReadLump(index, &m_FraggleScript[0]);
		index++;

		CheckName(mapname, "THINGS", mapfile->GetLumpName(index));
		thingslump = (char*)mapfile->ReadLump(index);
		thingssize = mapfile->GetLumpSize(index);
		index++;

		CheckName(mapname, "LINEDEFS", mapfile->GetLumpName(index));
		lineslump = (char*)mapfile->ReadLump(index);
		linessize = mapfile->GetLumpSize(index);
		index++;

		CheckName(mapname, "SIDEDEFS", mapfile->GetLumpName(index));
		sideslump = (char*)mapfile->ReadLump(index);
		sidessize = mapfile->GetLumpSize(index);
		index++;

		CheckName(mapname, "VERTEXES", mapfile->GetLumpName(index));
		vertexeslump = (char*)mapfile->ReadLump(index);
		vertexessize = mapfile->GetLumpSize(index);
		index++;

		if (CheckName(mapname, "SEGS", mapfile->GetLumpName(index), false))
		{
			m_Segs.Resize(mapfile->GetLumpSize(index));
			mapfile->ReadLump(index, &m_Segs[0]);
			index++;
		}

		if (CheckName(mapname, "SSECTORS", mapfile->GetLumpName(index), false))
		{
			m_SSectors.Resize(mapfile->GetLumpSize(index));
			mapfile->ReadLump(index, &m_SSectors[0]);
			index++;
		}

		if (CheckName(mapname, "NODES", mapfile->GetLumpName(index), false))
		{
			m_Nodes.Resize(mapfile->GetLumpSize(index));
			mapfile->ReadLump(index, &m_Nodes[0]);
			index++;
		}

		CheckName(mapname, "SECTORS", mapfile->GetLumpName(index));
		sectorslump = (char*)mapfile->ReadLump(index);
		sectorssize = mapfile->GetLumpSize(index);
		index++;

		if (CheckName(mapname, "REJECT", mapfile->GetLumpName(index), false))
		{
			m_Reject.Resize(mapfile->GetLumpSize(index));
			mapfile->ReadLump(index, &m_Reject[0]);
			index++;
		}

		if (CheckName(mapname, "BLOCKMAP", mapfile->GetLumpName(index), false))
		{
			m_Blockmap.Resize(mapfile->GetLumpSize(index));
			mapfile->ReadLump(index, &m_Blockmap[0]);
			index++;
		}

		if (CheckName(mapname, "BEHAVIOR", mapfile->GetLumpName(index), false))
		{
			m_Behavior.Resize(mapfile->GetLumpSize(index));
			mapfile->ReadLump(index, &m_Behavior[0]);
			index++;
			m_Extended = true;

			const char *lump_name = mapfile->GetLumpName(index);
			if (lump_name && !strnicmp(lump_name, "SCRIPT", 6))
			{
				m_ACSScript.Resize(mapfile->GetLumpSize(index));
				mapfile->ReadLump(index, &m_ACSScript[0]);
				index++;
			}

		}
		if (m_Mission[0]=='M' && m_Mission[1]=='A' && m_Mission[2]=='P')
		{
			char namebuf[9];
			sprintf(namebuf, "SCRIPT%c%c", m_Mission[3], m_Mission[4]);
			if (CheckName(mapname, namebuf, mapfile->GetLumpName(index), false))
			{
				m_StrifeConv.Resize(mapfile->GetLumpSize(index));
				mapfile->ReadLump(index, &m_StrifeConv[0]);
				index++;
			}
			if (CheckName(mapname, "CONVSCPT", mapfile->GetLumpName(index), false))
			{
				m_StrifeConvSrc.Resize(mapfile->GetLumpSize(index));
				mapfile->ReadLump(index, &m_StrifeConvSrc[0]);
				index++;
			}
		}

		LoadThings(thingslump, thingssize);
		LoadLines(lineslump, linessize);
		LoadSides(sideslump, sidessize);
		LoadVertexes(vertexeslump, vertexessize);
		LoadSectors(sectorslump, sectorssize);
	}
	catch (CRecoverableError &)
	{
		if (thingslump) free(thingslump);
		if (lineslump) free(lineslump);
		if (sideslump) free(sideslump);
		if (vertexeslump) free(vertexeslump);
		if (sectorslump) free(sectorslump);
		throw;
	}

	if (thingslump) free(thingslump);
	if (lineslump) free(lineslump);
	if (sideslump) free(sideslump);
	if (vertexeslump) free(vertexeslump);
	if (sectorslump) free(sectorslump);
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::LoadThings(const char *lumpdata, int size)
{
	if (!m_Extended) 
	{
		int count = size / sizeof(CThing1);
		CThing1 * thinglump = (CThing1 *)lumpdata;
		for(int i=0; i<count; i++) 
		{
			CThing * th = map.AddThing();
			*th = thinglump[i];
		}
	}
	else 
	{
		int count = size / sizeof(CThing2);
		CThing2 * thinglump = (CThing2 *)lumpdata;
		for(int i=0; i<count; i++) 
		{
			CThing * th = map.AddThing();
			*th = thinglump[i];
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::LoadLines(const char *lumpdata, int size)
{
	if (!m_Extended) 
	{
		int count = size / sizeof(CLineDef1);
		CLineDef1 * ldlump = (CLineDef1 *)lumpdata;
		for(int i=0; i<count; i++) 
		{
			CLine * ln = map.AddLine();
			ln->line = ldlump[i];
			ln->extended=false;
		}
	}
	else 
	{
		int count = size / sizeof(CLineDef2);
		CLineDef2 * ldlump = (CLineDef2 *)lumpdata;
		for(int i=0; i<count; i++) 
		{
			CLine * ln = map.AddLine();
			ln->line = ldlump[i];
			ln->extended=false;
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::LoadSides(const char *lumpdata, int size)
{
	CSideDef1 * sdlump = (CSideDef1 *)lumpdata;

	// This copies the sidedefs into the linedefs and 
	// at the same time decompresses them.
	// Sidedefs are not treated as separate data in the editor.
	for(int i=0; i<NumLines(); i++)
	{
		CLine * ln = GetLine(i);
		if (ln->line.Sidedef1!=NO_SIDE)
		{
			ln->sides[0]=sdlump[ln->line.Sidedef1];
		}
		else InitSideDef(&ln->sides[0], NULL);

		if (ln->line.Sidedef2!=NO_SIDE)
		{
			ln->sides[1]=sdlump[ln->line.Sidedef2];
		}
		else InitSideDef(&ln->sides[1], NULL, true);

		ln->buggy=false;
		ln->ispolyobj=false;
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::LoadVertexes(const char *lumpdata, int size)
{
	CMapVertex * vtlump = (CMapVertex *)lumpdata;
	int vtcount = size / sizeof(CMapVertex);

	m_Bounds.bottom=m_Bounds.right=-32767;
	m_Bounds.top=m_Bounds.left=32767;
	for(int i=0;i<vtcount;i++)
	{
		CVertex * v = map.AddVertex();
		*v = vtlump[i];
		if (vtlump[i].x<m_Bounds.left) m_Bounds.left=vtlump[i].x;
		if (vtlump[i].x>m_Bounds.right) m_Bounds.right=vtlump[i].x;
		if (vtlump[i].y<m_Bounds.top) m_Bounds.top=vtlump[i].y;
		if (vtlump[i].y>m_Bounds.bottom) m_Bounds.bottom=vtlump[i].y;
	}

	int realvertices=0;
	for(int i=0;i<NumLines();i++)
	{
		CLine * ln = GetLine(i);
		if (ln->line.Start>realvertices) realvertices=ln->line.Start;
		if (ln->line.End>realvertices) realvertices=ln->line.End;
	}
	if (realvertices>=vtcount)
	{
		I_Error("This map references more vertices than present!");
	}
	else
	{
		// strip all vertices added during the node build.
		map.StripVertices(realvertices+1);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::LoadSectors(const char *lumpdata, int size)
{
	int count = size / sizeof(CSector1);
	CSector1 * sclump = (CSector1 *)lumpdata;
	for(int i=0; i<count; i++) 
	{
		CSector *sp = map.AddSector();
		*sp = sclump[i];
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::UpdateStatusBar()
{
	if (m_Mode == modeThings)
	{
		wxGetApp().GetFrame()->m_ThingBar->Update(m_Selection, GetThing(m_Selection), m_TextMap);
	}
	else if (m_Mode == modeVertexes)
	{
		wxGetApp().GetFrame()->m_VertexBar->Update(m_Selection, GetVertex(m_Selection));
	}
	else if (m_Mode == modeLineDefs)
	{
		wxGetApp().GetFrame()->m_LineBar->Update(m_Selection, this, GetLine(m_Selection));
	}
	else if (m_Mode == modeSectors)
	{
		wxGetApp().GetFrame()->m_SectorBar->Update(m_Selection, GetSector(m_Selection));
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::InitSideDef(CSideDef * me, CSideDef * copyfrom, bool side)
{
	if (copyfrom)
	{
		*me=*copyfrom;
	}
	else
	{
		me->Init();
		me->xoff = 0;
		me->yoff = 0;
		strncpy( me->texUpper, "-",8);
		strncpy( me->texLower, "-",8);
		strncpy( me->texNormal, side? "-" : DEF_WALLTEX,8);
		me->sector = NumSectors() - 1;
	}
	m_changed=true;
	//UpdateStatusBar();
}



//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdStatistics(wxCommandEvent &)
{
	int i;
	int sides=0;

	for(i=0;i<NumLines();i++)
	{
		if (FrontSecNo(i)!=-1) sides++;
		if (BackSecNo(i)!=-1) sides++;
	}

	wxMessageBox(wxString::Format(
		"Number of Things:\t\t%d (%d Bytes)\n"
		"Number of Vertexes:\t%d (%d Bytes)\n"
		"Number of Sectors:\t\t%d (%d Bytes)\n"
		"Number of LineDefs:\t%d (%d Bytes)\n"
		"Number of SideDefs:\t%d (%d Bytes)\n"
		"",
		NumThings(), NumThings() * (m_Extended? sizeof(CThing1) : sizeof(CThing2)),
		NumVertices(), NumVertices() * sizeof(CVertex),
		NumSectors(), NumSectors() * sizeof(CSector1),
		NumLines(), NumLines() *  (m_Extended? sizeof(CLineDef1) : sizeof(CLineDef2)),
		sides, sides * sizeof(CSideDef1)
		), ZED_CAPTION);
}



//==========================================================================
//
// center the map around the object and zoom in if necessary
//
//==========================================================================

void CLevel::OnViewGoto(wxCommandEvent & event)
{
	char * gti_objecttype;
	int gti_maxobject;

	switch(m_Mode)
	{
	case modeThings:
		gti_objecttype="Thing";
		gti_maxobject=NumThings();
		break; 
	case modeVertexes:
		gti_objecttype="Vertex";
		gti_maxobject=NumVertices();
		break; 
	case modeLineDefs:
		gti_objecttype="LineDef";
		gti_maxobject=NumLines();
		break; 
	case modeSectors:
		gti_objecttype="Sector";
		gti_maxobject=NumSectors();
		break; 

	default:
		return;
	}

	long retval = wxGetNumberFromUser(
				wxString::Format("Please enter the number of the %s you want to jump to", gti_objecttype),
				wxString::Format("0 - %d", gti_maxobject-1),
				wxString::Format("Go to %s",gti_objecttype),
				0, 0, gti_maxobject-1, m_DrawWindow);

	if (retval>=0 && retval<gti_maxobject)
	{
		GoToObject(retval);
	}
}



//==========================================================================
//
// center the map around the object and zoom in if necessary
//
//==========================================================================

void CLevel::GoToObject(int objnum) 
{
	int   xpos, ypos;
	int   xpos2, ypos2;
	int   n;
	float oldscale=m_ZoomFactor;
	CRectFloat r;

	GetObjectCoords(objnum, &xpos, &ypos);
	SetPosition(xpos,ypos);

	while (m_ZoomFactor>5)
	{
		double ptx=0.4*m_ZoomFactor;
		r.Set(xpos-ptx,ypos-ptx,xpos+ptx,ypos+ptx);
		if (GetCurObject(&r) != objnum)
		{
			if (m_ZoomFactor>10) m_ZoomFactor-=10;
			else m_ZoomFactor=5;
			SetPosition(xpos,ypos);
		}
		else break;
	}

	/* Special case for Sectors: if several Sectors are one inside another, then    */
	/* zooming in on the center won't help.  So I choose a LineDef that borders the */
	/* Sector, move a few pixels towards the inside of the Sector, then zoom in.    */
	if (m_Mode==modeSectors && GetCurSector(xpos,ypos,xpos,ypos) != objnum)
	{
		/* restore the Scale */
		m_ZoomFactor = oldscale;
		for (n = 0; n < NumLines(); n++)
		{
			CLine * ln = GetLine(n);
			if (ln->sides[0].sector == objnum) break;
			if (ln->sides[1].sector == objnum) break;
		}
		if (n < NumLines())
		{
			m_Mode=modeLineDefs;
			GetObjectCoords( n, &xpos2, &ypos2);
			m_Mode=modeSectors;
			int d = ComputeDist( abs( xpos - xpos2), abs( ypos - ypos2)) / 7;
			if (d <= 1) d = 2;
			xpos = xpos2 + (xpos - xpos2) / d;
			ypos = ypos2 + (ypos - ypos2) / d;
			SetPosition(xpos,ypos);
			/* zoom in until the sector can be selected */

			int oldzoom=m_ZoomFactor;
			while (m_ZoomFactor>5)
			{
				double ptx=0.4*m_ZoomFactor;
				r.Set(xpos-ptx,ypos-ptx,xpos+ptx,ypos+ptx);
				if (GetCurObject(&r) != objnum)
				{
					if (m_ZoomFactor==5)
					{
						m_ZoomFactor=oldzoom;
						break;
					}
					else if (m_ZoomFactor>10) m_ZoomFactor-=10;
					else m_ZoomFactor=5;
					SetPosition(xpos,ypos);
				}
				else break;
			}
		}
	}
	CenterCursor();
	m_DrawWindow->Refresh();
	m_Selection=objnum;
	UpdateStatusBar();
}


//==========================================================================
//
//
//==========================================================================

void CLevel::CenterCursor()
{
	wxPoint p(SX(m_CenterX), SY(m_CenterY));
	m_DrawWindow->WarpPointer(p.x, p.y);
}

//==========================================================================
//
//
//==========================================================================

void CLevel::SetPosition(int x,int y)
{
	m_CenterX=x; 
	m_CenterY=y;
	m_DrawWindow->Refresh();
	m_DrawWindow->SetScrollBar(false);
}



//==========================================================================
/*
get the coordinates (approx.) of an object
*/
//==========================================================================

void CLevel::GetObjectCoords( int objnum, int *xpos, int *ypos) 
{
	int n;
	CVertex * v1, *v2;
	long accx, accy, num;

	switch (m_Mode)
	{
	case modeThings:
	{
		CThing * t = GetThing(objnum);
		*xpos = int(t->X());
		*ypos = int(t->Y());
		break;
	}
	case modeVertexes:
	{
		CVertex * v = GetVertex(objnum);
		*xpos = int(v->X());
		*ypos = int(v->Y());
		break;
	}
	case modeLineDefs:
		v1 = StVt(objnum);
		v2 = EnVt(objnum);
		*xpos = int((v1->X()+v2->X())/2);
		*ypos = int((v1->Y()+v2->Y())/2);
		break;

	case modeSectors:
		accx = 0L;
		accy = 0L;
		num = 0L;
		for (n = 0; n < NumLines(); n++)
		{
			v1 = StVt(n);
			v2 = EnVt(n);
			if ((FrontSecNo(n) == objnum) || (BackSecNo(n) == objnum))
			{
				/* if the Sector is closed, all Vertices will be counted twice */
				accx += (long) v1->X();
				accy += (long) v1->Y();
				num++;
				accx += (long) v2->X();
				accy += (long) v2->Y();
				num++;
			}
		}
		if (num > 0)
		{
			*xpos = (int) ((accx + num / 2L) / num);
			*ypos = (int) ((accy + num / 2L) / num);
		}
		else
		{
			*xpos = 
			*ypos = 0;
		}
		break;
	}
}



//==========================================================================
//
// check if there is something of interest inside the given box
//
//==========================================================================

int CLevel::GetCurObject( CRectFloat * rr)
{
	switch(m_Mode)
	{
	case modeThings:
		return GetThingFromPos(rr);
	case modeVertexes:
		return GetVertexFromPos(rr);
	case modeLineDefs:
		return GetLineDefFromPos(rr);
	case modeSectors:
		return GetCurSector(int(rr->left),int(rr->top),int(rr->right),int(rr->bottom));
	}
	return 0;
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnModeThings()
{
	if (m_Mode==modeLineDraw) CancelLineDraw();

	if (m_Mode!=modeThings)
	{
		UncheckAll();
		m_Mode=modeThings;
		m_Selection=0;
		m_DrawWindow->Refresh();
		UpdateStatusBar();
	}
}

void CLevel::OnModeThings(wxCommandEvent & event)
{
	OnModeThings();
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnModeLinedefs()
{
	if (m_Mode==modeLineDraw) CancelLineDraw();

	if (m_Mode!=modeLineDefs)
	{
		UncheckAll();
		m_Mode=modeLineDefs;
		m_Selection=0;
		m_DrawWindow->Refresh();
		UpdateStatusBar();
	}
}

void CLevel::OnModeLinedefs(wxCommandEvent & event)
{
	OnModeLinedefs();
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnModeSectors()
{
	if (m_Mode==modeLineDraw) CancelLineDraw();

	if (m_Mode!=modeSectors)
	{
		UncheckAll();
		m_Mode=modeSectors;
		m_Selection=0;
		m_DrawWindow->Refresh();
		UpdateStatusBar();
	}
}

void CLevel::OnModeSectors(wxCommandEvent & event)
{
	OnModeSectors();
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnModeVertices()
{
	if (m_Mode==modeLineDraw) CancelLineDraw();

	if (m_Mode!=modeVertexes)
	{
		UncheckAll();
		m_Mode=modeVertexes;
		m_Selection=0;
		m_DrawWindow->Refresh();
		UpdateStatusBar();
	}
}

void CLevel::OnModeVertices(wxCommandEvent & event)
{
	OnModeVertices();
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnUpdateModeThings(wxUpdateUIEvent & event)
{
	event.Check(m_Mode==modeThings);
}

void CLevel::OnUpdateModeLinedefs(wxUpdateUIEvent & event)
{
	event.Check(m_Mode==modeLineDefs);
}

void CLevel::OnUpdateModeSectors(wxUpdateUIEvent & event)
{
	event.Check(m_Mode==modeSectors);
}

void CLevel::OnUpdateModeVertices(wxUpdateUIEvent & event)
{
	event.Check(m_Mode==modeVertexes);
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::Select(int sel, bool what)
{
	if (what)
	{
		if (checked[sel]) return;
		Selection.Push(sel);
		checked[sel]=true;
	}
	else
	{
		if (!checked[sel]) return;
		checked[sel]=false;
		for(unsigned i=0;i<Selection.Size();i++)
		{
			if (Selection[i]==sel) 
			{
				Selection.Delete(i);
				break;
			}
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::UncheckAll()
{
	m_drawClipboard=false;
	Selection.Clear();
	checked.Clear();
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::PushChecked()
{
	checked_save = checked;
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::PopChecked()
{
	checked = checked_save;
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::CheckLineDefsFromSectors()
{
	CCheckArray checked_o = checked;

	checked.Clear();

	int i,c;
	CSector * pth1=NULL;

//	UncheckAll();

	if (NumSectors()==0) return;
	if (m_Selection<0) m_Selection=0;
	for(i=0;i<NumSectors();i++) if (checked_o[i]) break;
	if (i==NumSectors()) 
	{
		checked_o[m_Selection]=true;
	}

	for(i=0;i<NumSectors();i++)
	{
		if (checked_o[i])
		{
			for(c=0;c<NumLines();c++)
			{
				if (FrontSecNo(c)== i) checked[c]=true;
				if (BackSecNo(c) == i) checked[c]=true;
			}
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::CheckVertexesFromLineDefs()
{
	CCheckArray checked_o = checked;
	checked.Clear();

	int i;

	//UncheckAll();
	if (NumLines()==0) return;
	if (m_Selection<0) m_Selection=0;
	for(i=0;i<NumLines();i++) if (checked_o[i]) break;
	if (i==NumLines()) 
	{
		checked_o[m_Selection]=true;
	}

	for(i=0;i<NumLines();i++)
	{
		CLine * pld = GetLine(i);
		if (checked_o[i])
		{
			checked[pld->line.Start] = checked[pld->line.End] = true;
		}
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnViewZoomin(wxCommandEvent & event)
{
	if (m_Mode==mode3D) return ;

	if (m_ZoomFactor<=0.25f) m_ZoomFactor=0.25f;
	else if (m_ZoomFactor<=1) m_ZoomFactor-=0.25f;
	else if (m_ZoomFactor<=5) m_ZoomFactor--;
	else if (m_ZoomFactor<=10) m_ZoomFactor=5;
	else m_ZoomFactor-=10;
	m_DrawWindow->Refresh();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnViewZoomout(wxCommandEvent & event)
{
	if (m_Mode==mode3D) return ;

	if (m_ZoomFactor==200) return;
	if (m_ZoomFactor>=10) m_ZoomFactor+=10;
	else if (m_ZoomFactor>=5) m_ZoomFactor=10;
	else if (m_ZoomFactor>1) m_ZoomFactor++;
	else m_ZoomFactor+=0.25f;
	m_DrawWindow->Refresh();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdNext(bool shift, bool control)
{
	int max;
	int started=m_Selection;
	bool special=false;
	switch(m_Mode)
	{
	case modeThings: 
		max=NumThings();
		break;
	case modeVertexes:
		max=NumVertices();
		break;
	case modeSectors:
		max=NumSectors();
		break;
	case modeLineDefs:
		max=NumLines();
		break;
	default:
		return;
	}
	do
	{
		if (++m_Selection==max) m_Selection=0;
		if (m_Selection==started) return;
		if (shift)
		{
			switch(m_Mode)
			{
			case modeThings:
			{
				CThing * t = GetThing(m_Selection);
				if (t->special || t->thingid || !m_Extended) special=true;
				break;
			}
			case modeSectors:
			{
				CSector * s = GetSector(m_Selection);
				if (s->special) special=true;
				break;
			}
			case modeLineDefs:
				if (GetLine(m_Selection)->line.type) special=true;
				break;

			default:
				special=true;
			}
		}

	}
	while (shift && !special);
	if (control) GoToObject(m_Selection);
	m_DrawWindow->Refresh();
	UpdateStatusBar();
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnNext(wxCommandEvent & event)
{
	OnCmdNext(false, false);
}

void CLevel::OnNextspc(wxCommandEvent & event)
{
	OnCmdNext(true, false);
}

void CLevel::OnNextjump(wxCommandEvent & event)
{
	OnCmdNext(false, true);
}

void CLevel::OnNextspcjump(wxCommandEvent & event)
{
	OnCmdNext(true, true);
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdPrev(bool shift, bool control)
{
	int max;
	int started=m_Selection;
	bool special=false;
	switch(m_Mode)
	{
	case modeThings: 
		max=NumThings();
		break;
	case modeVertexes:
		max=NumVertices();
		break;
	case modeSectors:
		max=NumSectors();
			break;
	case modeLineDefs:
		max=NumLines();
		break;
	default:
		return;
	}
	do
	{
		if (--m_Selection<=-1) m_Selection=max-1;
		if (m_Selection==started) return;
		if (shift)
		{
			switch(m_Mode)
			{
			case modeThings:
			{
				CThing * t = GetThing(m_Selection);
				if (t->special || t->thingid || !m_Extended) special=true;
				break;
			}
			case modeSectors:
			{
				CSector * s = GetSector(m_Selection);
				if (s->special) special=true;
				break;
			}
			case modeLineDefs:
				if (GetLine(m_Selection)->line.type) special=true;
				break;

			default:
				special=true;
			}
		}

	}
	while (shift && !special);
	if (control) GoToObject(m_Selection);
	m_DrawWindow->Refresh();
	UpdateStatusBar();
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnPrev(wxCommandEvent & event)
{
	OnCmdPrev(false, false);
}

void CLevel::OnPrevspc(wxCommandEvent & event)
{
	OnCmdPrev(true, false);
}

void CLevel::OnPrevjump(wxCommandEvent & event)
{
	OnCmdPrev(false, true);
}

void CLevel::OnPrevspcjump(wxCommandEvent & event)
{
	OnCmdPrev(true, true);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdReduceSelection(wxCommandEvent & event)
{
	if (m_Mode!=modeLineDefs) return;

	long retval = wxGetNumberFromUser(
				"Reduce selection to sector",
				wxString::Format("0 - %d", NumSectors()-1),
				"Reduce Selection",
				0, 0, NumSectors()-1, m_DrawWindow);

	if (retval>=0 && retval<NumSectors())
	{
		for (int i=0;i<NumLines();i++)
		{
			if (checked[i])
			{
				if (FrontSecNo(i)==retval) continue;
				if (BackSecNo(i)==retval) continue;
				checked[i]=false;
			}
		}
		m_DrawWindow->Refresh();
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnViewClearselection(wxCommandEvent & event)
{
	/*
	if (m_Mode==modeLineDraw)
	{
		AbortLineDraw();
	}
	else
	*/
	{
		UncheckAll();
		m_DrawWindow->Refresh();
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::InsertRectangle( int x0, int y0, int xpos, int ypos) 
{
	int sector;
	int n;
	int a, b;

	int v,l;

	wxPoint p(100,100);

	if (Get2Numbers(m_DrawWindow, "Insert Rectangle", "Width", "Height", 1, 2000, 1, 2000, &p))
	{
		MakeBackup("Insert Rectangle", true, true, true, false);
		a=p.x;
		b=p.y;
		sector=GetCurSector(xpos,ypos,xpos,ypos);

		if (a < 8) a = 8;
		if (b < 8) b = 8;
		xpos = xpos - a / 2;
		ypos = ypos - b / 2;
		v=InsertVertex( xpos, ypos);
		InsertVertex( xpos + a, ypos);
		InsertVertex( xpos + a, ypos + b);
		InsertVertex( xpos, ypos + b);
		if (sector < 0) sector=InsertSector(NULL);
		for (n = 0; n < 4; n++)
		{
			l=InsertLineDef(NULL);
			CLine * ln = GetLine(l);
			InitSideDef(&ln->sides[0], NULL);
			ln->sides[0].sector = sector;
		}
		if (sector >= 0)
		{
			CLine * ln;
			ln=GetLine(l-3);
			ln->line.Start = v;
			ln->line.End = v+1;
			ln=GetLine(l-2);
			ln->line.Start = v+1;
			ln->line.End = v+2;
			ln=GetLine(l-1);
			ln->line.Start = v+2;
			ln->line.End = v+3;
			ln=GetLine(l);
			ln->line.Start = v+3;
			ln->line.End = v;
		}
		else
		{
			CLine * ln;
			ln=GetLine(l-3);
			ln->line.Start = v+3;
			ln->line.End = v+2;
			ln=GetLine(l-2);
			ln->line.Start = v+2;
			ln->line.End = v+1;
			ln=GetLine(l-1);
			ln->line.Start = v+1;
			ln->line.End = v;
			ln=GetLine(l);
			ln->line.Start = v;
			ln->line.End = v+3;
		}
	}
	m_DrawWindow->Refresh();
	UpdateStatusBar();
}	


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnEditInsertrectangle(wxCommandEvent & event)
{
	if (!(m_Mode&modeEdit)) return;

	int orgx = int(MX(m_ContextPoint.x));
	int orgy = int(MY(m_ContextPoint.y));
	InsertRectangle(orgx,orgy,orgx,orgy);
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::InsertPolygon( int x0, int y0, int xpos, int ypos)
{
	int sector;
	int n;
	int a, b;
	int v,s,l;
	wxPoint p(8, 100);

	if (Get2Numbers(m_DrawWindow, "Insert Polygon", "Number of sides", "Radius", 1, 32, 1, 2000, &p))
	{
		MakeBackup("Insert Polygon", true, true, true, false);
		a=p.x;
		b=p.y;
		sector=GetCurSector(xpos,ypos,xpos,ypos);
		if (a < 3) a = 3;
		if (b < 8) b = 8;
		v=NumVertices();
		InsertPolygonVertices( xpos, ypos, a, b);
		if (sector < 0) s=InsertSector(NULL);
		for (n = 0; n < a; n++)
		{
			l=InsertLineDef(NULL);
			CLine * ln = GetLine(l);
			InitSideDef(&ln->sides[0], NULL);
			ln->sides[0].sector = sector;
		}
		if (sector >= 0)
		{
			CLine * ln = GetLine(NumLines()-1);
			ln->line.Start = NumVertices() - 1;
			ln->line.End = NumVertices() - a;
			for (n = 2; n <= a; n++)
			{
				ln = GetLine(NumLines()-n);
				ln->line.Start = NumVertices() - n;
				ln->line.End = NumVertices() - n + 1;
			}
		}
		else
		{
			CLine * ln = GetLine(NumLines()-1);
			ln->line.Start = NumVertices() - a;
			ln->line.End = NumVertices() - 1;
			for (n = 2; n <= a; n++)
			{
				ln->line.Start = NumVertices() - n + 1;
				ln->line.End = NumVertices() - n;
			}
		}
	}
	m_DrawWindow->Refresh();
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnEditInsertpolygon(wxCommandEvent & event)
{
	if (!(m_Mode&modeEdit)) return;

	int orgx = int(MX(m_ContextPoint.x));
	int orgy = int(MY(m_ContextPoint.y));

	InsertPolygon(orgx,orgy,orgx,orgy);
}


//==========================================================================
//
//
//
//==========================================================================

int CLevel::GetLDRef(int sec)
{
	int l=-1;
	int tag=GetSector(sec)->tag;
	if (tag==0) return -32768;
	for(int i=0;i<NumLines();i++)
	{
		if (GetLine(i)->line.tag==tag)
		{
			if (l<0) l=i;
			else return -l;
		}
	}
	return l<0? -32768:l;
}


//==========================================================================
//
//
//
//==========================================================================

int CLevel::GetSecRef(int ld)
{
	int l=-1;
	int tag=GetLine(ld)->line.tag;
	if (tag==0) return -32768;
	for(int i=0;i<NumSectors();i++)
	{
		if (GetSector(i)->tag==tag)
		{
			if (l<0) l=i;
			else return -l;
		}
	}
	return l<0? -32768:l;
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnEditRemovezero(wxCommandEvent & event)
{
	if (!(m_Mode&modeEdit)) return;

	int c=0;
	for(int i=NumLines()-1;i>=0;i--)
	{
		if (StVt(i)->X() == EnVt(i)->X() &&	StVt(i)->Y() == EnVt(i)->Y())
		{
			if (c==0) MakeBackup("Remove zero-length linedefs", false, true, false, false);
			DeleteOneLineDef(i);
			c++;
		}

	}
	if (c>0)
	{
		wxMessageBox(wxString::Format("%d linedefs removed", c), ZED_CAPTION);
	}
	else wxMessageBox("No zero-length linedefs found", ZED_CAPTION);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnViewShowsidemarkers(wxCommandEvent & event)
{
	m_showSideMarkers=!m_showSideMarkers;
	if (config.SetSection("Settings", true))
	{
		config.SetIntValueForKey("Sidemarkers", m_showSideMarkers);
	}
	m_DrawWindow->Refresh();
}

void CLevel::OnUpdateViewShowsidemarkers(wxUpdateUIEvent & event)
{
	event.Check(m_showSideMarkers);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnViewCenterview(wxCommandEvent & event)
{
	m_CenterX = (m_Bounds.right+m_Bounds.left)/2;
	m_CenterY = (m_Bounds.bottom+m_Bounds.top)/2;
	m_ZoomFactor=10;
	while ((SX(m_Bounds.left)<0 || SY(m_Bounds.bottom)<0) && m_ZoomFactor<200)
	{
		m_ZoomFactor+=10;
	}
	m_DrawWindow->Refresh();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnSnaptogrid(wxCommandEvent & event)
{
	int n;

	if (!(m_Mode&modeEdit)) return;

	PushChecked();
	switch (m_Mode)
	{
	case modeThings:
		MakeBackup("Snap Things to Grid", false, false, false, true);

		if (Selection.Size()==0)
		{
			CThing * t = GetThing(m_Selection);
			SnapToGrid(t);
		}
		else for (n=0;n<NumThings();n++) if (checked[n])
		{
			CThing * t = GetThing(n);
			SnapToGrid(t);
		}
		m_changed=true;
		break;

	case modeSectors:
		CheckLineDefsFromSectors();
	case modeLineDefs:
		CheckVertexesFromLineDefs();
	case modeVertexes:
		MakeBackup("Snap Vertices to Grid", true, false, false, false);
		if (m_Mode==modeVertexes && Selection.Size()==0)
		{
			CVertex * v = GetVertex(m_Selection);
			SnapToGrid(v);
		} 
		else for (n=0;n<NumVertices();n++) if(checked[n])
		{
			CVertex * v = GetVertex(n);
			SnapToGrid(v);
		}
		m_changed=true;
		m_NeedNodeBuild=true;
		break;
	}
	PopChecked();
	m_DrawWindow->Refresh();
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnDistributelightlevel(wxCommandEvent & event)
{
	int   light1, light2;
	unsigned n;

	if (!(m_Mode&modeEdit)) return;

	MakeBackup("Distribute light level", false, false, true, false);
	if (Selection.Size())
	{
		light1 = GetSector(Selection[0])->light;
		light2 = GetSector(Selection.Last())->light;

		for (n=0;n<Selection.Size();n++)
		{
			GetSector(Selection[n])->light = light1 + int(n) * (light2-light1) / int(Selection.Size()-1);
		}
	}
	else wxMessageBox("No Sectors selected", ZED_CAPTION);
	m_DrawWindow->Refresh();
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnRaisefloorby8mapunits(wxCommandEvent & event)
{
	int n;

	if (m_Mode==modeLineDraw) return;

	MakeBackup("Raise floor height", false, false, true, false);
	if (Selection.Size()==0)
	{
		GetSector(m_Selection)->floorh+=8;
	}
	else for (n=0;n<NumSectors();n++) if (checked[n])
	{
		GetSector(n)->floorh+=8;
	}
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnLowerfloorby8mapunits(wxCommandEvent & event)
{
	int n;

	if (m_Mode==modeLineDraw) return;

	MakeBackup("Lower floor height", false, false, true, false);
	if (Selection.Size()==0)
	{
		GetSector(m_Selection)->floorh-=8;
	}
	else for (n=0;n<NumSectors();n++) if (checked[n])
	{
		GetSector(n)->floorh-=8;
	}
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnRaiseceilingby8mapunits(wxCommandEvent & event)
{
	int n;

	if (m_Mode==modeLineDraw) return;

	MakeBackup("Raise ceiling height", false, false, true, false);
	if (Selection.Size()==0)
	{
		GetSector(m_Selection)->ceilh+=8;
	}
	else for (n=0;n<NumSectors();n++) if (checked[n])
	{
		GetSector(n)->ceilh+=8;
	}
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnLowerceilingby8mapunits(wxCommandEvent & event)
{
	int n;

	if (m_Mode==modeLineDraw) return;

	MakeBackup("Lower ceiling height", false, false, true, false);
	if (Selection.Size()==0)
	{
		GetSector(m_Selection)->ceilh-=8;
	}
	else for (n=0;n<NumSectors();n++) if (checked[n])
	{
		GetSector(n)->ceilh-=8;
	}
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnRaiselightlevel(wxCommandEvent & event)
{
	int n;

	if (m_Mode==modeLineDraw) return;

	MakeBackup("Raise light level", false, false, true, false);
	if (Selection.Size()==0)
	{
		CSector * sec = GetSector(m_Selection);
		sec->light = min(sec->light+16, 255);
	}
	else for (n=0;n<NumSectors();n++) if (checked[n])
	{
		CSector * sec = GetSector(n);
		sec->light = min(sec->light+16, 255);
	}
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnLowerlightlevel(wxCommandEvent & event)
{
	int n;

	if (m_Mode==modeLineDraw) return;

	MakeBackup("Lower light level", false, false, true, false);
	if (Selection.Size()==0)
	{
		CSector * sec = GetSector(m_Selection);
		if (sec->light==255) sec->light=240;
		else sec->light = max(sec->light-16, 0);
	}
	else for (n=0;n<NumSectors();n++) if (checked[n])
	{
		CSector * sec = GetSector(n);
		if (sec->light==255) sec->light=240;
		else sec->light = max(sec->light-16, 0);
	}
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnUnselectone(wxCommandEvent & event)
{
	int n;

	if (m_Mode==modeLineDefs)
	{
		for (n=0;n<NumLines();n++) if (checked[n])
		{
			if (BackSecNo(n)==-1) Select(n, false);
		}
	}
	m_DrawWindow->Refresh();
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnUnselecttwo(wxCommandEvent & event)
{
	int n;

	if (m_Mode==modeLineDefs)
	{
		for (n=0;n<NumLines();n++) if (checked[n])
		{
			if (BackSecNo(n)!=-1) Select(n, false);
		}
	}
	m_DrawWindow->Refresh();
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

int CLevel::Deleteunusedvertices()
{
	int n;
	int m;
	int removed=0;
	TArray<bool> List;

	List.Resize(NumVertices());
	for (n = 0; n < NumVertices(); n++) List[n]=true;

	for (n = 0; n < NumLines(); n++)
	{
		CLine * ln = GetLine(n);
		m = ln->line.Start;
		if (m >= 0) List[m]=false;
		m = ln->line.End;
		if (m >= 0) List[m]=false;
	}
	for(m=NumVertices()-1;m>=0;m--) 
	{
		if (List[m]) 
		{
			DeleteOneVertex(m);
			removed++;
		}
	}
	return removed;
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnDeleteunusedvertices(wxCommandEvent & event)
{
	if (m_Mode==modeLineDraw) return;
	MakeBackup("Delete unused vertices", true, false, false, false);
	if (!Deleteunusedvertices()) RemoveBackup();
	UncheckAll();
	m_Selection=0;
	m_DrawWindow->Refresh();
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnDeleteunusedsectors(wxCommandEvent & event)
{
	int n;
	int m;
	int removed=0;
	TArray<bool> List;

	if (m_Mode==modeLineDraw) return;

	MakeBackup("Delete unused sectors", false, false, true, false);

	List.Resize(NumSectors());
	for (n = 0; n < NumSectors(); n++) List[n]=1;

	for (n = 0; n < NumLines(); n++)
	{
		CLine * ln = GetLine(n);
		m = ln->line.Start;
		if (m >= 0) List[m]=false;
		m = ln->line.End;
		if (m >= 0) List[m]=false;
	}

	for(n=NumSectors()-1;n>=0;n--) 
	{
		if (List[n]) 
		{
			DeleteOneSector(n);
			removed++;
		}
	}	
	UncheckAll();
	m_Selection=0;
	if (!removed) RemoveBackup();
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnSelectthingsinsectors(wxCommandEvent & event)
{
	if (m_Mode==modeSectors)
	{
		m_Mode=modeThings;
		m_Selection=0;
		Selection.Clear();
		for(int i=0;i<NumThings();i++)
		{
			CThing * t = GetThing(i);
			int sec = GetSectorFromPos(wxRealPoint(*t));
			if (sec!=-1 && checked[sec]) Selection.Push(i);
		}
		for(unsigned i=0;i<Selection.Size();i++) 
		{
			checked[Selection[i]]=true;
		}
		m_DrawWindow->Refresh();
		UpdateStatusBar();
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnSelectlinesfromsectors(wxCommandEvent & event)
{
	if (m_Mode==modeSectors)
	{
		m_Mode=modeLineDefs;
		CheckLineDefsFromSectors();
		m_Selection=0;
		Selection.Clear();
		for(int i=0;i<NumLines();i++) if (checked[i]) Selection.Push(i);
		m_DrawWindow->Refresh();
		UpdateStatusBar();
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnSelectverticesfromsectors(wxCommandEvent & event)
{
	if (m_Mode==modeSectors)
	{
		m_Mode=modeVertexes;
		CheckLineDefsFromSectors();
		CheckVertexesFromLineDefs();
		m_Selection=0;
		Selection.Clear();
		for(int i=0;i<NumVertices();i++) if (checked[i]) Selection.Push(i);
		m_DrawWindow->Refresh();
		UpdateStatusBar();
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnSelectverticesfromlines(wxCommandEvent & event)
{
	if (m_Mode==modeLineDefs)
	{
		m_Mode=modeVertexes;
		CheckVertexesFromLineDefs();
		m_Selection=0;
		Selection.Clear();
		for(int i=0;i<NumVertices();i++) if (checked[i]) Selection.Push(i);
		m_DrawWindow->Refresh();
		UpdateStatusBar();
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnSelectlinesfromvertices(wxCommandEvent & event)
{
	if (m_Mode==modeVertexes)
	{
		m_Mode=modeLineDefs;
		Selection.Clear();
		m_Selection=0;
		for(int i=0;i<NumLines();i++) 
		{
			CLine * ln = GetLine(i);
			if (checked[ln->line.Start] && checked[ln->line.End]) Selection.Push(i);
		}
		checked.Clear();
		for(unsigned i=0;i<Selection.Size();i++) 
		{
			checked[Selection[i]]=true;
		}
		m_DrawWindow->Refresh();
		UpdateStatusBar();
	}
}



//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnEditCopy(wxCommandEvent & event)
{
	clipboard.Copy(this);
}

void CLevel::OnEditCut(wxCommandEvent & event)
{
	clipboard.Copy(this);
	OnCmdDelete(event);
}

void CLevel::OnEditPaste(wxCommandEvent & event)
{
	m_drawClipboard=true;
	m_drawclipx = m_drawclipy = 100000;
}

void CLevel::OnUpdateEditCopyCut(wxUpdateUIEvent & event)
{
	event.Enable(m_Selection>0 || Selection.Size()>0);
}

void CLevel::OnUpdateEditPaste(wxUpdateUIEvent & event)
{
	event.Enable(clipboard.ContainsData());
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnEditEditacsscript(wxCommandEvent & event)
{
	if (!(m_Mode&modeEdit)) return;

	if (m_Extended)
	{
		if (!m_ScriptEdit[Script_ACS])
		{
			m_ScriptEdit[Script_ACS] = new CScriptEdit(m_DrawWindow, this, Script_ACS);
		}
		m_ScriptEdit[Script_ACS]->Show();
		m_ScriptEdit[Script_ACS]->SetFocus();
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnEditEditfragglescript(wxCommandEvent & event)
{
	if (!(m_Mode&modeEdit)) return;

	if (!m_ScriptEdit[Script_FS])
	{
		m_ScriptEdit[Script_FS] = new CScriptEdit(m_DrawWindow, this, Script_FS);
	}
	m_ScriptEdit[Script_FS]->Show();
	m_ScriptEdit[Script_FS]->SetFocus();
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnEditEditconversationscript(wxCommandEvent & event)
{
	if (!(m_Mode&modeEdit)) return;

	// Todo: config check!
	if (m_Mission[0]=='M' && m_Mission[1]=='A' && m_Mission[2]=='P')
	{
		if (!m_ScriptEdit[Script_Conv])
		{
			m_ScriptEdit[Script_Conv] = new CScriptEdit(m_DrawWindow, this, Script_Conv);
		}
		m_ScriptEdit[Script_Conv]->Show();
		m_ScriptEdit[Script_Conv]->SetFocus();
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnUpdateEditEditacsscript(wxUpdateUIEvent & event)
{
	event.Enable(m_Extended && m_Mode!=modeLineDraw);
}

void CLevel::OnUpdateEditEditfragglescript(wxUpdateUIEvent & event)
{
	// TODO:
	event.Enable(m_Mode!=modeLineDraw);
}

void CLevel::OnUpdateEditEditconversationscript(wxUpdateUIEvent & event)
{
	event.Enable((m_Mission[0]=='M' && m_Mission[1]=='A' && m_Mission[2]=='P') && m_Mode!=modeLineDraw);
}



