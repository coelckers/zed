
// Error checking code.

// Most of the code in this file has been taken from DEU, although it
// has been heavily rewritten and enhanced. However, the original copyright
// and license are still valid!

// Doom Editor Utility, by Brendon Wyber and Raphaël Quinet.

// You are allowed to use any parts of this code in another program, as
// long as you give credits to the authors in the documentation and in
// the program itself.  Read the file README.1ST for more information.

// This program comes with absolutely no warranty.

// Changes to original code (c) 1995-2008 by Christoph Oelckers


#include "StdAfx.h"
#include "doomtype.h"
#include "ZEd.h"
#include "texture.h"
#include "ResourceFile.h"
#include "Level.h"

wxString strOut;

static bool batchmode=false;
static bool error;


void Pump()
{
	if (batchmode) while (wxGetApp().Pending()) wxGetApp().Dispatch();
}

//==========================================================================
//
//
//
//==========================================================================

int CheckFailed(char *prompt1, char *prompt2, bool fatal,int MB_Flag)
{
	wxString str;

	error=true;
	str += wxString::Format("%s%s\n%s\n",batchmode? "" : "Verification failed\n", prompt1,prompt2? prompt2:"");
	if (!batchmode)
	{
		if (fatal == true)
		{
			str += "DOOM will crash if you play with this level.\n";
		}
		str += "Click 'Yes' to see the object";

		return wxMessageBox(str, ZED_CAPTION, MB_Flag);
	}
	else
	{
		strOut += str;
		return wxNO;
	}
}

//==========================================================================
//
//
//
//==========================================================================

int CheckFailed1(char *prompt1, char *prompt2, bool fatal,int MB_Flag)
{
	error=true;
	wxString str = wxString::Format("%s%s\n%s\n",batchmode? "" : "Verification failed\n", prompt1,prompt2? prompt2:"");
	if (!batchmode)
	{
		if (fatal == true)
		{
			str += "DOOM will crash if you play with this level.\n";
		}
		str += "Click 'Cancel' to see the object";
		return wxMessageBox(str, ZED_CAPTION, MB_Flag);
	}
	else
	{
		strOut += str;
		return wxNO;
	}
}

//==========================================================================
//
//
//
//==========================================================================

bool Confirm(char *prompt)
{
	if (!batchmode)
	{
		return wxMessageBox(prompt, ZED_CAPTION, wxYES_NO)==wxYES;
	}
	else
	{
		strOut+=prompt;
		strOut+="\n";
		return false;
	}
}


//==========================================================================
//
//
//
//==========================================================================

void Success()
{
	if (!batchmode) wxMessageBox(error? "Check failed":"Check successful",ZED_CAPTION);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::MarkPoly(int vertex)
{
	int i;

	for (i = 0; i < NumLines(); i++)
	{
		CLine * ln=GetLine(i);
		if ((ln->line.Start==vertex || ln->line.End==vertex) && !ln->ispolyobj)
		{
			ln->ispolyobj=true;
			MarkPoly(ln->line.Start==vertex? ln->line.End:ln->line.Start);
		}
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::CheckSectors(bool paranoid)
{
	int n;
	int s,sd;
	int ldcount;
	int bestld;
	signed char *ends;
	char msg1[80], msg2[80];

	ends = new signed char[NumVertices()];

	error=false;

	for (n = 0; n < NumLines(); n++)
	{
		GetLine(n)->ispolyobj=false;
	}
#if 0	// better not! Although it is not really critical it is still better to list them.
	if (m_Extended)
	{
		for (n = 0; n < NumLines(); n++)
		{
			if (m_Lines[n].line.type==1)
			{
				MarkPoly(m_Lines[n].line.start);
				MarkPoly(m_Lines[n].line.end);
			}
		}
		for (n = 0; n < NumLines(); n++)
		{
			if (m_Lines[n].line.type==5) m_Lines[n].ispolyobj=true;
		}
	}
#endif

	for (s = 0; s < NumSectors(); s++)
	{
		Pump();
		/* clear the "ends" array */
		for (n = 0; n < NumVertices(); n++) ends[n] = 0;

		// for each SideDef bound to the Sector, increment the counter for its starting Vertex 
		// and decrement it for its ending vertex
		// Anything that ends up with a non-zero counter is not fully closed
		ldcount=0;
		for (n = 0; n < NumLines(); n++)
		{
			CLine * ln = GetLine(n);
			// Ignore polyobjects - they are far too often malformed but this doesn't really matter
			if (ln->ispolyobj) continue;
			// Ignore sidedefs with the same sector on both sides
			if (IsLDInSector(n, false)) continue;

			ldcount++;

			if (ln->sides[0].sector == s)
			{
				ends[ln->line.Start] ++;
				ends[ln->line.End] --;
			}
			if (ln->sides[1].sector == s)
			{
				ends[ln->line.End] ++;
				ends[ln->line.Start] --;
			}
		}

		/* every entry in the "ends" array should be "0" */
		for (n = 0; n < NumVertices(); n++)
		{
			if (ends[n] > 0 )
			{
				sprintf( msg1, "Sector #%d is not closed!", s);
				sprintf( msg2, "There is no SideDef ending at Vertex #%d", n);
				GetSector(s)->buggy=1;
				if (CheckFailed(msg1, msg2, false,wxYES_NO)==wxYES)
				{
					OnModeVertices();
					GoToObject(n);
					delete [] ends;
					return;
				}
			}
			if (ends[n] < 0 )
			{
				sprintf( msg1, "Sector #%d is not closed!", s);
				sprintf( msg2, "There is no SideDef starting at Vertex #%d", n);
				GetSector(s)->buggy=1;
				if (CheckFailed(msg1, msg2, false,wxYES_NO)==wxYES)
				{
					OnModeVertices();
					GoToObject(n);
					delete [] ends;
					return;
				}
			}
		}
	}
	delete [] ends;

	/* now check if all ESideDefs are facing a SideDef with the same Sector number */

	//paranoid=true;
	for (n = 0; n < NumLines(); n++)
	{
		Pump();
		CLine * ln = GetLine(n);

		if (ln->ispolyobj) continue;

		if (IsLDInSector(n, paranoid)) continue;
		sd = ln->sides[0].sector;
		if (sd != -1)
		{
			s = GetOppositeSector(n, true,&bestld,paranoid);
			CLine * bestln=GetLine(bestld);

			if (bestld>=0 && bestln->ispolyobj) continue;
			if (s < 0 || sd != s)
			{
				if (1)
				{
					if (s < 0)
					{
						sprintf( msg1, "Sector #%d is not closed!", sd);
						sprintf( msg2, "Check LineDef #%d (first SideDef)", n);
						GetSector(sd)->buggy=1;
					}
					else
					{
						sprintf( msg1, "Sectors #%d and #%d are not closed!", sd, s);
						sprintf( msg2, "Check LineDef #%d (first SideDef) and the one facing it (%d)", n,bestld);
						GetSector(sd)->buggy=1;
						GetSector(s)->buggy=1;
					}
					if (CheckFailed(msg1, msg2, false,wxYES_NO)==wxYES)
					{
						OnModeLinedefs();
						GoToObject(n);
						return;
					}
				}
			}
		}
		sd = ln->sides[1].sector;
		if (sd != -1)
		{
			s = GetOppositeSector(n, false,&bestld,paranoid);
			CLine * bestln=GetLine(bestld);

			if (bestld>=0 && bestln->ispolyobj) continue;
			if (s < 0 || sd != s)
			{
				if ( 1)
				{
					if (s < 0)
					{
						sprintf( msg1, "Sector #%d is not closed!", sd);
						sprintf( msg2, "Check LineDef #%d (second SideDef)", n);
						GetSector(sd)->buggy=1;
					}
					else
					{
						sprintf( msg1, "Sectors #%d and #%d are not closed!", sd, s);
						sprintf( msg2, "Check LineDef #%d (second SideDef) and the one facing it (%d)", n, bestld);
						GetSector(sd)->buggy=1;
						GetSector(s)->buggy=1;
					}
					if (CheckFailed(msg1, msg2, false,wxYES_NO)==wxYES)
					{
						OnModeLinedefs();
						GoToObject(n);
						return;
					}
				}
			}
		}
	}
	Success();
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdSectorsClosed(wxCommandEvent & event)
{
	if (LineDrawNotAllowed()) return;
	batchmode=false;
	for(int i=0;i<NumSectors();i++) 	GetSector(i)->buggy=0;
	CheckSectors(false);
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdSectorsClosedParanoid(wxCommandEvent & event)
{
	if (LineDrawNotAllowed()) return;
	batchmode=false;
	for(int i=0;i<NumSectors();i++) GetSector(i)->buggy=0;

	CheckSectors(true);
}

//==========================================================================
/*
check cross-references and delete unused objects
*/
//==========================================================================

void CLevel::CheckCrossReferences()
{
	char   msg[80];
	char msg1[180], msg2[180];
	int    m;
	int n,r;
	bool didchange=false;

	TArray<int> List;

	*msg2=0;
	for (n = 0; n < (int)NumLines(); n++)
	{
		Pump();
		CLine * ln = GetLine(n);

		/* check for missing first ESideDefs */
		if (ln->sides[0].sector == -1)
		{
			sprintf( msg, "ERROR: LineDef #%d has no first SideDef!", n);
			ln->buggy=1;
			if (CheckFailed(msg, NULL, TRUE,wxYES_NO)==wxYES)
			{
				OnModeLinedefs();
				GoToObject(n);
				return;
			}
		}
		/* check for Vertices used twice in the same LineDef */
		if (ln->line.Start == ln->line.End)
		{
			sprintf( msg, "ERROR: LineDef #%d uses the same Vertex twice (#%d)", n, ln->line.Start);
			ln->buggy=1;
			if (CheckFailed(msg, NULL, TRUE,wxYES_NO)==wxYES)
			{
				OnModeLinedefs();
				GoToObject(n);
				return;
			}
		}
	}
	if (!batchmode) MakeBackup("Consistency check", true, true, true, false);

	/* check if there aren't two LineDefs between the same Vertices */
	List.Clear();
	for (n = NumLines() - 1; n >= 1; n--)
	{
		Pump();
		CLine * ln = GetLine(n);

		for (m = n - 1; m >= 0; m--)
		{
			CLine * lm = GetLine(m);

			if ((ln->line.Start == lm->line.Start && ln->line.End == lm->line.End)
				|| (ln->line.Start == lm->line.End && ln->line.End == lm->line.Start))
			{
				// From now on this is considered a fatal bug that cannot be circumvented by setting the ignore flag!

				sprintf( msg1, "FATAL ERROR: Linedefs #%d and %d share the same vertices", n,m);
				if (!batchmode) sprintf( msg2, "Do you want to delete the duplicate linedef?");
				ln->buggy=1;
				lm->buggy=1;

				if ((r=CheckFailed1(msg1, msg2, FALSE, wxYES_NO|wxCANCEL))==wxCANCEL)
				{
					OnModeLinedefs();
					GoToObject(n);
					return;
				}
				if (r==wxYES && !batchmode) 
				{
					List.Push(n);
				}
			}
		}
	}
	// delete after the check is complete!
	for(m=0;m<(int)List.Size();m++) 
	{
		didchange=true;
		DeleteOneLineDef(List[m]);
	}
	List.Clear();

	int bit=0;
	/* check for invalid flags in the m_Lines */
	if (m_TextMap) bit = cgc->CheckTextMapThingFlag("blocking");
	for (n = 0; n < (int)NumLines(); n++)
	{
		CLine * ln = GetLine(n);

		if ((ln->line.Flags.GetBit(bit)) == 0 && ln->sides[1].sector == -1)
		{
			List.Push(n);
		}
	}
	if (List.Size() && (Confirm("Some LineDefs have only one side but their Im bit is not set\r\nDo you want to set the 'Impassible' flag?")))
	{
		for(n=0;n<(int)List.Size();n++)
		{
			didchange=true;
			GetLine(List[n])->line.Flags.SetBit(bit);
		}
	}
	bit = 2;
	if (m_TextMap) bit = cgc->CheckTextMapThingFlag("twosided");
	List.Clear();
	for (n = 0; n < (int)NumLines(); n++)
	{
		CLine * ln = GetLine(n);

		if ((ln->line.Flags.GetBit(bit)) != 0 && ln->sides[1].sector == -1)
		{
			List.Push(n);
			ln->buggy=1;
		}
	}
	if (List.Size() && (Confirm("Some LineDefs have only one side but their 2S bit is set\r\nDo you want to clear the 'two-sided' flag?")))
	{
		for(n=0;n<(int)List.Size();n++)
		{
			didchange=true;
			GetLine(List[n])->line.Flags.ClearBit(bit);
		}
	}
	List.Clear();
	for (n = 0; n < (int)NumLines(); n++)
	{
		CLine * ln = GetLine(n);

		if (ln->line.Flags.GetBit(bit) == 0 && ln->sides[1].sector != -1)
		{
			List.Push(n);
			ln->buggy=1;
		}
	}
	if (List.Size() && (Confirm("Some LineDefs have two sides but their 2S bit is not set\r\nDo you want to set the 'two-sided' flag?")))
	{
		for(n=0;n<(int)List.Size();n++)
		{
			didchange=true;
			GetLine(List[n])->line.Flags.SetBit(bit);
		}
	}
	Pump();

	/* select all Vertices */
	List.Resize(NumVertices());
	for (n = 0; n < (int)NumVertices(); n++) List[n]=1;
	/* unselect Vertices used in a LineDef */
	for (n = 0; n < (int)NumLines(); n++)
	{
		CLine * ln = GetLine(n);

		m = ln->line.Start;
		if (m >= 0) List[m]=0;
		m = ln->line.End;
		if (m >= 0) List[m]=0;
	}
	for(m=n=0;n<(int)NumVertices();n++) m+=List[n];
	/* check if there are any Vertices left */
	if (m && (Confirm("Some Vertices are not bound to any LineDef\r\nDo you want to delete these unused Vertices?")))
	{
		didchange=true;
		for(n=NumVertices()-1;n>=0;n--) 
			if (List[n]) DeleteOneVertex(n);
	}

	/* select all Sectors */
	List.Resize(NumSectors());
	for (n = 0; n < NumSectors(); n++) List[n]=1;
	/* unselect m_Sectors bound to a SideDef */
	for (n = 0; n < (int)NumLines(); n++)
	{
		CLine * ln = GetLine(n);

		m = ln->sides[0].sector;
		if (m>=0) List[m]=0;
		m = ln->sides[1].sector;
		if (m>=0) List[m]=0;
	}
	/* check if there are any Sectors left */
	Pump();
	for(m=n=0;n<NumSectors();n++) m+=List[n];
	if (m && (Confirm("Some Sectors are not bound to any SideDef\r\nDo you want to delete these unused Sectors?")))
	{
		didchange=true;
		for(n=NumSectors()-1;n>=0;n--) 
			if (List[n]) DeleteOneSector(n);
	}	
	if (!didchange && !batchmode) RemoveBackup();
	UncheckAll();
	m_Selection=0;
	if (!batchmode)
	{
		Success();
		m_DrawWindow->Refresh();
		UpdateStatusBar();
	}
}



//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdXRef(wxCommandEvent & event)
{
	if (LineDrawNotAllowed()) return;
	batchmode=false;
	CheckCrossReferences();
}



//==========================================================================
/*
check for missing textures
*/
//==========================================================================

void CLevel::CheckTextures( void) 
{
	int  n,r;
	int  s1, s2;
	char msg1[180], msg2[180];
	bool didchange=false;

	TArray<int> no_bottomtexture, no_toptexture;
	TArray<bool> floorslope, ceilingslope;

	if (!batchmode) MakeBackup("Texture check", false, true, false, false);
	*msg2=0;

	error=false;
	no_bottomtexture.Resize(NumSectors());
	no_toptexture.Resize(NumSectors());
	floorslope.Resize(NumSectors());
	ceilingslope.Resize(NumSectors());

	memset(&floorslope[0], 0, NumSectors());
	memset(&ceilingslope[0], 0, NumSectors());

	for(n=0;n<NumLines();n++)
	{
		CLine * l = GetLine(n);

		if (l->sides[0].sector!=-1 && l->sides[1].sector!=-1 && m_Extended && l->line.type==181)
		{
			if (l->line.args[0]) floorslope[l->sides[0].sector] = floorslope[l->sides[1].sector] = true;
			if (l->line.args[1]) ceilingslope[l->sides[0].sector] = ceilingslope[l->sides[1].sector] = true;
		}
	}

	memset(&no_bottomtexture[0],0xff,sizeof(int)*NumSectors());
	memset(&no_toptexture[0],0xff,sizeof(int)*NumSectors());

	int bit = 2;
	if (m_TextMap) bit = cgc->CheckTextMapThingFlag("twosided");

	for (n = 0; n < NumLines(); n++)
	{
		Pump();
		CLine * l = GetLine(n);

		s1 = l->sides[0].sector;
		s2 = l->sides[1].sector;
		if (s1 >= 0 && s2 >= 0 && (l->line.Flags.GetBit(bit)))
		{
			CSector * sec1=GetSector(s1);
			CSector * sec2=GetSector(s2);
			if (sec1->ceilh > sec2->ceilh)
			{
				if (l->sides[0].texUpper[0]!= '-') no_toptexture[s1]=0;
				else
				{
					if (no_toptexture[s1]==-1) no_toptexture[s1]=sec2->ceilh+0x80000;
					else if (no_toptexture[s1]==0 || no_toptexture[s1]==-2) continue;
					else if (no_toptexture[s1]!=(sec2->ceilh+0x80000)) no_toptexture[s1]=-2;
				}
			}

			if (sec1->floorh<sec2->floorh)
			{
				if (l->sides[0].texLower[0]!= '-') no_bottomtexture[s1]=0;
				else 
				{
					if (no_bottomtexture[s1]==-1) no_bottomtexture[s1]=sec2->floorh+0x80000;
					else if (no_bottomtexture[s1]==0 || no_bottomtexture[s1]==-2) continue;
					else if (no_bottomtexture[s1]!=(sec2->floorh+0x80000)) no_bottomtexture[s1]=-2;
				}
			}

			s1 = l->sides[1].sector;
			s2 = l->sides[0].sector;
			sec1=GetSector(s1);
			sec2=GetSector(s2);
			if (sec1->ceilh>sec2->ceilh)
			{
				if (l->sides[1].texUpper[0]!= '-') no_toptexture[s1]=0;
				else
				{
					if (no_toptexture[s1]==-1) no_toptexture[s1]=sec2->ceilh+0x80000;
					else if (no_toptexture[s1]==0 || no_toptexture[s1]==-2) continue;
					else if (no_toptexture[s1]!=(sec2->ceilh+0x80000)) no_toptexture[s1]=-2;
				}
			}

			if (sec1->floorh<sec2->floorh)
			{
				if (l->sides[1].texLower[0]!= '-') no_bottomtexture[s1]=0;
				else
				{
					if (no_bottomtexture[s1]==-1) no_bottomtexture[s1]=sec2->floorh+0x80000;
					else if (no_bottomtexture[s1]==0 || no_bottomtexture[s1]==-2) continue;
					else if (no_bottomtexture[s1]!=(sec2->floorh+0x80000)) no_bottomtexture[s1]=-2;
				}
			}
		}
		else
		{
			if (s1!=-1)
			{
				no_toptexture[s1]=no_bottomtexture[s1]=0;
			}
			if (s2!=-1)
			{
				no_toptexture[s1]=no_bottomtexture[s1]=0;
			}
		}
	}


	for (n = 0; n < NumLines(); n++)
	{
		Pump();
		CLine * l = GetLine(n);

		s1 = l->sides[0].sector;
		s2 = l->sides[1].sector;
		CSector * sec1=GetSector(s1);
		CSector * sec2=GetSector(s2);

		if (s1 >= 0 && s2 < 0)
		{
			if (l->sides[0].texNormal[0] == '-' && l->sides[0].texNormal[1] == '\0')
			{
				if (sec1->ceilh > sec1->floorh)
				{
					sprintf( msg1, "Error in one-sided Linedef #%d: No normal texture (Sector %d)", n, s1);
					if (!batchmode) sprintf( msg2, "Do you want to set the texture to \"%s\" and continue?", DEF_WALLTEX);
					l->buggy=1;
					if ((r=CheckFailed1(msg1, msg2, FALSE,wxYES_NO|wxCANCEL))==wxCANCEL) 
					{
						goto showlinedef;
					}
					if (r==wxYES && !batchmode) 
					{
						didchange=true;
						strncpy( l->sides[0].texNormal, DEF_WALLTEX, 8);
					}
				}
			}
		}
		if (s1 >= 0 && s2 >= 0 && sec1->ceilh > sec2->ceilh)
		{
			if (sec1->floorh<sec1->ceilh || sec1->tag)
			{
				if (l->sides[0].texUpper[0] == '-' && l->sides[0].texUpper[1] == '\0'
					&& (/*strncmp( sec1->ceilt, "F_SKY", 5) ||*/ strncmp( sec2->ceilt, "F_SKY", 5)))
				{
					if (/*!no_toptexture[s1] &&*/ (l->line.Flags.GetBit(bit)))
					{
						if (!m_Extended || l->line.type!=181 || l->line.args[1]==0)
						{
							if (!ceilingslope[s1] && !ceilingslope[s2])
							{
								sprintf( msg1, "Error in first SideDef of Linedef #%d: No upper texture (Sector %d)", n, s1);
								if (!batchmode) sprintf( msg2, "Do you want to set the texture to \"%s\" and continue?", DEF_WALLTEX);
								l->buggy=1;
								if ((r=CheckFailed1(msg1, msg2, FALSE,wxYES_NO|wxCANCEL))==wxCANCEL)
								{
									goto showlinedef;
								}
								if (r==wxYES && !batchmode) 
								{
									didchange=true;
									strncpy( l->sides[0].texUpper, DEF_WALLTEX, 8);
								}
							}
						}
					}
				}
			}
		}

		if (s1 >= 0 && s2 >= 0 && sec1->floorh < sec2->floorh)
		{
			if (sec1->floorh<sec1->ceilh || sec1->tag)
			{
				if (l->sides[0].texLower[0] == '-' && l->sides[0].texLower[1] == '\0'
					&& (/*strncmp( sec1->ceilt, "F_SKY", 5) ||*/ strncmp( sec2->floort, "F_SKY", 5)))

				{
					if (/*!no_bottomtexture[s1] &&*/ (l->line.Flags.GetBit(bit)))
					{
						if (!m_Extended || l->line.type!=181 || l->line.args[0]==0)
						{
							if (!floorslope[s1] && !floorslope[s2])
							{
								sprintf( msg1, "Error in first SideDef of Linedef #%d: No lower texture (Sector %d)", n, s1);
								if (!batchmode) sprintf( msg2, "Do you want to set the texture to \"%s\" and continue?",DEF_WALLTEX);
								l->buggy=1;
								if ((r=CheckFailed1(msg1, msg2, FALSE,wxYES_NO|wxCANCEL))==wxCANCEL)
								{
									goto showlinedef;
								}
								if (r==wxYES && !batchmode) 
								{
									didchange=true;
									strncpy( l->sides[0].texLower, DEF_WALLTEX, 8);
								}
							}
						}
					}
				}
			}
		}
		if (s1 >= 0 && s2 >= 0 && sec2->ceilh > sec1->ceilh)
		{
			if (sec2->floorh<sec2->ceilh || sec2->tag)
			{
				if (l->sides[1].texUpper[0] == '-' && l->sides[1].texUpper[1] == '\0'
					&& (strncmp( sec1->ceilt, "F_SKY", 5)))
				{
					if (/*!no_toptexture[s2] &&*/ (l->line.Flags.GetBit(bit)))
					{
						if (!m_Extended || l->line.type!=181 || l->line.args[1]==0)
						{
							if (!ceilingslope[s1] && !ceilingslope[s2])
							{
								sprintf( msg1, "Error in second SideDef of Linedef #%d: No upper texture (Sector %d)", n,s2);
								if (!batchmode) sprintf( msg2, "Do you want to set the texture to \"%s\" and continue?", DEF_WALLTEX);
								l->buggy=1;
								if ((r=CheckFailed1(msg1, msg2, FALSE,wxYES_NO|wxCANCEL))==wxCANCEL)
								{
									goto showlinedef;
								}
								if (r==wxYES && !batchmode) 
								{
									didchange=true;
									strncpy( l->sides[1].texUpper, DEF_WALLTEX, 8);
								}
							}
						}
					}
				}
			}
		}
		if (s1 >= 0 && s2 >= 0 && sec2->floorh < sec1->floorh)
		{
			if (sec2->floorh<sec2->ceilh || sec2->tag)
			{
				if (l->sides[1].texLower[0] == '-' && l->sides[1].texLower[1] == '\0'
					&& (strncmp( sec1->floort, "F_SKY", 5)))
				{
					if  (/*!no_bottomtexture[s2] &&*/ (l->line.Flags.GetBit(bit)))
					{
						if (!m_Extended || l->line.type!=181 || l->line.args[0]==0)
						{
							if (!floorslope[s1] && !floorslope[s2])
							{
								sprintf( msg1, "Error in second SideDef of Linedef #%d: No lower texture (Sector %d)", n, s2);
								if (!batchmode) sprintf( msg2, "Do you want to set the texture to \"%s\" and continue?", DEF_WALLTEX);
								l->buggy=1;
								if ((r=CheckFailed1(msg1, msg2, FALSE,wxYES_NO|wxCANCEL))==wxCANCEL)
								{
									goto showlinedef;
								}
								if (r==wxYES && !batchmode) 
								{
									didchange=true;
									strncpy( l->sides[1].texLower, DEF_WALLTEX, 8);
								}
							}
						}
					}
				}
			}
		}
	}
	if (!didchange && !batchmode) RemoveBackup();
	Success();
	return;

showlinedef:

	if (!didchange && !batchmode) RemoveBackup();
	OnModeLinedefs();
	GoToObject( n);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdMissingTextures(wxCommandEvent & event)
{
	if (LineDrawNotAllowed()) return;
	batchmode=false;
	CheckTextures();
}



//==========================================================================
/*
check for invalid texture names
*/
//==========================================================================

void CLevel::CheckTextureNames( void) 
{
	int  n;
	char msg1[ 80], msg2[ 80];

	error=false;

	ValidateTextureManager();
	for (n = 0; n < NumSectors(); n++)
	{
		Pump();
		CSector * sec = GetSector(n);
		if (!texman->IsFlatTexture( sec->ceilt))
		{
			sprintf( msg1, "Invalid ceiling texture in Sector #%d", n);
			sprintf( msg2, "The name \"%.8s\" is not a floor/ceiling texture", sec->ceilt);
			//m_Sectors[n].buggy=1;
			if (CheckFailed(msg1, msg2, FALSE,wxYES_NO)==wxYES)
			{
				OnModeSectors();
				GoToObject(n);
				return;
			}
		}
		if (!texman->IsFlatTexture( sec->floort))
		{
			sprintf( msg1, "Invalid floor texture in Sector #%d", n);
			sprintf( msg2, "The name \"%.8s\" is not a floor/ceiling texture", sec->floort);
			//m_Sectors[n].buggy=1;
			if (CheckFailed(msg1, msg2, FALSE,wxYES_NO)==wxYES)
			{
				OnModeSectors();
				GoToObject(n);
				return;
			}
		}
	}
	for(n=0;n<NumLines();n++)
	{
		Pump();
		CLine * l = GetLine(n);

		if (l->sides[0].sector>=0 && 
			(!m_Extended &&
				l->line.type!=242 && l->line.type!=350 && l->line.type!=351 && l->line.type!=260 &&	
				l->line.type!=282) ||
			(!m_Extended && l->line.type!=209) 
		   )
		{
			if (!texman->IsWallTexture(l->sides[0].texUpper))
			{
				sprintf( msg1, "Invalid upper texture ob 1st side of LineDef #%d", n);
				sprintf( msg2, "The name \"%.8s\" is not a wall texture", l->sides[0].texUpper);
				//l->buggy=1;
				if (CheckFailed(msg1, msg2, FALSE,wxYES_NO)==wxYES)
				{
					OnModeLinedefs();
					GoToObject(n);
					return;
				}
			}
			if (!texman->IsWallTexture(l->sides[0].texLower))
			{
				sprintf( msg1, "Invalid lower texture ob 1st side of LineDef #%d", n);
				sprintf( msg2, "The name \"%.8s\" is not a wall texture", l->sides[0].texLower);
				//l->buggy=1;
				if (CheckFailed(msg1, msg2, FALSE,wxYES_NO)==wxYES)
				{
					OnModeLinedefs();
					GoToObject(n);
					return;
				}
			}
			if (!texman->IsWallTexture(l->sides[0].texNormal))
			{
				sprintf( msg1, "Invalid normal texture ob 1st side of LineDef #%d", n);
				sprintf( msg2, "The name \"%.8s\" is not a wall texture", l->sides[0].texNormal);
				//l->buggy=1;
				if (CheckFailed(msg1, msg2, FALSE,wxYES_NO)==wxYES)
				{
					OnModeLinedefs();
					GoToObject(n);
					return;
				}
			}
		}
		if (l->sides[1].sector>=0)
		{
			if (!texman->IsWallTexture(l->sides[1].texUpper))
			{
				sprintf( msg1, "Invalid upper texture ob 2nd side of LineDef #%d", n);
				sprintf( msg2, "The name \"%.8s\" is not a wall texture", l->sides[1].texUpper);
				//l->buggy=1;
				if (CheckFailed(msg1, msg2, FALSE,wxYES_NO)==wxYES)
				{
					OnModeLinedefs();
					GoToObject(n);
					return;
				}
			}
			if (!texman->IsWallTexture(l->sides[1].texLower))
			{
				sprintf( msg1, "Invalid lower texture ob 2nd side of LineDef #%d", n);
				sprintf( msg2, "The name \"%.8s\" is not a wall texture", l->sides[1].texLower);
				//l->buggy=1;
				if (CheckFailed(msg1, msg2, FALSE,wxYES_NO)==wxYES)
				{
					OnModeLinedefs();
					GoToObject(n);
					return;
				}
			}
			if (!texman->IsWallTexture(l->sides[1].texNormal))
			{
				sprintf( msg1, "Invalid normal texture ob 2nd side of LineDef #%d", n);
				sprintf( msg2, "The name \"%.8s\" is not a wall texture", l->sides[1].texNormal);
				//l->buggy=1;
				if (CheckFailed(msg1, msg2, FALSE,wxYES_NO)==wxYES)
				{
					OnModeLinedefs();
					GoToObject(n);
					return;
				}
			}
		}
	}
	Success();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdTextureNames(wxCommandEvent & event)
{
	if (LineDrawNotAllowed()) return;
	batchmode=false;
	CheckTextureNames();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::CheckAll(bool paranoid)
{
	int i;
	for(i=0;i<NumLines();i++) GetLine(i)->buggy=0;
	for(i=0;i<NumSectors();i++) GetSector(i)->buggy=0;
	batchmode=true;
	strOut="";
	wxBusyCursor wait;
#ifndef _DEBUG
	try
#endif
	{
		CheckSectors(paranoid);
		CheckCrossReferences();
		CheckTextures();
		//CheckTextureNames();
	}
#ifndef _DEBUG
	catch(...)
	{
		wxMessageBox("--------- Exception occured ---------\n", ZED_CAPTION);
		return;
	}
#endif

	DisplayText(m_DrawWindow, "Check level consistency", strOut);
	batchmode=false;
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdCheckAll(wxCommandEvent & event)
{
	CheckAll(false);
}

void CLevel::OnCmdCheckAllParanoid(wxCommandEvent & event)
{
	CheckAll(true);
}


//==========================================================================
//
//
//
//==========================================================================

bool CLevel::DoLevelCheck(wxProgressDialog & prog, int checkmode, int currentlev, int maxlev)
{
	char text[256];
	int i;
	for(i=0;i<NumLines();i++) GetLine(i)->buggy=0;
	for(i=0;i<NumSectors();i++) GetSector(i)->buggy=0;

	sprintf(text,"%s: processing", m_Mission);

#ifndef _DEBUG
	try
#endif
	{
		if (checkmode)
		{
			sprintf(text,"%s: checking sectors",m_Mission);
			if (!prog.Update(100*currentlev/maxlev, text)) return false;
			Pump();
			CheckSectors(checkmode==2);
			sprintf(text,"%s: checking cross references",m_Mission);
			if (!prog.Update(100*currentlev/maxlev + 25/maxlev, text)) return false;
			Pump();
			CheckCrossReferences();
		}
		if (checkmode<2)
		{
			sprintf(text,"%s: checking textures",m_Mission);
			if (!prog.Update(100*currentlev/maxlev + 50/maxlev, text)) return false;
			Pump();
			CheckTextures();
			if (checkmode==0)
			{
				sprintf(text,"%s: checking texture names",m_Mission);
				if (!prog.Update(100*currentlev/maxlev + 75/maxlev, text)) return false;
				Pump();
				CheckTextureNames();
			}
		}
	}
#ifndef _DEBUG
	catch(...)
	{
		strOut+="--------- Exception occured ---------\n";
	}
#endif
	return true;
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::CheckPWAD(int mode)
{
	TArray<MapRecord> maplist;
	wxProgressDialog prog("Checking levels", "", 100, m_DrawWindow, wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_CAN_ABORT);
	CResourceFile * rf = CResourceFile::GetResourceFile(m_currentWAD);
	if (!rf) return;

	batchmode=true;
	strOut="";

	rf->GetMapList(maplist);

	{
		wxWindowDisabler dis;

		for(unsigned i=0;i<maplist.Size();i++)
		{
			CLevel * level = new CLevel(false);
			int lumpno=maplist[i].maplump;
			try
			{
				level->Load(lumpno+(QWORD(m_currentWAD+1)<<32), maplist[i].mapname);
			}
			catch (CRecoverableError *)
			{
				continue;
			}
			strOut += maplist[i].mapname;
			strOut += "\n";
			if (!level->DoLevelCheck(prog, mode, i, maplist.Size()))
			{
				delete level;
				batchmode=false;
				return;
			}
			delete level;
		}
	}
	DisplayText(m_DrawWindow, "Check level consistency for PWAD", strOut);
	batchmode=false;
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdCheckAllPWAD(wxCommandEvent & event)
{
	CheckPWAD(1);
}

void CLevel::OnCmdCheckTexturesPWAD(wxCommandEvent & event)
{
	CheckPWAD(0);
}

void CLevel::OnCmdCheckAllPWADParanoid(wxCommandEvent & event)
{
	CheckPWAD(2);
}
