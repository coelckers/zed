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
// Main code file for level data maintenance
//


#include "StdAfx.h"
#include "configfile.h"
#include "ZEd.h"
#include "Level.h"



//==========================================================================
//
//
//
//==========================================================================

void CLevel::SaveGrid()
{
	if (config.SetSection("Settings", true))
	{
		config.SetIntValueForKey("GridOn",   m_GridToggled );
		config.SetIntValueForKey("GridSnap", m_SnapToGrid  );
		config.SetIntValueForKey("GridSize", m_GridSize    );
		config.SetIntValueForKey("GridSizeX",m_GridSizeX   );
		config.SetIntValueForKey("GridSizeY",m_GridSizeY   );
		config.SetIntValueForKey("GridOfsX", m_GridOfsX    );
		config.SetIntValueForKey("GridOfsY", m_GridOfsY    );
		config.SetIntValueForKey("GridHi64", m_GridHighlight);
		config.SetIntValueForKey("AutoSnap", m_SnapAutoToggle);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnEditTogglegrid(wxCommandEvent & event)
{
	m_GridToggled = !m_GridToggled;
	SaveGrid();
	m_DrawWindow->Refresh();
}

void CLevel::OnUpdateEditTogglegrid(wxUpdateUIEvent & event)
{
	event.Check(m_GridToggled);
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnViewSnaptogrid(wxCommandEvent & event)
{
	m_SnapToGrid=!m_SnapToGrid;
	SaveGrid();
	m_DrawWindow->Refresh();
}

void CLevel::OnUpdateViewSnaptogrid(wxUpdateUIEvent & event)
{
	event.Check(m_SnapToGrid);
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnViewHighlight64unitgrid(wxCommandEvent & event)
{
	m_GridHighlight=!m_GridHighlight;
	SaveGrid();
	m_DrawWindow->Refresh();
}

void CLevel::OnUpdateViewHighlight64unitgrid(wxUpdateUIEvent & event)
{
	event.Check(m_GridHighlight);
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnViewAuto(wxCommandEvent & event)
{
	m_SnapAutoToggle=!m_SnapAutoToggle;
	SaveGrid();
}

void CLevel::OnUpdateViewAuto(wxUpdateUIEvent & event)
{
	event.Check(m_SnapAutoToggle);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnEditChangegrid(wxCommandEvent & event)
{
	// This function emulates DEU's grid behavior
	// and overrides any grid settings done with 
	// 'Set Grid'!
	if (!m_SnapAutoToggle)
	{
		if (!m_GridToggled)
		{
			m_GridToggled=true;
			m_GridSize=128;
		}
		else
		{
			m_GridSize>>=1;
			if (m_GridSize<=1) 
			{
				m_GridSize=128;
				m_GridToggled=false;
			}
		}
	}
	else
	{
		if (!m_SnapToGrid)
		{
			m_SnapToGrid=true;
		}
		else if (!m_GridToggled)
		{
			m_GridToggled=true;
			m_GridSize=128;
		}
		else
		{
			m_GridSize>>=1;
			if (m_GridSize<=1) 
			{
				m_GridSize=128;
				m_SnapToGrid=false;
			}
		}
	}
	m_GridSizeX=m_GridSizeY=m_GridSize;
	m_GridOfsX=m_GridOfsY=0;
	SaveGrid();
	m_DrawWindow->Refresh();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnViewSetgrid(wxCommandEvent & event)
{
	wxPoint pt[2];

	if (GetGrid(m_DrawWindow, pt))
	{
		m_GridSizeX = pt[0].x;
		m_GridSizeY = pt[0].y;
		m_GridOfsX = pt[1].x;
		m_GridOfsY = pt[1].y;
		m_GridSize=1;
		while (m_GridSize<m_GridSizeX && m_GridSize<m_GridSizeY) m_GridSize<<=1;
		m_GridToggled = true;
		SaveGrid();
		m_DrawWindow->Refresh();
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::SnapToGrid(CVertex *v)
{
	int xx=int(v->X());
	int yy=int(v->Y());
	SnapToGrid(&xx, &yy);
	v->SetX(xx, false);
	v->SetY(yy, false);
}

void CLevel::SnapToGrid(CThing *v)
{
	int xx=int(v->X());
	int yy=int(v->Y());
	SnapToGrid(&xx, &yy);
	v->SetX(xx, false);
	v->SetY(yy, false);
}

void CLevel::SnapToGrid(double * x, double * y)
{
	int xx=int(*x);
	int yy=int(*y);
	SnapToGrid(&xx, &yy);
	*x=xx;
	*y=yy;
}

void CLevel::SnapToGrid(short * x, short * y)
{
	int xx=*x;
	int yy=*y;
	SnapToGrid(&xx, &yy);
	*x=xx;
	*y=yy;
}

void CLevel::SnapToGrid(int * x, int * y)
{
	if (m_GridToggled)
	{
		*x -= m_GridOfsX;
		*y -= m_GridOfsY;

		if (*x>0) *x = (*x + m_GridSizeX/2) / m_GridSizeX * m_GridSizeX;
		else *x = (*x - m_GridSizeX/2) / m_GridSizeX * m_GridSizeX;
		if (*y>0) *y = (*y + m_GridSizeY/2) / m_GridSizeY * m_GridSizeY;
		else *y = (*y - m_GridSizeY/2) / m_GridSizeY * m_GridSizeY;

		*x += m_GridOfsX;
		*y += m_GridOfsY;
	}
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::DrawGrid(wxDC & DC,CRectFloat * lpr)
{
	if (m_GridToggled)
	{
		int i;
		wxPen pen;
		wxPen penhi;
		wxPen penhinogrid;
		RECT r;

		bool hilight=false;

		// The 64 grid highlight will only be active if the grid is a standard power of 2 one!
		if ( (m_GridSizeX%64==0 || 64%m_GridSizeX==0) &&
			 (m_GridSizeY%64==0 || 64%m_GridSizeY==0) &&
			 m_GridOfsX%m_GridSizeX==0 && m_GridOfsY%m_GridSizeY==0) hilight=true;


		if (m_SnapToGrid) 
		{
			pen = wxPen(wxColour(35, 35, 70));
			penhi = wxPen(wxColour(64, 64, 64));
			penhinogrid = wxPen(wxColour(32, 32, 32));
		}
		else 
		{
			pen = wxPen(wxColour(24, 24, 48));
			penhi = wxPen(wxColour(48, 48, 48));
			penhinogrid = wxPen(wxColour(21, 21, 21));
		}

		DC.SetBackgroundMode(wxTRANSPARENT);

		r.top=SY(lpr->bottom);
		r.bottom=SY(lpr->top);
		r.left=SX(lpr->left);
		r.right=SX(lpr->right);

		if (hilight)
		{
			DC.SetPen(penhinogrid);
			if (m_GridSizeX>64)
			{
				for(i=int(lpr->left - 64) / 64 * 64 ;i<lpr->right+64;i+=64)
				{
					int j=SX(i);
					DC.DrawLine(j, r.top, j, r.bottom);
				}
			}
			if (m_GridSizeY>64)
			{
				for(i=int(lpr->top - 64)/ 64 * 64; i<lpr->bottom+64; i+=64)
				{
					int j=SY(i);
					DC.DrawLine(r.left, j, r.right, j);
				}
			}
		}

		DC.SetPen(pen);
		for(i=int(lpr->left - m_GridSizeX) / m_GridSizeX * m_GridSizeX + m_GridOfsX ;
			i<lpr->right+m_GridSizeX;i+=m_GridSizeX)
		{
			if (hilight)
			{
				if (m_GridSizeX<=64 && i%64==0) continue;
				if (m_GridSizeX>64 && i%m_GridSizeX!=0) continue;
			}
			int j=SX(i);
			DC.DrawLine(j, r.top, j, r.bottom);
		}
		for(i=int(lpr->top - m_GridSizeY) / m_GridSizeY * m_GridSizeY + m_GridOfsY;
			i<lpr->bottom+m_GridSizeY;i+=m_GridSizeY)
		{
			if (hilight)
			{
				if (m_GridSizeY<=64 && i%64==0) continue;
				if (m_GridSizeY>64 && i%m_GridSizeY!=0) continue;
			}
			int j=SY(i);
			DC.DrawLine(r.left, j, r.right, j);
		}

		if (hilight)
		{
			DC.SetPen(penhi);
			if (m_GridSizeX<=64)
			{
				for(i=int(lpr->left - 64) / 64 * 64 ;i<lpr->right+64;i+=64)
				{
					int j=SX(i);
					DC.DrawLine(j, r.top, j, r.bottom);
				}
			}
			if (m_GridSizeY<=64)
			{
				for(i=int(lpr->top - 64)/ 64 * 64; i<lpr->bottom+64; i+=64)
				{
					int j=SY(i);
					DC.DrawLine(r.left, j, r.right, j);
				}
			}
		}
	}
}

