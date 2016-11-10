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
// Saves a level
//

#include "StdAfx.h"
#include "ZEd.h"
#include "View2D.h"
#include "Level.h"

//----------------------------------------------------------------------------
//
// WAD Directory entry saving functions
//
//-----------------------------------------------------------------------------

static void WriteEntry(FILE * f,int start,int end,char * name)
{
	end-=start;
	fwrite(&start,1,4,f);
	fwrite(&end,1,4,f);
	fwrite(name,1,8,f);
}


#define NO_DEFAULT 0x80000000
#define NO_DEFAULT_F (1/65536.0)

//----------------------------------------------------------------------------
//
// Property writers
//
//-----------------------------------------------------------------------------

static void AddBoolProp(wxString &str, const char *key, bool val, bool def = false)
{
	if (val != def)
		str += wxString::Format("%s = %s;\n", key, val? "true":"false");
}

static void AddIntProp(wxString &str, const char *key, int val, int def = NO_DEFAULT)
{
	if (def == NO_DEFAULT || val != def)
		str += wxString::Format("%s = %d;\n", key, val);
}

static void AddFloatProp(wxString &str, const char *key, double val, double def = NO_DEFAULT_F)
{
	if (def == NO_DEFAULT_F || val != def) 
		str += wxString::Format("%s = %1.3f;\n", key, val);
}

static void AddStringProp(wxString &str, const char *key, int val, const char *def = NULL)
{
	const char *sval = GetStringFromIndex(val);
	if (sval != NULL && (def == NULL || !strcmp(sval, def)))
		str += wxString::Format("%s = \"%s\";\n", key, sval);
}

//----------------------------------------------------------------------------
//
// Save vertices
//
//-----------------------------------------------------------------------------

wxString CLevel::SaveVerticesAsText()
{
	wxString compose;

	for(int i=0;i<NumVertices();i++)
	{
		compose += wxString::Format("vertex { x = %1.3f; y = %1.3f; }\n", 
			GetVertex(i)->X() + 0.0001, GetVertex(i)->Y() + 0.0001);
	}
	return compose;
}

//----------------------------------------------------------------------------
//
// Save things
//
//-----------------------------------------------------------------------------

wxString CLevel::SaveThingsAsText()
{
	wxString compose;
	int maxflag = m_TextMap? MAX_FLAGS:16;

	for(int i=0;i<NumThings();i++)
	{
		compose += "thing {\n";

		CThing *th = GetThing(i);

		//
		// Common properties for all formats
		//
		AddIntProp(compose, "id", th->thingid);
		AddFloatProp(compose, "x", th->X());
		AddFloatProp(compose, "y", th->Y());
		AddFloatProp(compose, "height", th->Z(), 0);
		AddIntProp(compose, "angle", th->angle);
		AddIntProp(compose, "type", th->type);

		BitField Flags = th->Flags;
		for(int i=0;i<maxflag;i++)
		{
			wxString str = cgc->GetThingFlagText(i, 2, m_TextMap);
			if (str[0]!='!' && str[0]!=0 && Flags.GetBit(i))
			{
				compose << str << " = true;\n";
				Flags.ClearBit(i);
			}
		}

		//
		// Specials don't exist in Doom format maps
		//
		if (m_Extended || m_TextMap)
		{
			AddIntProp(compose, "special", th->special, 0);
			AddIntProp(compose, "arg0", th->args[0], 0);
			AddIntProp(compose, "arg1", th->args[1], 0);
			AddIntProp(compose, "arg2", th->args[2], 0);
			AddIntProp(compose, "arg3", th->args[3], 0);
			AddIntProp(compose, "arg4", th->args[4], 0);
		}

		//
		// Convert flags with different semantics in Doom format maps
		//
		if (!m_TextMap)
		{
			int flags = Flags.GetShort();
			if (flags & 1) compose += "skill1 = true; skill2 = true;\n";
			if (flags & 2) compose += "skill3 = true;\n";
			if (flags & 4) compose += "skill4 = true; skill5 = true;\n";
			if (!m_Extended)
			{
				bool single = true, coop = true, dm = true;

				if (flags & 16) single = false;
				if (flags & 32) coop = false;
				if (flags & 64) dm = false;
				if (single) compose += "single = true;\n";
				if (coop) compose += "coop = true;\n";
				if (dm) compose += "dm = true;\n";
			}
		}

		//
		// new textmap only properties
		//

		if (m_TextMap)
		{
			AddStringProp(compose, "comment", th->comment, "");
		}

		compose += "}\n";
	}
	return compose;
}

//----------------------------------------------------------------------------
//
// Save linedefs
//
//-----------------------------------------------------------------------------

wxString CLevel::SaveLinesAsText()
{
	wxString compose;
	int maxflag = m_TextMap? MAX_FLAGS:16;

	for(int i=0;i<NumLines();i++)
	{
		compose += "linedef {\n";

		//
		// Common properties for all formats
		//
		CLineDef *li = &GetLine(i)->line;

		AddIntProp(compose, "id", li->tag, -1);
		AddIntProp(compose, "v1", li->Start);
		AddIntProp(compose, "v2", li->End);
		AddIntProp(compose, "sidefront", li->Sidedef1);
		AddIntProp(compose, "sideback", li->Sidedef2, NO_SIDE);

		for(int i=0;i<maxflag;i++)
		{
			wxString str = cgc->GetLineFlagText(i, 2, m_TextMap);
			if (str[0]!='!' && str[0]!=0 && li->Flags.GetBit(i))
			{
				compose << str << " = true;\n";
			}
		}

		//
		// line specials
		// We need some additional conversions for line ID setting specials
		// for Hexen format maps and conversion of ZDoom's extended flags arg.
		//

		// Don't write Line_SetIdentification specials
		if (li->type!=121 || !m_Extended) AddIntProp(compose, "special", li->type, 0);
		if (m_Extended)
		{
			for(int i=0;i<5;i++)
			{
				if (!m_TextMap)
				{
					if (cgc->LineMap[li->type])
					{
						wxString str = cgc->LineMap[li->type]->args[i];
						if (!strnicmp(str.c_str(),"SetLineID:",10))
						{
							AddIntProp(compose, "id", li->args[i], 0);
							if (li->type == 121) continue;
							// NOTE: Some specials still need this as arg so write this for all others as both.
						}
						if (!strnicmp(str.c_str(),"ExtLineFlags:",14))
						{
							// These ZDoom flags are fixed so there's no need to look this up in the config.
							if (li->args[i] & 1) compose += "zoneboundary = true;\n";
							if (li->args[i] & 2) compose += "jumpover = true;\n";
							if (li->args[i] & 4) compose += "blockfloating = true;\n";
							if (li->args[i] & 8) compose += "clipmidtex = true;\n";
							if (li->args[i] & 16) compose += "wrapmidtex = true;\n";
							if (li->args[i] & 32) compose += "midtex3d = true;\n";
							if (li->args[i] & 64) compose += "checkswitchrange = true;\n";
							continue;	// don't write as arg.
						}
					}
				}

				char argx[] = "argx";
				argx[3] = i+'0';
				AddIntProp(compose, argx, li->args[i], 0);
			}
		}
		else if (li->type != 0)
		{
			// When converting Doom format maps the tag must be doubled as special arg
			AddIntProp(compose, "arg0", li->args[0], 0);
		}

		//
		// For Hexen format maps we need to convert trigger types to bit masks.
		//

		if (!m_TextMap)
		{
			if (m_Extended)
			{
				if (li->Flags.GetShort() & 512) compose += "repeatspecial = true;\n";
				switch ((li->Flags.GetShort() & 0x1c00)>>10)
				{
				case 0: compose += "playercross = true;\n"; break;
				case 1: compose += "playeruse = true;\n"; break;
				case 2: compose += "monstercross = true;\n"; break;
				case 3: compose += "impact = true;\n"; break;
				case 4: compose += "playerpush = true;\n"; break;
				case 5: compose += "missilecross = true;\n"; break;
				case 6: compose += "playeruse = true;\npassuse = true;\n"; break;
				case 7: compose += "impact = true;\nmissilecross = true;\n"; break;
				}
			}
		}


		//
		// new textmap only properties
		//

		if (m_TextMap)
		{
			AddStringProp(compose, "comment", li->comment, "");
			AddFloatProp(compose, "alpha", li->alpha, 1.);

			int bit = cgc->CheckTextMapLineFlag("!renderstyle");
			if (bit != -1 && li->Flags.GetBit(bit)) compose += "renderstyle = \"add\";\n";
		}

		compose += "}\n";
	}
	return compose;
}

//----------------------------------------------------------------------------
//
// Save sidedefs
//
//-----------------------------------------------------------------------------

wxString CLevel::SaveSidesAsText(TArray<CSideDef> &compressed_sds)
{
	wxString compose;

	for(unsigned i=0;i<compressed_sds.Size();i++)
	{
		compose += "sidedef {\n";

		CSideDef *sd = &compressed_sds[i];

		//
		// Common properties of Sidedefs are the same for all formats
		//

		AddIntProp(compose, "sector", sd->sector);
		AddIntProp(compose, "offsetx", sd->xoff, 0);
		AddIntProp(compose, "offsety", sd->yoff, 0);
		if (strncmp(sd->texUpper, "-", 8)) compose += wxString::Format("texturetop = \"%.8s\";\n", sd->texUpper);
		if (strncmp(sd->texLower, "-", 8)) compose += wxString::Format("texturebottom = \"%.8s\";\n", sd->texLower);
		if (strncmp(sd->texNormal, "-", 8)) compose += wxString::Format("texturemiddle = \"%.8s\";\n", sd->texNormal);

		//
		// new textmap only properties
		//

		if (m_TextMap)
		{
			AddStringProp(compose, "comment", sd->comment, "");
			AddFloatProp(compose, "offsetx_top", sd->texOffset[0][X], 0);
			AddFloatProp(compose, "offsety_top", sd->texOffset[0][Y], 0);
			AddFloatProp(compose, "offsetx_mid", sd->texOffset[1][X], 0);
			AddFloatProp(compose, "offsety_mid", sd->texOffset[1][Y], 0);
			AddFloatProp(compose, "offsetx_bottom", sd->texOffset[2][X], 0);
			AddFloatProp(compose, "offsety_bottom", sd->texOffset[2][Y], 0);
			AddFloatProp(compose, "scalex_top", sd->texScale[0][X], 1);
			AddFloatProp(compose, "scaley_top", sd->texScale[0][Y], 1);
			AddFloatProp(compose, "scalex_mid", sd->texScale[1][X], 1);
			AddFloatProp(compose, "scaley_mid", sd->texScale[1][Y], 1);
			AddFloatProp(compose, "scalex_bottom", sd->texScale[2][X], 1);
			AddFloatProp(compose, "scaley_bottom", sd->texScale[2][Y], 1);
			AddIntProp(compose, "light", sd->light, 0);

			for(int i=0;i<MAX_FLAGS;i++)
			{
				wxString str = cgc->GetSideFlagText(i, 2, true);
				if (str[0]!='!' && str[0]!=0 && sd->Flags.GetBit(i))
				{
					compose << str << " = true;\n";
				}
			}
		}


		compose += "}\n";
	}
	return compose;
}

//----------------------------------------------------------------------------
//
// Save sectors
//
//-----------------------------------------------------------------------------

wxString CLevel::SaveSectorsAsText()
{
	wxString compose;

	for(int i=0;i<NumSectors();i++)
	{
		compose += "sector {\n";

		CSector *se = GetSector(i);

		//
		// Common properties of Sectors are the same for all formats
		//

		AddIntProp(compose, "heightfloor", se->floorh, 0);
		AddIntProp(compose, "heightceiling", se->ceilh, 0);
		AddIntProp(compose, "lightlevel", se->light);
		AddIntProp(compose, "special", se->special, 0);
		AddIntProp(compose, "id", se->tag, 0);
		compose += wxString::Format("texturefloor = \"%.8s\";\n", se->floort);
		compose += wxString::Format("textureceiling = \"%.8s\";\n", se->ceilt);

		//
		// new textmap only properties
		//

		if (m_TextMap)
		{
			AddStringProp(compose, "comment", se->comment, "");
			AddFloatProp(compose, "xpanningfloor", se->offset[FLOOR][X], 0);
			AddFloatProp(compose, "ypanningfloor", se->offset[FLOOR][Y], 0);
			AddFloatProp(compose, "xpanningceiling", se->offset[CEILING][X], 0);
			AddFloatProp(compose, "ypanningceiling", se->offset[CEILING][Y], 0);
			AddFloatProp(compose, "xscalefloor", se->scale[FLOOR][X], 1.);
			AddFloatProp(compose, "yscalefloor", se->scale[FLOOR][Y], 1.);
			AddFloatProp(compose, "xscaleceiling", se->scale[CEILING][X], 1.);
			AddFloatProp(compose, "yscaleceiling", se->scale[CEILING][Y], 1.);
			AddFloatProp(compose, "rotationfloor", se->rotation[FLOOR], 0);
			AddFloatProp(compose, "rotationceiling", se->rotation[CEILING], 0);
			AddIntProp(compose, "lightfloor", se->planelight[FLOOR], 0);
			AddIntProp(compose, "lightceiling", se->planelight[CEILING], 0);
			AddFloatProp(compose, "gravity", se->gravity, 1);
		}

		compose += "}\n";
	}
	return compose;
}

//----------------------------------------------------------------------------
//
// Save as UDMF text format
//
//-----------------------------------------------------------------------------

void CLevel::SaveAsText(const char * fn)
{
	wxString composed;

	TArray<CSideDef> compressed_sds;
	CompressSidedefs(compressed_sds);

	composed
		<< "namespace = " << cgc->namespc << ";\n"
		<< SaveVerticesAsText()
		<< SaveThingsAsText()
		<< SaveLinesAsText()
		<< SaveSidesAsText(compressed_sds)
		<< SaveSectorsAsText();

	char buffer[256];
	FILE * f=fopen(fn,"wb");

	if (!f)
	{
		wxMessageBox("Unable to save level", ZED_CAPTION);
		return;
	}

	TArray<int> lumpofs;
	TArray<char*> lumpnames;
	TArray<int> lumpsize;

	fwrite("PWAD\xb\0\0\0\0\0\0\0",1,12,f);

	lumpofs.Push(ftell(f));
	lumpnames.Push(m_Mission);
	lumpsize.Push(m_FraggleScript.Size());

	fwrite(&m_FraggleScript[0],1,m_FraggleScript.Size(),f);

	lumpofs.Push(ftell(f));
	lumpnames.Push("TEXTMAP");
	lumpsize.Push(int(composed.Length()));
	fwrite(composed.c_str(), 1, composed.Length(),f);

	/*
	//if (!needNodes) for(;i<OrgData.nVertexes;i++) fwrite(&OrgData.pVertexes[i],1,sizeof(CVertex),f);
	adrSg=ftell(f);
	//fwrite(pSegs,1,nSegs,f);
	adrSs=ftell(f);
	//fwrite(pSSector,1,nSSectors,f);
	adrNo=ftell(f);
	//fwrite(pNodes,1,nNodes,f);
	adrBl=ftell(f);
	*/

	if (m_Extended)
	{
		lumpofs.Push(ftell(f));
		lumpnames.Push("BEHAVIOR");
		lumpsize.Push(m_Behavior.Size());
		fwrite(&m_Behavior[0],1,m_Behavior.Size(),f);
		if (m_ACSScript.Size() > 0)
		{
			lumpofs.Push(ftell(f));
			lumpnames.Push("SCRIPTS");
			lumpsize.Push(m_ACSScript.Size());
			fwrite(&m_ACSScript[0],1,m_ACSScript.Size(),f);
		}
	}

	if (m_StrifeConv.Size() > 0)
	{
		lumpofs.Push(ftell(f));
		lumpnames.Push("DIALOGUE");
		lumpsize.Push(m_StrifeConv.Size());
		fwrite(&m_StrifeConv[0], 1, m_StrifeConv.Size(), f);
	}
	if (m_StrifeConvSrc.Size() > 0)
	{
		lumpofs.Push(ftell(f));
		lumpnames.Push("CONVSCPT");
		lumpsize.Push(m_StrifeConvSrc.Size());
		fwrite(&m_StrifeConvSrc[0], 1, m_StrifeConvSrc.Size(), f);
	}
	long adrDir = ftell(f);

	for(unsigned i=0;i<lumpofs.Size();i++)
	{
		strncpy(buffer, lumpnames[i], 8);
		WriteEntry(f, lumpofs[i], lumpofs[i]+lumpsize[i], lumpnames[i]);
	}
	WriteEntry(f, 0,0, "ENDMAP");

	int l = lumpofs.Size()+1;
	fseek(f,4,0);
	fwrite(&l,1,4,f);
	fwrite(&adrDir,1,4,f);
	fclose(f);
	m_changed=false;

}


//----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------

void CLevel::CompressSidedefs(TArray<CSideDef> & sds)
{
	int i,j;
	unsigned k;
	char buffer[9];
	TArray<int> *sidespersector = NULL;
	
	if (m_Compress) sidespersector = new TArray<int>[map.NumSectors()];

	buffer[8]=0;
	for (i=0;i<NumLines();i++) for(j=0;j<2;j++)
	{
		CLine * ln = GetLine(i);
		CSideDef * sd = &ln->sides[j];

		if (sd->sector==-1)
		{
			if (j==0) ln->line.Sidedef1=NO_SIDE;
			if (j==1) ln->line.Sidedef2=NO_SIDE;
		}
		else
		{

			memcpy(sd->texUpper, strupr(strncpy(buffer, sd->texUpper, 8)), 8);
			memcpy(sd->texNormal, strupr(strncpy(buffer, sd->texNormal, 8)), 8);
			memcpy(sd->texLower, strupr(strncpy(buffer, sd->texLower, 8)), 8);

			if (m_Compress)
			{
				// Sidedef compression
				// This should only be used for ports that can handle it without problems (ZDoom etc.)
				k=-1;
				for(unsigned l = 0; l < sidespersector[sd->sector].Size(); l++)
				{
					if (!memcmp(sd, &sds[sidespersector[sd->sector][l]], sizeof(CSideDef))) 
					{
						k = sidespersector[sd->sector][l];
						break;
					}
				}
				if (k == -1)
				{
					k = sds.Push(*sd);
					sidespersector[sd->sector].Push(k);
				}
			}
			else
			{
				// Save uncompressed
				k=sds.Push(*sd);
			}

			if (j==0) ln->line.Sidedef1=k;
			if (j==1) ln->line.Sidedef2=k;
		}
	}
	if (sidespersector) delete sidespersector;
}


//----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------

void CLevel::SaveAsWad(const char * fn)
{
	int i;

	char buffer[256];
	FILE * f=fopen(fn,"wb");

	if (!f)
	{
		wxMessageBox("Unable to save level", ZED_CAPTION);
		return;
	}

	int adrSc,adrTh,adrLi,adrSi,adrVe,adrSg,adrSs,adrNo,adrSe,adrRe,adrBl,adrDir,adrBh,adrAc;
	int adrCvS, adrCvSS;

	fwrite("PWAD\xb\0\0\0\0\0\0\0",1,12,f);

	adrSc=ftell(f);
	fwrite(&m_FraggleScript[0],1,m_FraggleScript.Size(),f);

	adrTh=ftell(f);
	for(i=0;i<NumThings();i++) 
	{
		fwrite(buffer, 1, GetThing(i)->Save(buffer, m_Extended), f);
	}

	TArray<CSideDef> compressed_sds;
	CompressSidedefs(compressed_sds);

	adrLi=ftell(f);
	for(i=0;i<NumLines();i++) 
	{
		fwrite(buffer, 1, GetLine(i)->line.Save(buffer, m_Extended), f);
	}

	adrSi=ftell(f);

	for(unsigned ii=0;ii<compressed_sds.Size();ii++) 
	{
		CSideDef1 sdsave = compressed_sds[ii];
		fwrite(&sdsave,sizeof(CSideDef1),1,f);
	}

	adrVe=ftell(f);
	for(i=0;i<NumVertices();i++) 
	{
		CMapVertex v = *GetVertex(i);
		fwrite(&v, sizeof(CMapVertex), 1, f);
	}

	// This needs only to be handled if I ever decide to make a public release.
	// For my private fun there is no need to handle nodes and other baggage!

	//if (!needNodes) for(;i<OrgData.nVertexes;i++) fwrite(&OrgData.pVertexes[i],1,sizeof(CVertex),f);
	adrSg=ftell(f);
	//fwrite(pSegs,1,nSegs,f);
	adrSs=ftell(f);
	//fwrite(pSSector,1,nSSectors,f);
	adrNo=ftell(f);
	//fwrite(pNodes,1,nNodes,f);
	adrSe=ftell(f);

	for(i=0;i<NumSectors();i++) 
	{
		CSector1 sec = *GetSector(i);
		fwrite(&sec,1,sizeof(CSector1),f);
	}
	adrRe=ftell(f);

	/*
	// save only for configurations that need it!
	*/

	adrBl=ftell(f);

	/*
	// save only for configurations that need it!
	*/
	adrBh=ftell(f);

	if (m_Extended)
	{
		fwrite(&m_Behavior[0],1,m_Behavior.Size(),f);

		adrAc=ftell(f);
		fwrite(&m_ACSScript[0],1,m_ACSScript.Size(),f);

	}

	adrCvS=ftell(f);
	fwrite(&m_StrifeConv[0], 1, m_StrifeConv.Size(), f);

	adrCvSS=ftell(f);
	fwrite(&m_StrifeConvSrc[0], 1, m_StrifeConvSrc.Size(), f);

	adrDir=ftell(f); 

	strncpy(buffer, m_Mission, 8);

	long numlumps=11;
	WriteEntry(f,adrSc,adrTh,buffer);
	WriteEntry(f,adrTh,adrLi,"THINGS\0\0");
	WriteEntry(f,adrLi,adrSi,"LINEDEFS");
	WriteEntry(f,adrSi,adrVe,"SIDEDEFS");
	WriteEntry(f,adrVe,adrSg,"VERTEXES");
	WriteEntry(f,adrSg,adrSs,"SEGS\0\0\0\0");
	WriteEntry(f,adrSs,adrNo,"SSECTORS");
	WriteEntry(f,adrNo,adrSe,"NODES\0\0\0");
	WriteEntry(f,adrSe,adrRe,"SECTORS\0");
	WriteEntry(f,adrRe,adrBl,"REJECT\0\0");
	WriteEntry(f,adrBl,adrBh, "BLOCKMAP");
	if (m_Extended)
	{
		numlumps++;
		WriteEntry(f,adrBh,adrAc,"BEHAVIOR");
		if (adrAc!=adrDir) 
		{
			WriteEntry(f,adrAc,adrCvS,"SCRIPTS");
			numlumps++;
		}
	}
	if (adrCvS != adrCvSS) 
	{
		char namebuf[9];
		sprintf(namebuf, "SCRIPT%c%c", m_Mission[3], m_Mission[4]);
		WriteEntry(f, adrCvS, adrCvSS, namebuf);
		numlumps++;
	}
	if (adrCvSS!=adrDir) 
	{
		WriteEntry(f,adrCvSS, adrDir, "CONVSCPT");
		numlumps++;
	}
	fseek(f,4,0);
	fwrite(&numlumps,1,4,f);
	fwrite(&adrDir,1,4,f);
	fclose(f);
	m_changed=false;
}


//----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------

void CLevel::OnFileSaveas(wxCommandEvent & event)
{
	if (m_Mode==modeLineDraw) return;

	DirectorySaver ds("SaveMapAs");

	wxFileDialog fdlg (m_DrawWindow, "Save map as", ds.Dir(), m_Mission, "WAD Files|*.wad||", 
						wxSAVE|wxOVERWRITE_PROMPT|wxCHANGE_DIR);
	if (fdlg.ShowModal()==wxID_OK)
	{
		wxBusyCursor wait;
		wxString path = fdlg.GetPath();
		path.LowerCase();
		if (m_TextMap)
		{
			//SaveTextMap(fdlg.GetPath().c_str());
			SaveAsText(path.c_str());
		}
		else if (m_AsText)
		{
			SaveAsText(path.c_str());
		}
		else
		{
			SaveAsWad(path.c_str());
		}
	}
}

//----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------

void CLevel::OnFileEnablesidedefcompression(wxCommandEvent & event)
{
	m_Compress = !m_Compress;
	if (config.SetSection("Settings"))
	{
		config.SetIntValueForKey("CompressSD", m_Compress);
	}
}

void CLevel::OnUpdateFileEnablesidedefcompression(wxUpdateUIEvent & event)
{
	event.Check(m_Compress);
}

void CLevel::OnFileSaveAsText(wxCommandEvent & event)
{
	m_AsText = !m_AsText;
	if (config.SetSection("Settings"))
	{
		config.SetIntValueForKey("AsText", m_AsText);
	}
}

void CLevel::OnUpdateFileSaveAsText(wxUpdateUIEvent & event)
{
	event.Check(m_AsText);
}
