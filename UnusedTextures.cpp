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
// Finds unused textures in the current WAD
//

#include "stdafx.h"
#include "ZEd.h"
#include "doomerrors.h"
#include "ResourceFile.h"
#include "Level.h"
#include "texture.h"
#include "sc_man.h"

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
static char * usedlist;

inline void SetUsed(int i)
{
	if (i>=0 && i<texman->TextureCount()) usedlist[i]=true;
}

char IsUsed(int i)
{
	if (i>=0 && i<texman->TextureCount()) return usedlist[i];
	return true;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

class UnusedItem
{
public:
	virtual void ProcessTexture(int tno) =0;
};


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
class UAnimated : public UnusedItem
{
public:
	int first, last;

	UAnimated(int f,int l) { first=min(f,l); last=max(f,l); }
	virtual void ProcessTexture(int tno)
	{
		if (tno>=first && tno<=last)
		{
			for(int i=first;i<=last;i++)
			{
				SetUsed(i);
			}
		}
	}
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
class USwitch : public UnusedItem
{
public:
	int off, on;

	USwitch(int o,int f) { on=o; off=f; }

	virtual void ProcessTexture(int tno)
	{
		if (tno==on || tno==off)
		{
			SetUsed(on);
			SetUsed(off);
		}
	}
};


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
class UAnimDef : public UnusedItem
{
public:
	TArray<int> list;

	virtual void ProcessTexture(int tno)
	{
		for (unsigned i=0;i<list.Size();i++)
		{
			SetUsed(list[i]);
		}
	}
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
TArray<UnusedItem *> unusedlist;



#define MAXSWITCHES 400
#define VALID_CHARS "@ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_~{}()-"
//
// P_InitSwitchList
// Only called at game initialization.
//
// this is used to eliminate the alternate switch texture
#pragma pack(1)
typedef struct
{
  char name1[9];
  char name2[9];
  short episode;
} switchlist_t; //jff 3/23/98 pack to read from memory


typedef struct
{
  signed char istexture; //jff 3/23/98 make char for comparison // cph - make signed
  char        endname[9];           //  if false, it is a flat
  char        startname[9];
  int         speed;
} animdef_t; //jff 3/23/98 pack to read from memory

#pragma pack()


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void P_InitSwitchList(void)
{
	int i;
	switchlist_t * alphSwitchList;
	int alloc=MAXSWITCHES*2;
	
	SQWORD lump = (SQWORD)CResourceFile::GlobalFindLump("SWITCHES");
	if (lump<0) return;

	alphSwitchList=(switchlist_t*)CResourceFile::GlobalReadLump(lump);
	for (i=0;;i++)
	{
		if (alphSwitchList[i].episode<=0) break;
		
		int saddr=texman->CheckTextureNumForName(alphSwitchList[i].name1, tx_texture);
		int eaddr=texman->CheckTextureNumForName(alphSwitchList[i].name2, tx_texture);
		if (saddr<0 || eaddr<0)
		{
			continue;
		}
		USwitch * s=new USwitch(saddr,eaddr);
		unusedlist.Push(s);
	}
	free(alphSwitchList);
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void P_InitPicAnims(void)
{
    int		i;
    
    //	Init animation
	animdef_t * animdefs;
	SQWORD lump = (SQWORD)CResourceFile::GlobalFindLump("ANIMATED");
	if (lump<0) return;
	animdefs=(animdef_t*)CResourceFile::GlobalReadLump(lump);

	for (i=0 ; animdefs[i].istexture != -1 ; i++)
	{
		tx_namespace txn= (animdefs[i].istexture==1)? tx_texture:tx_flat;
	
		int saddr=texman->CheckTextureNumForName(animdefs[i].startname,txn);
		int eaddr=texman->CheckTextureNumForName(animdefs[i].endname,txn);
		if (saddr<0 || eaddr<0) continue;	

		UAnimated * s=new UAnimated(saddr,eaddr);
		unusedlist.Push(s);
	}
	free(animdefs);
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
static void ParseAnim (ScriptMan & sc, byte istex)
{
	int picnum;
	int frame;
	UAnimDef * ua=NULL;
	bool userange;

	sc.SC_MustGetString ();
	picnum = texman->CheckTextureNumForName (sc.sc_String, istex? tx_texture:tx_flat);

	while (sc.SC_GetString ())
	{
		if (sc.SC_Compare ("allowdecals")) continue;


		else if (sc.SC_Compare("range"))
		{
			userange=true;
		}
		else if (sc.SC_Compare ("pic"))
		{
			userange=false;
			if (picnum!=-1 && !ua) 
			{
				ua=new UAnimDef;
				unusedlist.Push(ua);
			}
		}
		else
		{
			sc.SC_UnGet ();
			break;
		}

		sc.SC_MustGetString();

		frame=texman->CheckTextureNumForName (sc.sc_String, istex? tx_texture:tx_flat);
		if (frame<0) frame=picnum+strtol(sc.sc_String,0,0)-1;

		if (userange)
		{
			UAnimated * un=new UAnimated(picnum, frame);
			unusedlist.Push(un);
		}
		else if (ua && frame>=0) 
		{
			ua->list.Push(frame);
		}
		sc.SC_MustGetString ();
		if (sc.SC_Compare ("tics"))
		{
			sc.SC_MustGetNumber ();
		}
		else if (sc.SC_Compare ("rand"))
		{
			sc.SC_MustGetNumber ();
			sc.SC_MustGetNumber ();
		}
		else
		{
		}
	}
}


void ParseAnimatedDoor(ScriptMan & sc)
{
	int picnum;
	int frame;
	UAnimDef * ua=NULL;


	sc.SC_MustGetString();
	picnum = texman->CheckTextureNumForName (sc.sc_String, tx_texture);

	while (sc.SC_GetString ())
	{
		if (sc.SC_Compare("opensound") || sc.SC_Compare("closesound"))
		{
			sc.SC_MustGetString();
			continue;
		}
		else if (sc.SC_Compare ("pic"))
		{
			if (picnum!=-1 && !ua) 
			{
				ua=new UAnimDef;
				unusedlist.Push(ua);
			}
		}
		else
		{
			sc.SC_UnGet ();
			break;
		}

		sc.SC_MustGetString();

		frame=texman->CheckTextureNumForName (sc.sc_String, tx_texture);
		if (frame<0) frame=picnum+strtol(sc.sc_String,0,0)-1;

		if (frame>=0 && ua) ua->list.Push(frame);
	}
}

////////////////////////////////////////////////////////////////////////////
void ParseSwitchDef (ScriptMan & sc, UAnimDef * ua)
{
	int numframes;
	int picnum;
	char * sound;

	numframes = 0;
	sound = NULL;

	while (sc.SC_GetString ())
	{
		if (sc.SC_Compare ("sound")) sc.SC_MustGetString ();
		else if (sc.SC_Compare ("pic"))
		{
			sc.SC_MustGetString ();
			picnum = texman->CheckTextureNumForName (sc.sc_String, tx_texture);
			ua->list.Push(picnum);
			sc.SC_MustGetString ();
			if (sc.SC_Compare ("tics")) sc.SC_MustGetNumber ();
			else if (sc.SC_Compare ("rand"))
			{
				sc.SC_MustGetNumber ();
				sc.SC_MustGetNumber ();
			}
		}
		else
		{
			sc.SC_UnGet ();
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
// Parse a switch block in ANIMDEFS and add the definitions to SwitchList
void P_ProcessSwitchDef (ScriptMan & sc)
{
	int picnum;
	UAnimDef * ua=new UAnimDef;

	sc.SC_MustGetString ();


	// Ignore the limiting keywords here. for texture checks they are not
	// critical.
	if (sc.SC_Compare ("doom")) { sc.SC_MustGetNumber (); }
	else if (sc.SC_Compare ("heretic")) {}
	else if (sc.SC_Compare ("hexen")) {}
	else if (sc.SC_Compare ("any")) {}
	else { 	sc.SC_UnGet (); }


	sc.SC_MustGetString ();
	picnum = texman->CheckTextureNumForName (sc.sc_String,tx_texture);
	ua->list.Push(picnum);

	while (sc.SC_GetString ())
	{
		if (sc.SC_Compare ("on"))
		{
			ParseSwitchDef (sc,ua);
		}
		else if (sc.SC_Compare ("off"))
		{
			ParseSwitchDef (sc,ua);
		}
		else
		{
			sc.SC_UnGet ();
			break;
		}
	}
	unusedlist.Push(ua);
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void P_InitPicAnimDefs(void)
{
	TArray<QWORD> list;
	ScriptMan sc;

	CResourceFile::GlobalFindLumps("ANIMDEFS", list);
	
	for(unsigned int i=0;i<list.Size();i++)
	{
		sc.SC_OpenLumpNum (list[i], "ANIMDEFS");

		while (sc.SC_GetString ())
		{
			if (sc.SC_Compare ("animateddoor"))
			{
				ParseAnimatedDoor(sc);
			}
			else if (sc.SC_Compare ("flat"))
			{
				ParseAnim (sc,false);
			}
			else if (sc.SC_Compare ("texture"))
			{
				ParseAnim (sc,true);
			}
			else if (sc.SC_Compare ("switch"))
			{
				P_ProcessSwitchDef (sc);
			}
			else if (sc.SC_Compare ("warp"))
			{
				sc.SC_MustGetString ();
				if (sc.SC_Compare ("flat"))
				{
					sc.SC_MustGetString ();
				}
				else if (sc.SC_Compare ("texture"))
				{
					sc.SC_MustGetString ();
				}
			}
		}
		sc.SC_Close ();
	}
}




////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
static void UnusedTexture(const char * tex, tx_namespace tn)
{
	int t;

	t=texman->CheckTextureNumForName(tex,tn);

	if (t>=0 && !IsUsed(t))
	{
		SetUsed(t);
	}
}

static void ProcessLists()
{
	for(int k=0;k<2;k++)
	{
		// this has to be done twice to catch all textures that are indirectly referenced by
		// both an animation and a switch
		for(int i=0;i<texman->TextureCount();i++)
		{
			if (IsUsed(i))
			{
				for (unsigned j=0;j<unusedlist.Size();j++) unusedlist[j]->ProcessTexture(i);
			}
		}
	}
}


static void UnusedSideDef(CSideDef * sd)
{

	if (sd->texUpper[0]!='-') UnusedTexture(sd->texUpper, tx_texture);
	if (sd->texNormal[0]!='-') UnusedTexture(sd->texNormal, tx_texture);
	if (sd->texLower[0]!='-') UnusedTexture(sd->texLower, tx_texture);
}




// Check ACS script for potential texture names
static void CheckACS(const char * lump, int len)
{
	const char * p;

	if (lump[0]=='A' && lump[1]=='C' && lump[2]=='S')// && lump[3]==0)
	{
		p=lump+ *(long*)(lump+4);

		if (p[-4]=='A' && p[-3]=='C' && p[-2]=='S')
		{
			// enhanced

			// Do this the brute force way. I have no desire to figure the format out fully
			long * q=(long *)lump;
			int c=0;

			while (memcmp(q, "STRL", 4) && c<len/4) q++,c++;
			if (c<len/4)
			{
				if (q[1])
				{
					lump=(const char *)q;
					int count=q[3];
					q+=5;
					lump+=8;
					
					for(long i=0;i<count;i++)
					{
						UnusedTexture(lump+ q[i], tx_texture);
						UnusedTexture(lump+ q[i], tx_flat);
					}
				}
			}
		}
		else
		{
			// old
			long * offs=(long*)p;
			long l=*offs;
			offs+=1+l*3;
			l=*offs;
			p++;

			for(long i=0;i<l;i++)
			{
				UnusedTexture(lump+ offs[i], tx_texture);
				UnusedTexture(lump+ offs[i], tx_flat);
			}

		}
	}

}


// Check FraggleScript for potential texture names.
// This only looks for strings and treats them as texture 
// names. This might get too many but it will mark everything
// that is being used.
static void CheckFS(const char * lump, int len)
{
	char buffer[10];
	int i;

	for(i=0;i<len;i++)
	{
		if (lump[i]=='"')
		{
			int start=++i;
			while (lump[i]!='"' && i<len) i++;
			if (i-start<10)
			{
				memcpy(buffer,&lump[start],i-start);
				buffer[i-start]=0;
				UnusedTexture(buffer,tx_texture);
				UnusedTexture(buffer,tx_flat);
			}
		}
	}
}


// Searc ACS libraries as well!
void DoGlobals()
{
	/*
	unsigned i;

	int start=Wads.CheckNumForName("A_START");
	int end=Wads.CheckNumForName("A_END");

	if (start>0 && start<end)
	{
		for(i=start+1;i<end;i++)
		{
			if (Wads.GetLumpFile(i)<GetIWadCount()) continue;
			int l=Wads.LumpLength(i);
			FMemLump lu=Wads.ReadLump(i);
			const char * c=(const char *)lu.GetMem();
			CheckACS(c,l);
		}
	}
	*/

	// CheckFS(FSGLOBAL)
}


void CLevel::OnUnusedTextures(wxCommandEvent & event)
{
	wxString composed;
	TArray<const char *> maplist;

	{
		wxProgressDialog prog("Listing unused textures", "Initializing", 100, m_DrawWindow, wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_CAN_ABORT);
		wxBusyCursor wait;
		wxWindowDisabler dis;

		int w;

		try
		{
			ValidateTextureManager();
			P_InitSwitchList();
			P_InitPicAnims();
			P_InitPicAnimDefs();
		}
		catch(CRecoverableError * err)
		{
			wxMessageBox(err->GetMessage(), ZED_CAPTION);
			return;
		}

		usedlist= new char[texman->TextureCount()];
		memset(usedlist,0,sizeof(char)*texman->TextureCount());

		CResourceFile * rf = CResourceFile::GetResourceFile(m_currentWAD);
		if (!rf) return;

		rf->GetMapList(maplist);

		for(unsigned l=0;l<maplist.Size();l++)
		{
			CLevel * level = new CLevel(false);
			int lumpno=rf->FindLump(maplist[l]);
			try
			{
				level->Load(lumpno+(QWORD(m_currentWAD+1)<<32), maplist[l]);
			}
			catch(CRecoverableError * err)
			{
				composed += wxString::Format("%s: %s\n", maplist[l], err->GetMessage());
				continue;
			}
			if (!prog.Update(100*l/maplist.Size(), wxString::Format("processing %s",maplist[l]))) return;

			for(w=0;w<level->NumLines();w++)
			{
				CLine * ln = level->GetLine(w);
				for (int k=0;k<2;k++)
				{
					if (ln->sides[k].sector!=-1)
					{
						UnusedSideDef(&ln->sides[k]);
					}
				}
			}
			for(w=0;w<level->NumSectors();w++)
			{
				CSector * sec = level->GetSector(w);

				UnusedTexture(sec->ceilt, tx_flat);
				UnusedTexture(sec->floort, tx_flat);
			}
			if (level->m_Behavior.Size()) CheckACS(&level->m_Behavior[0],level->m_Behavior.Size());
			if (level->m_FraggleScript.Size()) CheckACS(&level->m_FraggleScript[0],level->m_FraggleScript.Size());
			delete level;
		}
		if (!prog.Update(99, "reporting")) return;
		DoGlobals();
		ProcessLists();

		composed += "Unused textures\n---------------\n";
		for(w=0;w<texman->TextureCount();w++)
		{

			if (usedlist[w]==0 && texman->GetTextureType(w)>tx_loprio && texman->GetOwner(w)==m_currentWAD+1)	
				// belongs to the current WAD but isn't used
			{
				composed += wxString::Format("%d: %.8s, Type=%s\r\n",w, texman->GetTextureName(w), 
											TextureTypeNames[texman->GetTextureType(w)]);
			}
		}
		delete [] usedlist;
		for(w=0;w<(int)unusedlist.Size();w++)
		{
			delete unusedlist[w];
			unusedlist.Clear();
		}
	}
	DisplayText (m_DrawWindow, "Unused Textures", composed);
}


void CLevel::OnUsedTextures(wxCommandEvent & event)
{
	wxString composed;
	TArray<const char *> maplist;

	{
		wxProgressDialog prog("Listing used textures", "Initializing", 100, m_DrawWindow, wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_CAN_ABORT);
		wxBusyCursor wait;
		wxWindowDisabler dis;

		int w;

		try
		{
			ValidateTextureManager();
			P_InitSwitchList();
			P_InitPicAnims();
			P_InitPicAnimDefs();
		}
		catch(CRecoverableError * err)
		{
			wxMessageBox(err->GetMessage(), ZED_CAPTION);
			return;
		}

		usedlist= new char[texman->TextureCount()];
		memset(usedlist,0,sizeof(char)*texman->TextureCount());

		CResourceFile * rf = CResourceFile::GetResourceFile(m_currentWAD);
		if (!rf) return;

		rf->GetMapList(maplist);

		for(unsigned l=0;l<maplist.Size();l++)
		{
			CLevel * level = new CLevel(false);
			int lumpno=rf->FindLump(maplist[l]);
			try
			{
				level->Load(lumpno+(QWORD(m_currentWAD+1)<<32), maplist[l]);
			}
			catch(CRecoverableError * err)
			{
				composed += wxString::Format("%s: %s\n", maplist[l], err->GetMessage());
				continue;
			}
			if (!prog.Update(100*l/maplist.Size(), wxString::Format("processing %s",maplist[l]))) return;

			for(w=0;w<level->NumLines();w++)
			{
				CLine * ln = level->GetLine(w);
				for (int k=0;k<2;k++)
				{
					if (ln->sides[k].sector!=-1)
					{
						UnusedSideDef(&ln->sides[k]);
					}
				}
			}
			for(w=0;w<level->NumSectors();w++)
			{
				CSector * sec = level->GetSector(w);

				UnusedTexture(sec->ceilt, tx_flat);
				UnusedTexture(sec->floort, tx_flat);
			}
			if (level->m_Behavior.Size()) CheckACS(&level->m_Behavior[0],level->m_Behavior.Size());
			if (level->m_FraggleScript.Size()) CheckACS(&level->m_FraggleScript[0],level->m_FraggleScript.Size());
			delete level;
		}
		if (!prog.Update(99, "reporting")) return;
		DoGlobals();
		ProcessLists();

		composed += "Used textures\n---------------\n";
		for(w=0;w<texman->TextureCount();w++)
		{

			if (usedlist[w]!=0)	
				// belongs to the current WAD but isn't used
			{
				composed += wxString::Format("%d: %.8s, Type=%s\r\n",w, texman->GetTextureName(w), 
											TextureTypeNames[texman->GetTextureType(w)]);
			}
		}
		delete [] usedlist;
		for(w=0;w<(int)unusedlist.Size();w++)
		{
			delete unusedlist[w];
			unusedlist.Clear();
		}
	}
	DisplayText (m_DrawWindow, "Used Textures", composed);
}


