//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2000-2005 Christoph Oelckers
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
// WAD statistics
//
#include "StdAfx.h"
#include "ZEd.h"
#include "texture.h"
#include "GameConfig.h"
#include "ResourceFile.h"
#include "Level.h"


#define MAXTHING 65536
#define THINGFLAGS

void CLevel::OnWadListallusedthingsincurrentpwad(wxCommandEvent & event)
{
	TArray<const char *> maplist;
	wxString composed;

	if (cgc==NULL)
	{
		wxMessageBox("No Wads open", ZED_CAPTION);
		return;
	}
	{
		wxBusyCursor wait;
		wxWindowDisabler dis;

		CResourceFile * rf = CResourceFile::GetResourceFile(m_currentWAD);
		if (!rf) return;

		rf->GetMapList(maplist);

		short * list_thing;
		short * list_thing_sp;
		char * used_thing;
		int i,l,w;

		i=maplist.Size();

		list_thing=new short[i*MAXTHING];
		list_thing_sp=new short[i*MAXTHING];
		used_thing=new char[MAXTHING];

		memset(list_thing,0,sizeof(short)*i*MAXTHING);
		memset(list_thing_sp,0,sizeof(short)*i*MAXTHING);
		memset(used_thing,0,sizeof(char)*MAXTHING);

		int testbit = 0;
		/*
		if (m_TextMap)
		{
			int bit = cgc->CheckTextMapThingFlag("single");
			if (bit != -1) testbit |= (1<<bit);
		}
		*/

		for(l=0;l<i;l++)
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
			for(int w=0;w<level->NumThings();w++)
			{
				int t=level->GetThing(w)->type;

				if (t>=0 && t<MAXTHING)
				{
					list_thing[l*MAXTHING+t]++;
					used_thing[t]=1;

					if (!m_TextMap)
					{
						if (!cgc->IsExtended)
						{
							if (!(level->GetThing(w)->Flags.GetShort()&16)) list_thing_sp[l*MAXTHING+t]++;
						}
						else
						{
							if (level->GetThing(w)->Flags.GetShort()&256) list_thing_sp[l*MAXTHING+t]++;
						}
					}
				}
			}
			delete level;
		}

		// Get the extended config because it contains all the things.
		GameConfig * gc = confman.GetRealConfig(true, cgc);
		for(w=0;w<MAXTHING;w++)
		{
			if (used_thing[w]==0) continue;
			composed += wxString::Format("%s\n",gc->GetThingName(w,true,false)); 

			for (l=0;l<i;l++)
			{
				if (list_thing[l*MAXTHING+w])
				{
					composed += wxString::Format("    %s: %dx (%dx)\n",(const char *)maplist[l],list_thing[l*MAXTHING+w],list_thing_sp[l*MAXTHING+w]);
				}
			}
		}
		delete [] list_thing;
		delete [] list_thing_sp;
		delete [] used_thing;
	}
	DisplayText (m_DrawWindow, "List of all used things in PWAD", composed);
}


void CLevel::OnWadListallusedlinedefandsectortypesinpwad(wxCommandEvent & event)
{
	TArray<const char *> maplist;
	wxString composed;

	if (cgc==NULL)
	{
		wxMessageBox("No Wads open", ZED_CAPTION);
		return;
	}
	{
		wxBusyCursor wait;
		wxWindowDisabler dis;

		CResourceFile * rf = CResourceFile::GetResourceFile(m_currentWAD);
		if (!rf) return;

		rf->GetMapList(maplist);

		int j;
		short * list_line_doom;
		short * list_line_hexen;
		char * used_line_doom;
		char * used_line_hexen;
		short * list_sector;
		char * used_sector;
		int i,l;
		int w;


		i=maplist.Size();

		list_line_doom=new short[i*65536];
		used_line_doom=new char[65536];

		list_line_hexen=new short[2*i*256*16];
		used_line_hexen=new char[2*4096];

		list_sector=new short[i*65536];
		used_sector=new char[65536];

		memset(list_line_doom,0,sizeof(short)*i*65536);
		memset(list_line_hexen,0,sizeof(short)*i*2*256*16);
		memset(used_line_doom,0,sizeof(char)*32768);
		memset(used_line_hexen,0,sizeof(char)*2*4096);
		memset(list_sector,0,sizeof(short)*i*65536);
		memset(used_sector,0,sizeof(char)*65536);

		for(l=0;l<i;l++)
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

			// Both map formats can be mixed in the same WAD so we have to check for both in each level!
			if (!level->m_Extended)
			{
				for(w=0;w<level->NumLines();w++)
				{
					CLine * ln = level->GetLine(w);
					int type=ln->line.type;
					if (type<=32768 && type>0) 
					{
						list_line_doom[l*32768+type]++;
						used_line_doom[type]=1;
					}
				}
			}
			else
			{
				for (w=0;w<level->NumLines();w++)
				{
					CLine * ln = level->GetLine(w);

					int type;

					type=ln->line.type;
					if (type>255 || type<=0) continue;

					int trigger=m_TextMap? 0: ((ln->line.Flags.GetShort()&0x1e00)>>9);
					list_line_hexen[l*4096+type*16+trigger]++;
					used_line_hexen[type*16+trigger]=1;

				}
			}
			for(w=0;w<level->NumSectors();w++)
			{
				CSector * sec = level->GetSector(w);
				if (sec->special==0) continue;

				int special=sec->special;
				if (level->m_Extended)
				{
					special|=0x8000;
				}
				list_sector[l*65536+special]++;
				used_sector[special]=1;
			}
		}

		GameConfig * gc = confman.GetRealConfig(false, cgc);

		int first=0;
		for(w=1;w<32768;w++)
		{
			if (used_line_doom[w]==0) continue;
			if (!first)
			{
				composed += "Linedef types in Doom format maps\n";
				composed += "---------------------------------\n";
				first=1;
			}
			composed += gc->GetLDName(w);
			composed += '\n';

			for (l=0;l<i;l++)
			{
				if (list_line_doom[l*32768+w])
				{
					composed += wxString::Format("    %s: %dx\n",(const char *)maplist[l],list_line_doom[l*32768+w]);
				}
			}
		}

		gc = confman.GetRealConfig(true, cgc);
		first=0;

		for(w=16;w<256*16;w+=16)
		{
			int k;
			for(j=k=0;k<16;k++)
			{
				if (used_line_hexen[w+k]==0) continue;
				if (!first)
				{
					composed += "\nLinedef types in Hexen format maps\n";
					composed += "----------------------------------\n";
					first=1;
				}
				composed += wxString::Format("Ext:%s\n",gc->GetLineDefDescription(w/16,k,NULL,j++!=0)); 
				for (l=0;l<i;l++)
				{
					if (list_line_hexen[w+k+l*4096])
					{
						composed += wxString::Format("    %s: %dx\n",(const char *)maplist[l],list_line_hexen[w+k+l*4096]);
					}
				}
			}
		}

		gc = confman.GetRealConfig(false, cgc);
		first=0;


		for(w=1;w<65536;w++)
		{
			if (w==32768)
			{
				gc = confman.GetRealConfig(true, cgc);
				first=0;
			}
			if (used_sector[w]==0) continue;

			if (!first)
			{
				if (w<32768) 
					composed += "\n\nSector types in Doom format maps\n--------------------------------\n";
				else
					composed += "\n\nSector types in Hexen format maps\n---------------------------------\n";
				first=1;
			}

			composed += wxString::Format("%s\n",gc->GetSectorName(w&0x7fff)); 
			for (l=0;l<i;l++)
			{
				if (list_sector[l*65536+w])
				{
					composed += wxString::Format("    %s: %dx\n",(const char *)maplist[l],list_sector[l*65536+w]);
				}
			}
		}

		delete list_line_doom;
		delete used_line_doom;
		delete list_line_hexen;
		delete used_line_hexen;
		delete list_sector;
		delete used_sector;
	}
	DisplayText (m_DrawWindow, "List of all used linedef and sector in PWAD", composed);
}

