#include "StdAfx.h"
#include ".\level.h"


// Coordinate utilities.

// Most of the code in this file has been taken from DEU, although it
// has been heavily rewritten and enhanced. However, the original copyright
// and license are still valid!

// Doom Editor Utility, by Brendon Wyber and Raphaël Quinet.

// You are allowed to use any parts of this code in another program, as
// long as you give credits to the authors in the documentation and in
// the program itself.  Read the file README.1ST for more information.

// This program comes with absolutely no warranty.

// Changes to original code (c) 1995-2008 by Christoph Oelckers



//==========================================================================
//
//
//
//==========================================================================

int CLevel::SX(double x)
{
	return int((x-m_CenterX)*10/m_ZoomFactor) + (m_ScreenCenterX);
}

int CLevel::SY(double y)
{
	return -int((y-m_CenterY)*10/m_ZoomFactor) + (m_ScreenCenterY);
}

double CLevel::MX(int x)
{
	return (x-(m_ScreenCenterX))*m_ZoomFactor/10+m_CenterX;
}

double CLevel::MY(int y)
{
	return -(y-(m_ScreenCenterY))*m_ZoomFactor/10+m_CenterY;
}


//==========================================================================
//
//
//==========================================================================
bool CLevel::IsLineDefInside( int l, double x0, double y0, double x1, double y1)
{
	CLine * cel=GetLine(l);
	CVertex * v1 = StVt(cel);
	CVertex * v2 = EnVt(cel);
	double lx0 = v1->X();
	double ly0 = v1->Y();
	double lx1 = v2->X();
	double ly1 = v2->Y();
	double i;

	/* do you like mathematics? */
	if ((lx0 >= x0 && lx0 <= x1 && ly0 >= y0 && ly0 <= y1) ||
		(lx1 >= x0 && lx1 <= x1 && ly1 >= y0 && ly1 <= y1)) return true;
	if ((ly0 > y0) != (ly1 > y0))
	{
		i = lx0 + ((y0 - ly0) * (lx1 - lx0) / (ly1 - ly0));
		if (i >= x0 && i <= x1) return true; /* the LineDef crosses the y0 side (left) */
	}
	if ((ly0 > y1) != (ly1 > y1))
	{
		i = lx0 + ((y1 - ly0) * (lx1 - lx0) / (ly1 - ly0));
		if (i >= x0 && i <= x1) return true; /* the LineDef crosses the y1 side (right) */
	}
	if ((lx0 > x0) != (lx1 > x0))
	{
		i = ly0 + ((x0 - lx0) * (ly1 - ly0) / (lx1 - lx0));
		if (i >= y0 && i <= y1) return true; /* the LineDef crosses the x0 side (down) */
	}
	if ((lx0 > x1) != (lx1 > x1))
	{
		i = ly0 + ((x1 - lx0) * (ly1 - ly0) / (lx1 - lx0));
		if (i >= y0 && i <= y1) return true; /* the LineDef crosses the x1 side (up) */
	}
	return false;
}


//==========================================================================
//
//
//
//==========================================================================

bool CLevel::IsSectorInside(int cesno,double x0,double y0,double x1,double y1)
{
	int n;

	for(n=0;n<NumLines();n++)
	{
		if ((FrontSecNo(n)==cesno) || (BackSecNo(n)==cesno))
		{
			if (!IsLineDefInside(n,x0,y0,x1,y1)) return false;
		}
	}
	return true;
}


//==========================================================================
//
//
//==========================================================================

int CLevel::GetThingFromPos(CRectFloat * lpr)
{
	int i;

	for(i=0;i<NumThings();i++)
	{
		if (lpr->PointInRect(*GetThing(i))) return i;
	}
	return -1;
}

//==========================================================================
//
//
//==========================================================================

int CLevel::GetVertexFromPos(CRectFloat * lpr)
{
	int i;

	for(i=0;i<NumVertices();i++) 
	{
		if (lpr->PointInRect((wxRealPoint)*GetVertex(i))) return i;
	}
	return -1;
}

//==========================================================================
//
//
//==========================================================================

int CLevel::GetVertexFromPoint(wxPoint lpt)
{
	int i;

	for(i=0;i<NumVertices();i++) 
	{
		CVertex * pth = GetVertex(i);
		if (int(pth->X())==lpt.x && int(pth->Y())==lpt.y) return i;
	}
	return -1;
}

//==========================================================================
//
//
//==========================================================================

void CLevel::GetAttachedLines(int vt, TArray<int> & list)
{
	for (int l = 0; l < NumLines(); l++)
	{
		if (StVtNo(l)==vt || EnVtNo(l)==vt) list.Push(l);
	}
}

//==========================================================================
//
//
//==========================================================================

int CLevel::GetLineDefFromPos(CRectFloat * r)
{
	int n;

	for (n=0;n<NumLines();n++)
	{
		if (IsLineDefInside(n,r->left,r->top,r->right,r->bottom)) return n;
	}
	return -1;
}


//==========================================================================
//
//
//==========================================================================

int CLevel::GetSectorFromPos(wxRealPoint p)
{
	int n;
	CLine * curld=NULL;

	double curx=m_Bounds.right+1;

	for(n=0;n<NumLines();n++)
	{
		CLine * cel = GetLine(n);
		if ((StVt(cel)->Y() > p.y) != (EnVt(cel)->Y() > p.y))
		{
			double lx0 = StVt(cel)->X();
			double ly0 = StVt(cel)->Y();
			double lx1 = EnVt(cel)->X();
			double ly1 = EnVt(cel)->Y();
			double m=lx0+((p.y-ly0)*(lx1-lx0)/(ly1-ly0));
			if (m>=p.x && m<curx)
			{
				curx=m;
				curld=cel;
			}
		}
	}
	/* now look if this LineDef has a SideDef bound to one sector */
	if (curld)
	{
		int side = (StVt(curld)->Y() > EnVt(curld)->Y())? 0:1;
		return curld->sides[side].sector;
	}
	return -1;
}







