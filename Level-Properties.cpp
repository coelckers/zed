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
// Code that manages the property dialogs
//
#include "StdAfx.h"
#include "ZEd.h"
#include "Level.h"

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdProperties(wxCommandEvent & event)
{
	switch(m_Mode)
	{
	case modeThings:
		OnCmdPropertiesThings();
		break;

	case modeVertexes:
		OnCmdPropertiesVertexes();
		break;

	case modeSectors:
		OnCmdPropertiesSectors();
		break;

	case modeLineDefs:
		OnCmdPropertiesLineDefs();
		break;
	}
	UpdateStatusBar();
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdPropertiesThings()
{
	CThing save, edit;
	int copy;

	if (NumThings()==0) return;
	if (m_Selection==-1) m_Selection=0;

	if (Selection.Size()==0 || checked[m_Selection]) copy = m_Selection;
	else copy = Selection[0];
	save = edit = *GetThing(copy);

	//save.CopyProperties();
	//edit.CopyProperties();

	if (Selection.Size()>1)
	{
		edit.multi=true;
		edit.Flags.Clear();
		edit.notflags.Set();
		for(unsigned int i=0;i<Selection.Size();i++)
		{
			CThing * t = GetThing(Selection[i]);
			edit.Flags|=t->Flags;
			edit.notflags&=t->Flags;
		}
	}
	else edit.multi=false;

	int changes = EditThing(m_DrawWindow, this, &edit);
	if (changes!=-1)
	{
		m_changed=true;
		MakeBackup("Change thing properties", false, false, false, true);
		if (edit.multi)
		{
			for(unsigned int i=0;i<Selection.Size();i++)
			{
				CThing * t = GetThing(Selection[i]);
				if (changes&THF_Type) t->type = edit.type;
				if (changes&THF_Angle) t->angle = edit.angle;
				t->SetX(t->X() + edit.X()-save.X(), true);
				t->SetY(t->Y() + edit.Y()-save.Y(), true);
				t->SetZ(t->Z() + edit.Z()-save.Z(), true);
				if (changes&THF_Special) t->special = edit.special;
				if (changes&THF_TID) t->thingid = edit.thingid;
				for(int k=0;k<5;k++) if (changes&(THF_Arg1<<k)) t->args[k] = edit.args[k];
				t->Flags&=~edit.notflags;
				t->Flags|=edit.Flags;
			}
		}
		else
		{
			map.CopyThing(copy, &edit);
		}	
		m_DrawWindow->Refresh();
		UpdateStatusBar();
	}
	//save.ReleaseProperties();
	//edit.ReleaseProperties();
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdPropertiesVertexes()
{
	wxRealPoint save, editf;
	wxPoint edit;
	int copy;
	bool res;

	if (Selection.Size()==0 || checked[m_Selection]) copy = m_Selection;
	else copy = Selection[0];
	save = *GetVertex(copy);

	if (!m_TextMap)
	{
		edit.x = save.x;
		edit.y = save.y;
		res = Get2Numbers(m_DrawWindow, "Edit Vertex", "x position", "y position", -32767, 32767, -32767, 32767, &edit);
		editf.x = edit.x;
		editf.y = edit.y;
	}
	else
	{
		editf = save;
		res = Get2Floats(m_DrawWindow, "Edit Vertex", "x position", "y position", -32767, 32767, -32767, 32767, &editf);
	}

	if (res)
	{
		m_changed=true;
		MakeBackup("Change vertex coordinates", true, false, false, false);
		if (Selection.Size()>0)
		{
			for(unsigned int i=0;i<Selection.Size();i++)
			{
				CVertex * v = GetVertex(Selection[i]);
				v->SetX(v->X() + editf.x-save.x, m_TextMap);
				v->SetY(v->Y() + editf.y-save.y, m_TextMap);
			}
		}
		else
		{
			CVertex *v = GetVertex(copy);
			v->SetX(editf.x, m_TextMap);
			v->SetY(editf.y, m_TextMap);
		}
		m_DrawWindow->Refresh();
		UpdateStatusBar();
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdPropertiesSectors()
{
	CSector edit, save;
	int copy;

	if (Selection.Size()==0 || checked[m_Selection]) copy = m_Selection;
	else copy = Selection[0];
	save = edit = *GetSector(copy);

	//save.CopyProperties();
	//edit.CopyProperties();

	edit.multi=false;
	edit.flags=edit.special;
	if (Selection.Size()>0)
	{
		edit.multi=true;
		edit.flags=0;
		edit.notflags=-1;
		for(unsigned int i=0;i<Selection.Size();i++)
		{
			CSector * sec = GetSector(Selection[i]);
			edit.flags|=sec->special;
			edit.notflags&=sec->special;
		}

		edit.Flags.Clear();
		edit.notFlags.Set();
		for(unsigned int i=0;i<Selection.Size();i++)
		{
			CThing * t = GetThing(Selection[i]);
			edit.Flags|=t->Flags;
			edit.notFlags&=t->Flags;
		}

	}
	else edit.multi=false;

	int changes = EditSector(m_DrawWindow, this, &edit);
	if (changes!=-1)
	{
		m_changed=true;
		MakeBackup("Change sector properties", false, false, true, false);
		if (edit.multi)
		{
			for(unsigned int i=0;i<Selection.Size();i++)
			{
				CSector * sec = GetSector(Selection[i]);
				int mask;
				if (changes & SF_Type)
				{
					mask = m_Extended? 255:31;
					sec->special= (sec->special&~mask)|(edit.special&mask);
				}
				sec->special &= ~edit.notflags;
				sec->special |= edit.flags;

				sec->Flags&=~edit.notFlags;
				sec->Flags|=edit.Flags;

				if (changes & SF_Tag) sec->tag=edit.tag;
				if (changes & SF_Light) sec->light=edit.light;
				if (changes & SF_FloorT) memcpy(sec->floort, edit.floort, 8);
				if (changes & SF_CeilT) memcpy(sec->ceilt, edit.ceilt, 8);

				sec->ceilh += edit.ceilh-save.ceilh;
				sec->floorh += edit.floorh-save.floorh;

				for(int i=0;i<2;i++) for (int j=0;j<2;j++)
					sec->offset[i][j] += edit.offset[i][j] - save.offset[i][j];

				if (changes & SF_FScaleX) sec->scale[FLOOR][X] = edit.scale[FLOOR][X];
				if (changes & SF_CScaleX) sec->scale[CEILING][X] = edit.scale[CEILING][X];
				if (changes & SF_FScaleY) sec->scale[FLOOR][Y] = edit.scale[FLOOR][Y];
				if (changes & SF_CScaleY) sec->scale[CEILING][Y] = edit.scale[CEILING][Y];
				if (changes & SF_CRot) sec->rotation[CEILING] = edit.rotation[CEILING];
				if (changes & SF_FRot) sec->rotation[FLOOR] = edit.rotation[FLOOR];
				if (changes & SF_CLite) sec->planelight[CEILING] = edit.planelight[CEILING];
				if (changes & SF_FLite) sec->planelight[FLOOR] = edit.planelight[FLOOR];
				if (changes & SF_Gravity) sec->gravity = edit.gravity;
				if (changes & SF_LiteCol) sec->color = edit.color;
				if (changes & SF_FadeCol) sec->fade =edit.fade;
				if (changes & SF_Desat) sec->desaturation = edit.desaturation;
			}
		}
		else
		{
			edit.special &= ~edit.notflags;
			edit.special |= edit.flags;
			map.CopySector(copy, &edit);
		}
		m_DrawWindow->Refresh();
		UpdateStatusBar();
	}
	//edit.ReleaseProperties();
	//save.ReleaseProperties();
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCmdPropertiesLineDefs()
{
	CLine edit, save;
	int copy;

	if (Selection.Size()==0 || checked[m_Selection]) copy = m_Selection;
	else copy = Selection[0];
	save = edit = *GetLine(copy);

	if (Selection.Size()>0)
	{
		edit.multi=true;
		edit.line.Flags.Clear();
		edit.line.notflags.Set();
		edit.sides[0].sector=edit.sides[1].sector=-3;
		for(unsigned int i=0;i<Selection.Size();i++)
		{
			CLine * ln = GetLine(Selection[i]);
			edit.line.Flags|=ln->line.Flags;
			edit.line.notflags&=ln->line.Flags;
			for(int k=0;k<2;k++)
			{
				if (edit.sides[k].sector!=-3 && edit.sides[k].sector!=ln->sides[k].sector)
				{
					edit.sides[k].sector=-2;
				}
				else
				{
					edit.sides[k].sector = ln->sides[k].sector;
				}
			}
		}
	}
	else edit.multi=false;

	
	int changes = EditLine(m_DrawWindow, this, &edit);
	if (changes!=-1)
	{
		m_changed=true;
		MakeBackup("Change linedef properties", false, true, false, false);
		if (edit.multi)
		{
			for(unsigned int i=0;i<Selection.Size();i++)
			{
				CLine * ln = GetLine(Selection[i]);

				ln->line.Flags&=~edit.line.notflags;
				ln->line.Flags|=edit.line.Flags;

				ln->sides[0].Flags&=~edit.sides[0].notflags;
				ln->sides[0].Flags|=edit.sides[0].Flags;

				ln->sides[1].Flags&=~edit.sides[1].notflags;
				ln->sides[1].Flags|=edit.sides[1].Flags;

				if (changes & LDF_Special) ln->line.type = edit.line.type;
				if (changes & LDF_Tag) ln->line.tag = edit.line.tag;
				if (changes & LDF_Alpha) ln->line.alpha = edit.line.alpha;
				for(int k=0;k<5;k++) if (changes & (LDF_Arg1<<k)) ln->line.args[k] = edit.line.args[k];

				for(int k=0;k<2;k++)
				{
					int a = k==0? 1:LDF_Texture2U/LDF_Texture1U;

					if (changes & (a*LDF_Texture1U)) memcpy(ln->sides[k].texUpper, edit.sides[k].texUpper, 8);
					if (changes & (a*LDF_Texture1M)) memcpy(ln->sides[k].texNormal, edit.sides[k].texNormal, 8);
					if (changes & (a*LDF_Texture1L)) memcpy(ln->sides[k].texLower, edit.sides[k].texLower, 8);
					if (changes & (LDF_Sector1<<k))  ln->sides[k].sector = edit.sides[k].sector;
					ln->sides[k].xoff += edit.sides[k].xoff - save.sides[k].xoff;
					ln->sides[k].yoff += edit.sides[k].yoff - save.sides[k].yoff;
					if (m_TextMap)
					{
						if (changes & (LDF_LiteF<<k))  ln->sides[k].light = edit.sides[k].light;
						for(int j=0;j<3;j++) for(int l=0;l<2;l++)
						{
							ln->sides[k].texOffset[j][l] += edit.sides[k].texOffset[j][l] - save.sides[k].texOffset[j][l];

							if (edit.sides[k].texScale[j][l] != save.sides[k].texScale[j][l])
								ln->sides[k].texScale[j][l] = edit.sides[k].texScale[j][l];
						}
					}
				}
			}
		}
		else
		{
			map.CopyLine(copy, &edit);
		}
		m_DrawWindow->Refresh();
		UpdateStatusBar();
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnCopyproperties(wxCommandEvent & event)
{
	if (!(m_Mode&modeEdit)) return;

	if (m_Selection<0) return;
	switch(m_Mode)
	{
	case modeThings:
		if (m_Selection<NumThings()) 
		{
			//m_ClipThing.ReleaseProperties();
			m_ClipThing = *GetThing(m_Selection);
			//m_ClipThing.CopyProperties();
		}
		break;

	case modeLineDefs:
		if (m_Selection<NumLines()) 
		{
			//m_ClipLine.ReleaseProperties();
			m_ClipLine = *GetLine(m_Selection);
			//m_ClipLine.CopyProperties();
		}
		break;

	case modeSectors:
		if (m_Selection<NumSectors()) 
		{
			//m_ClipSector.ReleaseProperties();
			m_ClipSector = *GetSector(m_Selection);
			//m_ClipSector.CopyProperties();
		}
		break;

	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnPasteproperties(wxCommandEvent & event)
{
	CThing tempThing;
	CLine tempLine;
	int i;

	if (!(m_Mode&modeEdit)) return;

	switch(m_Mode)
	{
	case modeThings:
		MakeBackup("Paste Thing Properties", false, false, false, true);
		if (Selection.Size()==0)
		{
			CThing * t = GetThing(m_Selection);
			// Copy everything except the coordinates
			tempThing = m_ClipThing;
			tempThing.SetX(t->X(), true);
			tempThing.SetY(t->Y(), true);
			tempThing.SetZ(t->Z(), true);
			map.CopyThing(m_Selection, &tempThing);
			m_changed=true;
		}
		else for(i=0;i<NumThings();i++) if (checked[i])
		{
			CThing * t = GetThing(i);
			// Copy everything except the coordinates
			tempThing = m_ClipThing;
			tempThing.SetX(t->X(), true);
			tempThing.SetY(t->Y(), true);
			tempThing.SetZ(t->Z(), true);
			map.CopyThing(m_Selection, &tempThing);
			m_changed=true;
		}
		break;

	case modeLineDefs:
		MakeBackup("Paste Linedef Properties", false, true, false, false);
		if (Selection.Size()==0)
		{
			CLine * ln = GetLine(m_Selection);

			// Copy everything except the vertices
			tempLine = m_ClipLine;
			tempLine.line.Start = ln->line.Start;
			tempLine.line.End = ln->line.End;

			// Don't paste non-existent sidedefs
			if (tempLine.sides[0].sector==-1) tempLine.sides[0] = ln->sides[0];
			if (tempLine.sides[1].sector==-1) tempLine.sides[1] = ln->sides[1];

			// Keep the old sector references
			tempLine.sides[0].sector = ln->sides[0].sector;
			tempLine.sides[1].sector = ln->sides[1].sector;
			map.CopyLine(m_Selection, &tempLine);
			m_changed=true;
		}
		else for(i=0;i<NumLines();i++) if (checked[i])
		{
			CLine * ln = GetLine(i);

			// Copy everything except the vertices
			tempLine = m_ClipLine;
			tempLine.line.Start = ln->line.Start;
			tempLine.line.End = ln->line.End;

			// Don't paste non-existent sidedefs
			if (tempLine.sides[0].sector==-1) tempLine.sides[0] = ln->sides[0];
			if (tempLine.sides[1].sector==-1) tempLine.sides[1] = ln->sides[1];

			// Keep the old sector references
			tempLine.sides[0].sector = ln->sides[0].sector;
			tempLine.sides[1].sector = ln->sides[1].sector;
			map.CopyLine(i, &tempLine);
			m_changed=true;
		}
		break;

	case modeSectors:
		MakeBackup("Paste Sector Properties", false, false, true, false);
		if (Selection.Size()==0)
		{
			map.CopySector(m_Selection, &m_ClipSector);
			m_changed=true;
		}
		else for(i=0;i<NumSectors();i++) if (checked[i])
		{
			map.CopySector(i, &m_ClipSector);
			m_changed=true;
		}
		break;
	}
	m_DrawWindow->Refresh();
	UpdateStatusBar();
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnUpdateCopyproperties(wxUpdateUIEvent & event)
{
	int max = m_Mode==modeThings? NumThings() : m_Mode==modeSectors? NumSectors() : NumLines();
	event.Enable(m_Mode!=modeVertexes && m_Selection>=0 && m_Selection<max && m_Mode!=modeLineDraw);
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnUpdatePasteproperties(wxUpdateUIEvent & event)
{
	int max = m_Mode==modeThings? NumThings() : m_Mode==modeSectors? NumSectors() : NumLines();
	event.Enable((Selection.Size()>=1 || (m_Selection>=0 && m_Selection<max)) && m_Mode!=modeVertexes && m_Mode!=modeLineDraw);
}
