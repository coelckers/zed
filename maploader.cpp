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
XSECTOR xsector[4096];
XWALL xwall[16384];
XSPRITE xsprite[16384];

class BitReader {
public:
	int nBitPos;
	int nSize;
	uint8_t* pBuffer;
	BitReader(uint8_t* _pBuffer, int _nSize, int _nBitPos) { pBuffer = _pBuffer; nSize = _nSize; nBitPos = _nBitPos; nSize -= nBitPos >> 3; }
	BitReader(uint8_t* _pBuffer, int _nSize) { pBuffer = _pBuffer; nSize = _nSize; nBitPos = 0; }
	int readBit()
	{
		if (nSize <= 0)
			return 0;
		int bit = ((*pBuffer) >> nBitPos) & 1;
		if (++nBitPos >= 8)
		{
			nBitPos = 0;
			pBuffer++;
			nSize--;
		}
		return bit;
	}
	void skipBits(int nBits)
	{
		nBitPos += nBits;
		pBuffer += nBitPos >> 3;
		nSize -= nBitPos >> 3;
		nBitPos &= 7;
	}
	unsigned int readUnsigned(int nBits)
	{
		unsigned int n = 0;
		for (int i = 0; i < nBits; i++)
			n += readBit() << i;
		return n;
	}
	int readSigned(int nBits)
	{
		int n = (int)readUnsigned(nBits);
		n <<= 32 - nBits;
		n >>= 32 - nBits;
		return n;
	}
};



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

static void dbCrypt(char* pPtr, int nLength, int nKey)
{
    for (int i = 0; i < nLength; i++)
    {
        pPtr[i] = pPtr[i] ^ nKey;
        nKey++;
    }
}


void loadBloodMap(FileReader &fr, spritetype& startpos, bool encrypted)
{
    int16_t tpskyoff[256];

    MAPHEADER mapHeader;
    fr.Read(&mapHeader, 37/* sizeof(mapHeader)*/);
    if (mapHeader.mattid != 0 && mapHeader.mattid != 0x7474614d && mapHeader.mattid != 0x4d617474) {
        dbCrypt((char*)&mapHeader, sizeof(mapHeader), 0x7474614d);
    }

    mapHeader.x = LittleLong(mapHeader.x);
    mapHeader.y = LittleLong(mapHeader.y);
    mapHeader.z = LittleLong(mapHeader.z);
    mapHeader.ang = LittleShort(mapHeader.ang);
    mapHeader.sect = LittleShort(mapHeader.sect);
    mapHeader.pskybits = LittleShort(mapHeader.pskybits);
    mapHeader.visibility = LittleLong(mapHeader.visibility);
    mapHeader.mattid = LittleLong(mapHeader.mattid);
    mapHeader.revision = LittleLong(mapHeader.revision);
    mapHeader.numsectors = LittleShort(mapHeader.numsectors);
    mapHeader.numwalls = LittleShort(mapHeader.numwalls);
    mapHeader.numsprites = LittleShort(mapHeader.numsprites);

    startpos.x = mapHeader.x;
    startpos.y = mapHeader.y;
    startpos.z = mapHeader.z;
    startpos.ang = mapHeader.ang;
    startpos.sectnum = mapHeader.sect;
    //gVisibility = g_visibility = mapHeader.visibility;
    int gMattId = mapHeader.mattid;
    //parallaxtype = mapHeader.parallax;
    int gMapRev = mapHeader.revision;
    numsectors = mapHeader.numsectors;
    numwalls = mapHeader.numwalls;
    numsprites = mapHeader.numsprites;
    //dbInit();
    MAPHEADER2 mh2;
    if (encrypted)
    {
        fr.Read(&mh2, 128);
        dbCrypt((char*)&mh2, 128, numwalls);
    }
    else
    {
        mh2 = {};
    }
    int gSkyCount = 1 << mapHeader.pskybits;
    fr.Seek(gSkyCount * sizeof(tpskyoff[0]), SEEK_CUR);
    /*
    fr.Read(tpskyoff, gSkyCount * sizeof(tpskyoff[0]));
    if (encrypted)
    {
        dbCrypt((char*)tpskyoff, gSkyCount * sizeof(tpskyoff[0]), gSkyCount * 2);
    }
    */

    for (int i = 0; i < numsectors; i++)
    {
        sectortype* pSector = &sector[i];
        sectortypedisk load;
        fr.Read(&load, sizeof(sectortypedisk));
        if (encrypted)
        {
            dbCrypt((char*)&load, sizeof(sectortypedisk), gMapRev * sizeof(sectortypedisk));
        }
        pSector->wallptr = LittleShort(load.wallptr);
        pSector->wallnum = LittleShort(load.wallnum);
        pSector->ceilingz = LittleLong(load.ceilingz);
        pSector->floorz = LittleLong(load.floorz);
        pSector->ceilingstat = LittleShort(load.ceilingstat);
        pSector->floorstat = LittleShort(load.floorstat);
        pSector->ceilingpicnum = LittleShort(load.ceilingpicnum);
        pSector->ceilingheinum = LittleShort(load.ceilingheinum);
        pSector->floorpicnum = LittleShort(load.floorpicnum);
        pSector->floorheinum = LittleShort(load.floorheinum);
        pSector->lotag = LittleShort(load.type);
        pSector->hitag = LittleShort(load.hitag);
        pSector->extra = LittleShort(load.extra);
        pSector->ceilingshade = load.ceilingshade;
        pSector->ceilingpal = load.ceilingpal;
        pSector->ceilingxpan = load.ceilingxpanning;
        pSector->ceilingypan = load.ceilingypanning;
        pSector->floorshade = load.floorshade;
        pSector->floorpal = load.floorpal;
        pSector->floorxpan = load.floorxpanning;
        pSector->floorypan = load.floorypanning;
        pSector->visibility = load.visibility;
        //qsector_filler[i] = load.fogpal;
        pSector->fogpal = 0;

        if (sector[i].extra > 0)
        {
            uint8_t pBuffer[nXSectorSize];
            XSECTOR* pXSector = &xsector[i];
            memset(pXSector, 0, sizeof(XSECTOR));
            int nCount;
            if (!encrypted)
            {
                nCount = nXSectorSize;
            }
            else
            {
                nCount = mh2.numxsectors;
            }
            assert(nCount <= nXSectorSize);
            fr.Read(pBuffer, nCount);
            BitReader bitReader(pBuffer, nCount);
            pXSector->reference = bitReader.readSigned(14);
            pXSector->state = bitReader.readUnsigned(1);
            pXSector->busy = bitReader.readUnsigned(17);
            pXSector->data = bitReader.readUnsigned(16);
            pXSector->txID = bitReader.readUnsigned(10);
            pXSector->busyWaveA = bitReader.readUnsigned(3);
            pXSector->busyWaveB = bitReader.readUnsigned(3);
            pXSector->rxID = bitReader.readUnsigned(10);
            pXSector->command = bitReader.readUnsigned(8);
            pXSector->triggerOn = bitReader.readUnsigned(1);
            pXSector->triggerOff = bitReader.readUnsigned(1);
            pXSector->busyTimeA = bitReader.readUnsigned(12);
            pXSector->waitTimeA = bitReader.readUnsigned(12);
            pXSector->restState = bitReader.readUnsigned(1);
            pXSector->interruptable = bitReader.readUnsigned(1);
            pXSector->amplitude = bitReader.readSigned(8);
            pXSector->freq = bitReader.readUnsigned(8);
            pXSector->reTriggerA = bitReader.readUnsigned(1);
            pXSector->reTriggerB = bitReader.readUnsigned(1);
            pXSector->phase = bitReader.readUnsigned(8);
            pXSector->wave = bitReader.readUnsigned(4);
            pXSector->shadeAlways = bitReader.readUnsigned(1);
            pXSector->shadeFloor = bitReader.readUnsigned(1);
            pXSector->shadeCeiling = bitReader.readUnsigned(1);
            pXSector->shadeWalls = bitReader.readUnsigned(1);
            pXSector->shade = bitReader.readSigned(8);
            pXSector->panAlways = bitReader.readUnsigned(1);
            pXSector->panFloor = bitReader.readUnsigned(1);
            pXSector->panCeiling = bitReader.readUnsigned(1);
            pXSector->Drag = bitReader.readUnsigned(1);
            pXSector->Underwater = bitReader.readUnsigned(1);
            pXSector->Depth = bitReader.readUnsigned(3);
            pXSector->panVel = bitReader.readUnsigned(8);
            pXSector->panAngle = bitReader.readUnsigned(11);
            bitReader.readUnsigned(1);
            pXSector->decoupled = bitReader.readUnsigned(1);
            pXSector->triggerOnce = bitReader.readUnsigned(1);
            pXSector->isTriggered = bitReader.readUnsigned(1);
            pXSector->Key = bitReader.readUnsigned(3);
            pXSector->Push = bitReader.readUnsigned(1);
            pXSector->Vector = bitReader.readUnsigned(1);
            pXSector->Reserved = bitReader.readUnsigned(1);
            pXSector->Enter = bitReader.readUnsigned(1);
            pXSector->Exit = bitReader.readUnsigned(1);
            pXSector->Wallpush = bitReader.readUnsigned(1);
            pXSector->color = bitReader.readUnsigned(1);
            bitReader.readUnsigned(1);
            pXSector->busyTimeB = bitReader.readUnsigned(12);
            pXSector->waitTimeB = bitReader.readUnsigned(12);
            pXSector->stopOn = bitReader.readUnsigned(1);
            pXSector->stopOff = bitReader.readUnsigned(1);
            pXSector->ceilpal = bitReader.readUnsigned(4);
            pXSector->offCeilZ = bitReader.readSigned(32);
            pXSector->onCeilZ = bitReader.readSigned(32);
            pXSector->offFloorZ = bitReader.readSigned(32);
            pXSector->onFloorZ = bitReader.readSigned(32);
            pXSector->marker0 = bitReader.readUnsigned(16);
            pXSector->marker1 = bitReader.readUnsigned(16);
            pXSector->Crush = bitReader.readUnsigned(1);
            pSector->ceilingxpan += bitReader.readUnsigned(8) / 256.f;
            pSector->ceilingypan += bitReader.readUnsigned(8) / 256.f;
            pSector->floorxpan += bitReader.readUnsigned(8) / 256.f;
            pXSector->damageType = bitReader.readUnsigned(3);
            pXSector->floorpal = bitReader.readUnsigned(4);
            pSector->floorypan += bitReader.readUnsigned(8) / 256.f;
            pXSector->locked = bitReader.readUnsigned(1);
            pXSector->windVel = bitReader.readUnsigned(10);
            pXSector->windAng = bitReader.readUnsigned(11);
            pXSector->windAlways = bitReader.readUnsigned(1);
            pXSector->dudeLockout = bitReader.readUnsigned(1);
            pXSector->bobTheta = bitReader.readUnsigned(11);
            pXSector->bobZRange = bitReader.readUnsigned(5);
            pXSector->bobSpeed = bitReader.readSigned(12);
            pXSector->bobAlways = bitReader.readUnsigned(1);
            pXSector->bobFloor = bitReader.readUnsigned(1);
            pXSector->bobCeiling = bitReader.readUnsigned(1);
            pXSector->bobRotate = bitReader.readUnsigned(1);
            pXSector->reference = i;
            pXSector->busy = pXSector->state << 16;

        }
    }
    for (int i = 0; i < numwalls; i++)
    {
        walltype* pWall = &wall[i];
        walltypedisk load;
        fr.Read(&load, sizeof(walltypedisk));
        if (encrypted)
        {
            dbCrypt((char*)&load, sizeof(walltypedisk), (gMapRev * sizeof(sectortypedisk)) | 0x7474614d);
        }
        pWall->x = LittleLong(load.x);
        pWall->y = LittleLong(load.y);
        pWall->point2 = LittleShort(load.point2);
        pWall->nextwall = LittleShort(load.nextwall);
        pWall->nextsector = LittleShort(load.nextsector);
        pWall->cstat = LittleShort(load.cstat);
        pWall->picnum = LittleShort(load.picnum);
        pWall->overpicnum = LittleShort(load.overpicnum);
        pWall->lotag = LittleShort(load.type);
        pWall->hitag = LittleShort(load.hitag);
        pWall->extra = LittleShort(load.extra);
        pWall->shade = load.shade;
        pWall->pal = load.pal;
        pWall->xrepeat = load.xrepeat;
        pWall->xpan = load.xpanning;
        pWall->yrepeat = load.yrepeat;
        pWall->ypan = load.ypanning;

        if (wall[i].extra > 0)
        {
            uint8_t pBuffer[nXWallSize];
            XWALL* pXWall = &xwall[i];
            memset(pXWall, 0, sizeof(XWALL));
            int nCount;
            if (!encrypted)
            {
                nCount = nXWallSize;
            }
            else
            {
                nCount = mh2.numxwalls;
            }
            assert(nCount <= nXWallSize);
            fr.Read(pBuffer, nCount);
            BitReader bitReader(pBuffer, nCount);
            pXWall->reference = bitReader.readSigned(14);
            pXWall->state = bitReader.readUnsigned(1);
            pXWall->busy = bitReader.readUnsigned(17);
            pXWall->data = bitReader.readSigned(16);
            pXWall->txID = bitReader.readUnsigned(10);
            bitReader.readUnsigned(6);
            pXWall->rxID = bitReader.readUnsigned(10);
            pXWall->command = bitReader.readUnsigned(8);
            pXWall->triggerOn = bitReader.readUnsigned(1);
            pXWall->triggerOff = bitReader.readUnsigned(1);
            pXWall->busyTime = bitReader.readUnsigned(12);
            pXWall->waitTime = bitReader.readUnsigned(12);
            pXWall->restState = bitReader.readUnsigned(1);
            pXWall->interruptable = bitReader.readUnsigned(1);
            pXWall->panAlways = bitReader.readUnsigned(1);
            pXWall->panXVel = bitReader.readSigned(8);
            pXWall->panYVel = bitReader.readSigned(8);
            pXWall->decoupled = bitReader.readUnsigned(1);
            pXWall->triggerOnce = bitReader.readUnsigned(1);
            pXWall->isTriggered = bitReader.readUnsigned(1);
            pXWall->key = bitReader.readUnsigned(3);
            pXWall->triggerPush = bitReader.readUnsigned(1);
            pXWall->triggerVector = bitReader.readUnsigned(1);
            pXWall->triggerTouch = bitReader.readUnsigned(1);
            bitReader.readUnsigned(2);
            pWall->xpan += bitReader.readUnsigned(8) / 256.f;
            pWall->ypan += bitReader.readUnsigned(8) / 256.f;
            pXWall->locked = bitReader.readUnsigned(1);
            pXWall->dudeLockout = bitReader.readUnsigned(1);
            bitReader.readUnsigned(4);
            bitReader.readUnsigned(32);
            pXWall->reference = i;
            pXWall->busy = pXWall->state << 16;

        }
    }
    for (int i = 0; i < mapHeader.numsprites; i++)
    {
        spritetypedisk load;
        spritetype* pSprite = &sprite[i];
        fr.Read(&load, sizeof(spritetypedisk)); // load into an intermediate buffer so that spritetype is no longer bound by file formats.
        if (encrypted) // What were these people thinking? :(
        {
            dbCrypt((char*)&load, sizeof(spritetypedisk), (gMapRev * sizeof(spritetypedisk)) | 0x7474614d);
        }

        pSprite->x = LittleLong(load.x);
        pSprite->y = LittleLong(load.y);
        pSprite->z = LittleLong(load.z);
        pSprite->cstat = LittleShort(load.cstat);
        pSprite->lotag = LittleShort(load.picnum); // swapped
        pSprite->sectnum = LittleShort(load.sectnum);
        pSprite->statnum = LittleShort(load.statnum);
        pSprite->ang = LittleShort(load.ang);
        pSprite->owner = LittleShort(load.owner);
        pSprite->xvel = LittleShort(load.index);
        pSprite->yvel = LittleShort(load.yvel);
        pSprite->zvel = LittleShort(load.inittype);
        pSprite->picnum = LittleShort(load.type);   // swapped - because type is more important.
        pSprite->hitag = LittleShort(load.hitag);
        pSprite->extra = LittleShort(load.extra);
        pSprite->pal = load.pal;
        pSprite->clipdist = load.clipdist;
        pSprite->xrepeat = load.xrepeat;
        pSprite->yrepeat = load.yrepeat;
        pSprite->xoffset = load.xoffset;
        pSprite->yoffset = load.yoffset;
        pSprite->detail = load.detail;
        pSprite->shade = load.shade;
        pSprite->blend = 0;
        if (pSprite->extra > 0)
        {
            uint8_t pBuffer[nXSpriteSize];
            XSPRITE* pXSprite = &xsprite[i];
            memset(pXSprite, 0, sizeof(XSPRITE));
            int nCount;
            if (!encrypted)
            {
                nCount = nXSpriteSize;
            }
            else
            {
                nCount = mh2.numxsprites;
            }
            assert(nCount <= nXSpriteSize);
            fr.Read(pBuffer, nCount);
            BitReader bitReader(pBuffer, nCount);
            pXSprite->reference = bitReader.readSigned(14);
            pXSprite->state = bitReader.readUnsigned(1);
            pXSprite->busy = bitReader.readUnsigned(17);
            pXSprite->txID = bitReader.readUnsigned(10);
            pXSprite->rxID = bitReader.readUnsigned(10);
            pXSprite->command = bitReader.readUnsigned(8);
            pXSprite->triggerOn = bitReader.readUnsigned(1);
            pXSprite->triggerOff = bitReader.readUnsigned(1);
            pXSprite->wave = bitReader.readUnsigned(2);
            pXSprite->busyTime = bitReader.readUnsigned(12);
            pXSprite->waitTime = bitReader.readUnsigned(12);
            pXSprite->restState = bitReader.readUnsigned(1);
            pXSprite->Interrutable = bitReader.readUnsigned(1);
            bitReader.readUnsigned(2);
            pXSprite->respawnPending = bitReader.readUnsigned(2);
            bitReader.readUnsigned(1);
            pXSprite->lT = bitReader.readUnsigned(1);
            pXSprite->dropMsg = bitReader.readUnsigned(8);
            pXSprite->Decoupled = bitReader.readUnsigned(1);
            pXSprite->triggerOnce = bitReader.readUnsigned(1);
            pXSprite->isTriggered = bitReader.readUnsigned(1);
            pXSprite->key = bitReader.readUnsigned(3);
            pXSprite->Push = bitReader.readUnsigned(1);
            pXSprite->Vector = bitReader.readUnsigned(1);
            pXSprite->Impact = bitReader.readUnsigned(1);
            pXSprite->Pickup = bitReader.readUnsigned(1);
            pXSprite->Touch = bitReader.readUnsigned(1);
            pXSprite->Sight = bitReader.readUnsigned(1);
            pXSprite->Proximity = bitReader.readUnsigned(1);
            bitReader.readUnsigned(2);
            pXSprite->lSkill = bitReader.readUnsigned(5);
            pXSprite->lS = bitReader.readUnsigned(1);
            pXSprite->lB = bitReader.readUnsigned(1);
            pXSprite->lC = bitReader.readUnsigned(1);
            pXSprite->DudeLockout = bitReader.readUnsigned(1);
            pXSprite->data1 = bitReader.readSigned(16);
            pXSprite->data2 = bitReader.readSigned(16);
            pXSprite->data3 = bitReader.readSigned(16);
            pXSprite->goalAng = bitReader.readUnsigned(11);
            pXSprite->dodgeDir = bitReader.readSigned(2);
            pXSprite->locked = bitReader.readUnsigned(1);
            pXSprite->medium = bitReader.readUnsigned(2);
            pXSprite->respawn = bitReader.readUnsigned(2);
            pXSprite->data4 = bitReader.readUnsigned(16);
            bitReader.readUnsigned(6);
            pXSprite->lockMsg = bitReader.readUnsigned(8);
            pXSprite->health = bitReader.readUnsigned(12);
            pXSprite->dudeDeaf = bitReader.readUnsigned(1);
            pXSprite->dudeAmbush = bitReader.readUnsigned(1);
            pXSprite->dudeGuard = bitReader.readUnsigned(1);
            pXSprite->dudeFlag4 = bitReader.readUnsigned(1);
            pXSprite->target = bitReader.readSigned(16);
            pXSprite->targetX = bitReader.readSigned(32);
            pXSprite->targetY = bitReader.readSigned(32);
            pXSprite->targetZ = bitReader.readSigned(32);
            pXSprite->burnTime = bitReader.readUnsigned(16);
            pXSprite->burnSource = bitReader.readSigned(16);
            pXSprite->height = bitReader.readUnsigned(16);
            pXSprite->stateTimer = bitReader.readUnsigned(16);
            bitReader.skipBits(32);
            pXSprite->reference = i;
            pXSprite->busy = pXSprite->state << 16;
            if (!encrypted) {
                pXSprite->lT |= pXSprite->lB;
            }

        }
    }

    /*
    if ((header.version & 0xff00) == 0x600)
    {
        switch (header.version & 0xff)
        {
        case 0:
            for (int i = 0; i < numsectors; i++)
            {
                sectortype* pSector = &sector[i];
                if (pSector->extra > 0)
                {
                    XSECTOR* pXSector = &xsector[pSector->extra];
                    pXSector->busyTimeB = pXSector->busyTimeA;
                    if (pXSector->busyTimeA > 0)
                    {
                        if (!pXSector->restState)
                        {
                            pXSector->reTriggerA = 1;
                        }
                        else
                        {
                            pXSector->waitTimeB = pXSector->busyTimeA;
                            pXSector->waitTimeA = 0;
                            pXSector->reTriggerB = 1;
                        }
                    }
                }
            }
            fallthrough__;
        case 1:
            for (int i = 0; i < numsectors; i++)
            {
                sectortype* pSector = &sector[i];
                if (pSector->extra > 0)
                {
                    XSECTOR* pXSector = &xsector[pSector->extra];
                    pXSector->freq >>= 1;
                }
            }
            fallthrough__;
        case 2:
            for (int i = 0; i < kMaxSprites; i++)
            {
            }
            break;

        }
    }
    */
}



void engineLoadBoard(FileReader &fr, spritetype &startpos)
{
	numsectors = numsprites = numwalls = 0;
	int mapversion = fr.ReadInt32();
	if (mapversion < 5 || mapversion > 9) // 9 is most likely useless but let's try anyway.
	{
        if (mapversion == MAKE_ID('B', 'L', 'M', 0x1a))
        {
            mapversion = fr.ReadInt16();
            bool encrypted = ((mapversion & 0xff00) == 0x700);
            loadBloodMap(fr, startpos, encrypted);
            return;
        }
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
