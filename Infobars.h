
#ifndef __INFOBARS_H__
#define __INFOBARS_H__

#include "texture.h"

class ThingBar : public wxPanel
{
private:
	wxStaticBox		*m_frame;

	wxStaticText	*m_Items[2][7];

	TextureBox		*m_sprite;

public:
	ThingBar(wxWindow *parent);

	void Update(int num, CThing * th, bool textmap);
};

class VertexBar : public wxPanel
{
private:
	wxStaticBox		*m_frame;
	wxStaticText	*m_x;
	wxStaticText	*m_y;

public:
	VertexBar(wxWindow *parent);

	void Update(int num, CVertex * vt);
};

class LineBar : public wxPanel
{
private:
	wxStaticBox		* m_lineframe;	// This contains the linedef number
	wxStaticText	* m_Items[3][7];
	/*
	wxStaticText	* m_length;
	wxStaticText	* m_vertices;
	wxStaticText	* m_flags;
	wxStaticText	* m_special;
	wxStaticText	* m_tag;
	wxStaticText	* m_offset[2];

	wxStaticText	*m_arg[5];
	*/

	wxStaticBox		*m_sideframe[2];
	wxStaticText	*m_texname[2][3];
	TextureBox		*m_texture[2][3];

public:
	LineBar(wxWindow *parent);

	void Update(int num, CLevel * lev, CLine * ln);
};

class SectorBar : public wxPanel
{
private:
	wxStaticBox		*m_frame;
	wxStaticText	*m_floorheight;
	wxStaticText	*m_ceilingheight;
	wxStaticText	*m_height;
	wxStaticText	*m_lightlevel;
	wxStaticText	*m_type;
	wxStaticText	*m_tag;

	wxStaticText	*m_texname[2];
	TextureBox		*m_texture[2];

public:
	SectorBar(wxWindow *parent);

	void Update(int num, CSector * sec);
};

#endif