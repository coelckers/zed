/*
** maploader.cpp
**
** Map loader for non-Blood maps
**
**---------------------------------------------------------------------------
** Copyright 2020 Christoph Oelckers
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/

#include "stdafx.h"
#include <stdint.h>
#include "buildtypes.h"
#include "files.h"
#include "templates.h"

int numsectors, numwalls, numsprites;
sectortype sector[4096];
walltype wall[16384];
spritetype sprite[16384];

static void ReadSectorV7(FileReader& fr, sectortype& sect)
{
	sect.wallptr = fr.ReadInt16();
	sect.wallnum = fr.ReadInt16();
	sect.ceilingz = fr.ReadInt32();
	sect.floorz = fr.ReadInt32();
	sect.ceilingstat = fr.ReadUInt16();
	sect.floorstat = fr.ReadUInt16();
	sect.ceilingpicnum = fr.ReadUInt16();
	sect.ceilingheinum = fr.ReadInt16();
	sect.ceilingshade = fr.ReadInt8();
	sect.ceilingpal = fr.ReadUInt8();
	sect.ceilingxpan = fr.ReadUInt8();
	sect.ceilingypan = fr.ReadUInt8();
	sect.floorpicnum = fr.ReadUInt16();
	sect.floorheinum = fr.ReadInt16();
	sect.floorshade = fr.ReadInt8();
	sect.floorpal = fr.ReadUInt8();
	sect.floorxpan = fr.ReadUInt8();
	sect.floorypan = fr.ReadUInt8();
	sect.visibility = fr.ReadUInt8();
	sect.fogpal = fr.ReadUInt8(); // note: currently unused, except for Blood.
	sect.lotag = fr.ReadInt16();
	sect.hitag = fr.ReadInt16();
	sect.extra = fr.ReadInt16();
}

static void ReadSectorV6(FileReader& fr, sectortype& sect)
{
	sect.wallptr = fr.ReadUInt16();
	sect.wallnum = fr.ReadUInt16();
	sect.ceilingpicnum = fr.ReadUInt16();
	sect.floorpicnum = fr.ReadUInt16();
	sect.ceilingheinum = clamp(fr.ReadInt16() << 5, -32768, 32767);
	sect.floorheinum = clamp(fr.ReadInt16() << 5, -32768, 32767);
	sect.ceilingz = fr.ReadInt32();
	sect.floorz = fr.ReadInt32();
	sect.ceilingshade = fr.ReadInt8();
	sect.floorshade = fr.ReadInt8();
	sect.ceilingxpan = fr.ReadUInt8();
	sect.floorxpan = fr.ReadUInt8();
	sect.ceilingypan = fr.ReadUInt8();
	sect.floorypan = fr.ReadUInt8();
	sect.ceilingstat = fr.ReadUInt8();
	sect.floorstat = fr.ReadUInt8();
	sect.ceilingpal = fr.ReadUInt8();
	sect.floorpal = fr.ReadUInt8();
	sect.visibility = fr.ReadUInt8();
	sect.lotag = fr.ReadInt16();
	sect.hitag = fr.ReadInt16();
	sect.extra = fr.ReadInt16();
}


static void ReadSectorV5(FileReader& fr, sectortype& sect)
{
	sect.wallptr = fr.ReadInt16();
	sect.wallnum = fr.ReadInt16();
	sect.ceilingpicnum = fr.ReadUInt16();
	sect.floorpicnum = fr.ReadUInt16();
	sect.ceilingheinum = clamp(fr.ReadInt16() << 5, -32768, 32767);
	sect.floorheinum = clamp(fr.ReadInt16() << 5, -32768, 32767);
	sect.ceilingz = fr.ReadInt32();
	sect.floorz = fr.ReadInt32();
	sect.ceilingshade = fr.ReadInt8();
	sect.floorshade = fr.ReadInt8();
	sect.ceilingxpan = fr.ReadUInt8();
	sect.floorxpan = fr.ReadUInt8();
	sect.ceilingypan = fr.ReadUInt8();
	sect.floorypan = fr.ReadUInt8();
	sect.ceilingstat = fr.ReadUInt8();
	sect.floorstat = fr.ReadUInt8();
	sect.ceilingpal = fr.ReadUInt8();
	sect.floorpal = fr.ReadUInt8();
	sect.visibility = fr.ReadUInt8();
	sect.lotag = fr.ReadInt16();
	sect.hitag = fr.ReadInt16();
	sect.extra = fr.ReadInt16();
	if ((sect.ceilingstat & 2) == 0) sect.ceilingheinum = 0;
	if ((sect.floorstat & 2) == 0) sect.floorheinum = 0;
}

static void ReadWallV7(FileReader& fr, walltype& wall)
{
	wall.x = fr.ReadInt32();
	wall.y = fr.ReadInt32();
	wall.point2 = fr.ReadInt16();
	wall.nextwall = fr.ReadInt16();
	wall.nextsector = fr.ReadInt16();
	wall.cstat = fr.ReadUInt16();
	wall.picnum = fr.ReadInt16();
	wall.overpicnum = fr.ReadInt16();
	wall.shade = fr.ReadInt8();
	wall.pal = fr.ReadUInt8();
	wall.xrepeat = fr.ReadUInt8();
	wall.yrepeat = fr.ReadUInt8();
	wall.xpan = fr.ReadUInt8();
	wall.ypan = fr.ReadUInt8();
	wall.lotag = fr.ReadInt16();
	wall.hitag = fr.ReadInt16();
	wall.extra = fr.ReadInt16();
}

static void ReadWallV6(FileReader& fr, walltype& wall)
{
	wall.x = fr.ReadInt32();
	wall.y = fr.ReadInt32();
	wall.point2 = fr.ReadInt16();
	wall.nextsector = fr.ReadInt16();
	wall.nextwall = fr.ReadInt16();
	wall.picnum = fr.ReadInt16();
	wall.overpicnum = fr.ReadInt16();
	wall.shade = fr.ReadInt8();
	wall.pal = fr.ReadUInt8();
	wall.cstat = fr.ReadUInt16();
	wall.xrepeat = fr.ReadUInt8();
	wall.yrepeat = fr.ReadUInt8();
	wall.xpan = fr.ReadUInt8();
	wall.ypan = fr.ReadUInt8();
	wall.lotag = fr.ReadInt16();
	wall.hitag = fr.ReadInt16();
	wall.extra = fr.ReadInt16();
}

static void ReadWallV5(FileReader& fr, walltype& wall)
{
	wall.x = fr.ReadInt32();
	wall.y = fr.ReadInt32();
	wall.point2 = fr.ReadInt16();
	wall.picnum = fr.ReadInt16();
	wall.overpicnum = fr.ReadInt16();
	wall.shade = fr.ReadInt8();
	wall.cstat = fr.ReadUInt16();
	wall.xrepeat = fr.ReadUInt8();
	wall.yrepeat = fr.ReadUInt8();
	wall.xpan = fr.ReadUInt8();
	wall.ypan = fr.ReadUInt8();

	wall.nextsector = fr.ReadInt16();
	wall.nextwall = fr.ReadInt16();
	fr.Seek(4, SEEK_SET); // skip over 2 unused 16 bit values

	wall.lotag = fr.ReadInt16();
	wall.hitag = fr.ReadInt16();
	wall.extra = fr.ReadInt16();
}

static void SetWallPalV5()
{
	for (int i = 0; i < numsectors; i++)
	{
		int startwall = sector[i].wallptr;
		int endwall = startwall + sector[i].wallnum;
		for (int w = startwall; w < endwall; w++)
		{
			wall[w].pal = sector[i].floorpal;
		}
	}
}


static void ReadSpriteV7(FileReader& fr, spritetype& spr)
{
	spr.x = fr.ReadInt32();
	spr.y = fr.ReadInt32();
	spr.z = fr.ReadInt32();
	spr.cstat = fr.ReadUInt16();
	spr.picnum = fr.ReadInt16();
	spr.shade = fr.ReadInt8();
	spr.pal = fr.ReadUInt8();
	spr.clipdist = fr.ReadUInt8();
	spr.blend = fr.ReadUInt8();
	spr.xrepeat = fr.ReadUInt8();
	spr.yrepeat = fr.ReadUInt8();
	spr.xoffset = fr.ReadInt8();
	spr.yoffset = fr.ReadInt8();
	spr.sectnum = fr.ReadInt16();
	spr.statnum = fr.ReadInt16();
	spr.ang = fr.ReadInt16();
	spr.owner = fr.ReadInt16();
	spr.xvel = fr.ReadInt16();
	spr.yvel = fr.ReadInt16();
	spr.zvel = fr.ReadInt16();
	spr.lotag = fr.ReadInt16();
	spr.hitag = fr.ReadInt16();
	spr.extra = fr.ReadInt16();
	spr.detail = 0;
}

static void ReadSpriteV6(FileReader& fr, spritetype& spr)
{
	spr.x = fr.ReadInt32();
	spr.y = fr.ReadInt32();
	spr.z = fr.ReadInt32();
	spr.cstat = fr.ReadUInt16();
	spr.shade = fr.ReadInt8();
	spr.pal = fr.ReadUInt8();
	spr.clipdist = fr.ReadUInt8();
	spr.xrepeat = fr.ReadUInt8();
	spr.yrepeat = fr.ReadUInt8();
	spr.xoffset = fr.ReadInt8();
	spr.yoffset = fr.ReadInt8();
	spr.picnum = fr.ReadInt16();
	spr.ang = fr.ReadInt16();
	spr.xvel = fr.ReadInt16();
	spr.yvel = fr.ReadInt16();
	spr.zvel = fr.ReadInt16();
	spr.owner = fr.ReadInt16();
	spr.sectnum = fr.ReadInt16();
	spr.statnum = fr.ReadInt16();
	spr.lotag = fr.ReadInt16();
	spr.hitag = fr.ReadInt16();
	spr.extra = fr.ReadInt16();
	spr.blend = 0;
	spr.detail = 0;
}

static void ReadSpriteV5(FileReader& fr, spritetype& spr)
{
	spr.x = fr.ReadInt32();
	spr.y = fr.ReadInt32();
	spr.z = fr.ReadInt32();
	spr.cstat = fr.ReadUInt16();
	spr.shade = fr.ReadInt8();
	spr.xrepeat = fr.ReadUInt8();
	spr.yrepeat = fr.ReadUInt8();
	spr.picnum = fr.ReadInt16();
	spr.ang = fr.ReadInt16();
	spr.xvel = fr.ReadInt16();
	spr.yvel = fr.ReadInt16();
	spr.zvel = fr.ReadInt16();
	spr.owner = fr.ReadInt16();
	spr.sectnum = fr.ReadInt16();
	spr.statnum = fr.ReadInt16();
	spr.lotag = fr.ReadInt16();
	spr.hitag = fr.ReadInt16();
	spr.extra = fr.ReadInt16();

	int sec = spr.sectnum;
	if ((sector[sec].ceilingstat & 1) > 0)
		spr.pal = sector[sec].ceilingpal;
	else
		spr.pal = sector[sec].floorpal;

	spr.blend = 0;
	spr.clipdist = 32;
	spr.xoffset = 0;
	spr.yoffset = 0;
	spr.detail = 0;
}




void engineLoadBoard(FileReader &fr, spritetype &startpos)
{
	numsectors = numsprites = numwalls = 0;
	int mapversion = fr.ReadInt32();
	if (mapversion < 5 || mapversion > 9) // 9 is most likely useless but let's try anyway.
	{
		//printf("Invalid map format, expcted 5-9, got %d", mapversion);
		//exit(1);
		return;
	}
	startpos = {};
	startpos.picnum = INT16_MIN;
	startpos.x = fr.ReadInt32();
	startpos.y = fr.ReadInt32();
	startpos.z = fr.ReadInt32();
	startpos.ang = fr.ReadInt16() & 2047;
	startpos.sectnum = fr.ReadUInt16();

	numsectors = fr.ReadUInt16();
	for (int i = 0; i < numsectors; i++)
	{
		switch (mapversion)
		{
		case 5: ReadSectorV5(fr, sector[i]); break;
		case 6: ReadSectorV6(fr, sector[i]); break;
		default: ReadSectorV7(fr, sector[i]); break;
		}
	}

	numwalls = fr.ReadUInt16();
	for (int i = 0; i < numwalls; i++)
	{
		switch (mapversion)
		{
		case 5: ReadWallV5(fr, wall[i]); break;
		case 6: ReadWallV6(fr, wall[i]); break;
		default: ReadWallV7(fr, wall[i]); break;
		}
	}

	numsprites = fr.ReadUInt16();
	for (int i = 0; i < numsprites; i++)
	{
		switch (mapversion)
		{
		case 5: ReadSpriteV5(fr, sprite[i]); break;
		case 6: ReadSpriteV6(fr, sprite[i]); break;
		default: ReadSpriteV7(fr, sprite[i]); break;
		}
	}
}
