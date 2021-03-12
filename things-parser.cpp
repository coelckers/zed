//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2003-2005 Christoph Oelckers
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
//
// DECORATE parser

#include "StdAfx.h"
#include "ZEd.h"
#include "ResourceFile.h"
#include "GameConfig.h"
#include "sc_man.h"

static const char * flagnames[]={
	"NOSECTOR",
	"COUNTKILL",
	"SPECIAL",
	"MONSTER",
	"COUNTITEM",
	"INVBAR",
	"ALWAYSPICKUP",
	"µ",
	"µ",
	"µ",
	"FRIENDLY",
	NULL
};
						
enum
{
	FLAG_NOSECTOR=1,
	FLAG_COUNTKILL=2,
	FLAG_SPECIAL=4,
	FLAG_MONSTER=8,
	FLAG_COUNTITEM=16,
	FLAG_INVBAR=32,
	FLAG_ALWAYSPICKUP=64,
	FLAG_SEE=128,
	FLAG_ATTACK=256,
	FLAG_DIE=512,
	FLAG_FRIENDLY=1024,
	FLAG_LIGHT=2048,
	FLAG_LARGE=4096
};


//////////////////////////////////////////////////////////////////////////////////77
//////////////////////////////////////////////////////////////////////////////////77
//////////////////////////////////////////////////////////////////////////////////77


void GameConfig::AddObject(mobjinfo_t * mo)
{
	ThingDesc * td = new ThingDesc;

	td->DoomEdNum=mo->doomednum;
	td->name=mo->classname;
	td->sprite=mo->spriteframe? mo->spriteframe:"";
	td->radius=20;

	if (mo->flags&FLAG_MONSTER && mo->flags&FLAG_COUNTKILL)  mo->penname=NULL;

	if (mo->penname)
	{
		
		if (!strncmp(mo->penname,"Bonus",5)) 
		{
			if (mo->flags&(FLAG_COUNTITEM|FLAG_INVBAR|FLAG_ALWAYSPICKUP)) td->statclass=STAT_ITEM;
			else td->statclass=STAT_NCITEM;		
			if (!strcmp(mo->penname,"Bonus1"))
			{
				if (mo->flags&(FLAG_COUNTITEM|FLAG_INVBAR|FLAG_ALWAYSPICKUP)) mo->penname="Bonus2";
			}
		}
		else if (!strcmp(mo->penname,"Key")) td->statclass=STAT_KEY;
		else if (!strcmp(mo->penname,"Weapon")) td->statclass=STAT_WEAPON;
		else if (!strcmp(mo->penname,"Ammo")) td->statclass=STAT_AMMO;
		else if (!strcmp(mo->penname,"EnemyExtra")) goto redo;
		else if (!strcmp(mo->penname,"Friendly")) goto redo;
	}
	else
	{
redo:
		if (mo->flags&FLAG_FRIENDLY) 
		{
			mo->penname="Friendly";
			td->statclass=STAT_FRIENDLY;
		}
		else if (mo->flags&FLAG_MONSTER && mo->flags&FLAG_COUNTKILL) 
		{
			mo->penname="EnemyExtra";
			td->statclass=STAT_MONSTER;
		}
		else if (mo->flags&FLAG_ATTACK || mo->flags&FLAG_SEE) 
		{
			mo->penname="Special";
			td->statclass=STAT_SPECIAL;
		}
		else if (mo->flags&FLAG_DIE || mo->flags&FLAG_NOSECTOR) mo->penname="Shootable";
		else mo->penname="DecoExtra";
	}
	td->penname=mo->penname;
	td->realpens=Colors.GetPen(td->penname.c_str());
	Objects.Push(*mo);
	if (mo->doomednum!=-1)
	{
		wxString buffer;

		if (!ThingMap[mo->doomednum])
		{
			ThingMap[mo->doomednum]=td;
			CustomThings.Push(td);

			buffer.Printf("[%04d] %s",mo->doomednum,mo->classname);
			if (menucount%32==31) hmg->Break();
			hmg->Append(2000+mo->doomednum, buffer);
			menucount++;
		}
		else delete td;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
mobjinfo_t * GameConfig::FindMobjInfo(const char * name)
{
	for(unsigned int i=0;i<Objects.Size();i++)
	{
		if (!strcmp(Objects[i].classname,name)) return &Objects[i];
	}
	return NULL;
}


mobjinfo_t * GameConfig::NewMobj(mobjinfo_t * info)
{
	memset(info,0,sizeof(*info));
	info->doomednum=-1;
	info->flags=FLAG_COUNTKILL;
	info->parentclassindex=-1;
	return info;
}

bool GameConfig::IsKindOf(mobjinfo_t * mo, const char * string)
{
	while (true)
	{
		if (!stricmp(mo->classname, string)) return true;
		if (mo->parentclassindex==-1) return false;
		mo=&Objects[mo->parentclassindex];
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
mobjinfo_t * GameConfig::CreateNewMobj(ScriptMan & sc,mobjinfo_t ** parent)
{
	static mobjinfo_t i;

	mobjinfo_t * info=NewMobj(&i);

	sc.SC_MustGetString();
	info->classname=_strdup(sc.sc_String);

	if (parent)
	{
		*parent=NULL;
		sc.SC_MustGetString();
		if (sc.SC_Compare(":"))
		{
			sc.SC_MustGetString();
			mobjinfo_t * parentc=FindMobjInfo(sc.sc_String);
			if (!parentc)
			{
				mobjinfo_t parentd;
				NewMobj(&parentd);
				parentd.classname=_strdup(sc.sc_String);
				parentd.spriteframe=NULL;
				parentd.flags=0;
				parentc=&Objects[Objects.Push(parentd)];
			}
			
			
			short saveden=info->doomednum;
			char * savecls=info->classname;
			
			*info=*parentc;
			info->doomednum=saveden;
			info->classname=savecls;
			if (info->spriteframe) info->spriteframe=_strdup(info->spriteframe);
			info->parentclassindex=parentc-&Objects[0];
			if (parent) *parent=parentc;

			if (IsKindOf(info, "Inventory") || IsKindOf(info, "FakeInventory") || IsKindOf(info, "Pickup") ||
				IsKindOf(info, "BasicArmorPickup") || IsKindOf(info, "BasicArmorBonus") ||
				IsKindOf(info, "HexenArmor") || IsKindOf(info, "Health") || IsKindOf(info, "HealthPickup"))
			{
				info->penname="Bonus1";
			}
			else if (IsKindOf(info, "Ammo") || IsKindOf(info, "Backpack"))
			{
				info->penname="Ammo";
			}
			else if (IsKindOf(info, "Weapon") || IsKindOf(info, "WeaponPiece"))
			{
				info->penname="Weapon";
			}
			else if (IsKindOf(info, "Key") || IsKindOf(info, "PuzzleItem"))
			{
				info->penname="Key";
			}
			else if (IsKindOf(info, "PowerupGiver"))
			{
				info->penname="Bonus3";
			}
		}
		else sc.SC_UnGet();
	}
	
	if (sc.SC_GetNumber()) info->doomednum=sc.sc_Number;
	else info->doomednum=-1;

	//if (dontlist) info->doomednum=-1;

	return info;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void SkipToEOL(ScriptMan & sc)
{
	while (sc.SC_GetString() && !sc.sc_Crossed);
	if (!sc.sc_End) sc.SC_UnGet();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GameConfig::ProcessStates(ScriptMan & sc,mobjinfo_t * info)
{
	sc.ChkBraceOpn();
	while (!sc.TestBraceCls())
	{
		sc.SC_MustGetString();
		if (sc.SC_Compare("MELEE:") || sc.SC_Compare("MISSILE:"))
		{
			info->flags|=FLAG_ATTACK;
		}
		else if (sc.SC_Compare("DEATH:"))
		{
			info->flags|=FLAG_DIE;
		}
		else if (sc.SC_Compare("SEE:"))
		{
			info->flags|=FLAG_SEE;
		}
		else if (sc.SC_Compare("SPAWN:"))
		{
			sc.SC_MustGetString();
			info->spriteframe=new char[9];
			sprintf(info->spriteframe,"%.4sA0",sc.sc_String);
		}
		else if (sc.sc_String[0]!=0 && sc.sc_String[strlen(sc.sc_String)-1]==':')
		{
		}

		else SkipToEOL(sc);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
mobjinfo_t * GameConfig::ProcessActor(ScriptMan & sc,bool subclass)
{
	int currentstate=0;
	mobjinfo_t * parent=NULL;

	mobjinfo_t * info=CreateNewMobj(sc,&parent);

	do
	{
		sc.SC_GetString();
	}
	while (!sc.SC_Compare("{"));

	while (sc.SC_GetString())
	{
		if (sc.SC_Compare("}")) goto finish;

		if (sc.SC_Compare("SKIP_SUPER"))
		{
			info->flags=0;
			info->spriteframe=NULL;
			info->penname=NULL;
		}
		else if (sc.SC_Compare("STATES"))
		{
			ProcessStates(sc,info);
		}
		else if (sc.SC_Compare("MONSTER"))
		{
			info->penname=NULL;
			info->flags|=FLAG_MONSTER;
		}
		else
		{
			char mod=*sc.sc_String;
			if (mod=='+' || mod== '-')
			{
				if (sc.sc_String[1]) memmove(sc.sc_String,sc.sc_String+1,strlen(sc.sc_String));
				else sc.SC_MustGetString();
				int v=sc.SC_MatchString(flagnames);
				if (v!=-1)
				{
					int fmod=1<<v;
					if (mod=='+') info->flags|=fmod;
					else if (mod=='-') info->flags&=~fmod;
				}
			}
			else
			{
				// ignore everything we can't use
				SkipToEOL(sc);
			}
		}
	}
finish:
	AddObject(info);
	return info;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GameConfig::ParseDecorateObject(ScriptMan & sc,int def)
{
	static char * pens[]={"Bonus1","DecoExtra","Invalid","Shootable"};
	mobjinfo_t i;
	mobjinfo_t * info=NewMobj(&i);

	sc.SC_MustGetString ();
	info->classname=_strdup(sc.sc_String);

	while (sc.SC_MustGetString (),!sc.SC_Compare ("{")) ;

	while (sc.SC_MustGetString (),!sc.SC_Compare ("}"))
	{
		if (sc.SC_Compare ("DoomEdNum"))
		{
			sc.SC_MustGetNumber ();
			info->doomednum = sc.sc_Number;
		}
		else if (sc.SC_Compare ("Sprite"))
		{
			sc.SC_MustGetString ();
			if (strlen (sc.sc_String) != 4)
			{
				sc.SC_ScriptError ("Sprite name must be exactly four characters long");
			}
			info->spriteframe=new char[9];
			sprintf(info->spriteframe,"%.4sA0",sc.sc_String);
		}
		else SkipToEOL(sc);
	}

	info->flags=0;
	info->penname=pens[def];
	AddObject(info);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GameConfig::ProcessMobjScript(QWORD lump,wxMenu * hm)
{
	ScriptMan sc("{}|=(),+");
	sc.SC_OpenLumpNum (lump, "DECORATE");

	hmg=hm;

	try
	{
	while (sc.SC_SavePos(),sc.SC_GetString ())
	{
		if (sc.SC_Compare("#include"))
		{
			sc.SC_MustGetString();
			QWORD includelump = CResourceFile::GlobalFindLumpByFullName(sc.sc_String);
			if (includelump == ~0)
			{
				includelump = CResourceFile::GlobalFindLump(sc.sc_String);
				if (includelump == ~0)
				{
					sc.SC_ScriptError("Include lump '%s' not found", sc.sc_String);
				}
			}
			/*
			wxString filename = sc_String;
			wxMenu * hmnew= new wxMenu;

			if (menucount) hm->Append(-1, rf->GetName(), hmnew);
			else delete hmnew;
			*/
			ProcessMobjScript(includelump, hm);
		}
		else if (sc.SC_Compare("ACTOR"))
		{
			ProcessActor(sc,false);
		}
		else if (sc.SC_Compare("PICKUP"))
		{
			ParseDecorateObject(sc,0);
		}
		else if (sc.SC_Compare("PROJECTILE"))
		{
			ParseDecorateObject(sc,2);
		}
		else if (sc.SC_Compare("BREAKABLE"))
		{
			ParseDecorateObject(sc,3);
		}
		else
		{
			if (sc.TestBraceOpn())
			{
				sc.SC_RestorePos();
				ParseDecorateObject(sc,1);
			}
			else
			{
				sc.SC_RestorePos();
				sc.SC_GetString();	// get the token again - it has been trashed by TestBraceOpn!
				sc.SC_Close();
				return;
			}
		}
	}
	}
	catch (...)
	{
	}
	sc.SC_Close();
}


void GameConfig::ParseDefThings()
{
	ScriptMan sc;
	int stat;
	int flags;
	char * penname;

	auto fn = GetConfigDir() + "items.itm";

	sc.SC_OpenFile(fn);
	while (sc.SC_GetString())
	{
		if (sc.SC_Compare("Friendlies"))
		{
			stat=STAT_FRIENDLY;
			flags=FLAG_FRIENDLY;
			penname="Friendly";
		}
		else if (sc.SC_Compare("Keys"))
		{
			stat=STAT_KEY;
			flags=0;
			penname="Key";
		}
		else if (sc.SC_Compare("Weapons"))
		{
			stat=STAT_WEAPON;
			flags=0;
			penname="Weapon";
		}
		else if (sc.SC_Compare("Ammo"))
		{
			stat=STAT_AMMO;
			flags=0;
			penname="Ammo";
		}
		else if (sc.SC_Compare("Monsters"))
		{
			stat=STAT_MONSTER;
			flags=FLAG_MONSTER|FLAG_COUNTKILL;
			penname="EnemyExtra";
		}
		else if (sc.SC_Compare("Small_Pickups"))
		{
			stat=STAT_NCITEM;
			flags=0;
			penname="Bonus1";
		}
		else if (sc.SC_Compare("Large_Pickups"))
		{
			stat=STAT_ITEM;
			flags=0;
			penname="Bonus3";
		}
		else if (sc.SC_Compare("Lights"))
		{
			stat=STAT_LIGHT;
			flags=0;
			penname="Light";
		}
		else
		{
			stat=STAT_NONE;
			flags=0;
			penname="DecoExtra";
		}
		sc.SC_MustGetStringName("{");
		while (!sc.TestBraceCls())
		{
			mobjinfo_t mi;
			sc.SC_MustGetString();

			mi.classname=_strdup(sc.sc_String);
			mi.doomednum=-1;
			mi.flags=flags;
			mi.parentclassindex=-1;
			mi.penname=penname;
			mi.spriteframe=0;
			AddObject(&mi);
		}
	}
	sc.SC_Close();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
wxMenu * GameConfig::InitializeThings()
{
	TArray<QWORD> list;
	//int lumpno,end;

	Objects.Clear();
	for(unsigned i=0;i<CustomThings.Size();i++) delete CustomThings[i];
	CustomThings.Clear();

	int lastlump=-1;

	ParseDefThings();

	if (CResourceFile::GlobalFindLumps("DECORATE", list))
	{
		wxMenu * hm= new wxMenu;
		for(unsigned i=0;i<list.Size();i++)
		{
			menucount=0;
			wxMenu * hmnew = new wxMenu;
			ProcessMobjScript(list[i],hmnew);

			CResourceFile * rf = CResourceFile::GetResourceFile(int(list[i]>>32)-1);

			if (menucount) hm->Append(-1, rf->GetName(), hmnew);
			else delete hmnew;
		}
		return hm;
	}
	return NULL;
}


