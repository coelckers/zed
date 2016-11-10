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
// Statistics code for the current level
//

#include "StdAfx.h"
#include "texture.h"
#include "GameConfig.h"
#include "ResourceFile.h"
#include "ZEd.h"
#include "View2D.h"
#include "Level.h"


//==========================================================================
//
//
//
//==========================================================================

struct texinf
{
	char texnames[9];
	short wallcount;
	short flatcount;
};

static TArray<texinf> texdata;

static const char * TTNames_Formatted[]={
	"",
	"Sprite ",
	"Patch  ",
	"",
	"Flat   ",
	"Texture",
	"",
	"TX_    "
};

const char * TextureType(int tex)
{
	if (tex<0) return "invalid";
	else return TTNames_Formatted[texman->GetTextureType(tex)];
}

//==========================================================================
//
//
//
//==========================================================================

static wxString LT_Save()
{
	wxString composed;
	int x,y;
	unsigned int i;
	for (i=0;i<texdata.Size();i++)
	{
		if (texdata[i].texnames[0]!='-')
		{
			int wty = texman->CheckTextureNumForName(texdata[i].texnames,tx_texture);
			int fty = texman->CheckTextureNumForName(texdata[i].texnames,tx_flat);

			if (wty==fty && texdata[i].wallcount && texdata[i].flatcount)
			{
				x = texman->TextureWidth(wty);
				y = texman->TextureHeight(wty);
				composed += wxString::Format("%8s: Type=%s Size= %d x %d, %dx as wall, %dx as flat\n",
					texdata[i].texnames,TextureType(wty),x,y,texdata[i].wallcount,texdata[i].flatcount);
			}
			else 
			{
				if (texdata[i].wallcount)
				{
					x = texman->TextureWidth(wty);
					y = texman->TextureHeight(wty);
					composed += wxString::Format("%8s: Type=%s Size= %d x %d, %dx as wall\n",
						texdata[i].texnames,TextureType(wty),x,y,texdata[i].wallcount,texdata[i].flatcount);
				}
				if (texdata[i].flatcount)
				{
					x = texman->TextureWidth(fty);
					y = texman->TextureHeight(fty);
					composed += wxString::Format("%8s: Type=%s Size= %d x %d, %dx as flat\n",
						texdata[i].texnames,TextureType(fty),x,y,texdata[i].flatcount);
				}
			}
		}
	}
	return composed;
}


//==========================================================================
//
//
//
//==========================================================================

static void LT_Add(bool flat,char * t)
{
unsigned i;
texinf td;

	for(i=0;i<texdata.Size();i++)
    {
    	if (!strnicmp(t,texdata[i].texnames,8)) 
		{
			if (!flat) texdata[i].wallcount++;
			else texdata[i].flatcount++;
			return;
		}
    }
    strncpy(td.texnames,t,8);
	td.texnames[8]=0;
	td.wallcount=!flat;
	td.flatcount=flat;
	texdata.Push(td);
}


//==========================================================================
//
//
//
//==========================================================================

static void LT_ProcessSideDef(CSideDef * si)
{
	if (si->texLower[0]!='-') LT_Add(false,si->texLower);
	if (si->texUpper[0]!='-') LT_Add(false,si->texUpper);
	if (si->texNormal[0]!='-') LT_Add(false,si->texNormal);
}


//==========================================================================
//
//
//
//==========================================================================

static void LT_ProcessSector(CSector * s)
{
	LT_Add(true,s->ceilt);
    LT_Add(true,s->floort);
}


//==========================================================================
//
//
//
//==========================================================================

static int __cdecl LT_Cmp(const void * a,const void * b)
{
	return _stricmp(((texinf*)a)->texnames,((texinf*)b)->texnames);
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnEditListallusedtextures(wxCommandEvent & event)
{
	int i;

	ValidateTextureManager();
	texdata.Clear();
	for (i=0;i<NumLines();i++)
	{
		CLine * ln = GetLine(i);
		if (ln->sides[0].sector!=-1) LT_ProcessSideDef(&ln->sides[0]);
		if (ln->sides[1].sector!=-1) LT_ProcessSideDef(&ln->sides[1]);
	}
	for(i=0;i<NumSectors();i++)
	{
		LT_ProcessSector(GetSector(i));
	}
	qsort(&texdata[0],texdata.Size(),sizeof(texdata[0]),LT_Cmp);
	DisplayText(m_DrawWindow, "List all used textures", LT_Save());
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnEditListLDT(wxCommandEvent & event)
{
	wxString composed;
	unsigned short * ldtcount=new unsigned short[65536];
	int i;
	int j,k;

	memset(ldtcount,0,65536*sizeof(short));

	composed="\n\nList of all used linedef types\n";
	if (!m_Extended)
	{
		for (i=0;i<NumLines();i++)
		{
			ldtcount[GetLine(i)->line.type]++;
		}
		
		for(i=0;i<65536;i++)
		{
			if (ldtcount[i])
			{
				composed += wxString::Format("%3d x %s\n",ldtcount[i],cgc->GetLDName(i)); 
			}
		}
	}
	else
	{
		for (i=0;i<NumLines();i++)
		{
			CLine * ln = GetLine(i);
			int trigger= m_TextMap? 0 : ((ln->line.Flags.GetShort()&0x1e00)>>9);
			trigger&=0xffff;
			ldtcount[ln->line.type*16+trigger]++;
		}
		
		for(i=0;i<256*16;i+=16)
		{
			for(k=j=0;j<16;j++)
				if (ldtcount[i+j])
				{
					const char * s = cgc->GetLineDefDescription(int(i/16),j,NULL,k++!=0);
					composed += wxString::Format("%3d x %s\n",ldtcount[i+j],s); 
				}
		}
	}

	memset(ldtcount,0,65536*sizeof(short));
	for(i=0;i<NumSectors();i++)
	{
		int spc = GetSector(i)->special;
		if (spc!=0) ldtcount[spc]++;
	}
	composed+="\n\nList of all used sector types\n";
	for(i=0;i<65536;i++)
	{
		if (ldtcount[i])
		{
			composed += wxString::Format("%3d x %s\n",ldtcount[i],cgc->GetSectorName(i)); 
		}
	}
	
	delete ldtcount;
	DisplayText(m_DrawWindow, "List all used linedef and sector types", composed);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnEditListallusedsectortags(wxCommandEvent & event)
{
	int i,j,k;
	wxString composed;

	for(j=1;j<32767;j++)
	{
		k=0;
		for (i=0;i<NumSectors();i++)
		{
			if (GetSector(i)->tag==j)
			{
				if (k==0)
				{
					composed += wxString::Format("Tag %d:\n", j);
					k=1;
				}
				composed += wxString::Format("\tSector %d\n", i);
			}
		}

		if (!m_Extended)
		{
			for (i=0;i<NumLines();i++)
			{
				if (GetLine(i)->line.tag==j)
				{
					if (k==0)
					{
						composed += wxString::Format("Tag %d:\n", j);
						k=1;
					}
					composed += wxString::Format("\tLinedef %d\n", i);
				}
			}
		}
	}
	DisplayText(m_DrawWindow, "List all used sector tags", composed);
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnEditListthings(wxCommandEvent & event)
{
	int w;
	int total[12]={0};
	wxString composed;
	short * statP=new short[65536];

	memset(statP,0,65536*sizeof(short));

	for(w=0;w<NumThings();w++)
	{
		statP[GetThing(w)->type]++;
	}
	for (w=0;w<65536;w++)
	{
		if (statP[w])
		{
			composed += wxString::Format("%s: %dx\n",cgc->GetThingName(w,true),statP[w]);
		}
	}
	delete statP;
	DisplayText(m_DrawWindow, "List all used things", composed);
}



//==========================================================================
//
//
//
//==========================================================================

void CLevel::DoExtendedMapStatistics(unsigned short * statP,int mode)
{
	int w;
	static int oneflags[]=  {    1,    2,    4,    1,    2,    4,    1,    2,    4};
	static int zeroflags[]= { 6288, 6288, 6288, 6304, 6304, 6304, 6336, 6336, 6336};

	static int oneflagsx[]= {  257,  258,  260,  513,  514,  516, 1025, 1026, 1028};

	BitField oneflagsx_tm[9];

	static const char *keys[9][2]={
		{ "single", "skill2" },
		{ "coop", "skill2" },
		{ "dm", "skill2" },
		{ "single", "skill3" },
		{ "coop", "skill3" },
		{ "dm", "skill3" },
		{ "single", "skill4" },
		{ "coop", "skill4" },
		{ "dm", "skill4" }
	};

	int res=0;

	if (m_TextMap)
	{
		for(int i=0;i<9;i++) 
		{
			oneflagsx_tm[i].Clear();
			for(int j=0;j<2;j++)
			{
				int bit = cgc->CheckTextMapThingFlag(keys[i][j]);
				if (bit != -1) oneflagsx_tm[i].SetBit(bit);
			}
		}
	}

	memset(statP,0,65536*sizeof(short));
	for(w=0;w<NumThings();w++)
	{
		CThing * t = GetThing(w);
		if (m_TextMap)
		{
			if ((t->Flags & oneflagsx_tm[mode]) != oneflagsx_tm[mode]) continue;
		}
		else if (!m_Extended)
		{
			if (((t->Flags.GetShort())&oneflags[mode])!=oneflags[mode]) continue;
			if (((t->Flags.GetShort())&zeroflags[mode])!=0) continue;
		}
		else
		{
			if (((t->Flags.GetShort())&oneflagsx[mode])!=oneflagsx[mode]) continue;
		}
		res=t->type;
		statP[res]++;
	}
}


//==========================================================================
//
//
//
//==========================================================================

static char * descript[]={ "Monsters","Friendlies", "Items","Weapons","Ammo","Keys","non-countable items","Special Items","Characters",""};
static int statflags[]={STAT_MONSTER,STAT_FRIENDLY,STAT_ITEM,STAT_WEAPON,STAT_AMMO,STAT_KEY,STAT_NCITEM,STAT_SPECIAL,-1};
TArray<int> ThingOrder;

void CreateThingOrder()
{
	unsigned i,j;
	int v=65536,f;

	ThingOrder.Clear();
	ThingOrder.Push(v);
	for(f=0;statflags[f]>0;f++)
	{
		for(i=0;i<(unsigned)cgc->ThingGroups.Size();i++)
		{
			for(j=0;j<(unsigned)cgc->ThingGroups[i]->Size();j++)
			{
				ThingDesc * td=(*cgc->ThingGroups[i])[j];

				if (td->statclass==statflags[f])
				{
					if (cgc->ThingMap[td->DoomEdNum]==td) ThingOrder.Push(td->DoomEdNum);
				}
			}
		}
		for(i=0;i<cgc->CustomThings.Size();i++)
		{
			ThingDesc * td=cgc->CustomThings[i];
			if (td->statclass==statflags[f]) ThingOrder.Push(td->DoomEdNum);
		}

		v++;
		ThingOrder.Push(v);
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnEditItemstatistics(wxCommandEvent & event)
{
	int i;
	unsigned short * statArr[12];
	int total[12]={0};
	int totalc;
	wxString composed;

	CreateThingOrder();

	for(i=0;i<9;i++)
	{
		statArr[i]=new unsigned short[65536];

		DoExtendedMapStatistics(statArr[i],i);
	}

	composed += wxString::Format("%-40s  Single Player     Deathmatch     Cooperative\n","");
	composed += wxString::Format("%-40s  Easy Medi Hard  Easy Medi Hard  Easy Medi Hard\n","Thing");

	for(i=0;i<(int)ThingOrder.Size();i++)
	{
		if (ThingOrder[i]==65536)
		{
			composed += wxString::Format("\n%s\n----------------------------------------------------------------------------------------\n",
							descript[ThingOrder[i]-65536]
					);
		}
		else if (ThingOrder[i]>65536)
		{
			composed += wxString::Format("\n%-40s  %4d %4d %4d  %4d %4d %4d  %4d %4d %4d \n"
						 "\n%s\n----------------------------------------------------------------------------------------\n",
							"Total",
							total[0],
							total[1],
							total[2],
							total[3],
							total[4],
							total[5],
							total[6],
							total[7],
							total[8],
							descript[ThingOrder[i]-65536]
					);
			memset(total,0,sizeof(total));
		}
		else if (ThingOrder[i]>0)
		{
			totalc=0;
			for(int j=0;j<9;j++) 
			{
				total[j]+=statArr[j][ThingOrder[i]];
				totalc+=statArr[j][ThingOrder[i]];
			}
			if (totalc) composed += wxString::Format("%-40s  %4d %4d %4d  %4d %4d %4d  %4d %4d %4d \n",
				cgc->GetThingName(ThingOrder[i],false),
				statArr[0][ThingOrder[i]],
				statArr[1][ThingOrder[i]],
				statArr[2][ThingOrder[i]],
				statArr[3][ThingOrder[i]],
				statArr[4][ThingOrder[i]],
				statArr[5][ThingOrder[i]],
				statArr[6][ThingOrder[i]],
				statArr[7][ThingOrder[i]],
				statArr[8][ThingOrder[i]]
				);
		}
	}
	for(i=0;i<9;i++)
	{
		delete statArr[i];
	}
	DisplayText(m_DrawWindow, "Item statistics", composed);
}

