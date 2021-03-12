//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2002-2005 Christoph Oelckers
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
// Editor configuration file parser
//
//
#include "stdafx.h"
#include "ZEd.h"
#include "sc_man.h"
#include "tarray.h"
#include "GameConfig.h"
#include "GenericTriggers.h"
#include "doomerrors.h"
#include "cmdlib.h"

ColorList Colors;
wxColour dummypen(0,0,128);

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

static void ParseColor(ScriptMan & sc)
{
	int r,g,b;
	int rp=-1,gp=-1,bp=-1;
	char buffer[60];

	sc.SC_MustGetString();
	strcpy(buffer,sc.sc_String);

	sc.SC_MustGetNumber();
	r=sc.sc_Number;

	sc.SC_MustGetNumber();
	g=sc.sc_Number;

	sc.SC_MustGetNumber();
	b=sc.sc_Number;

	if (sc.TestCom())
	{
		sc.SC_MustGetNumber();
		rp=sc.sc_Number;

		sc.SC_MustGetNumber();
		gp=sc.sc_Number;

		sc.SC_MustGetNumber();
		bp=sc.sc_Number;
	}

	CColor * c=new CColor(buffer,r,g,b,rp,gp,bp);

	Colors.Push(c);
}

bool ParseColors()
{
	ScriptMan sc("{}|=,");
	wxString fn = GetConfigDir() + "colors.cfg";

	try
	{
		sc.SC_OpenFile(fn);

		if (!sc.SC_GetString())
		{
			sc.SC_ScriptError("colors.cfg not found or empty!");
		}
		sc.SC_UnGet();
		
		while (sc.SC_GetString())
		{
			if (sc.SC_Compare("COLOR"))
			{
				ParseColor(sc);
			}
		}
		sc.SC_Close();
	}
	catch (CRecoverableError * err)
	{
		wxMessageBox(err->GetMessage());
		return false;
	}
	return true;
}

static void InitColor(CColor ** pPen,const char * penname)
{

	if (penname == NULL) penname = "invalid";
	//if (force || !*pPen)
	{
		for(unsigned i=0;i<Colors.Size();i++)
		{
			if (!stricmp(penname,Colors[i]->name.c_str()))
			{
				(*pPen)=Colors[i];
			}
		}
	}
	if (*pPen==NULL)
	{
	}
}


/***************************************************************************/
/*                                                                         */
/* Things                                                                  */
/*                                                                         */
/***************************************************************************/


/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

ThingDesc * GameConfig::ParseThing(ScriptMan & sc, ThingDesc & defaultthing)
{
	ThingDesc * td = new ThingDesc;
	unsigned i;

	if (!sc.SC_CheckString("thing"))
	{
		sc.SC_MustGetNumber();
		td->DoomEdNum=sc.sc_Number;
		sc.SC_MustGetString();
		td->name = sc.sc_String;
		sc.SC_MustGetString();
		td->sprite = sc.sc_String;
		sc.SC_MustGetString();
		td->penname = sc.sc_String;
		sc.SC_MustGetNumber();
		td->radius=sc.sc_Number;
		sc.SC_MustGetString();
		const char * statclasses[]={ "None", "Monster", "Key", "Item", "NCItem", "Weapon", "Ammo", "Special", "Friend", NULL };
		td->statclass=sc.SC_MustMatchString(statclasses);
		for (i=0;i<5;i++)
		{
			sc.SC_MustGetString();
			if (sc.sc_Crossed)
			{
				sc.SC_UnGet();
				break;
			}
			td->argdescript[i] = sc.sc_String;
		}
	}
	else
	{
		/*
		sc.SC_MustGetNumber();
		td->DoomEdNum=sc.sc_Number;
		sc.SC_MustGetString();
		td->name = sc.sc_String;
		sc.SC_MustGetStringName("{");
		while (!sc.SC_CheckString("}"))
		{
			if (!GetThingProperty(td))
			{
				sc.SC_ScriptError("Unknown thing property '%s'", sc.sc_String);
			}
		}
		*/
	}
	return td;
}

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

ThingGroup * GameConfig::ParseThingGroup(ScriptMan & sc, ThingDesc & defaultthing, bool discard)
{
	ThingDesc def = defaultthing;
	ThingGroup * tg = NULL;
	bool old=false;
	def = defaultthing;

	sc.SC_MustGetString();
	if (!discard) for(int i=0;i<ThingGroups.Size();i++)
	{
		if (ThingGroups[i]->name == sc.sc_String)
		{
			tg=ThingGroups[i];
			old=true;
			break;
		}
	}
	if (tg==NULL) 
	{
		tg=new ThingGroup;
		tg->name = sc.sc_String;

		if (sc.SC_CheckString(":"))
		{
			sc.SC_MustGetString();
			tg->parent=sc.sc_String;
		}
	}

	if (sc.TestBraceOpn())
	{
		while (!sc.TestBraceCls())
		{
			sc.SC_MustGetString();
			if (sc.SC_Compare("clear"))
			{
				tg->Clear();
			}
			else
			{
				sc.SC_UnGet();
				ThingDesc * td = ParseThing(sc, def);

				if (!discard) 
				{
					tg->Push(td);
					if (td->DoomEdNum>=0 && td->DoomEdNum<32768) ThingMap[td->DoomEdNum]=td;
				}
				else delete td;
			}
		}
	}
	if (discard)
	{
		delete tg;
		return NULL;
	}
	return old? NULL:tg;
}

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

void GameConfig::ParseThings(ScriptMan & sc, bool discard)
{
	ThingDesc def;

	if (sc.TestBraceOpn())
	{
		while (!sc.TestBraceCls())
		{
			sc.SC_MustGetString();
			if (sc.SC_Compare("import"))
			{
				sc.SC_MustGetString();
				if (!discard)
				{
					wxString s = sc.sc_String;
					ParseConfig(s, true, false, false, false, false, false);
				}
			}
			else if (sc.SC_Compare("group"))
			{
				ThingGroup * tg = ParseThingGroup(sc, def, discard);
				if (tg) ThingGroups.Push(tg);
			}
			else if (sc.SC_Compare("flags"))
			{
				sc.SC_MustGetStringName("{");
				while (!sc.SC_CheckString("}"))
				{
					sc.SC_MustGetNumber();
					if (sc.sc_Number<0 || sc.sc_Number>15)
					{
						sc.SC_ScriptError("Flag index %d out of range", sc.sc_Number);
					}
					sc.SC_MustGetString();
					if (!discard) ThingFlags[sc.sc_Number].shortname = sc.sc_String;
					sc.SC_MustGetString();
					if (!discard) ThingFlags[sc.sc_Number].longname = sc.sc_String;
					sc.SC_MustGetString();
					if (!discard) ThingFlags[sc.sc_Number].textname = sc.sc_String;
				}
			}
			else if (sc.SC_Compare("textmapflags"))
			{
				int flagindex = 0;
				if (!discard) ThingGroupCount = 0;

				sc.SC_MustGetStringName("{");
				while (!sc.SC_CheckString("}"))
				{
					sc.SC_MustGetStringName("Group");
					sc.SC_MustGetString();
					if (!discard)
					{
						TextMapThingFlagGroups[ThingGroupCount].name = sc.sc_String;
						TextMapThingFlagGroups[ThingGroupCount].firstindex = flagindex;
						TextMapThingFlagGroups[ThingGroupCount].modeflag = 0;
					}
					if (!sc.SC_CheckString("{"))
					{
						sc.SC_MustGetString();
						if (!discard)
						{
							if (sc.SC_Compare("Hexen"))
								TextMapThingFlagGroups[ThingGroupCount].modeflag = 2;
							else if (sc.SC_Compare("Doom"))
								TextMapThingFlagGroups[ThingGroupCount].modeflag = 1;
						}

						sc.SC_MustGetStringName("{");
					}
					while (!sc.SC_CheckString("}"))
					{
						if (flagindex >=MAX_FLAGS)
						{
							sc.SC_ScriptError("Too many thing flags");
						}
						sc.SC_MustGetString();
						if (!discard) TextMapThingFlags[flagindex].shortname = sc.sc_String;
						sc.SC_MustGetString();
						if (!discard) TextMapThingFlags[flagindex].longname = sc.sc_String;
						sc.SC_MustGetString();
						if (!discard) TextMapThingFlags[flagindex].textname = sc.sc_String;
						flagindex++;
					}
					if (!discard)
					{
						TextMapThingFlagGroups[ThingGroupCount].lastindex = flagindex-1;
						ThingGroupCount++;
					}
				}
			}
		}
	}
}

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/
void GameConfig::ParseKeys(ScriptMan & sc, bool discard)
{
	sc.ChkBraceOpn();
	while (!sc.TestBraceCls())
	{
		sc.SC_MustGetString();
		if (sc.SC_Compare("import"))
		{
			sc.SC_MustGetString();
			if (!discard)
			{
				wxString s = sc.sc_String;
				ParseConfig(s, false, true, false, false, false, false);
			}
		}
		else
		{
			Lock * l = new Lock;

			sc.SC_UnGet();
			sc.SC_MustGetNumber();
			l->lockvalue=sc.sc_Number;
			sc.SC_MustGetString();
			l->descript=sc.sc_String;
			sc.SC_MustGetString();
			l->penname=sc.sc_String;
			InitColor(&l->realpen, l->penname.c_str());
			if (!discard) 
			{
				if (LockMap[sc.sc_Number]!=NULL) delete LockMap[sc.sc_Number];
				LockMap[sc.sc_Number]=l;
			}
			else delete l;
		}
	}
}


/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


LineSpecial * GameConfig::ParseLineSpecial(ScriptMan & sc, bool extended)
{
	LineSpecial * ls = new LineSpecial;

	sc.SC_MustGetNumber();
	ls->number=sc.sc_Number;
	if (ls->number!=-1)
	{
		sc.SC_MustGetString();
		ls->descript = sc.sc_String;

		if (!extended)
		{

			if (!strstr(sc.sc_String,"%k"))
			{
				sc.SC_MustGetString();
				if (!sc.sc_Crossed)
				{
					ls->penname = sc.sc_String;
				}
				else 
				{
					sc.SC_UnGet();
				}
				ls->param=0;
			}
			else
			{
				sc.SC_MustGetNumber();
				ls->param=sc.sc_Number;
			}
		}
		else
		{
			for(int i=0;i<5;i++)
			{
				sc.SC_MustGetString();
				if (sc.SC_Compare("TRUE") || sc.SC_Compare("FALSE")) 
				{
					sc.SC_UnGet();
					break;
				}
				ls->args[i] = sc.sc_String;
			}
			sc.SC_MustGetString();
			ls->param = sc.SC_Compare("TRUE");
			if (sc.SC_GetString() && !sc.sc_Crossed)
			{
				ls->penname = sc.sc_String;
			}
			else sc.SC_UnGet();
		}
	}

	ls->tagmask = ls->tidmask = ls->lineidmask=0;
	for(int a=0;a<5;a++)
	{
		if (!strnicmp(ls->args[a].c_str(),"Tag:",4)) ls->tagmask |= 1<<a;
		if (!strnicmp(ls->args[a].c_str(),"Tid:",4)) ls->tidmask |= 1<<a;
		if (!strnicmp(ls->args[a].c_str(),"LineID:",7)) ls->lineidmask |= 1<<a;
		if (!strnicmp(ls->args[a].c_str(),"SetLineID:",10)) ls->lineidmask |= 1<<a;
	}

	return ls;
}


/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

LineGroup * GameConfig::ParseLineGroup(ScriptMan & sc, bool extended, bool discard)
{
	LineGroup * ol=NULL;
	bool old=false;

	sc.SC_MustGetString();
	if (!discard) for(int i=0;i<LineGroups.Size();i++)
	{
		if (LineGroups[i]->name == sc.sc_String)
		{
			ol=LineGroups[i];
			old=true;
			break;
		}
	}
	if (ol==NULL) 
	{
		ol=new LineGroup;
		ol->name = sc.sc_String;

		if (sc.SC_CheckString(":"))
		{
			sc.SC_MustGetString();
			ol->parent=sc.sc_String;
		}
	}

	if (sc.TestBraceOpn())
	{
		while (!sc.TestBraceCls())
		{
			sc.SC_MustGetString();
			if (sc.SC_Compare("clear"))
			{
				ol->Clear();
			}
			else
			{
				sc.SC_UnGet();
				LineSpecial * ls = ParseLineSpecial(sc, extended);
				if (!discard) 
				{
					ol->Push(ls);
					if (ls->number>=0 && ls->number<8192) LineMap[ls->number]=ls;
				}
				else delete ls;
			}
		}
	}
	if (discard)
	{
		delete ol;
		return NULL;
	}
	return old? NULL:ol;
}


/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

void GameConfig::ParseLines(ScriptMan & sc, bool extended, bool discard)
{
	if (sc.TestBraceOpn())
	{
		while (!sc.TestBraceCls())
		{
			sc.SC_MustGetString();
			if (sc.SC_Compare("import"))
			{
				sc.SC_MustGetString();
				if (!discard)
				{
					wxString s = sc.sc_String;
					ParseConfig(s, false, false, true, false, false, false);
				}
			}
			else if (sc.SC_Compare("generic"))
			{
				if (!discard) genericlinedefs=true;
			}
			else if (sc.SC_Compare("flags"))
			{
				sc.SC_MustGetStringName("{");
				while (!sc.SC_CheckString("}"))
				{
					sc.SC_MustGetNumber();
					if (sc.sc_Number<0 || sc.sc_Number> 15)
					{
						sc.SC_ScriptError("Flag index %d out of range", sc.sc_Number);
					}
					sc.SC_MustGetString();
					if (!discard) LineFlags[sc.sc_Number].shortname = sc.sc_String;
					sc.SC_MustGetString();
					if (!discard) LineFlags[sc.sc_Number].longname = sc.sc_String;
					sc.SC_MustGetString();
					if (!discard) LineFlags[sc.sc_Number].textname = sc.sc_String;
				}
			}
			else if (sc.SC_Compare("textmapflags"))
			{
				int flagindex = 0;
				if (!discard) LineGroupCount = 0;

				sc.SC_MustGetStringName("{");
				while (!sc.SC_CheckString("}"))
				{
					sc.SC_MustGetStringName("Group");
					sc.SC_MustGetString();
					if (!discard)
					{
						TextMapLineFlagGroups[LineGroupCount].name = sc.sc_String;
						TextMapLineFlagGroups[LineGroupCount].firstindex = flagindex;
						TextMapLineFlagGroups[LineGroupCount].modeflag = 0;
					}
					if (!sc.SC_CheckString("{"))
					{
						sc.SC_MustGetString();
						if (!discard)
						{
							if (sc.SC_Compare("Hexen"))
								TextMapLineFlagGroups[LineGroupCount].modeflag = 2;
							else if (sc.SC_Compare("Doom"))
								TextMapLineFlagGroups[LineGroupCount].modeflag = 1;
						}

						sc.SC_MustGetStringName("{");
					}
					while (!sc.SC_CheckString("}"))
					{
						if (flagindex >=MAX_FLAGS)
						{
							sc.SC_ScriptError("Too many Line flags");
						}
						sc.SC_MustGetString();
						if (!discard) TextMapLineFlags[flagindex].shortname = sc.sc_String;
						sc.SC_MustGetString();
						if (!discard) TextMapLineFlags[flagindex].longname = sc.sc_String;
						sc.SC_MustGetString();
						if (!discard) TextMapLineFlags[flagindex].textname = sc.sc_String;
						flagindex++;
					}
					if (!discard)
					{
						TextMapLineFlagGroups[LineGroupCount].lastindex = flagindex-1;
						LineGroupCount++;
					}
				}
			}
			else if (sc.SC_Compare("group"))
			{
				LineGroup * lg = ParseLineGroup(sc, extended, discard);
				if (lg) LineGroups.Push(lg);
			}
		}
	}
}

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

void GameConfig::ParseSides(ScriptMan & sc, bool discard)
{
	if (sc.TestBraceOpn())
	{
		while (!sc.TestBraceCls())
		{
			sc.SC_MustGetString();
			if (sc.SC_Compare("import"))
			{
				sc.SC_MustGetString();
				if (!discard)
				{
					wxString s = sc.sc_String;
					ParseConfig(s, false, false, false, true, false, false);
				}
			}
			else if (sc.SC_Compare("textmapflags"))
			{
				int flagindex = 0;
				if (!discard) SideGroupCount = 0;

				sc.SC_MustGetStringName("{");
				while (!sc.SC_CheckString("}"))
				{
					sc.SC_MustGetStringName("Group");
					sc.SC_MustGetString();
					if (!discard)
					{
						TextMapSideFlagGroups[SideGroupCount].name = sc.sc_String;
						TextMapSideFlagGroups[SideGroupCount].firstindex = flagindex;
						TextMapSideFlagGroups[SideGroupCount].modeflag = 0;
					}
					if (!sc.SC_CheckString("{"))
					{
						sc.SC_MustGetString();
						if (!discard)
						{
							if (sc.SC_Compare("Hexen"))
								TextMapSideFlagGroups[SideGroupCount].modeflag = 2;
							else if (sc.SC_Compare("Doom"))
								TextMapSideFlagGroups[SideGroupCount].modeflag = 1;
						}

						sc.SC_MustGetStringName("{");
					}
					while (!sc.SC_CheckString("}"))
					{
						if (flagindex >=MAX_FLAGS)
						{
							sc.SC_ScriptError("Too many Side flags");
						}
						sc.SC_MustGetString();
						if (!discard) TextMapSideFlags[flagindex].shortname = sc.sc_String;
						sc.SC_MustGetString();
						if (!discard) TextMapSideFlags[flagindex].longname = sc.sc_String;
						sc.SC_MustGetString();
						if (!discard) TextMapSideFlags[flagindex].textname = sc.sc_String;
						flagindex++;
					}
					if (!discard)
					{
						TextMapSideFlagGroups[SideGroupCount].lastindex = flagindex-1;
						SideGroupCount++;
					}
				}
			}
		}
	}
}

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/
void GameConfig::ParseArgTypes(ScriptMan & sc, bool discard)
{

	sc.SC_MustGetStringName("{");
	while (!sc.SC_CheckString("}"))
	{
		sc.SC_MustGetString();
		if (sc.SC_Compare("import"))
		{
			sc.SC_MustGetString();
			if (!discard)
			{
				wxString s = sc.sc_String;
				ParseConfig(s, false, false, false, false, false, true);
			}
		}
		else if (sc.SC_Compare("type"))
		{
			ArgType * argt = discard? NULL : new ArgType;
			sc.SC_MustGetString();
			if (!discard) argt->name=sc.sc_String;
			sc.SC_MustGetStringName("{");
			while (!sc.SC_CheckString("}"))
			{
				static const char * tokens[]={"value", "flag", NULL};
				sc.SC_MustGetString();
				switch (sc.SC_MustMatchString(tokens))
				{
				case 0:
					sc.SC_MustGetNumber();
					if (!discard) argt->value_values.Push(sc.sc_Number);
					sc.SC_MustGetString();
					if (!discard) argt->values.Add(sc.sc_String);
					break;

				case 1:
					sc.SC_MustGetNumber();
					if (!discard) argt->flag_values.Push(sc.sc_Number);
					sc.SC_MustGetString();
					if (!discard) argt->flags.Add(sc.sc_String);
					break;


				default:
					return;
				}
			}
			if (!discard) ArgTypes.Push(argt);
		}
	}
}

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

SectorSpecial * GameConfig::ParseSector(ScriptMan & sc)
{
	SectorSpecial * ols = new SectorSpecial;
	sc.SC_MustGetNumber();
	ols->number = sc.sc_Number;
	if (ols->number!=-1)
	{
		sc.SC_MustGetString();
		ols->descript = sc.sc_String;
	}
	return ols;
}


/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

SectorGroup * GameConfig::ParseSectorGroup(ScriptMan & sc, bool discard)
{
	SectorGroup * tg = NULL;
	bool old=false;
	sc.SC_MustGetString();
	if (!discard) for(int i=0;i<SectorGroups.Size();i++)
	{
		if (SectorGroups[i]->name == sc.sc_String)
		{
			tg=SectorGroups[i];
			old=true;
			break;
		}
	}
	if (tg==NULL) 
	{
		tg=new SectorGroup;
		tg->name = sc.sc_String;

		if (sc.SC_CheckString(":"))
		{
			sc.SC_MustGetString();
			tg->parent=sc.sc_String;
		}
	}

	if (sc.TestBraceOpn())
	{
		while (!sc.TestBraceCls())
		{
			sc.SC_MustGetString();
			if (sc.SC_Compare("clear"))
			{
				tg->Clear();
			}
			else
			{
				sc.SC_UnGet();
				SectorSpecial * td = ParseSector(sc);
				if (!discard) 
				{
					tg->Push(td);
					if (td->number >=0 && td->number<256) SectorMap[td->number]=td;
					else if (td->number <0 && td->number>-256) SectorMap[32768-td->number]=td;
				}
				else delete td;
			}
		}
	}
	if (discard)
	{
		delete tg;
		return NULL;
	}
	return old?NULL:tg;
}

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

void GameConfig::ParseSectors(ScriptMan & sc, bool discard)
{
	if (sc.TestBraceOpn())
	{
		while (!sc.TestBraceCls())
		{
			sc.SC_MustGetString();
			if (sc.SC_Compare("import"))
			{
				sc.SC_MustGetString();
				if (!discard)
				{
					wxString s = sc.sc_String;
					ParseConfig(s, false, false, false, false, true, false);
				}
			}
			else if (sc.SC_Compare("generic"))
			{
				if (!discard) genericsectors=true;
			}
			else if (sc.SC_Compare("heretic"))
			{
				if (!discard) hereticsectors=true;
			}
			else if (sc.SC_Compare("group"))
			{
				SectorGroup * tg = ParseSectorGroup(sc, discard);
				if (tg) SectorGroups.Push(tg);
			}
			else if (sc.SC_Compare("textmapflags"))
			{
				int flagindex = 0;
				if (!discard) SectorGroupCount = 0;

				sc.SC_MustGetStringName("{");
				while (!sc.SC_CheckString("}"))
				{
					sc.SC_MustGetStringName("Group");
					sc.SC_MustGetString();
					if (!discard)
					{
						TextMapSectorFlagGroups[SectorGroupCount].name = sc.sc_String;
						TextMapSectorFlagGroups[SectorGroupCount].firstindex = flagindex;
						TextMapSectorFlagGroups[SectorGroupCount].modeflag = 0;
					}
					sc.SC_MustGetStringName("{");
					while (!sc.SC_CheckString("}"))
					{
						if (flagindex >=MAX_FLAGS)
						{
							sc.SC_ScriptError("Too many Sector flags");
						}
						sc.SC_MustGetString();
						if (!discard) TextMapSectorFlags[flagindex].shortname = sc.sc_String;
						sc.SC_MustGetString();
						if (!discard) TextMapSectorFlags[flagindex].longname = sc.sc_String;
						sc.SC_MustGetString();
						if (!discard) TextMapSectorFlags[flagindex].textname = sc.sc_String;
						flagindex++;
					}
					if (!discard)
					{
						TextMapSectorFlagGroups[SectorGroupCount].lastindex = flagindex-1;
						SectorGroupCount++;
					}
				}
			}
		}
	}
}


/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/
bool GameConfig::ParseConfig(wxString config, bool things, bool keys, bool lines, bool sides, bool sectors, bool argtypes)
{
	ScriptMan sc;
	bool thisfile_extended = false;
	bool parseall = things && keys && lines && sectors && sides;

	wxString fn = GetConfigDir() + config;

	sc.SC_OpenFile(fn);

	if (!sc.SC_GetString())
	{
		sc.SC_ScriptError("%s:\nConfig file not found", config.c_str());
	}
	if (!sc.SC_Compare("MAPFORMAT"))
	{
		// Only check for top level, not for includes!
		if (parseall)
		{
			sc.SC_ScriptError("%s:\nInvalid Config file", config.c_str());
		}
		else
		{
			sc.SC_UnGet();
		}
	}
	else
	{
		sc.SC_MustGetString();
		if (sc.SC_Compare("DOOM"))
		{
			thisfile_extended=false;
			if (parseall)
			{
				IsExtended=false;
			}
		}
		else if (sc.SC_Compare("HEXEN"))
		{
			thisfile_extended=true;
			if (parseall)
			{
				IsExtended=true;
			}
		}
		else
		{
			sc.SC_ScriptError("%s:\nInvalid Config file", config.c_str());
		}
	}

	if (sc.SC_CheckString("NAMESPACE"))
	{
		sc.SC_MustGetString();
		if (parseall)
		{
			namespc = sc.sc_String;
		}
	}

	if (sc.SC_CheckString("ALTCONFIG"))
	{
		sc.SC_MustGetString();
		if (parseall)
		{
			altconfig = sc.sc_String;
		}
	}
	if (sc.SC_CheckString("DEFAULTCONFIG"))
	{
		sc.SC_MustGetString();
		if (parseall)
		{
			defaultconfig = strupr(sc.sc_String);
		}
	}
	
	while (sc.SC_GetString())
	{
		if (sc.SC_Compare("THINGS"))
		{
			ParseThings(sc, !things);
		}
		else if (sc.SC_Compare("KEYS"))
		{
			ParseKeys(sc, !keys);
		}
		else if (sc.SC_Compare("LINES"))
		{
			ParseLines(sc, thisfile_extended, !lines);
		}
		else if (sc.SC_Compare("SIDES"))
		{
			ParseSides(sc, !sides);
		}
		else if (sc.SC_Compare("ARGTYPES"))
		{
			ParseArgTypes(sc, !argtypes);
		}
		else if (sc.SC_Compare("SECTORS"))
		{
			ParseSectors(sc, !sectors);
		}
	}
	sc.SC_Close();
	return true;
}



void GameConfig::InitConfig(wxString filename)
{
	thisconfig=filename;
	ParseConfig(filename, true, true, true, true, true, true);

	for(int i=0;i<32768;i++)
	{
		if (ThingMap[i]!=NULL)
			InitColor(&ThingMap[i]->realpens,ThingMap[i]->penname.c_str());
	}

	hmThings=MakeThingMenu();
	hmLines=MakeLineMenu();
	hmSectors=MakeSectorMenu();
}




/***************************************************************************/
/*                                                                         */
/*  Thing Menu management                                                  */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

void GameConfig::AddThingToMenu(wxMenu * hm,ThingDesc * td)
{
	wxString label;

	if (td->DoomEdNum>=0 && td->DoomEdNum<32768) ThingMap[td->DoomEdNum]=td;
	label.Printf("[%04d] %s", td->DoomEdNum, td->name.c_str());
	hm->Append(2000+td->DoomEdNum, label);
}

/***************************************************************************/
/*                                                                         */
/***************************************************************************/
wxMenu * GameConfig::MakeThingGroupMenu(ThingGroup & group)
{
	wxMenu * hm = new wxMenu;
	for(int j=0;j<group.Size();j++)
	{
		if (j && !(j%33)) hm->Break();
		AddThingToMenu(hm,group[j]);
	}
	return hm;
}

/***************************************************************************/
/*                                                                         */
/***************************************************************************/

wxMenu * GameConfig::MakeThingMenu()
{
	TArray<wxMenu *> menus;

	wxMenu * hm = new wxMenu;

	for (int i=0;i<ThingGroups.Size();i++)
	{
		wxMenu * hmsub=MakeThingGroupMenu(*ThingGroups[i]);

		menus.Push(hmsub);

		if (!!ThingGroups[i]->parent)
		{
			for (int j=0;j<ThingGroups.Size();j++)
			{
				if (ThingGroups[j]->name == ThingGroups[i]->parent)
				{
					menus[j]->Append(-1, ThingGroups[i]->name, hmsub);
					hmsub=NULL;
					break;
				}
			}
		}
		if (hmsub!=NULL)
		{
			hm->Append(-1, ThingGroups[i]->name, hmsub);
		}
	}
	wxMenu * hmDecorate=InitializeThings();
	if (hmDecorate)
	{
		hm->Append(-1, "DECORATE", hmDecorate);
	}
	return hm;
}


/***************************************************************************/
/*                                                                         */
/*  Linedef Menu management                                                */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

bool GameConfig::AddLineToMenu(wxMenu * hm,LineSpecial * td)
{
	wxString label;

	if (td->number<0)
	{
		hm->AppendSeparator();
		return false;	// separators don't count!
	}

	if (!!td->penname) InitColor(&td->realpen,td->penname.c_str());
	if (!IsExtended)
	{
		if (td->number<8192) LineMap[td->number]=td;
		label.Printf("[%03d] %s",td->number,td->descript.c_str());

		int pos = label.Find("%k");
		if (pos>=0)
		{
			label.Truncate(pos);
			if (td->param>=0 && td->param<256 && LockMap[td->param])
			{
				label += LockMap[td->param]->descript;
			}
			else
			{
				label += wxString::Format("Unknown key #%d",td->param);
			}
		}
	}
	else
	{
		if (td->number>=256) return false;

		LineMap[td->number]=td;
		label.Printf("[%03d] %s",td->number,td->descript.c_str());
	}
	hm->Append(2000+td->number, label);
	return true;
}

/***************************************************************************/
/*                                                                         */
/***************************************************************************/
wxMenu * GameConfig::MakeLineGroupMenu(LineGroup & group)
{
	wxMenu * hm = new wxMenu;
	for(int j=0;j<group.Size();j++)
	{
		AddLineToMenu(hm,group[j]);
	}
	return hm;
}

/***************************************************************************/
/*                                                                         */
/***************************************************************************/
wxMenu * GameConfig::MakeLineMenu()
{
	TArray<wxMenu *> menus;

	wxMenu * hm=new wxMenu;

	for (int i=0;i<LineGroups.Size();i++)
	{
		wxMenu * hmsub=MakeLineGroupMenu(*LineGroups[i]);

		menus.Push(hmsub);

		if (!!LineGroups[i]->parent)
		{
			for (int j=0;j<LineGroups.Size();j++)
			{
				if (LineGroups[j]->name == LineGroups[i]->parent)
				{
					menus[j]->Append(-1, LineGroups[i]->name, hmsub);
					hmsub=NULL;
					break;
				}
			}
		}
		if (hmsub!=NULL)
		{
			hm->Append(-1, LineGroups[i]->name, hmsub);
		}
	}
	if (genericlinedefs)
	{
		wxMenu * hmsub = new wxMenu;

		hmsub->Append(50000, "Generic Doors...");
		hmsub->Append(50001, "Generic Locked Doors...");
		hmsub->Append(50002, "Generic Floors...");
		hmsub->Append(50003, "Generic Ceilings...");
		hmsub->Append(50004, "Generic Lifts...");
		hmsub->Append(50005, "Generic Stairs...");
		hmsub->Append(50006, "Generic Crushers...");
		hm->Append(-1, "Generic Types", hmsub);
	}
	return hm;
}


/***************************************************************************/
/*                                                                         */
/* Sector Menu management			                                       */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

bool GameConfig::AddSectorToMenu(wxMenu * hm,SectorSpecial * td)
{

	wxString label;
	int menuid;

	if (td->number<0)
	{
		if (!IsExtended) td->number=32768-td->number;
		else return false;
	}

	if (td->number<32768) 
	{
		if (td->number<256) SectorMap[td->number]=td;
		label.Printf("[%03d] %s",td->number,td->descript.c_str());
		menuid=td->number+2000;
	}
	else 
	{
		int n=td->number&32767;
		if (n<256) SectorMap[td->number]=td;
		label.Printf("[X%03d] %s",n,td->descript.c_str());
		menuid=td->number-32768+2500;
	}
	hm->Append(menuid, label);
	return true;
}

/***************************************************************************/
/*                                                                         */
/***************************************************************************/
wxMenu * GameConfig::MakeSectorGroupMenu(SectorGroup & group)
{
	wxMenu * hm = new wxMenu;
	for(int j=0;j<group.Size();j++)
	{
		AddSectorToMenu(hm,group[j]);
	}
	return hm;
}

/***************************************************************************/
/*                                                                         */
/***************************************************************************/
wxMenu * GameConfig::MakeSectorMenu()
{
	TArray<wxMenu *> menus;

	wxMenu * hm=new wxMenu;

	for (int i=0;i<SectorGroups.Size();i++)
	{
		wxMenu * hmsub=MakeSectorGroupMenu(*SectorGroups[i]);

		menus.Push(hmsub);

		if (!!SectorGroups[i]->parent)
		{
			for (int j=0;j<SectorGroups.Size();j++)
			{
				if (SectorGroups[j]->name == SectorGroups[i]->parent)
				{
					menus[j]->Append(-1, SectorGroups[i]->name, hmsub);
					hmsub=NULL;
					break;
				}
			}
		}
		if (hmsub!=NULL)
		{
			hm->Append(-1, SectorGroups[i]->name, hmsub);
		}
	}
	return hm;
}




/***************************************************************************/
/*                                                                         */
/***************************************************************************/
wxString GameConfig::GetThingName(int thing, bool withnumber,bool forcedoom)
{
	wxString ret;


	ThingDesc * td=thing>=0 && thing<32768? ThingMap[thing] : NULL;

	if (!td)
	{
		ret.Printf("<UNKNOWN %04d>", thing);
		return ret;
	}
	else if (withnumber)
	{
		ret.Printf("[%04d] %s",thing,td->name.c_str());
		return ret;
	}
	else return td->name;
}

/***************************************************************************/
/*                                                                         */
/***************************************************************************/
wxString GameConfig::GetThingSprite(int thing)
{
	ThingDesc * td=thing>=0 && thing<32768? ThingMap[thing] : NULL;

	if (td) return td->sprite;
	return "";
}

/***************************************************************************/
/*                                                                         */
/***************************************************************************/
wxString GameConfig::GetThingArg(int thing, int arg)
{
	ThingDesc * td=thing>=0 && thing<32768? ThingMap[thing] : NULL;

	if (td && arg>=0 && arg<5) return td->argdescript[arg];
	return "";
}

/***************************************************************************/
/*                                                                         */
/***************************************************************************/
wxColour GameConfig::GetThingColor(int thing)
{
	ThingDesc * td=thing>=0 && thing<32768? ThingMap[thing] : NULL;
	CColor * col;
//bool printerpen=thing>65535;

	if (!td || !td->realpens) col = Colors.GetPen("Invalid");
	//else if (printerpen && td->realpens->printerpen.m_hObject) return &td->realpens->printerpen;
	else col = td->realpens;
	if (col) return col->pen;
	else return dummypen;
}

/***************************************************************************/
/*                                                                         */
/***************************************************************************/
int GameConfig::GetThingRadius( int thing)
{
	ThingDesc * td=thing>=0 && thing<32768? ThingMap[thing] : NULL;

	if (!td) return 2*20;
	else return 2*td->radius;
}


/***************************************************************************/
/*                                                                         */
/***************************************************************************/
wxString GameConfig::GetLDName(int type)
{
	wxString ret;

	if (type>=8192)
	{
		ret = GetGenText(type);
		if (ret.length()==0) ret.Printf("[%03d] ?? UNKNOWN",type);
	}
	else if (type<0 || !LineMap[type]) ret.Printf("[%03d] ?? UNKNOWN",type);
	else 
	{
		LineSpecial * td=LineMap[type];
		ret.Printf("[%03d] %s",td->number,td->descript.c_str());

		int pos = ret.Find("%k");
		if (pos>=0)
		{
			ret.Truncate(pos);
			if (td->param>0 && td->param<256 && LockMap[td->param])
			{
				ret += LockMap[td->param]->descript;
			}
			else
			{
				ret += wxString::Format("Unknown key #%d",td->param);
			}
		}
	}
	return ret;
}

/***************************************************************************/
/*                                                                         */
/***************************************************************************/
const char * GameConfig::GetLDTrigger(int type)
{
	const char * GetGenTrigger(int type);

	if (type>=0x2f80 && type<32768) return GetGenTrigger(type);
	else if (type<0 || type>8192 || !LineMap[type]) return "--";
	else return LineMap[type]->descript.c_str();
}

/***************************************************************************/
/*                                                                         */
/***************************************************************************/
void GameConfig::SectorType(wxString & str, int type)
{
	if (SectorMap[type] && !!SectorMap[type]->descript) 
	{
		str += SectorMap[type]->descript;
	}
	else str += "UNKNOWN";
}

void GameConfig::SectorBits(wxString & buffer, int flags)
{
	if (flags&0x1f00)
	{
		buffer += " ( ";
		if (flags&0x300)
		{
			static const char * dtypes[]={"", "2/5% damage ", "5/10% damage ", "10/20% damage "};
			buffer += dtypes[(flags>>8)&3];
		}
		if (flags&0x400) buffer += "Secret ";
		if (flags&0x800) buffer += "Wind ";
		if (flags&0x1000) buffer += "Friction ";
		buffer += ")";
	}
}

wxString GameConfig::GetSectorName(int type, bool wantnumber)
{
	wxString buildbuffer;
	if (wantnumber) buildbuffer.Printf("[%03d] ", type);
	if (IsExtended)
	{
		SectorType(buildbuffer, type&255);
		SectorBits(buildbuffer, type&0x1f00);
	}
	else 
	{
		if (type&32768)
		{
			SectorType(buildbuffer, type&0x80ff);
			SectorBits(buildbuffer, type&0x7f00);
		}
		else
		{
			SectorType(buildbuffer, genericsectors? type&(hereticsectors? 63:31):type);
			if (genericsectors)
			{
				SectorBits(buildbuffer, (type& (hereticsectors? 0x380:0x3e0) ) << 3);
			}
		}
	}
	return buildbuffer;
}

bool GameConfig::IsSectorType(int type)
{
	return (SectorMap[type] && !!SectorMap[type]->descript) ;
}


static CColor *defaultc;
static CColor *twosided;
static CColor *invalid;
static CColor *genericW;
static CColor *genericS;
static CColor *genericG;
static CColor *genericD;
static CColor *line_W;
static CColor *line_S;
static CColor *line_G;
static CColor *line_D;
static CColor *line_M;
static CColor *line_P;
static CColor *line_R;
static CColor *line_3DM;
static CColor *line_BE;
/***************************************************************************/
/*                                                                         */
/***************************************************************************/
wxColour GameConfig::GetLDColor(CLineDef * ld, bool isTextMap)
{
	static bool initcolors=false;
	CColor * penc=NULL;

	if (!initcolors)
	{
		initcolors=true;
		InitColor(&defaultc,"Line");
		InitColor(&twosided,"Line2S");
		InitColor(&invalid,"LineInvalid");
		InitColor(&genericW,"LineGenericWalk");
		InitColor(&genericS,"LineGenericSwitch");
		InitColor(&genericG,"LineGenericShoot");
		InitColor(&genericD,"LineGenericDoor");
		InitColor(&line_W,"LineWalk");
		InitColor(&line_S,"LineSwitch");
		InitColor(&line_G,"LineShoot");
		InitColor(&line_D,"LineDoor");
		InitColor(&line_M,"LineMonster");
		InitColor(&line_P,"LinePush");
		InitColor(&line_R,"LineMCross");
		InitColor(&line_3DM,"Line3DMid");
		InitColor(&line_BE,"LineBE");
	}
	if (!ld) return dummypen;

	int type=ld->type;

	if (!IsExtended)
	{
		if (type<0) penc=invalid;
		else if (type==0)
		{
			if (ld->Sidedef2 == NO_SIDE) penc=defaultc;
			else penc=twosided;
		}
		else if (type>=GENERIC_START && type <GENERIC_END && genericlinedefs)
		{
			int lock=GetGenLock(type);
			if (lock!=-1)
			{
				Lock * l=LockMap[lock];
				if (l && l->realpen) penc = l->realpen;
				else penc = invalid;
			}
			else
			{
				switch(GetGenTrigger(type)[8])
				{
				case 'W': 
					penc=genericW;
					break;
				case 'S': 
					penc=genericS;
					break;
				case 'G':
					penc=genericG;
					break;
				case 'D': 
					penc=genericD;
					break;
				}
			}
		}
		else if (type<8192 && LineMap[type]) 
		{
			if (strstr(LineMap[type]->descript.c_str(),"%k") && LockMap[LineMap[type]->param]) 
			{
				penc=LockMap[LineMap[type]->param]->realpen;
			}
			else
			{
				if (!LineMap[type]->realpen)
				{
					InitColor(&LineMap[type]->realpen,LineMap[type]->penname.c_str());
					if (!LineMap[type]->realpen) LineMap[type]->realpen=invalid;
				}
				penc=LineMap[type]->realpen;
			}
		}
	}
	else if (!isTextMap)
	{
		int type=ld->type;

		if (type==0 /*|| type==121*/)
		{
			if (ld->Sidedef2==NO_SIDE) penc=defaultc;
			else penc=twosided;
		}
		else
		{
			if (!LineMap[type]) penc = invalid;
			else if (LineMap[type]->realpen) penc = LineMap[type]->realpen;
			else
			{
				for(int i=0;i<5;i++)
				{
					if (!strnicmp(LineMap[type]->args[i].c_str(),"K:",2))
					{
						int lock=ld->args[i];
						if (lock>=0 && lock<256 && LockMap[lock])
						{
							InitColor(&LockMap[lock]->realpen,LockMap[lock]->penname.c_str());
							penc = LockMap[lock]->realpen;
						}
					}
				}
				if (penc==NULL)
				{
					static CColor ** typemap[]={&line_W,&line_S,&line_M,&line_G,&line_P,&line_R,&line_S,&line_R};
					int ltype=(ld->Flags.GetShort()&0x1c00)>>10;

					if ((ltype==1 || ltype==6) && ld->args[0]==0 && (!LineMap[type]->args[0].c_str() || !strnicmp(LineMap[type]->args[0].c_str(),"Tag:",2)))
					{
						penc = line_D;
					}
					else penc = *typemap[ltype];
				}
			}
		}
	}
	else
	{
		if (type==0 /*|| type==121*/)
		{
			if (ld->Sidedef2==NO_SIDE) penc=defaultc;
			else penc=twosided;
		}
		else
		{
			if (!LineMap[type]) penc = invalid;
			else if (LineMap[type]->realpen) penc = LineMap[type]->realpen;
			else
			{
				#pragma message ("todo")
			}
		}
	}
	// Make sure it returns a valid pen in any case!
	if (!penc) return dummypen;
	return penc->pen;
}


/***************************************************************************/
/*                                                                         */
/***************************************************************************/
wxString GameConfig::GetLDXName(unsigned char type)
{
	wxString temp;

	if (LineMap[type]) temp.Printf("[%03d] %s",type,LineMap[type]->descript);
	else temp.Printf("[%03d] UNKNOWN",type);
	return temp;
}

/***************************************************************************/
/*                                                                         */
/***************************************************************************/
wxString GameConfig::GetLineDefDescription(int type,int trigger,int * args, int spacenumber)
{
	wxString buffer;

	static const char Trigger[]="WSMGPRUC";
	if (spacenumber==1) buffer = "      ";
	else if (spacenumber==0) buffer.Printf("[%03d] ",type);
	if (LineMap[type]) 
	{
		if (!LineMap[type]->param && trigger>=0 && trigger<15)
		{
			buffer += Trigger[trigger>>1];
			buffer += (trigger&1)? 'R':'1';
			buffer += ' ';
		}
		else if (trigger>0) buffer += "-- ";
		buffer += LineMap[type]->descript;

		if (args) for(int a=0;a<5;a++)
		{
			if (!strnicmp(LineMap[type]->args[a].c_str(),"K:",2))
			{
				buffer += ", ";
				int lock=args[a];
				if (lock>=0 && lock<256 && LockMap[lock])
				{
					buffer += LockMap[lock]->descript;
				}
				else
				{
					buffer += wxString::Format("Unknown key #%d",lock);
				}
				break;
			}
		}
	}
	else buffer.Printf("[%03d] UNKNOWN",type);
	return buffer;
}

wxString GameConfig::GetLineDefDescription(CLineDef * ld,int spacenumber)
{
	if (!IsExtended) return GetLDName(ld->type);
	else 
	{
		int trig = (ld->Flags.GetShort()&0x1e00)>>9;
		// todo: textmap
		return GetLineDefDescription(ld->type, trig, ld->args, spacenumber);
	}
}

//==========================================================================
//
//
//
//==========================================================================

int GameConfig::GetTagArg(int type)
{
	if (LineMap[type] && type!=0)  return LineMap[type]->tagmask;
	return 0;
}

//==========================================================================
//
//
//
//==========================================================================

int GameConfig::GetTidArg(int type)
{
	if (LineMap[type] && type!=0)  return LineMap[type]->tidmask;
	return 0;
}

//==========================================================================
//
//
//
//==========================================================================

int GameConfig::GetLineIDArg(int type)
{
	if (LineMap[type] && type!=0)  return LineMap[type]->lineidmask;
	return 0;
}

//==========================================================================
//
//
//
//==========================================================================

wxString GameConfig::GetThingFlagText(int flag, int shortt, bool textmap)
{
	FlagDesc *flags = textmap? TextMapThingFlags : ThingFlags;
	if (flag>=0 && flag<= (textmap?MAX_FLAGS-1:31) )
	{
		if (shortt == 2)
		{
			return flags[flag].textname;
		}
		else if (shortt) 
		{
			return flags[flag].shortname;
		}
		else
		{
			return flags[flag].longname;
		}
	}
	return "";
}

//==========================================================================
//
//
//
//==========================================================================

wxString GameConfig::GetLineFlagText(int flag, int shortt, bool textmap)
{
	FlagDesc *flags = textmap? TextMapLineFlags : LineFlags;
	if (flag>=0 && flag<=(textmap?MAX_FLAGS-1:31))
	{
		if (shortt == 2)
		{
			return flags[flag].textname;
		}
		else if (shortt) 
		{
			return flags[flag].shortname;
		}
		else
		{
			return flags[flag].longname;
		}
	}
	return "";
}


//==========================================================================
//
//
//
//==========================================================================

wxString GameConfig::GetSideFlagText(int flag, int shortt, bool textmap)
{
	if (textmap)
	{
		FlagDesc *flags = TextMapSideFlags;
		if (flag>=0 && flag<=(textmap?MAX_FLAGS-1:31))
		{
			if (shortt == 2)
			{
				return flags[flag].textname;
			}
			else if (shortt) 
			{
				return flags[flag].shortname;
			}
			else
			{
				return flags[flag].longname;
			}
		}
	}
	return "";
}


//==========================================================================
//
//
//
//==========================================================================

wxString GameConfig::GetSectorFlagText(int flag, int shortt)
{
	FlagDesc *flags = TextMapSectorFlags;
	if (flag>=0 && flag<=MAX_FLAGS)
	{
		if (shortt == 2)
		{
			return flags[flag].textname;
		}
		else if (shortt) 
		{
			return flags[flag].shortname;
		}
		else
		{
			return flags[flag].longname;
		}
	}
	return "";
}


//==========================================================================
//
//
//
//==========================================================================

int GameConfig::CheckTextMapThingFlag(const char *key)
{
	for(unsigned i=0;i<MAX_FLAGS;i++)
	{
		if (!stricmp(key, TextMapThingFlags[i].textname)) return i;
	}
	return -1;
}

//==========================================================================
//
//
//
//==========================================================================

int GameConfig::CheckTextMapLineFlag(const char *key)
{
	for(unsigned i=0;i<MAX_FLAGS;i++)
	{
		if (!stricmp(key, TextMapLineFlags[i].textname)) return i;
	}
	return -1;
}


//==========================================================================
//
//
//
//==========================================================================

int GameConfig::CheckTextMapSideFlag(const char *key)
{
	for(unsigned i=0;i<MAX_FLAGS;i++)
	{
		if (!stricmp(key, TextMapSideFlags[i].textname)) return i;
	}
	return -1;
}


//==========================================================================
//
//
//
//==========================================================================

int GameConfig::CheckTextMapSectorFlag(const char *key)
{
	for(unsigned i=0;i<MAX_FLAGS;i++)
	{
		if (!stricmp(key, TextMapSectorFlags[i].textname)) return i;
	}
	return -1;
}


//==========================================================================
//
//
//
//==========================================================================

GameConfig::GameConfig()
{
	memset(ThingMap, 0, sizeof(ThingMap));
	memset(LineMap, 0, sizeof(LineMap));
	memset(SectorMap, 0, sizeof(SectorMap));
	memset(LockMap, 0, sizeof(LockMap));
	SectorGroupCount = ThingGroupCount = LineGroupCount = 0;

	hmThings = hmLines = hmSectors= NULL;

	IsExtended=false;
	genericlinedefs=false;
}

//==========================================================================
//
//
//
//==========================================================================

GameConfig::~GameConfig()
{
	for(int i=0;i<256;i++) if (LockMap[i]) 
	{
		delete LockMap[i];
		LockMap[i]=NULL;
	}
	for(unsigned i=0;i<Objects.Size();i++)
	{
		free(Objects[i].classname);
		if (Objects[i].spriteframe) delete [] Objects[i].spriteframe;
	}
	for(unsigned i=0;i<CustomThings.Size();i++) delete CustomThings[i];

	if (hmThings) delete hmThings;
	if (hmLines) delete hmLines;
	if (hmSectors) delete hmSectors;
	hmThings = hmLines = hmSectors = NULL;
}



//==========================================================================
//
//
//
//==========================================================================

GameConfig * ConfigManager::GetConfig(wxString name)
{
	for(unsigned i=0;i<configs.Size();i++)
	{
		if (configs[i]->thisconfig == name) return configs[i];
	}
	GameConfig * gc=new GameConfig;
	gc->InitConfig(name);
	configs.Push(gc);
	return gc;
}

//==========================================================================
//
//
//
//==========================================================================

GameConfig * ConfigManager::GetRealConfig(bool extended, GameConfig * current)
{
	if (extended==current->IsExtended) return current;
	if (!!current->altconfig) return GetConfig(current->altconfig);
	return current;
}

//==========================================================================
//
//
//
//==========================================================================

GameConfig * ConfigManager::FindDefaultConfig(const char *game, bool extended, bool textmap)
{
	config.SetSection("Defaultconfigs", true);
	wxString gamename = game;

	if (extended) gamename += "_extended";
	if (textmap) gamename += "_textmap";

	const char *configname = config.GetValueForKey(gamename);
	if (configname == NULL) return NULL;
	return GetConfig(configname);
}

//==========================================================================
//
//
//
//==========================================================================

void ConfigManager::RefreshConfigs()
{
	wxString preserve = cgc? cgc->thisconfig : "";

	for(unsigned i=0;i<configs.Size();i++)
	{
		if (configs[i]!=cgc) delete configs[i];
	}
	configs.Clear();
	if (!!preserve) 
	{
		GameConfig * oldcgc = cgc;
		cgc = GetConfig(preserve);
		delete oldcgc;	// This must not be deleted until the new config has been created!
	}
		
}

