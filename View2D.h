#ifndef __VIEW2D_H
#define __VIEW2D_H

class CLevel;


class C2DMapview : public wxWindow
{
	CLevel * m_Level;

public:
	C2DMapview(wxWindow * parent);
	~C2DMapview();

	bool Detach(bool force);
	void Attach(CLevel * level);
	void OnPaint(wxPaintEvent& event);
	void SetScrollBar(bool setcenter);
	void OnScroll(wxScrollWinEvent & event);
	void OnKeyDown(wxKeyEvent & event);
	CLevel * GetLevel() const { return m_Level; }

	void OnLButtonDown(wxMouseEvent & event);
	void OnMouseMove(wxMouseEvent & event);
	void OnLButtonUp(wxMouseEvent & event);
	void OnRButtonDown(wxMouseEvent & event);

	void Refresh()
	{
		wxWindow::Refresh(false);
	}

    DECLARE_EVENT_TABLE()
};


#endif