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
// Undo code - with some improvements inspired by SLADE
//

#include "stdafx.h"
#include "ZEd.h"
#include "Level.h"

//==========================================================================
//
//
//
//==========================================================================

void CLevel::ChangeUndoMenu()
{
	ZEdApp & app = wxGetApp();
	ZEdFrame * frame = app.GetFrame();
	wxMenuBar * menu = frame->m_MainMenu;
	wxMenuItem * undo = menu->FindItem(CMD_UNDO);
	wxMenuItem * redo = menu->FindItem(CMD_REDO);

	if (m_undolist.Size())
	{
		undo->SetText(wxString::Format("Undo (%s)", m_undolist[m_undolist.Size()-1]->m_descript));
	}
	else 
	{
		undo->SetText("Undo");
	}

	if (m_redolist.Size())
	{
		redo->SetText(wxString::Format("Redo (%s)", m_redolist[m_redolist.Size()-1]->m_descript));
	}
	else 
	{
		redo->SetText("Redo");
	}
	RefreshAccelerator(CMD_UNDO);
	RefreshAccelerator(CMD_REDO);
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::MakeBackup(const char * action, bool vertices, bool lines, bool sectors, bool things)
{
	CUndoBuffer * ub = new CUndoBuffer(this, action, vertices, lines, sectors, things);
	CUndoBuffer * un;

	if (m_undolist.Size()>50) m_undolist.Delete(0);
	m_undolist.Push(ub);
	// Adding something to the undo stack invalidates the redo stack completely!
	while (m_redolist.Pop(un)) delete un;
	ChangeUndoMenu();
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::RemoveBackup()
{
	m_undolist.Delete(m_undolist.Size()-1);
	ChangeUndoMenu();
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnEditUndo(wxCommandEvent & event)
{
	CUndoBuffer * ub;

	if (LineDrawNotAllowed()) return;

	if (m_undolist.Size()>0)
	{
		m_undolist.Pop(ub);

		CUndoBuffer * redo = new CUndoBuffer(this, ub->m_descript, 
								ub->ChangeVertices(), ub->ChangeLines(), ub->ChangeSectors(), ub->ChangeThings());
		m_redolist.Push(redo);

		ub->Apply();
		delete ub;
		m_DrawWindow->Refresh();
		UpdateStatusBar();
		ChangeUndoMenu();
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnUpdateEditUndo(wxUpdateUIEvent & event)
{
	event.Enable(m_undolist.Size()!=0 && m_Mode!=modeLineDraw);
}


//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnEditRedo(wxCommandEvent & event)
{
	CUndoBuffer * ub;

	if (m_Mode==modeLineDraw) return;

	if (m_redolist.Size()>0)
	{
		m_redolist.Pop(ub);

		CUndoBuffer * redo = new CUndoBuffer(this, ub->m_descript, 
			ub->ChangeVertices(), ub->ChangeLines(), ub->ChangeSectors(), ub->ChangeThings());
		m_undolist.Push(redo);

		ub->Apply();
		delete ub;
		m_DrawWindow->Refresh();
		UpdateStatusBar();
		ChangeUndoMenu();
	}
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::OnUpdateEditRedo(wxUpdateUIEvent & event)
{
	event.Enable(m_redolist.Size()!=0 && m_Mode!=modeLineDraw);
}

//==========================================================================
//
//
//
//==========================================================================

void CLevel::Revert()
{
	if (m_undolist.Size()>0)
	{
		m_undolist[m_undolist.Size()-1]->Apply();
	}
}

