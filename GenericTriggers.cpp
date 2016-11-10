//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2000-2005 Christoph Oelckers
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
// Code that generates meaningful description for Boom's generalized linedef types
//

#include "stdafx.h"
#include "GameConfig.h"
#include "GenericTriggers.h"

static char * Tx_Trigger[]={"W1","WR","S1","SR","G1","GR","D1","DR"};
static char * Tx_Speed[]={"slow","normal","fast","turbo"};
static char * Tx_Model[]={"trigger","neighboring sector"};
static char * Tx_Monster[]={"",", Monster"};
static char * Tx_Direction[]={"Lower","Raise"};
static char * Tx_TargetF[]={"to highest neighboring floor","to lowest neighboring floor","to next neighboring floor",
							"to lowest neighboring ceiling","to ceiling","to shortest lower texture","24","32"};
static char * Tx_TargetC[]={"to highest neighboring ceiling","to lowest neighboring ceiling","to next neighboring ceiling",
							"to highest neighboring floor","to floor","to shortest lower texture","24","32"};
static char * Tx_Change1[]={"",", change texture"," and change texture"," and change texture and type"};
static char * Tx_Change2[]={""," and reset type","",""};
static char * Tx_Crush[]={"",", Crusher"};

static char * Tx_Doortype[]={"[%05d] %s Open Door (closes in %d seconds, %s%s)","[%05d] %s Open Door (stays open, %s%s)",
							 "[%05d] %s Close Door (opens in %d seconds, %s%s)","[%05d] %s Close Door (stays closed, %s%s)" };
static int Num_Doordelay[]={1,4,9,30};

static int Num_Lock[2][8]={{100,1,2,3,4,5,6,101},{100,129,130,131,132,133,134,229}};

static char * Tx_Locktype[]={"closes after 4 seconds","stays open"};

static int Num_Liftdelay[]={1,3,5,10};
static char * Tx_Lifttarget[]={"lowest neighboring floor","next neighboring floor","lowest neighboring ceiling"};

static int Num_Stairstep[]={4,8,16,24};
static char * Tx_Tex[]={"",", ignore texture"};
static char * Tx_Noise[]={"","silent "};


const char * GetGenTrigger(int v)
{
	return Tx_Trigger[GET(v,TRIGGER)];
}


int GetGenLock(int v)
{
	if (v>=LOCK_BASE && v<LOCK_BASE+LOCK_COUNT) return Num_Lock[GET(v,LOCKSISK)][GET(v,LOCKLOCK)];
	else return -1;
}

wxString GetGenText(int v)
{
	if (v>=FLOOR_BASE && v<FLOOR_BASE+FLOOR_COUNT)
	{
		if (GET(v,CHANGE)) return wxString::Format("[%05d] %s %s Floor %s%s to %s%s (%s%s)",
			v,
			Tx_Trigger[GET(v,TRIGGER)],
			Tx_Direction[GET(v,DIRECTION)],
			Tx_TargetF[GET(v,TARGET)],
			Tx_Change1[GET(v,CHANGE)],
			Tx_Model[GET(v,MODEL)],
			Tx_Change2[GET(v,CHANGE)],
			Tx_Speed[GET(v,SPEED)],
			Tx_Crush[GET(v,CRUSH)]);
		else return wxString::Format("[%05d] %s %s Floor %s (%s%s%s)",
			v,
			Tx_Trigger[GET(v,TRIGGER)],
			Tx_Direction[GET(v,DIRECTION)],
			Tx_TargetF[GET(v,TARGET)],
			Tx_Speed[GET(v,SPEED)],
			Tx_Monster[GET(v,MODEL)],
			Tx_Crush[GET(v,CRUSH)]);
	}
	else if (v>=CEILING_BASE && v<CEILING_BASE+CEILING_COUNT)
	{
		if (GET(v,CHANGE)) return wxString::Format("[%05d] %s %s Ceiling %s%s to %s%s (%s%s)",
			v,
			Tx_Trigger[GET(v,TRIGGER)],
			Tx_Direction[GET(v,DIRECTION)],
			Tx_TargetC[GET(v,TARGET)],
			Tx_Change1[GET(v,CHANGE)],
			Tx_Model[GET(v,MODEL)],
			Tx_Change2[GET(v,CHANGE)],
			Tx_Speed[GET(v,SPEED)],
			Tx_Crush[GET(v,CRUSH)]);
		else return wxString::Format("[%05d] %s %s Ceiling %s (%s%s%s)",
			v,
			Tx_Trigger[GET(v,TRIGGER)],
			Tx_Direction[GET(v,DIRECTION)],
			Tx_TargetC[GET(v,TARGET)],
			Tx_Speed[GET(v,SPEED)],
			Tx_Monster[GET(v,MODEL)],
			Tx_Crush[GET(v,CRUSH)]);
	}
	else if (v>=DOOR_BASE && v<DOOR_BASE+DOOR_COUNT)
	{
		if ((GET(v,DOORTYPE)&1)==0) return wxString::Format(Tx_Doortype[GET(v,DOORTYPE)],
			v,
			Tx_Trigger[GET(v,TRIGGER)],
			Num_Doordelay[GET(v,DOORDELAY)],
			Tx_Speed[GET(v,SPEED)],
			Tx_Monster[GET(v,DOORMONSTER)]);
		else return wxString::Format(Tx_Doortype[GET(v,DOORTYPE)],
			v,
			Tx_Trigger[GET(v,TRIGGER)],
			Tx_Speed[GET(v,SPEED)],
			Tx_Monster[GET(v,DOORMONSTER)]);
	} 
	else if (v>=LOCK_BASE && v<LOCK_BASE+LOCK_COUNT)
	{
		Lock * lock=cgc->LockMap[Num_Lock[GET(v,LOCKSISK)][GET(v,LOCKLOCK)]];

		return wxString::Format("[%05d] %s Open Door (%s, %s), %s",
			v,
			Tx_Trigger[GET(v,TRIGGER)],
			Tx_Locktype[GET(v,LOCKTYPE)],
			Tx_Speed[GET(v,SPEED)],
			lock? lock->descript.c_str():"Unknown key");
	}
	else if (v>=LIFT_BASE && v<LIFT_BASE+LIFT_COUNT)
	{
		if (GET(v,LIFTTARGET)!=3) return wxString::Format("[%5d] %s Lower Lift to %s, Delay=%d sec. (%s%s)",
			v,
			Tx_Trigger[GET(v,TRIGGER)],
			Tx_Lifttarget[GET(v,LIFTTARGET)],
			Num_Liftdelay[GET(v,LIFTDELAY)],
			Tx_Speed[GET(v,SPEED)],
			Tx_Monster[GET(v,MONSTER)]);
		else return wxString::Format("[%05d] %s Start moving floor, Delay=%d sec. (%s%s)",
			v,
			Tx_Trigger[GET(v,TRIGGER)],
			Num_Liftdelay[GET(v,LIFTDELAY)],
			Tx_Speed[GET(v,SPEED)],
			Tx_Monster[GET(v,MONSTER)]);
	}
	else if (v>=STAIR_BASE && v<STAIR_BASE+STAIR_COUNT)
	{
		return wxString::Format("[%05d] %s %s stairs, step=%d (%s%s%s)",
			v,
			Tx_Trigger[GET(v,TRIGGER)],
			Tx_Direction[GET(v,STAIRDIR)],
			Num_Stairstep[GET(v,STAIRSTEP)],
			Tx_Speed[GET(v,SPEED)],
			Tx_Monster[GET(v,MONSTER)],
			Tx_Tex[GET(v,STAIRTEX)]);

	}
	else if (v>=CRUSHER_BASE && v<CRUSHER_BASE+CRUSHER_COUNT)
	{
		return wxString::Format("[%05d] %s Start %scrushing ceiling (%s%s)",
			v,
			Tx_Trigger[GET(v,TRIGGER)],
			Tx_Noise[GET(v,CRUSHSILENT)],
			Tx_Speed[GET(v,SPEED)],
			Tx_Monster[GET(v,MONSTER)]);
	}
	return wxString::Format("[%05d] UNKNOWN", v);
}

