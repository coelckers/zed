
// Editing utilities
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
#include "ZEd.h"
#include "View2D.h"
/*
#include "RotateInput.h"
*/
#include "texture.h"
#include "Level.h"




//==========================================================================
//
//
//
//==========================================================================

static bool IsSelected(void *_array,void * ptr,int arcount)
{
	void **array=(void**)_array;
	for(int i=0;i<arcount;i++) if (array[i]==ptr) return true;
	return false;
}

static bool IsSelected(int *array,int ptr,int arcount)
{
	for(int i=0;i<arcount;i++) if (array[i]==ptr) return true;
	return false;
}

static bool IsSelectedS(short *array,int ptr,int arcount)
{
	for(int i=0;i<arcount;i++) if (array[i]==ptr) return true;
	return false;
}



//==========================================================================
/*
   check if there is something of interest inside the given box
*/
//==========================================================================


int CLevel::GetCurSector(int x0, int y0, int x1, int y1)
{
	int n, m, cur, curx;
	int lx0, ly0, lx1, ly1;
	int midx, midy;
	
	cur = -1;
	if (x1 < x0)
	{
		n = x0;
		x0 = x1;
		x1 = n;
	}
	if (y1 < y0)
	{
		n = y0;
		y0 = y1;
		y1 = n;
	}
	
	/* hack, hack...  I look for the first LineDef crossing an horizontal half-line drawn from the cursor */
	curx = 65535;
	cur = -1;
	midx = (x0 + x1) / 2;
	midy = (y0 + y1) / 2;
	for (n = 0; n < NumLines(); n++)
	{										
		if ((StVt(n)->Y() > midy) != (EnVt(n)->Y() > midy))
		{
			lx0 = int(StVt(n)->X());
			ly0 = int(StVt(n)->Y());
			lx1 = int(EnVt(n)->X());
			ly1 = int(EnVt(n)->Y());
			m = lx0 + (int) ((long) (midy - ly0) * (long) (lx1 - lx0) / (long) (ly1 - ly0));
			if (m >= midx && m < curx)
			{
				curx = m;
				cur = n;
			}
		}
	}
	/* now look if this LineDef has a SideDef bound to one sector */
	if (cur >= 0)
	{
		if (StVt(cur)->Y() > EnVt(cur)->Y())
			cur = FrontSecNo(cur);
		else
			cur = BackSecNo(cur);
	}
	else
		cur = -1;
	return cur;
}



bool CLevel::IsLDInSector(int ld, bool paranoid)
{
	if (FrontSecNo(ld)==-1 || BackSecNo(ld)==-1 ) return false;
	if (FrontSecNo(ld) != BackSecNo(ld)) return false;
	return !paranoid;
}

//==========================================================================
/*
   get the Sector number of the SideDef opposite to this SideDef
   (returns -1 if it cannot be found)
*/
//==========================================================================

int CLevel::GetOppositeSector(int ld1, bool firstside,int * pBest, bool paranoid) /* SWAP! */
{
	double x0, y0, dx0, dy0;
	double x1, y1, dx1, dy1;
	double x2, y2, dx2, dy2;
	double dist;
	int ld2;
	int bestld;
	double bestdist, bestmdist;
	double xy1;

	/* get the coords for this LineDef */
	x0  = StVt(ld1)->X();
	y0  = StVt(ld1)->Y();
	dx0  = EnVt(ld1)->X() - x0;
	dy0  = EnVt(ld1)->Y() - y0;
	
	/* find the normal vector for this LineDef */
	x1  = (dx0 + x0 + x0) / 2;
	y1  = (dy0 + y0 + y0) / 2;
	if (firstside)
	{
		dx1 = dy0;
		dy1 = -dx0;
	}
	else
	{
		dx1 = -dy0;
		dy1 = dx0;
	}
	
	bestld = -1;

	/* use a parallel to an axis instead of the normal vector (faster method) */
	if (fabs( dy1) > fabs( dx1))
	{
		xy1=x1+0.25;	// Don't let it go through integer coordinates
						// If the projected line goes through a vertex it might
						// miss either line connected to it.

		if (dy1 > 0)
		{
			/* get the nearest LineDef in that direction (increasing Y's: North) */
			bestdist = 32767;
			bestmdist = 32767;
			for (ld2 = 0; ld2 < NumLines(); ld2++)
			{
				if (ld2 != ld1 && ((StVt(ld2)->X() > xy1) != (EnVt(ld2)->X() > xy1)))
				{
					// Ignore linedefs with the same sector on both sides
					if (IsLDInSector(ld2, paranoid)) continue;

					x2  = StVt(ld2)->X();
					y2  = StVt(ld2)->Y();
					dx2 = EnVt(ld2)->X() - x2;
					dy2 = EnVt(ld2)->Y() - y2;
					dist = y2 + (xy1 - x2) * dy2 / dx2;
					if (dist > y1 && (dist < bestdist || (dist == bestdist && (y2 + dy2 / 2) < bestmdist)))
					{
						bestld = ld2;
						bestdist = dist;
						bestmdist = y2 + dy2 / 2;
					}
				
				}
			}
		}
		else
		{
			/* get the nearest LineDef in that direction (decreasing Y's: South) */
			bestdist = -32767;
			bestmdist = -32767;
			for (ld2 = 0; ld2 < NumLines(); ld2++)
			{
				if (ld2 != ld1 && ((StVt(ld2)->X() > xy1) != (EnVt(ld2)->X() > xy1)))
				{
					// Ignore linedefs with the same sector on both sides
					if (IsLDInSector(ld2, paranoid)) continue;

					x2  = StVt(ld2)->X();
					y2  = StVt(ld2)->Y();
					dx2 = EnVt(ld2)->X() - x2;
					dy2 = EnVt(ld2)->Y() - y2;
					dist = y2 +  ( double(xy1 - x2) * double(dy2) / double(dx2));
					if (dist < y1 && (dist > bestdist || (dist == bestdist && (y2 + dy2 / 2) > bestmdist)))
					{
						bestld = ld2;
						bestdist = dist;
						bestmdist = y2 + dy2 / 2;
					}
				}
			}
		}
	}
	else
	{
		xy1=y1+0.25;
		if (dx1 > 0)
		{
			/* get the nearest LineDef in that direction (increasing X's: East) */
			bestdist = 32767;
			bestmdist = 32767;
			for (ld2 = 0; ld2 < NumLines(); ld2++)
			{
				if (ld2 != ld1 && ((StVt(ld2)->Y() > xy1) != (EnVt(ld2)->Y() > xy1)))
				{
					// Ignore linedefs with the same sector on both sides
					if (IsLDInSector(ld2, paranoid)) continue;

					x2  = StVt(ld2)->X();
					y2  = StVt(ld2)->Y();
					dx2 = EnVt(ld2)->X() - x2;
					dy2 = EnVt(ld2)->Y() - y2;
					dist = x2 + (double(xy1 - y2) * double(dx2) / double(dy2));
					if (dist > x1 && (dist < bestdist || (dist == bestdist && (x2 + dx2 / 2) < bestmdist)))
					{
						bestld = ld2;
						bestdist = dist;
						bestmdist = x2 + dx2 / 2;
					}
				}
			}
		}
		else
		{
			/* get the nearest LineDef in that direction (decreasing X's: West) */
			bestdist = -32767;
			bestmdist = -32767;
			for (ld2 = 0; ld2 < NumLines(); ld2++)
			{
				if (ld2 != ld1 && ((StVt(ld2)->Y() > xy1) != (EnVt(ld2)->Y() > xy1)))
				{
					// Ignore linedefs with the same sector on both sides
					if (IsLDInSector(ld2, paranoid)) continue;

					x2  = StVt(ld2)->X();
					y2  = StVt(ld2)->Y();
					dx2 = EnVt(ld2)->X() - x2;
					dy2 = EnVt(ld2)->Y() - y2;
					dist = x2 + (double(xy1 - y2) * double(dx2) / double(dy2));
					if (dist < x1 && (dist > bestdist || (dist == bestdist && (x2 + dx2 / 2) > bestmdist)))
					{
						bestld = ld2;
						bestdist = dist;
						bestmdist = x2 + dx2 / 2;
					}
					else if (dist==bestdist && bestmdist==x2+dx2/2)
					{
						int v11=GetLine(ld1)->line.Start;
						int v12=GetLine(ld1)->line.End;
						int v21=GetLine(ld2)->line.Start;
						int v22=GetLine(ld2)->line.End;

						if (v11==v21 || v12==v21 || v11==v22 || v12==v22) bestld=ld2;
					}
				}
			}
		}
	}
	
	/* no intersection: the LineDef was pointing outwards! */
	if (bestld < 0)
		return -1;
	
	if (pBest) *pBest=bestld;

	/* OK, we got it -- return the Sector number */
	if (fabs( dy1) > fabs( dx1))
	{
		if ((StVt(bestld)->X() < EnVt(bestld)->Y()) == (dy1 > 0))
			ld2= FrontSecNo(bestld);
		else
			ld2= BackSecNo(bestld);
	}
	else
	{
		if ((StVt(bestld)->Y() < EnVt(bestld)->Y()) != (dx1 > 0))
			ld2= FrontSecNo(bestld);
		else
			ld2= BackSecNo(bestld);
	}
	return ld2;
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdTag(wxCommandEvent & event)
{
	int tag=FindFreeTag();
	if (!m_Extended)
		wxMessageBox(wxString::Format("First free tag is %d",tag), ZED_CAPTION);
	else
		wxMessageBox(wxString::Format("First free tag is %d\nFirst free tid is %d\nFirst free line ID is %d", tag, 
			FindFreeTid(), FindFreeLineId()), ZED_CAPTION);
}


//==========================================================================
//
//
//
//==========================================================================

int CLevel::FindFreeTag() 
{
	int  tag;
	int n;
	bool ok;
	
	tag = 1;
	ok = false;
	while (! ok)
	{
		ok = true;
		if (!m_Extended)
		{
			for (n = 0; n < NumLines(); n++)
			{
				if (GetLine(n)->line.tag == tag)
				{
					ok = false;
					break;
				}
			}
		}
		else
		{
			for (n = 0; n < NumLines(); n++)
			{
				int spec = GetLine(n)->line.type;
				if (spec>0 && spec<=255 && cgc->LineMap[spec])
				{
					for(int j=0;j<5;j++)
					{
						if (GetLine(n)->line.args[j] == tag)
						{
							wxString str = cgc->LineMap[spec]->args[j];
							if (!strnicmp(str.c_str(), "Tag:", 4))
							{
								ok = false;
								goto break2;
							}
						}
					}
				}
			}
			break2: ;
		}
		if (ok)	
		{
			for (n = 0; n < NumSectors(); n++)
			{
				if (GetSector(n)->tag == tag)
				{
					ok = false;
					break;
				}
			}
		}
		tag++;
	}
	return tag - 1;
}


//==========================================================================
//
//
//
//==========================================================================

int CLevel::FindFreeTid() 
{
	int  tag;
	int n;
	bool ok;
	
	if (!m_Extended) return 0;
	tag = 1;
	ok = false;
	while (! ok)
	{
		ok=true;

		for (n = 0; n < NumLines(); n++)
		{
			int spec = GetLine(n)->line.type;
			if (spec>0 && spec<=255 && cgc->LineMap[spec])
			{
				for(int j=0;j<5;j++)
				{
					if (GetLine(n)->line.args[j] == tag)
					{
						wxString str = cgc->LineMap[spec]->args[j];
						if (!strnicmp(str.c_str(), "Tid:", 4))
						{
							ok = false;
							goto break2;
						}
					}
				}
			}
		}
		break2:

		for(n=0;n<NumThings();n++)
		{
			if (GetThing(n)->thingid==tag)
			{
				ok=false;
				break;
			}
		}
		tag++;
	}
	return tag - 1;
}

//==========================================================================
//
//
//
//==========================================================================

int CLevel::FindFreeLineId() 
{
	int  tag;
	int n;
	bool ok;
	
	if (!m_Extended) return 0;
	tag = 1;
	ok = false;
	while (! ok)
	{
		ok=true;

		for (n = 0; n < NumLines(); n++)
		{
			int spec = GetLine(n)->line.type;
			if (spec>0 && spec<=255 && cgc->LineMap[spec])
			{
				for(int j=0;j<5;j++)
				{
					if (GetLine(n)->line.args[j] == tag)
					{
						wxString str = cgc->LineMap[spec]->args[j];
						if (!strnicmp(str.c_str(), "LineID:", 7))
						{
							ok = false;
							goto break2;
						}
					}
				}
			}
		}
		break2:

		tag++;
	}
	return tag - 1;
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdJoinLD(wxCommandEvent & event)
{
	int i;
	int done=false;

	if (m_Mode!=modeVertexes) return;

	MakeBackup("Join Linedefs", true, true, true, false);
	if (m_Selection<0) m_Selection=0;
	for (i=NumVertices()-1;i>=0;i--)
	{
		if (checked[i])
		{
			DeleteVerticesJoinLineDefs(i);
			done=true;
		}
	}
	if (!done) DeleteVerticesJoinLineDefs(m_Selection);
	m_DrawWindow->Refresh();
	m_Selection=0;
	UpdateStatusBar();

}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdJoinLD2(wxCommandEvent & event)
{
	int sel[2];
	int selc=0;
	int done=false;

	if (m_Mode!=modeLineDefs) return;

	MakeBackup("Join Linedefs", true, true, true, false);

	/* change the LineDefs starts & ends */
	for (int l = 0; l < NumLines(); l++)
	{
		if (checked[l])
		{
			if (selc < 2)
			{
				sel[selc++] = l;
			}
			else
			{
				wxMessageBox("You must select exactly 2 lines", ZED_CAPTION);
				return;
			}
		}
	}
	if (selc != 2)
	{
		wxMessageBox("You must select exactly 2 lines", ZED_CAPTION);
		return;
	}
	CLine * ln1=GetLine(sel[0]);
	CLine * ln2=GetLine(sel[1]);

	if (strnicmp(ln1->sides[0].texLower, ln2->sides[0].texLower,8) ||
		strnicmp(ln1->sides[0].texNormal, ln2->sides[0].texNormal,8) ||
		strnicmp(ln1->sides[0].texUpper, ln2->sides[0].texUpper,8) ||
		ln1->sides[0].yoff != ln2->sides[0].yoff ||
		strnicmp(ln1->sides[1].texLower, ln2->sides[1].texLower,8) ||
		strnicmp(ln1->sides[1].texNormal, ln2->sides[1].texNormal,8) ||
		strnicmp(ln1->sides[1].texUpper, ln2->sides[1].texUpper,8) ||
		ln1->sides[1].yoff != ln2->sides[1].yoff)
	{
		wxMessageBox("Lines dont match.", ZED_CAPTION);
		return;
	}



	if (ln2->line.Start == ln1->line.End)
	{
		ln1->line.End = ln2->line.End;
		ln1->sides[1] = ln2->sides[1];
		DeleteOneLineDef(sel[1]);
	}
	else if (ln1->line.Start == ln2->line.End)
	{
		ln2->line.End = ln1->line.End;
		ln2->sides[1] = ln1->sides[1];
		DeleteOneLineDef(sel[0]);
	}
	else
	{
		wxMessageBox("Lines are not connected.", ZED_CAPTION);
		return;
	}
	m_DrawWindow->Refresh();
	m_Selection=0;
	checked.Clear();
	UpdateStatusBar();

}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::DeleteVerticesJoinLineDefs(int vert)
{
	int    lstart, lend, l;
	
	lstart = -1;
	lend = -1;
	for (l = 0; l < NumLines(); l++)
	{
		CLine * ln=GetLine(l);
		if (ln->line.Start == vert)
		{
			if (lstart == -1) lstart = l;
			else lstart = -2;
		}
		if (ln->line.End == vert)
		{
			if (lend == -1) lend = l;
			else lend = -2;
		}
	}
	if (lstart < 0 || lend < 0)
	{
		wxMessageBox(wxString::Format(
			"Cannot delete Vertex #%d and join the LineDefs.\n"
			"The Vertex must be the start of one LineDef and the end of another one", vert), ZED_CAPTION);
		return;
	}
	GetLine(lend)->line.End = GetLine(lstart)->line.End;
	DeleteOneLineDef(lstart);
	DeleteOneVertex(vert);

}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdMergeVertices(wxCommandEvent & event)
{
	TArray<int> vtlist(100);
	int cvtlist=0;
	int i;
	int done=false;
	int newsel=-1;

	if (m_Mode!=modeVertexes) return;

	MakeBackup("Merge Vertices", true, true, true, false);
	for (i=0;i<NumVertices();i++)
	{
		if (checked[i])
		{
			vtlist.Push(i);
			if (!done) newsel=i;
			done=true;
		}
	}
	if (done)
	{
		MergeVertices(&vtlist[0],vtlist.Size());
		m_DrawWindow->Refresh();
		m_Selection=newsel;
		UpdateStatusBar();
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::MergeVertices(int vtlist[],int cvtlist) 
{
	int    v, l;
	
	if (cvtlist<2)
	{
		wxMessageBox("You must select at least two vertices", ZED_CAPTION);
		return;
	}
	/* change the LineDefs starts & ends */
	for (l = 0; l < NumLines(); l++)
	{
		CLine * ln=GetLine(l);
		if (IsSelected( vtlist, ln->line.Start,cvtlist))
		{
			/* don't change a LineDef that has both ends on the same spot */
			if (!IsSelected(vtlist, ln->line.End, cvtlist) && ln->line.End != vtlist[0])
				ln->line.Start = vtlist[0];
		}
		else if (IsSelected( vtlist, ln->line.End, cvtlist))
		{
			/* idem */
			if (ln->line.Start != vtlist[0])
				ln->line.End = vtlist[0];
		}
	}
	for (v=cvtlist-1;v>0;v--) DeleteOneVertex(vtlist[v]);
	UncheckAll();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdSplitSector(wxCommandEvent & event)
{
	int i;
	int done=false;
	int ldlist[2];
	int cldlist=0;

	if (m_Mode!=modeVertexes) return;

	MakeBackup("Split Sector", false, true, true, false);
	for (i=0;i<(int)NumVertices();i++)
	{
		if (checked[i])
		{
			if (cldlist<2) ldlist[cldlist++]=i;
			else 
			{
				wxMessageBox("You must select exactly 2 Vertexes for this function", ZED_CAPTION);
				return;
			}
		}
	}
	if (cldlist<2) wxMessageBox("You must select exactly 2 Vertexes for this function", ZED_CAPTION);
	else SplitSector(ldlist[0],ldlist[1]);
	m_DrawWindow->Refresh();
	m_Selection=NumSectors()-1;
	UpdateStatusBar();
}


///////////////////////////////////////////////////////////////
void CLevel::SplitSector(int v1,int v2) 
{
	TArray<CLine *>llist;

	int    s3,s2,s, l;
	int curv;
	CSideDef * sd;

	CVertex * vertex1 = GetVertex(v1);
	CVertex * vertex2 = GetVertex(v2);
	/* check if there is a Sector between the two Vertices (in the middle) */
	s = GetCurSector(int(vertex1->X()),int(vertex1->Y()),int(vertex2->X()),int(vertex2->Y()));
	if (s < 0)
	{
		wxMessageBox(wxString::Format("There is no Sector between Vertex #%d and Vertex #%d", v1, v2), ZED_CAPTION);
		return;
	}
	/* check if there is a closed path from vertex1 to vertex2, along the edge of the Sector s */
	curv = v1;
	while (curv != v2)
	{
		for (l = 0; l < NumLines(); l++)
		{
			CLine * ln = GetLine(l);
			sd = &ln->sides[0];
			if (sd && sd->sector == s && ln->line.Start == curv)
			{
				curv = ln->line.End;
				llist.Push(ln);
				break;
			}
			sd = &ln->sides[1];
			if (sd && sd->sector == s && ln->line.End == curv)
			{
				curv = ln->line.Start;
				llist.Push(ln);
				break;
			}
		}
		if (l >= NumLines())
		{
			wxString msg1 = wxString::Format("Cannot find a closed path from Vertex #%d to Vertex #%d", v1, v2);
			if (curv == v1) 
				msg1+=wxString::Format("\nThere is no SideDef starting from Vertex #%d on Sector #%d", msg1,v1, s);
			else
				msg1+=wxString::Format("\nCheck if Sector #%d is closed (cannot go past Vertex #%d)", msg1,s, curv);
			wxMessageBox(msg1, ZED_CAPTION);
			return;
		}
		if (curv == v1)
		{
			wxMessageBox(wxString::Format("Vertex #%d is not on the same Sector (#%d) as Vertex #%d", v2, s, v1), ZED_CAPTION);
			return;
		}
	}
	/* now, the list of LineDefs for the new Sector is in llist */
	
	/* add the new Sector, LineDef and SideDefs */
	s2=InsertSector(GetSector(s));
	l=InsertLineDef(NULL);
	CLine * ln=GetLine(l);
	ln->line.Start = v1;
	ln->line.End = v2;

	int bit = 2;
	if (m_TextMap) bit = cgc->CheckTextMapThingFlag("twosided");
	ln->line.Flags.SetBit(bit);

	ln->sides[0].sector=s;
	ln->sides[1].sector=s2;
	
	/* bind all LineDefs in llist to the new Sector */
	for(l=0;l<(int)llist.Size();l++)
	{
		if (llist[l]->sides[0].sector==s) llist[l]->sides[0].sector=s2;
		if (llist[l]->sides[1].sector==s) llist[l]->sides[1].sector=s2;
	}
	
	/* second check... useful for Sectors within Sectors */

	for (l = 0; l < NumLines(); l++)
	{
		CLine * ln=GetLine(l);

		sd = &ln->sides[0];
		if (sd && sd->sector == s)
		{
			s3 = GetOppositeSector(l, true);
			if (s3 == s2) sd->sector=s2;
		}
		sd = &ln->sides[1];
		if (sd && sd->sector == s)
		{
			s3 = GetOppositeSector(l, false);
			if (s3 == s2) sd->sector=s2;
		}
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdSplitLinedefs(wxCommandEvent & event)
{
	int i;
	int done=false;

	if (m_Mode!=modeLineDefs) return;

	MakeBackup("Split Linedefs", true, true, true, false);
	for (i=0;i<NumLines();i++)
	{
		if (checked[i])
		{
			SplitLineDefs(i);
			done=true;
		}
	}
	if (!done) SplitLineDefs(m_Selection);
	m_DrawWindow->Refresh();
	UncheckAll();
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::SplitLineDefs( int ld)
{
	int nv,nl;
	
	CVertex * VStart, * VEnd;
	
	VStart=StVt(ld);
	VEnd=EnVt(ld);
	
	nv=InsertVertex((VStart->X()+VEnd->X())/2,(VStart->Y()+VEnd->Y())/2);
	nl=InsertLineDef(GetLine(ld));
	GetLine(ld)->line.End=nv;
	GetLine(nl)->line.Start=nv;
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdSplitLinedefsAndSector(wxCommandEvent & event)
{
	int i;
	int done=false;
	int ldlist[2];
	int cldlist=0;

	if (m_Mode!=modeLineDefs) return;

	MakeBackup("Split Linedefs and Sector", true, true, true, false);
	for (i=0;i<NumLines();i++)
	{
		if (checked[i])
		{
			if (cldlist<2) ldlist[cldlist++]=i;
			else 
			{
				wxMessageBox("You must select exactly 2 Linedefs for this function", ZED_CAPTION);
				return;
			}
		}
	}
	if (cldlist<2) wxMessageBox("You must select exactly 2 Linedefs for this function", ZED_CAPTION);
	else SplitLineDefsAndSector(ldlist[0],ldlist[1]);
	m_DrawWindow->Refresh();
	UncheckAll();
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::SplitLineDefsAndSector(int L1,int L2)
{
	int se1,se2,se3,se4;
	
	/* check if the two LineDefs are adjacent to the same Sector */
	se1 = FrontSecNo(L1);
	se2 = BackSecNo(L1);
	se3 = FrontSecNo(L2);
	se4 = BackSecNo(L2);
	
	if ((se1==-1 || (se1 != se3 && se1 != se4)) && (se2 == -1 || (se2 != se3 && se2 != se4)))
	{
		wxMessageBox(wxString::Format("LineDefs #%d and #%d are not adjacent to the same Sector", L1,L2), ZED_CAPTION);
		return;
	}
	/* split the two LineDefs and create two new Vertices */
	SplitLineDefs(L1);
	SplitLineDefs(L2);
	/* split the Sector and create a LineDef between the two Vertices */
	SplitSector(NumVertices()-1,NumVertices()-2);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnJoinsectors(wxCommandEvent & event)
{
	if (Selection.Size()>1 && m_Mode==modeSectors)
	{
		MakeBackup("Join Sectors", false, true, true, false);
		for(int i=0;i<NumLines();i++)
		{
			CLine * ln = GetLine(i);
			for(unsigned j=1;j<Selection.Size();j++)
			{
				if (ln->sides[0].sector==Selection[j]) ln->sides[0].sector=Selection[0];
				if (ln->sides[1].sector==Selection[j]) ln->sides[1].sector=Selection[0];
			}
		}
		m_Selection=Selection[0];
		for(int i=NumSectors()-1;i>=0;i--)
		{
			if (checked[i] && i!=m_Selection)
			{
				DeleteOneSector(i);
				if (i<m_Selection) m_Selection--;
			}
		}
		m_changed=true;
		m_NeedNodeBuild=true;
		m_DrawWindow->Refresh();
		UpdateStatusBar();
	}
}
//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdDeleteLinedefsJoinSectors(wxCommandEvent & event)
{
	int i;
	int done=false;

	if (m_Mode!=modeLineDefs) return;

	MakeBackup("Join Sectors", false, true, true, false);
	if (m_Selection<0) m_Selection=0;
	for (i=NumLines()-1;i>=0;i--)
	{
		if (checked[i])
		{
			DeleteLineDefsJoinSectors(i);
			done=true;
		}
	}
	if (!done) DeleteLineDefsJoinSectors(m_Selection);
	m_DrawWindow->Refresh();
	UncheckAll();
	m_Selection=0;
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::DeleteLineDefsJoinSectors(int ld) 
{
	int s1 = FrontSecNo(ld);
	int s2 = BackSecNo(ld);

   /* first, do the tests for all LineDefs */

	if (s1==-1 || s2==-1)
	{
		wxMessageBox(wxString::Format("LineDef #%d has only one side", ld), ZED_CAPTION);
		return;
	}

	/* then join the Sectors and delete the LineDefs */
	for (int n = 0; n < NumLines(); n++)
	{
		CLine * ln=GetLine(n);
		if (ln->sides[0].sector==s2) ln->sides[0].sector=s1;
		if (ln->sides[1].sector==s2) ln->sides[1].sector=s1;
	}
	if (s2!=s1) DeleteOneSector(s2);
	DeleteOneLineDef(ld);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdFlipLinedefs(wxCommandEvent & event)
{
	int i;
	int done=false;

	if (m_Mode!=modeLineDefs) return;

	MakeBackup("Flip Linedefs", false, true, false, false);
	if (m_Selection<0) m_Selection=0;
	for (i=NumLines()-1;i>=0;i--)
	{
		if (checked[i])
		{
			FlipLineDefs(i,true);
			done=true;
		}
	}
	if (!done) FlipLineDefs(m_Selection,true);
	m_DrawWindow->Refresh();
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdSwapSidedefs(wxCommandEvent & event)
{
	int i;
	int done=false;

	if (m_Mode!=modeLineDefs) return;

	MakeBackup("Swap Sidedefs", false, true, false, false);
	if (m_Selection<0) m_Selection=0;
	for (i=NumLines()-1;i>=0;i--)
	{
		if (checked[i])
		{
			FlipLineDefs(i,false);
			done=true;
		}
	}
	if (!done) FlipLineDefs(m_Selection,false);
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::FlipLineDefs(int lineno, bool swapvertices)
{
	int    tmp;
	CSideDef tmps;
	CLine * line=GetLine(lineno);
	
	if (swapvertices)
	{
		/* swap starting and ending Vertices */
		tmp = line->line.End;
		line->line.End=line->line.Start;
		line->line.Start=tmp;
	}
	/* swap first and second SideDefs */
	tmps=line->sides[0];
	line->sides[0]=line->sides[1];
	line->sides[1]=tmps;
	m_NeedNodeBuild=true;
	m_changed = true;
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdDel2nd(wxCommandEvent & event)
{
	int i;
	int done=false;

	if (m_Mode!=modeLineDefs) return;

	MakeBackup("Delete Second Sidedef", false, true, false, false);
	if (m_Selection<0) m_Selection=0;
	for (i=NumLines()-1;i>=0;i--)
	{
		if (checked[i])
		{
			DeleteSecondSideDef(i);
			done=true;
		}
	}
	if (!done) DeleteSecondSideDef(m_Selection);
	m_DrawWindow->Refresh();
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::DeleteSecondSideDef(int ln)
{
	CLine * ld=GetLine(ln);
	int sector=ld->sides[1].sector;

	ld->sides[1].sector=-1;

	int bit = 2;
	if (m_TextMap) bit = cgc->CheckTextMapThingFlag("twosided");
	ld->line.Flags.ClearBit(bit);

	bit = 0;
	if (m_TextMap) bit = cgc->CheckTextMapThingFlag("blocking");
	ld->line.Flags.SetBit(bit);

	if (sector!=-1)
	{
		for(int i=0;i<NumLines();i++)
		{
			if (FrontSecNo(i)==sector) return;
			if (BackSecNo(i)==sector) return;
		}
	}
	DeleteOneSector(sector);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdAlignX(int type_sd,int type_tex,int type_off)
{
	TArray<int> sdlist;
	CSideDef * psd;
	unsigned int i;


	if (m_Mode!=modeLineDefs) return;

	MakeBackup("Align textures X", false, true, false, false);
	for(i=0;i<Selection.Size();i++)
	{
		CLine * ln = GetLine(Selection[i]);
		psd=&ln->sides[type_sd];
		if (psd->sector==-1) 
		{
			wxMessageBox(wxString::Format("One of the selected lines doesn't have a %s sidedef",
				type_sd? "second":"first"),	ZED_CAPTION);
			return;
		}
		sdlist.Push(Selection[i]);
	}
	if (sdlist.Size()>1) 
	{
		AlignTexturesX(&sdlist[0],sdlist.Size(),type_sd,type_tex,type_off);
	}
	else 
	{
		wxMessageBox("You have to select at least two lines to perform this function", ZED_CAPTION);
	}
	return;
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnAlignSide1Tex(wxCommandEvent & event)
{
	OnCmdAlignX(0, 1, 0);
}

void CLevel::OnAlignSide1TexOfs(wxCommandEvent & event)
{
	OnCmdAlignX(0, 1, 1);
}

void CLevel::OnAlignSide1(wxCommandEvent & event)
{
	OnCmdAlignX(0, 0, 0);
}

void CLevel::OnAlignSide1Ofs(wxCommandEvent & event)
{
	OnCmdAlignX(0, 0, 1);
}

void CLevel::OnAlignSide2Tex(wxCommandEvent & event)
{
	OnCmdAlignX(1, 1, 0);
}

void CLevel::OnAlignSide2TexOfs(wxCommandEvent & event)
{
	OnCmdAlignX(1, 1, 1);
}

void CLevel::OnAlignSide2(wxCommandEvent & event)
{
	OnCmdAlignX(1, 0, 0);
}

void CLevel::OnAlignSide2Ofs(wxCommandEvent & event)
{
	OnCmdAlignX(0, 0, 1);
}



//==========================================================================
//
//
//
//==========================================================================

void CLevel::AlignTexturesX(int * sdlist,int csdlist,int type_sd,int type_tex,int type_off)
{
	SIZE sz;
	char texname[9];       	/* FIRST texture name used in the highlited objects */
	int  ldef=0;			/* linedef number */
	CSideDef * sd1;     /* current SideDef in *sdlist */
	int  vert1=-1, vert2=-1;/* vertex 1 and 2 for the linedef under scrutiny */
	int  xoffset=0;			/* xoffset accumulator */
	int  useroffset=0;      /* user input offset for first input */
	int  texlength=0;     	/* the length of texture to format to */
	int  length;			/* length of linedef under scrutiny */
	int  i;


	ValidateTextureManager();
	sd1 = &GetLine(sdlist[0])->sides[type_sd];


	/* get texture name of the SideDef in the *sdlist) */
	strncpy( texname,sd1->texNormal, 8);
	if (texname[0] == '-') strncpy( texname,sd1->texLower, 8);
	if (texname[0] == '-') strncpy( texname,sd1->texUpper, 8);

	/* test if there is a texture there */
	if (texname[0] == '-')
	{
		wxMessageBox(wxString::Format("No texture for Linedef %d, side %d.",ldef, type_sd+1), ZED_CAPTION);
		return;
	}

	if (!texman->GetWallTextureSize( &sz, texname))
	{
		wxMessageBox(wxString::Format("Invalid texture for Linedef %d, side %d.",ldef, type_sd+1), ZED_CAPTION);
		return;
	}
	texlength=sz.cx;

	/* get initial offset to use (if requrired) */
	if(type_off == 1)   
	{
		useroffset = wxGetNumberFromUser("Enter an initial texture offset", "", 
										"Align textures (X-axis)", 0, -10000, 10000, m_DrawWindow);
		if (useroffset==-1) return;
	}


	for(i=csdlist-1;i>=0;i--)
	{
		CLine * ln = GetLine(sdlist[i]);

		sd1 = &ln->sides[type_sd];
		ldef = sdlist[i];

		if(type_tex == 1) /* do we test for same textures for the SideDef in question?? */
		{
			if (strnicmp(sd1->texNormal, texname,8))
			{
				wxMessageBox(wxString::Format("Texture mismatch for LineDef #%d.", ldef), ZED_CAPTION);
				return;
			}
		}

		if (type_sd==0) vert1 = ln->line.Start;
		else vert1 = ln->line.End;
		/* test for linedef highlight continuity */
		if (vert1 != vert2 && vert2 != -1)
		{
			wxMessageBox(wxString::Format("LineDef #%d is not contiguous with the previous LineDef, please reselect.", ldef), ZED_CAPTION);
			return;
		}

		if (type_sd==0) vert2 = ln->line.End;
		else vert2 = ln->line.Start;

		/* calculate length of LineDef */
		CVertex * v1 = GetVertex(vert1);
		CVertex * v2 = GetVertex(vert2);
		length = int(ComputeDist( v2->X() - v1->X(), v2->Y() - v1->Y())+0.5);

		sd1->xoff = useroffset;
		useroffset += length;
		useroffset %= texlength;	/* remove multiples of texlength from xoffset */
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnAlignY(wxCommandEvent & event)
{
	int h, refh;

	if (m_Mode!=modeLineDefs) return;

	/* get the reference height from the first SideDef */
	refh = GetTextureRefHeight(GetLine(Selection[0]), 0);

	if (Selection.Size()<2)
	{
		wxMessageBox("You have to select at least two lines to perform this function", ZED_CAPTION);
		return;
	}

	MakeBackup("Align textures Y", false, true, false, false);
	/* adjust Y offset in all other SideDefs */
	for(unsigned i=0;i<Selection.Size();i++) for(int j=0;j<2;j++) 
	{
		CLine * ln = GetLine(Selection[i]);
		if (ln->sides[j].sector!=-1)
		{
			h = GetTextureRefHeight(ln, j);
			ln->sides[j].yoff = (refh - h) % 128;
		}
	}
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

int CLevel::GetTextureRefHeight(CLine * line, int side)
{
	CSideDef * sidedef = &line->sides[side];
	CSideDef * otherside = &line->sides[1-side];
	int l, sector;
	
	/* get the Sector number */
	sector = sidedef->sector;
	CSector * sec=GetSector(sector);
	/* if the upper texture is displayed, then the reference is taken from the other Sector */
	l=otherside->sector;
	if (l!=-1)
	{
		CSector * sec2 = GetSector(l);
		if (sec2->ceilh<sec->ceilh && sec2->ceilh>sec->floorh)
		{
			sector = l;
			sec = sec2;
		}
	}
	/* return the altitude of the ceiling */
	if (sector >= 0) return sec->ceilh; /* textures are drawn from the ceiling down */
	else return 0; /* yuck! */
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdMakeDoor(wxCommandEvent & event)
{
	int i;
	int done=false;

	if (m_Mode!=modeSectors) return;

	MakeBackup("Make Door", false, true, true, false);
	if (m_Selection<0) m_Selection=0;
	for (i=0;i<NumSectors();i++)
	{
		if (checked[i])
		{
			MakeDoor(i);
			done=true;
		}
	}
	if (!done) MakeDoor(m_Selection);
	m_DrawWindow->Refresh();
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::MakeDoor(int sectorno) 
{
	CSideDef *Sd1,*Sd2;

	TArray<int> ldok;
	TArray<int> ldflip;
	TArray<int> ld1s;

	int n;
	int s;
	
	s = 0;
	/* build lists of LineDefs that border the Sector */
	for (n = 0; n < NumLines(); n++)
	{
		int sec1 = FrontSecNo(n);
		int sec2 = BackSecNo(n);
		if (sec1!=-1 && sec2!=-1)
		{
			if (sec2 == sectorno) ldok.Push(n); 
			if (sec1 == sectorno) ldflip.Push(n); 
		}
		else if (sec1!=-1 && sec2==-1)
		{
			if (sec1 == sectorno) ld1s.Push(n);
		}
	}
	/* a normal door has two sides... */
	if (ldok.Size()+ldflip.Size() < 2)
	{
		wxMessageBox("The door must be connected to two other Sectors.", ZED_CAPTION);
		return;
	}
	/* flip the LineDefs that have the wrong orientation */
	for (n=0;n<(int)ldflip.Size();n++) 
	{
		FlipLineDefs(n,true);
		ldok.Push(n);
	}
	for(n=0;n<(int)ldok.Size();n++)
	{
		CLine * ln = GetLine(ldok[n]);
		if (m_TextMap)
		{
			int bit;
			bit = cgc->CheckTextMapThingFlag("twosided");
			ln->line.Flags.SetBit(bit);
			if (!m_Extended) 
			{
				ln->line.type = 1;
			}
			else
			{
				ln->line.type = 11;
				ln->line.args[0]=0;
				ln->line.args[1]=16;
				ln->line.args[2]=150;
				ln->line.args[3]=0;
				ln->line.args[4]=0;
				ln->line.Flags.Clear();
				bit = cgc->CheckTextMapThingFlag("playeruse");
				ln->line.Flags.SetBit(bit);
				bit = cgc->CheckTextMapThingFlag("repeatspecial");
				ln->line.Flags.SetBit(bit);
			}
		}
		else if (!m_Extended) 
		{
			ln->line.type = 1;
			ln->line.Flags.SetShort(0x04);
		}
		else
		{
			ln->line.type = 11;
			ln->line.args[0]=0;
			ln->line.args[1]=16;
			ln->line.args[2]=150;
			ln->line.args[3]=0;
			ln->line.args[4]=0;
			ln->line.Flags.SetShort(0x04|0x200|0x400);
		}
		Sd1=&ln->sides[0];
		Sd2=&ln->sides[1];
		/* adjust the textures for the SideDefs */

		if (strncmp(Sd1->texNormal, "-", 8))
		{
			if (!strncmp(Sd1->texUpper, "-", 8)) strncpy(Sd1->texUpper,Sd1->texNormal, 8);
			strncpy(Sd1->texNormal, "-", 8);
		}
		if (!strncmp(Sd1->texUpper, "-", 8)) strncpy(Sd1->texUpper, DEF_DOORTEX, 8);
		strncpy(Sd2->texNormal, "-", 8);
	}
	for(n=0;n<(int)ld1s.Size();n++)
	{
		CLine * ln = GetLine(ld1s[n]);
		/* give the "door side" flags to the LineDef */
		if (!m_TextMap)
		{
			ln->line.Flags.SetShort(0x11);
		}
		else
		{
			ln->line.Flags.Clear();
			int bit = cgc->CheckTextMapThingFlag("blocking");
			ln->line.Flags.SetBit(bit);
			bit = cgc->CheckTextMapThingFlag("dontpegbottom");
			ln->line.Flags.SetBit(bit);
		}
		Sd1=&ln->sides[0];
		/* adjust the textures for the SideDef */
		if (!strncmp(Sd1->texNormal, "-", 8)) strncpy(Sd1->texNormal, DEF_DOORTRAK, 8);
		strncpy(Sd1->texUpper, "-", 8);
		strncpy(Sd1->texLower, "-", 8);
	}
	/* adjust the ceiling height */
	CSector * sector = GetSector(sectorno);
	sector->ceilh=sector->floorh;

}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdMakeLift(wxCommandEvent & event)
{
	int i;
	int done=false;

	if (m_Mode!=modeSectors) return;

	MakeBackup("Make Lift", false, true, true, false);
	if (m_Selection<0) m_Selection=0;
	for (i=0;i<NumSectors();i++)
	{
		if (checked[i])
		{
			MakeLift(i);
			done=true;
		}
	}
	if (!done) MakeLift(m_Selection);
	m_DrawWindow->Refresh();
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::MakeLift(int secno)
{
	CSideDef *Sd1,*Sd2;

	TArray<int> ldok;
	TArray<int> ldflip;
	TArray<int> ld1s;
	TArray<int> sect;

	int    n, tag;
	int    minh, maxh;

	/* build lists of LineDefs that border the Sector */
	for (n = 0; n < NumLines(); n++)
	{
		CLine * ln = GetLine(n);
		Sd1=&ln->sides[0];
		Sd2=&ln->sides[1];
		
		if (Sd1->sector==Sd2->sector) continue;

		if (Sd1->sector!=-1 && Sd2->sector!=-1)
		{
			if (Sd2->sector==secno)
			{
				ldok.Push(n);
				if (Sd1->sector!=secno && !IsSelected(&sect[0],Sd1->sector,sect.Size()))
				{
					sect.Push(Sd1->sector);
				}
			}
			if (Sd1->sector == secno)
			{
				ldflip.Push(n);
				if (Sd2->sector!=secno && !IsSelected(&sect[0],Sd2->sector,sect.Size()))
				{
					sect.Push(Sd2->sector);
				}
			}
		}
		else if (Sd1->sector!=-1 && Sd2->sector==-1)
		{
			if (Sd1->sector==secno) ld1s.Push(n);
		}
	}
	/* there must be a way to go on the lift... */
	if (sect.Size() == 0)
	{
		wxMessageBox("The lift must be connected to at least one other Sector.", ZED_CAPTION);
		return;
	}
	/* flip the LineDefs that have the wrong orientation */
	for (n=0;n<(int)ldflip.Size();n++) 
	{
		FlipLineDefs(ldflip[n],true);
		ldok.Push(ldflip[n]);
	}
	
	/* find a free tag number */
	tag = FindFreeTag();
	
	/* find the minimum and maximum altitudes */
	minh = 32767;
	maxh = -32767;

	for(n=0;n<(int)sect.Size();n++)
	{
		CSector * s = GetSector(sect[n]);
		if (s->floorh<minh) minh=s->floorh;
		if (s->floorh>maxh) maxh=s->floorh;
	}

	CSector * sector = GetSector(secno);
	if (sector->floorh < maxh) sector->floorh = maxh;
	if (sector->ceilh < maxh + 56) sector->ceilh = maxh + 56;
	sector->tag = tag;
	
	/* change the LineDefs and SideDefs */
	for(n=0;n<(int)ldok.Size();n++)
	{
		/* give the "lower lift" type and flags to the LineDef */

		CLine * ln = GetLine(ldok[n]);
		if (m_TextMap)
		{
			int bit;
			bit = cgc->CheckTextMapThingFlag("twosided");
			ln->line.Flags.SetBit(bit);
			if (!m_Extended) 
			{
				ln->line.type = 62;
			}
			else
			{
				ln->line.type = 206;
				ln->line.args[0] = tag;
				ln->line.args[1] = 16;
				ln->line.args[2] = 100;
				ln->line.args[3] = 0;
				ln->line.args[4] = 0;
				ln->line.Flags.Clear();
				bit = cgc->CheckTextMapThingFlag("playeruse");
				ln->line.Flags.SetBit(bit);
				bit = cgc->CheckTextMapThingFlag("repeatspecial");
				ln->line.Flags.SetBit(bit);
			}
		}
		else if (!m_Extended)
		{
			ln->line.type = 62; /* lower lift (switch) */
			ln->line.tag = tag;
			ln->line.Flags.SetShort(0x04);
		}
		else
		{
			ln->line.type = 206;
			ln->line.args[0] = tag;
			ln->line.args[1] = 16;
			ln->line.args[2] = 100;
			ln->line.args[3] = 0;
			ln->line.args[4] = 0;
			ln->line.Flags.SetShort(0x04|0x200|0x400);
		}
		Sd1 = &ln->sides[0]; /* outside */
		Sd2 = &ln->sides[1]; /* inside */
		/* adjust the textures for the SideDef visible from the outside */
		if (strncmp( Sd1->texNormal, "-", 8))
		{
			if (!strncmp(Sd1->texLower, "-", 8)) strncpy(Sd1->texLower,Sd1->texNormal, 8);
			strncpy( Sd1->texNormal, "-", 8);
		}
		if (!strncmp(Sd1->texLower, "-", 8)) strncpy(Sd1->texLower, DEF_LIFTTEX, 8);
		/* adjust the textures for the SideDef visible from the lift */
		//strncpy(Sd2->texNormal, "-", 8);

		CSector * Sec=GetSector(Sd1->sector);
		if (Sec->floorh > minh)
		{
			if (strncmp(Sd2->texNormal, "-", 8))
			{
				if (!strncmp(Sd2->texLower, "-", 8)) strncpy(Sd2->texLower,Sd2->texNormal, 8);
				//strncpy(Sd2->texNormal, "-", 8);
			}
			if (!strncmp(Sd2->texLower, "-", 8)) strncpy(Sd2->texLower, DEF_LIFTTEX, 8);
		}
		else
		{
			strncpy(Sd2->texLower, "-", 8);
		}
		strncpy(Sd2->texNormal, "-", 8);

		/* if the ceiling of the Sector is lower than that of the lift */
		if (Sec->ceilh < sector->ceilh)
		{
			if (strncmp(Sd2->texUpper, "-", 8)) strncpy(Sd2->texUpper, DEF_WALLTEX, 8);
		}
		/* if the floor of the Sector is above the lift */
		if (Sec->floorh >= sector->floorh)
		{
			CLine * ln = GetLine(ldok[n]);
			if (!m_Extended) ln->line.type = 88; /* lower lift (walk through) */
			else 
			{
				if (!m_TextMap) ln->line.Flags.SetShort(0x04|0x200);
				else
				{
					ln->line.Flags.Clear();
					int bit = cgc->CheckTextMapThingFlag("playercross");
					ln->line.Flags.SetBit(bit);
					bit = cgc->CheckTextMapThingFlag("repeatspecial");
					ln->line.Flags.SetBit(bit);
				}
			}

			/* flip it, just for fun */
			FlipLineDefs(n,true);
		}
		/* done with this LineDef */
	}
	
	for(n=0;n<(int)ld1s.Size();n++)
	{
		CLine * ln = GetLine(ld1s[n]);
		/* these are the lift walls (one-sided) */
		if (!m_TextMap)
		{
			ln->line.Flags.SetShort(0x01);
		}
		else
		{
			ln->line.Flags.Clear();
			int bit = cgc->CheckTextMapThingFlag("blocking");
			ln->line.Flags.SetBit(bit);
		}
		Sd1=&ln->sides[0];
		/* adjust the textures for the SideDef */
		if (!strncmp(Sd1->texNormal, "-", 8)) strncpy(Sd1->texNormal,DEF_WALLTEX, 8);
		strncpy(Sd1->texUpper, "-", 8);
		strncpy(Sd1->texLower, "-", 8);
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdDistFloor(wxCommandEvent & event)
{
	int   floor1h, floor2h;
	unsigned n;
	
	if (m_Mode!=modeSectors) return;

	MakeBackup("Distribute floor heights", false, false, true, false);
	if (Selection.Size())
	{
		floor1h = GetSector(Selection[0])->floorh;
		floor2h = GetSector(Selection.Last())->floorh;

		for (n=0;n<Selection.Size();n++)
		{
			GetSector(Selection[n])->floorh=floor1h+int(n)*(floor2h-floor1h)/int(Selection.Size()-1);
		}
	}
	else wxMessageBox("No Sectors selected", ZED_CAPTION);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdDistCeil(wxCommandEvent & event)
{
	int   ceil1h, ceil2h;
	unsigned n;

	if (m_Mode!=modeSectors) return;

	MakeBackup("Distribute ceiling heights", false, false, true, false);
	if (Selection.Size())
	{
		ceil1h = GetSector(Selection[0])->ceilh;
		ceil2h = GetSector(Selection.Last())->ceilh;

		for (n=0;n<Selection.Size();n++)
		{
			GetSector(Selection[n])->ceilh=ceil1h+int(n)*(ceil2h-ceil1h)/int(Selection.Size()-1);
		}
	}
	else wxMessageBox("No Sectors selected", ZED_CAPTION);
}	
	




//==========================================================================
/*
   rotate and scale a group of objects around the center of gravity
*/
//==========================================================================

void CLevel::OnCmdRotate(wxCommandEvent & event)
{
	wxRealPoint rot;

	if (GetRotation(m_DrawWindow, &rot))
	{
		RotateAndScaleObjects(rot.x, rot.y, rot.y);
	}
}

void CLevel::OnRotateselection90L(wxCommandEvent & event)
{
	if (!(m_Mode&modeEdit)) return;

	RotateAndScaleObjects(90, 1, 1);
}

void CLevel::OnRotateselection90R(wxCommandEvent & event)
{
	if (!(m_Mode&modeEdit)) return;

	RotateAndScaleObjects(270, 1, 1);
}

void CLevel::OnRotateselection180(wxCommandEvent & event)
{
	if (!(m_Mode&modeEdit)) return;

	RotateAndScaleObjects(180, 1, 1);
}

void CLevel::OnMirrorselectionhorizontally(wxCommandEvent & event)
{
	if (!(m_Mode&modeEdit)) return;

	RotateAndScaleObjects(0, -1, 1);
}

void CLevel::OnMirrorselectionvertically(wxCommandEvent & event)
{
	if (!(m_Mode&modeEdit)) return;

	RotateAndScaleObjects(0, 1, -1);
}


//==========================================================================
//
//
//
//==========================================================================

static void RotateAndScaleCoords( double *x, double *y, double angle, double scalex, double scaley)
{
	double r, theta;

	r = hypot( *x, *y);
	theta = atan2( *y, *x);
	*x = r * scalex * cos( theta + angle);
	*y = r * scaley * sin( theta + angle);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::RotateAndScaleObjects(double angle, double scalex, double scaley)
{
	int    n;
	double dx, dy;
	double centerx, centery;
	double accx, accy, num;
	const char * action;


	angle=angle*3.1415926535897932384626433832795/180;
	PushChecked();
	switch (m_Mode)
	{
	case modeThings:
		action = angle!=0? "Rotate things" : scalex*scaley!=-1? "Scale things" : "Mirror things";
		MakeBackup(action, false, false, false, true);
		accx = 0;
		accy = 0;
		num = 0L;
		for (n=0;n<NumThings();n++) if (checked[n])
		{
			CThing * t = GetThing(n);
			accx += t->X();
			accy += t->Y();
			num++;
		}
		centerx = (accx + num / 2L) / num;
		centery = (accy + num / 2L) / num;
		for (n=0;n<NumThings();n++) if (checked[n])
		{
			CThing * t = GetThing(n);
			dx = t->X() - centerx;
			dy = t->Y() - centery;
			RotateAndScaleCoords( &dx, &dy, angle, scalex, scaley);
			if (!m_TextMap) dx+=0.5, dy+=0.5;
			t->SetX(centerx + dx, m_TextMap);
			t->SetY(centery + dy, m_TextMap);
		}
		break;

	case modeSectors:
		CheckLineDefsFromSectors();
	case modeLineDefs:
		CheckVertexesFromLineDefs();
	case modeVertexes:
		action = angle!=0? "Rotate vertices" : scalex*scaley!=-1? "Scale vertices" : "Mirror vertices";
		MakeBackup(action, true, false, false, false);
		accx = 0L;
		accy = 0L;
		num = 0L;
		for (n=0;n<(int)NumVertices();n++) if(checked[n])
		{
			CVertex * v = GetVertex(n);
			accx += v->X();
			accy += v->Y();
			num++;
		}
		centerx = (accx + num / 2L) / num;
		centery = (accy + num / 2L) / num;
		for (n=0;n<(int)NumVertices();n++) if(checked[n])
		{
			CVertex * v = GetVertex(n);
			dx = v->X() - centerx;
			dy = v->Y() - centery;
			RotateAndScaleCoords( &dx, &dy, angle, scalex, scaley);
			if (!m_TextMap) dx+=0.5, dy+=0.5;
			v->SetX(v->X() + (centerx + dx ), m_TextMap);
			v->SetY(v->Y() + (centery + dy ), m_TextMap);
		}
		break;
	}
	PopChecked();
	m_DrawWindow->Refresh();
	UpdateStatusBar();
}


