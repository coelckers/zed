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
// Drawing/Screen update code
//


#include "StdAfx.h"
#include "Level.h"
#include "ZEd.h"
#include "GameConfig.h"


//==========================================================================
//
//
//
//==========================================================================

void CLevel::MapLine(wxDC & DC, wxColour col, CVertex * v1, CVertex * v2, int pensize)
{
	DC.SetPen(*GetPen(col, pensize));
	DC.DrawLine(SX(v1->X()), SY(v1->Y()), SX(v2->X()), SY(v2->Y()));
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::MapLine(wxDC & DC, wxColour col, wxPoint * v1,wxPoint * v2, int pensize)
{
	DC.SetPen(*GetPen(col, pensize));
	DC.DrawLine(SX(v1->x), SY(v1->y), SX(v2->x), SY(v2->y));
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::MapLine(wxDC & DC, wxColour col, double x1,double y1,double x2,double y2, int pensize)
{
	DC.SetPen(*GetPen(col, pensize));
	DC.DrawLine(SX(x1), SY(y1), SX(x2), SY(y2));
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::DrawLinedef(wxDC & DC, wxColour col, CVertex * v1,CVertex * v2, int pensize)
{
	wxRealPoint pt[2];

	MapLine(DC, col, v1, v2, pensize);
	if (m_showSideMarkers)
	{
		pt[0].x= (v1->X() + v2->X()) /2;
		pt[0].y= (v1->Y() + v2->Y()) /2;


		pt[1].x=  v2->X() - v1->Y();
		pt[1].y=-(v2->X() - v1->Y());
		double f = (0.6*m_ZoomFactor/hypot(pt[1].x,pt[1].y));
		pt[1].x = pt[0].x + f * pt[1].x;
		pt[1].y = pt[0].y + f * pt[1].y;
		MapLine(DC, col, pt[0].x, pt[0].y, pt[1].x, pt[1].y, pensize);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::MapEllipse(wxDC & DC, wxColour col, double x1,double y1,double x2,double y2)
{
	DC.SetPen(*GetPen(col));
	DC.DrawEllipse(SX(x1),SY(y1),SX(x2)-SX(x1),SY(y2)-SY(y1));
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::MapRect(wxDC & DC, wxColour col, double x1,double y1,double x2,double y2)
{
	DC.SetPen(*GetPen(col));
	return DC.DrawRectangle(SX(x1),SY(y1),SX(x2)-SX(x1),SY(y2)-SY(y1));
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::DrawMapArrow(wxDC & hDC, wxColour col, double scrXstart, double scrYstart, unsigned angle)
{
	double scrXend   = scrXstart + (50 * cos(angle / 57.295779512));
	double scrYend   = scrYstart + (50 * sin(angle / 57.295779512));

	double r         = hypot( scrXstart - scrXend, scrYstart - scrYend);
	double scrXoff   = (r >= 1.0) ? ((scrXstart-scrXend)*12.0 / r) : 0.0;
	double scrYoff   = (r >= 1.0) ? ((scrYstart-scrYend)*12.0 / r) : 0.0;

	MapLine(hDC, col, scrXstart, scrYstart, scrXend, scrYend);
	scrXstart = scrXend + 3 * scrXoff;
	scrYstart = scrYend + 3 * scrYoff;
	MapLine(hDC, col, scrXstart - scrYoff, scrYstart + scrXoff, scrXend, scrYend);
	MapLine(hDC, col, scrXstart + scrYoff, scrYstart - scrXoff, scrXend, scrYend);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::DrawMapVector(wxDC & hDC, wxColour col, int scrXstart, int scrYstart, int scrXend, int scrYend)
{
	double r         = hypot( (double) (scrXstart - scrXend), (double) (scrYstart - scrYend));
	int    scrXoff   = (r >= 1.0) ? (int) ((scrXstart - scrXend) * 8.0 / r) : 0;
	int    scrYoff   = (r >= 1.0) ? (int) ((scrYstart - scrYend) * 8.0 / r) : 0;

	MapLine(hDC, col, scrXstart, scrYstart, scrXend, scrYend, 3);
	scrXstart = scrXend + 2 * scrXoff;
	scrYstart = scrYend + 2 * scrYoff;
	MapLine(hDC, col, scrXstart - scrYoff, scrYstart + scrXoff, scrXend, scrYend, 3);
	MapLine(hDC, col, scrXstart + scrYoff, scrYstart - scrXoff, scrXend, scrYend, 3);
}



//==========================================================================
//
//
//
//==========================================================================

void CLevel::HighlightLineDef(wxDC & DC, int line, wxColour col, bool hilighttags)
{
	CLine * cel = GetLine(line);
	DC.SetLogicalFunction(wxXOR);
	double n=(StVt(cel)->X() + EnVt(cel)->X()) / 2;
	double m=(StVt(cel)->Y() + EnVt(cel)->Y()) / 2;
	MapLine(DC, col, n, m,n + (EnVt(cel)->Y() - StVt(cel)->Y()) / 3,m + (StVt(cel)->X() - EnVt(cel)->X()) / 3);
	DrawMapVector(DC, col, StVt(cel)->X(), StVt(cel)->Y(), EnVt(cel)->X(), EnVt(cel)->Y());
	if (hilighttags)
	{
		if (!m_Extended && cel->GetTag() > 0)
		{
			for (int i = 0; i < NumSectors(); i++)
			{
				CSector * sec = GetSector(i);
				if (cel->GetTag() == sec->tag) HighlightSector(DC, i, hpLr, false);
			}
		}

		if (m_Extended)
		{
			int argmask = cgc->GetTagArg(cel->line.type);
			for(int i=0;i<5;i++)
			{
				if (argmask&(1<<i) && cel->line.args[i]!=0)
				{
					for (int k = 0; k < NumSectors(); k++)
					{
						CSector * sec = GetSector(k);
						if (cel->line.args[i] == sec->tag) HighlightSector(DC, k, hpLr, false);
					}
				}
			}

			argmask = cgc->GetLineIDArg(cel->line.type);
			for(int i=0;i<5;i++)
			{
				if (argmask&(1<<i) && cel->line.args[i]!=0)
				{
					for (int k = 0; k < NumLines(); k++)
					{
						CLine * ln = GetLine(i);
						int argmask = cgc->GetLineIDArg(ln->line.type);
						for(int j=0;j<5;j++)
						{
							if (ln->line.args[j]==cel->line.args[i])
							{
								HighlightLineDef(DC, k, hpLr, false);
							}
						}
					}
				}
			}
			DC.SetLogicalFunction(wxCOPY);
			argmask = cgc->GetTidArg(cel->line.type);
			for(int i=0;i<5;i++)
			{
				if (argmask&(1<<i) && cel->line.args[i]!=0)
				{
					for (int k = 0; k < NumThings(); k++)
					{
						CThing * th = GetThing(k);
						if (cel->line.args[i] == th->thingid) 
							DrawThing(DC, th, th->X(), th->Y(), true);
					}
				}
			}
		}
	}
	DC.SetLogicalFunction(wxCOPY);
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::HighlightSector(wxDC & DC, int s_index, wxColour col, bool hilighttags)
{
	int n,m;

	CSector * ces = GetSector(s_index);

	DC.SetLogicalFunction(wxXOR);
	for (n=0;n<NumLines();n++)
	{
		CLine * cel = GetLine(n);
		if (cel->sides[0].sector==s_index || cel->sides[1].sector==s_index)
			MapLine(DC, col, StVt(cel)->X(),StVt(cel)->Y(),EnVt(cel)->X(),EnVt(cel)->Y(), 3);
	}
	if (hilighttags && ces->tag > 0)
	{
		for (m = 0; m < NumLines(); m++)
		{
			CLine * cel = GetLine(m);
			if (!m_Extended)
			{
				if (cel->GetTag() == ces->tag) HighlightLineDef(DC, m, hpGr, false);
			}
			else
			{
				int argmask = cgc->GetTagArg(cel->line.type);
				for(int i=0;i<5;i++)
				{
					if (argmask&(1<<i) && cel->line.args[i]==ces->tag)
					{
						HighlightLineDef(DC, m, hpGr, false);
					}
				}
			}
		}
	}
	DC.SetLogicalFunction(wxCOPY);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::MarkThing(wxDC & DC,CThing * pth, wxColour col, bool hilighttags)
{
	CRectFloat r;
	int i=cgc->GetThingRadius(pth->type);

	r.Set(pth->X() - (i>>1)-3, pth->Y() - (i>>1)-3, pth->X() + (i>>1)+4, pth->Y() + (i>>1)+4);

	DC.SetLogicalFunction(wxXOR);
	MapRect(DC,col,r.left,r.top,r.right,r.bottom);
	DrawMapArrow(DC,col,pth->X(),pth->Y(),pth->angle);

	/*
	if (hilighttags && cel->GetTid() > 0)
	{
	for (m = 0; m < m_Things.size(); m++)
	if (cel->GetTid() == m_Things[m].tid) MarkThing(DC,&m_Things[m],hpLr);
	}
	*/
	DC.SetLogicalFunction(wxCOPY);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::MarkVertex(wxDC & DC,CVertex * pv,bool CheckOrSelect)
{
	CRectFloat r;
	wxColour hp;

	double disp=(m_ZoomFactor*0.5f);
	if (disp>12) disp=12;
	if (disp<1) disp=1;

	r.Set(pv->X()-disp,pv->Y()-disp,pv->X()+disp,pv->Y()+disp);
	if (!CheckOrSelect) hp = hpGr;
	else hp = hpYe;
	DC.SetLogicalFunction(wxXOR);
	MapRect(DC,hp,r.left,r.top,r.right,r.bottom);
	DC.SetLogicalFunction(wxCOPY);
}


//==========================================================================
//
// Unfortunately the Windows Path functions are not supported by wxWidgets
// so this has to be done natively...
//
//==========================================================================

void CLevel::FillSector(wxDC & DC,int s_index,bool swapxy, bool toprinter, wxColour color)
{
#if defined(__WIN32__)

	HDC hDC = (HDC)DC.GetHDC();
	HBRUSH br = CreateSolidBrush((COLORREF)color.GetPixel());
	br = (HBRUSH)SelectObject(hDC, br);

	TArray<CLine*> cel;
	POINT * pp=new POINT[NumVertices()];
	CLine * pl;
	CVertex * vt;
	int ppcount;
	int i;

	for(i=0;i<NumLines();i++)
	{
		if ((FrontSecNo(i)==s_index) ^ (BackSecNo(i)==s_index))
		{
			cel.Push(GetLine(i));
		}
	}
	BeginPath(hDC);

	int celcount = cel.Size();
	while (celcount)
	{
		pl=cel[--celcount];
		if (!toprinter)
		{
			pp[0].x=SX(StVt(pl)->X());
			pp[0].y=SY(StVt(pl)->Y());
			pp[1].x=SX(EnVt(pl)->X());
			pp[1].y=SY(EnVt(pl)->Y());
		}
		else
		{
			pp[0].x=int(StVt(pl)->X());
			pp[0].y=int(StVt(pl)->Y());
			pp[1].x=int(EnVt(pl)->X());
			pp[1].y=int(EnVt(pl)->Y());
		}
		ppcount=2;
		vt=EnVt(pl);
		while (celcount)
		{
			for(i=0;i<celcount;i++)
			{
				pl=cel[i];
				if (StVt(pl)==vt) vt=EnVt(pl);
				else if (EnVt(pl)==vt) vt=StVt(pl);
				else continue;
				if (!toprinter) 
				{
					pp[ppcount].x=SX(vt->X());
					pp[ppcount].y=SY(vt->Y());
				}
				else
				{
					pp[ppcount].x=int(vt->X());
					pp[ppcount].y=int(vt->Y());
				}
				ppcount++;
				break;
			}
			if (i!=celcount)
			{
				memmove(&cel[i],&cel[i+1],(celcount-i-1)*sizeof(CLine*));
				celcount--;
			}
			else if (i==celcount && i)
			{
				// sector is open
				AbortPath(hDC);
				delete pp;
				DeleteObject(SelectObject(hDC, br));
				return;
			}
			if (pp[ppcount-1].x==pp[0].x && pp[ppcount-1].y==pp[0].y) break;
		}
		if (swapxy)
		{
			for(i=0;i<ppcount;i++) 
			{
				int p=pp[i].x;
				pp[i].x=pp[i].y;
				pp[i].y=p;
			}
		}
		Polyline(hDC, pp,ppcount);
	}
	EndPath(hDC);
	FillPath(hDC);
	delete pp;
	DeleteObject(SelectObject(hDC, br));
#endif
}



//==========================================================================
//
//
//
//==========================================================================

void CLevel::DrawMarkRect(wxDC & DC,wxRealPoint from,wxRealPoint to)
{
	double l,t,r,b;

	l=min(from.x, to.x);
	r=max(from.x, to.x);
	t=min(from.y, to.y);
	b=max(from.y, to.y);

	DC.SetLogicalFunction(wxXOR);
	MapRect(DC, hpGr, l, t, r, b);
	DC.SetLogicalFunction(wxCOPY);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnDraw(wxDC & DC,CRectFloat * r)
{
	r->Set(MX(r->left), MY(r->top), MX(r->right), MY(r->bottom));
	DrawGrid(DC, r);

	switch(m_Mode)
	{
	case modeThings:
		OnDrawThings(DC,*r);
		break;

	case modeVertexes:
		OnDrawVertexes(DC,*r);
		break;

	case modeSectors:
		OnDrawSectors(DC,*r);
		break;

	case modeLineDefs:
		OnDrawLineDefs(DC,*r);
		break;

	case modeLineDraw:
		OnDrawLineDefs(DC,*r);
		//DrawLineDraw(DC,r);
		break;

	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::DrawThing(wxDC & DC, CThing * pth, double x, double y, bool halfbright)
{
	int i=cgc->GetThingRadius(pth->type);
	CRectFloat r,r1;

	r.Set(x-(i>>1)-3,y-(i>>1)-3,x+(i>>1)+4,y+(i>>1)+4);
	r1=r;

	wxColour c=cgc->GetThingColor(pth->type);

	// Mark items only for multiplayer differently!
	if (this->m_TextMap)
	{
		int bit = cgc->CheckTextMapThingFlag("single");
		if (bit < 0 || pth->Flags.GetBit(bit))
		{
			MapLine(DC,c,r.left,y,r.right,y);
			MapLine(DC,c,x,r.top,x,r.bottom);
		}
	}
	else if (!m_Extended)
	{
		if (!(pth->Flags.GetShort()&16))
		{
			MapLine(DC,c,r.left,y,r.right,y);
			MapLine(DC,c,x,r.top,x,r.bottom);
		}
	}
	else
	{
		if (pth->Flags.GetShort()&256)
		{
			MapLine(DC,c,r.left,y,r.right,y);
			MapLine(DC,c,x,r.top,x,r.bottom);
		}
	}
	MapEllipse(DC,c,r.left+2,r.top+2,r.right-2,r.bottom-2);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnDrawThings(wxDC & DC,CRectFloat rcClip)
{
	int i,j;
	CRectFloat r,r1;
	wxColour c(255,255,255);

	for(i=0;i<NumLines();i++)
	{
		CLine * pl=GetLine(i);
		r.Set(min(StVt(pl)->X(),EnVt(pl)->X())-1,min(StVt(pl)->Y(),EnVt(pl)->Y())-1,
			  max(StVt(pl)->X(),EnVt(pl)->X())+1,max(StVt(pl)->Y(),EnVt(pl)->Y())+1);
		if (r.IntersectRect(rcClip))
		{
			c=cgc->GetLDColor(&pl->line, m_TextMap);
			DrawLinedef(DC,c,StVt(pl),EnVt(pl));
		}
	}
	for(j=0;j<NumThings();j++)
	{
		CThing * pth = GetThing(j);
		int i=cgc->GetThingRadius(pth->type);
		r1.Set(pth->X() - (i>>1)-3,pth->Y() - (i>>1)-3, pth->X() + (i>>1)+4, pth->Y() + (i>>1)+4);
		if (r1.IntersectRect(rcClip))
		{
			DrawThing(DC, pth, pth->X(), pth->Y());
		}
	}

	DC.SetLogicalFunction(wxXOR);
	for(j=0;j<NumThings();j++)
	{
		CThing * pth = GetThing(j);
		int i=cgc->GetThingRadius(pth->type);
		r.Set(pth->X() - (i>>1)-3, pth->Y() - (i>>1)-3, pth->X() + (i>>1)+4,pth->Y() + (i>>1)+4);
		r1=r;
		if (r1.IntersectRect(rcClip))
		{
			if (checked[j])
			{
				MapRect(DC,hpGr,r.left,r.top,r.right,r.bottom);
				DrawMapArrow(DC,hpGr,pth->X(),pth->Y(),pth->angle);
			}
			if (m_Selection==j)
			{
				MapRect(DC,hpYe,r.left,r.top,r.right,r.bottom);
				DrawMapArrow(DC,hpYe,pth->X(),pth->Y(),pth->angle);
			}
		}
	}

	if (m_Selection!=-1 && m_Extended)
	{
		CThing * th = GetThing(m_Selection);
		if (th->thingid!=0)
		{
			for (int m = 0; m < NumLines(); m++)
			{
				CLine * cel = GetLine(m);
				int argmask = cgc->GetTidArg(cel->line.type);
				for(int i=0;i<5;i++)
				{
					if (argmask&(1<<i) && cel->line.args[i]!=0)
					{
						HighlightLineDef(DC, m, hpGr, false);
					}
				}
			}
		}
	}
	DC.SetLogicalFunction(wxCOPY);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnDrawVertexes(wxDC & DC,CRectFloat rcClip)
{
	int i;
	CVertex * pv;
	CRectFloat r,r1;

	for(i=0;i<NumLines();i++)
	{
		CLine * pl=GetLine(i);
		r.Set(min(StVt(pl)->X(),EnVt(pl)->X())-1,min(StVt(pl)->Y(),EnVt(pl)->Y())-1,
			  max(StVt(pl)->X(),EnVt(pl)->X())+1,max(StVt(pl)->Y(),EnVt(pl)->Y())+1);
		if (r.IntersectRect(rcClip)) DrawLinedef(DC,hpG,StVt(pl),EnVt(pl));
	}
	for(i=0;i<NumThings();i++)
	{
		CThing * pth = GetThing(i);
		r.Set(pth->X() - 11, pth->Y() - 11,pth->X() + 12,pth->Y() + 12);
		r1=r;
		if (r1.IntersectRect(rcClip))
		{
			MapLine(DC, hpG, pth->X(), r.top, pth->X(), r.bottom);
			MapLine(DC, hpG, r.left, pth->Y(), r.right, pth->Y());
		}
	}

	for(i=0;i<NumVertices();i++)
	{
		pv=GetVertex(i);
		r.Set(pv->X()-12,pv->Y()-12,pv->X()+13,pv->Y()+13);
		r1=r;
		if (r1.IntersectRect(rcClip))
		{
			float disp=m_ZoomFactor*0.5f;
			if (disp>12) disp=12;
			MapLine(DC,hpGr, pv->X()-disp,pv->Y()+disp,pv->X()+disp,pv->Y()-disp);
			MapLine(DC,hpGr, pv->X()+disp,pv->Y()+disp,pv->X()-disp,pv->Y()-disp);
		}
	}
	for(i=0;i<NumVertices();i++)
	{
		pv=GetVertex(i);
		r.Set(pv->X()-12,pv->Y()-12,pv->X()+13,pv->Y()+13);
		r1=r;
		if (r1.IntersectRect(rcClip))
		{
			if (checked[i])
			{
				MarkVertex(DC, pv, false);
			}
			if (m_Selection==i)
			{
				MarkVertex(DC, pv, true);
			}
		}
	}
}


bool isSecret(int num)
{
	if (cgc->genericsectors)
	{
		if (cgc->IsExtended || num&0x8000) return !!(num&1024);
		else return !!(num&128) || (num&31)==9;
	}
	else if (!cgc->IsExtended) return num==9;
	else return false;
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnDrawSectors(wxDC & DC,CRectFloat rcClip)
{
	int i;
	CRectFloat r,r1;
	wxColour c;

	for(i=0;i<NumSectors();i++)
	{
		CSector * psc = GetSector(i);
		if (isSecret(psc->special))
		{
			FillSector(DC, i, false, false, wxColour(100, 100, 160));
		}

		if (psc->buggy)
		{
			FillSector(DC, i, false, false, wxColour(160, 100, 10));
		}
	}
	int r2 = DC.GetLogicalFunction();
	DC.SetLogicalFunction(wxCOPY);
	for(i=0;i<NumLines();i++)
	{
		CLine * pl=GetLine(i);
		r.Set(	min(StVt(pl)->X(),EnVt(pl)->X())-1,min(StVt(pl)->Y(),EnVt(pl)->Y())-1,
				max(StVt(pl)->X(),EnVt(pl)->X())+1,max(StVt(pl)->Y(),EnVt(pl)->Y())+1);
		if (r.IntersectRect(rcClip))
		{
			c=hpG;
			if (pl->sides[0].sector==-1) c=hpRe;
			else if ((pl->sides[0].sector!=-1 && FrontSec(pl)->tag) || 
					 (pl->sides[1].sector!=-1 && BackSec(pl)->tag)) c=hpGr;
			else if ((pl->sides[0].sector!=-1 && FrontSec(pl)->special) || 
					 (pl->sides[1].sector!=-1 && BackSec(pl)->special)) c=hpCy;
			else if (/*pl->line.flags&0x8001 ||*/ pl->sides[1].sector==-1) c=hpW;

			DrawLinedef(DC,c,StVt(pl),EnVt(pl));
		}
	}
	for(i=0;i<NumSectors();i++)
	{
		if (checked[i]) HighlightSector(DC,i,hpGr,false);
		if (m_Selection==i) HighlightSector(DC,i,hpYe,true);
	}

	DC.SetLogicalFunction(wxXOR);
	for(i=0;i<NumThings();i++)
	{
		CThing * pth = GetThing(i);
		r.Set(pth->X()-11, pth->Y()-11,pth->X()+12,pth->Y()+12);
		r1=r;
		if (r1.IntersectRect(rcClip))
		{
			MapLine(DC,hpG,pth->X(),r.top,pth->X(),r.bottom);
			MapLine(DC,hpG,r.left,pth->Y(),r.right,pth->Y());
		}
	}

	// These must be restored!
	DC.SetLogicalFunction(r2);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnDrawLineDefs(wxDC & DC,CRectFloat rcClip)
{
	int i;
	CRectFloat r,r1;
	wxColour c;

	for(i=0;i<NumLines();i++)
	{
		CLine * pl=GetLine(i);
		r.Set(min(StVt(pl)->X(),EnVt(pl)->X())-1,min(StVt(pl)->Y(),EnVt(pl)->Y())-1,
			  max(StVt(pl)->X(),EnVt(pl)->X())+1,max(StVt(pl)->Y(),EnVt(pl)->Y())+1);
		if (r.IntersectRect(rcClip))
		{
			c=cgc->GetLDColor(&pl->line, m_TextMap);
			if (pl->buggy) c=hpRe;
			DrawLinedef(DC,c,StVt(pl),EnVt(pl));
		}
	}
	for(i=0;i<NumLines();i++) if (checked[i] || m_Selection==i)
	{
		r.Set(min(StVt(i)->X(),EnVt(i)->X())-2,min(StVt(i)->Y(),EnVt(i)->Y())-2,
			max(StVt(i)->X(),EnVt(i)->X())+2,max(StVt(i)->Y(),EnVt(i)->Y())+2);
		if (r.IntersectRect(rcClip))
		{
			if (checked[i]) HighlightLineDef(DC,i,hpGr,false);
			if (m_Selection==i) HighlightLineDef(DC,i,hpYe,true);
		}
	}

	DC.SetLogicalFunction(wxXOR);
	for(i=0;i<NumThings();i++)
	{
		CThing * pth = GetThing(i);
		r.Set(pth->X()-11, pth->Y()-11,pth->X()+12,pth->Y()+12);
		r1=r;
		if (r1.IntersectRect(rcClip))
		{
			MapLine(DC,hpG,pth->X(),r.top,pth->X(),r.bottom);
			MapLine(DC,hpG,r.left,pth->Y(),r.right,pth->Y());
		}
	}
	DC.SetLogicalFunction(wxCOPY);
}




