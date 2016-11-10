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
// Texture creation code
//

#include "stdafx.h"
#include "ResourceFile.h"
#include "texture.h"
#include "m_swap.h"
#include "zip/zlib/zlib.h"


extern unsigned char * playpal;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GLTexture::AddPatchToTexture(byte *buffer, const patch_t *patch, int originx, int originy)
{
	int x,y,j;
	int xs,xe;
	int js,je;
	column_t *p_bColumn_t;
	byte *p_bColumn;
	int pos;
	if (!patch || !patch->width || !patch->height)	return;

	xs=0;
	xe=LittleShort(patch->width);
	if ((xs+originx)>=width) return;
	if ((xe+originx)<=0) return;
	if ((xs+originx)<0) xs=-originx;
	if ((xe+originx)>width) xe+=(width-(xe+originx));
	for (x=xs;x<xe;x++)
	{
		int index=LittleLong(patch->columnofs[x])&0xffffff;
		p_bColumn_t=(column_t *)((byte *)patch+index);
		while (p_bColumn_t->topdelta != 0xff)
		{

			/*
			if (p_bColumn_t->topdelta <= top)
			{
				top += p_bColumn_t->topdelta;
			}
			else
			{
				top = p_bColumn_t->topdelta;
			}
			*/

			y=(p_bColumn_t->topdelta+originy);
			js=0;
			je=p_bColumn_t->length;
			if ((js+y)>=height) goto nextrun;
			if ((je+y)<=0) goto nextrun;
			if ((js+y)<0) js=-y;
			if ((je+y)>height) je+=(height-(je+y));
			p_bColumn=(byte *)p_bColumn_t + 3;
			pos=3*(((js+y)*width)+x+originx);
			for (j=js;j<je;j++,pos+=(3*width))
			{	
				if ((pos+2)>=buffer_size)
				{
					return;
				}
				memcpy(buffer+pos, &playpal[p_bColumn[j]*3], 3);
			}
nextrun:
			p_bColumn_t = (column_t *)(  (byte *)p_bColumn_t + p_bColumn_t->length + 4);
		}
	}
}


void GLTexture::AddRawToTexture(byte * buffer, wxImage & patch, int originx, int originy)
{
	int x,y,pos;
	int srcwidth=patch.GetWidth();
	int srcheight=patch.GetHeight();
	int w=srcwidth;
	unsigned char * pixels = patch.GetData();
	unsigned char * alpha = patch.GetAlpha();
	
	if (originx<0)
	{
		srcwidth+=originx;
		pixels-=3*originx;
		originx=0;
		if (srcwidth<=0) return;
	}
	if (originx+srcwidth>width)
	{
		srcwidth=width-originx;
		if (srcwidth<=0) return;
	}
		
	if (originy<0)
	{
		srcheight+=originy;
		pixels-=3*originy*w;
		originy=0;
		if (srcheight<=0) return;
	}
	if (originy+srcheight>height)
	{
		srcheight=height-originy;
		if (srcheight<=0) return;
	}
	buffer+=3*originx + 3*width*originy;
		
	for (y=0;y<srcheight;y++)
	{
		pos=3*(y*width);
		for (x=0;x<srcwidth;x++,pos+=3)
		{
			if (!alpha || alpha[y*w+x])
			{
				memcpy(buffer+pos, pixels +(y*w+x)*3, 3);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GLTexture::AddFlatToTexture(unsigned char *buffer, const unsigned char *flat,int pitch)
{
	int x,y,pos;
	unsigned int palindex=0;
	
	if (!flat) return;

	for (y=0;y<height;y++)
	{
		pos=3*(y*width);
		for (x=0;x<width;x++,pos+=3)
		{
			if (pos>=buffer_size)
			{
				return;
			}
			int v=flat[y*pitch+x];
			memcpy(buffer+pos, &playpal[v*3], 3);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GLTexture::TexInit(int _width,int _height)
{

	width=_width;
	height=_height;
	buffer_size=width*height*3;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
wxBitmap * GLTexture::CreateTexBuffer()
{
	unsigned char * buffer = CreateBuffer();
	for (int i=0; i<texd->patchcount; i++)
	{
		char * texdata = (char*)CResourceFile::GlobalReadLump(texd->patches[i].patch);
		if (!memcmp(texdata,"\x89PNG",4) || !memcmp(texdata+6, "JFIF", 4))
		{
			int len = CResourceFile::GlobalGetLumpSize(texd->patches[i].patch);
			wxImage img(wxMemoryInputStream(texdata, len), wxBITMAP_TYPE_ANY);

			AddRawToTexture(buffer, img, texd->patches[i].originx,texd->patches[i].originy);
		}
		else if (texd->prio!=tx_flat)
		{
			AddPatchToTexture(buffer,(patch_t*)texdata,texd->patches[i].originx,texd->patches[i].originy);
		}
		else
		{
			AddFlatToTexture(buffer,(const unsigned char *)texdata,width);
		}
	}
	image = new wxBitmap(wxImage(width, height, buffer, true));
	free(buffer);

	return image;
}

