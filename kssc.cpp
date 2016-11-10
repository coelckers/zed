//-------------------------------------------------------------------------
// KSSC
// Script Compiling Code for Strife
// Main compiling code
//
// (c) 10/26/2005 Samuel 'Kaiser' Villarreal
//
// Adapted for use in ZEd by Christoph Oelckers
//
//--------------------------------------------------------------------------
#include "stdafx.h"
#include <stdlib.h>
#include <string.h>
#include "ZEd.h"
#include "Level.h"
#include "tarray.h"
#include "sc_man.h"
#include "doomerrors.h"

typedef struct
{
	int GiveType;
	int Item[3];
	int Count[3];
	char Reply[32];	
	char Yes[80];
	int Link;
	int Log;
	char No[80];
} npcChoice_t;

typedef struct
{
	int SpeakerType;
	int DropType;
	int ItemCheck[3];
	int Link;
	char Name[16];
	char Sound[8];
	char Backdrop[8];
	char Dialogue[320];	
	npcChoice_t Responses[5];
} npc_t;

#define NPCITEMCHECK	0
#define CHOICEITEMCHECK	1
#define CHOICEAMOUNTCHECK	2
#define CHOICENUM	3

static TArray<npc_t> compiled;
static ScriptMan sc;

//==========================================================================
//
// K_CheckStringSize - Checks if current string length is larger than its actual size
//
//==========================================================================
static bool K_CheckStringSize(char* string, size_t buf)
{
	if(strlen(string) > buf)
	{
		sc.SC_ScriptError("The text %s is too long!\n", string);
		return true;
	}
	return false;
}

//==========================================================================
//
// main - Setup the files, write the data into binary form, then save as a lump
//
//==========================================================================
int CompileConversationScript(CLevel * level_dest, char * script)
{
	int checkSets[4];
	bool choiceFinish = false;
	int blockNumber = 1;
	int currentMobj;
	bool getNPC = false;

	try
	{
		sc.SC_OpenMem("StrifeConv", script, (int)strlen(script));
		compiled.Resize(1);
		npc_t* rogueSpeech = &compiled[0];
		memset(checkSets, 0, sizeof(checkSets));
		while(sc.SC_GetString())
		{
			if(sc.SC_Compare("$MOBJ"))
			{
				if(getNPC == true)
				{
					sc.SC_ScriptError("Previous block has not been closed properly, or missing $END token!");
					return 1;
				}
				sc.SC_MustGetNumber();
				rogueSpeech->SpeakerType = sc.sc_Number;
				getNPC = true;
			}
			if(sc.SC_Compare("DROP"))
			{
				sc.SC_MustGetNumber();
				rogueSpeech->DropType = sc.sc_Number;
			}
			if(sc.SC_Compare("IF_ITEM"))
			{
				sc.SC_MustGetNumber();
				rogueSpeech->ItemCheck[checkSets[NPCITEMCHECK]] = sc.sc_Number;
				checkSets[NPCITEMCHECK]++;
			}
			if(sc.SC_Compare("GOTO"))
			{
				sc.SC_MustGetNumber();
				rogueSpeech->Link = sc.sc_Number;
			}
			if(sc.SC_Compare("NAME"))
			{
				sc.SC_MustGetString();
				strcpy(rogueSpeech->Name, sc.sc_String);
				if(K_CheckStringSize(rogueSpeech->Name, sizeof(rogueSpeech->Name)))	return 1;
			}
			if(sc.SC_Compare("VOICE"))
			{
				sc.SC_MustGetString();
				strcpy(rogueSpeech->Sound, sc.sc_String);
				if(K_CheckStringSize(rogueSpeech->Sound, sizeof(rogueSpeech->Sound))) return 1;
			}
			if(sc.SC_Compare("PANEL"))
			{
				sc.SC_MustGetString();
				strcpy(rogueSpeech->Backdrop, sc.sc_String);
				if(K_CheckStringSize(rogueSpeech->Backdrop,	sizeof(rogueSpeech->Backdrop)))	return 1;
			}
			if(sc.SC_Compare("DIALOG"))
			{
				sc.SC_MustGetString();
				strcpy(rogueSpeech->Dialogue, sc.sc_String);
				if(K_CheckStringSize(rogueSpeech->Dialogue,	sizeof(rogueSpeech->Dialogue)))	return 1;
			}
			if(sc.SC_Compare("}"))
			{
				sc.SC_ScriptError("Found a wandering '}'");
				return 1;
			}
			if(sc.SC_Compare("{") && choiceFinish == false)
			{
				while(sc.SC_GetString() && choiceFinish == false)
				{
					if(sc.SC_Compare("GIVEMOBJ"))
					{
						sc.SC_MustGetNumber();
						rogueSpeech->Responses[checkSets[CHOICENUM]].GiveType = sc.sc_Number;
						//if(rogueSpeech->Responses[checkSets[CHOICENUM]].GiveType == 0)
						//	printf("Warning! 'GIVEMOBJ' is 0! Are you sure about this?\n");
					}
					if(sc.SC_Compare("IF_HAS"))
					{
						sc.SC_MustGetNumber();
						rogueSpeech->Responses[checkSets[CHOICENUM]].Item[checkSets[CHOICEITEMCHECK]] = sc.sc_Number;
						checkSets[CHOICEITEMCHECK]++;
						while(sc.SC_GetString())
						{
							if(sc.SC_Compare("*") == false)
							{
								sc.SC_ScriptError("Please specify amount for the 'IF_HAS' token (use '* itemamount' after token)");
								return 1;
							}
							sc.SC_MustGetNumber();
							rogueSpeech->Responses[checkSets[CHOICENUM]].Count[checkSets[CHOICEAMOUNTCHECK]] = sc.sc_Number;
							checkSets[CHOICEAMOUNTCHECK]++;
							sc.SC_UnGet();
							break;

						}
					}
					if(sc.SC_Compare("TEXT"))
					{
						sc.SC_MustGetString();
						strcpy(rogueSpeech->Responses[checkSets[CHOICENUM]].Reply, sc.sc_String);
						//set the default no and yes text
						sprintf(rogueSpeech->Responses[checkSets[CHOICENUM]].No,
							"NO.  you don't have what i want for the  %s!",sc.sc_String);
						sprintf(rogueSpeech->Responses[checkSets[CHOICENUM]].Yes,
							"you got the %s!", sc.sc_String);
						if(K_CheckStringSize(
							rogueSpeech->Responses[checkSets[CHOICENUM]].Reply,
							sizeof(rogueSpeech->Responses[checkSets[CHOICENUM]].Reply)))
							return 1;
					}
					if(sc.SC_Compare("YESTEXT"))
					{
						memset(rogueSpeech->Responses[checkSets[CHOICENUM]].Yes, 0, 80);
						sc.SC_MustGetString();
						strcpy(rogueSpeech->Responses[checkSets[CHOICENUM]].Yes, sc.sc_String);
						if(K_CheckStringSize(
							rogueSpeech->Responses[checkSets[CHOICENUM]].Yes,
							sizeof(rogueSpeech->Responses[checkSets[CHOICENUM]].Yes)))
							return 1;
					}
					if(sc.SC_Compare("LINK"))
					{
						sc.SC_MustGetNumber();
						rogueSpeech->Responses[checkSets[CHOICENUM]].Link = sc.sc_Number;
					}
					if(sc.SC_Compare("LOG"))
					{
						sc.SC_MustGetNumber();
						rogueSpeech->Responses[checkSets[CHOICENUM]].Log = sc.sc_Number;
					}
					if(sc.SC_Compare("NOTEXT"))
					{
						sc.SC_MustGetString();
						strcpy(rogueSpeech->Responses[checkSets[CHOICENUM]].No, sc.sc_String);
						if(K_CheckStringSize(
							rogueSpeech->Responses[checkSets[CHOICENUM]].No,
							sizeof(rogueSpeech->Responses[checkSets[CHOICENUM]].No)))
							return 1;
					}
					//check for unclosed choicedefs
					if(sc.SC_Compare("{"))
					{
						sc.SC_ScriptError("'{' found within a choicedef!");
						return 1;
					}
					if(sc.SC_Compare("}") && choiceFinish == false)
					{
						checkSets[CHOICENUM]++;
						choiceFinish = true;
						break;
					}
				}
			}
			choiceFinish = false;
			if(sc.SC_Compare("$END"))
			{
				getNPC = false;
				(currentMobj != rogueSpeech->SpeakerType) ?
					(currentMobj = rogueSpeech->SpeakerType, blockNumber = 1) : blockNumber++;
				//printf("Writting NPC #%i '%s', block #%i\n", rogueSpeech->SpeakerType,
				//	rogueSpeech->Name, blockNumber);
				//printf("\n");
				if(rogueSpeech->SpeakerType <= 0)
				{
					sc.SC_ScriptError("Need to specify a map thing!");
					return 1;
				}
				compiled.Resize(compiled.Size()+1);
				rogueSpeech=&compiled.Last();
				memset(rogueSpeech,0,sizeof(npc_t));
				choiceFinish = false;
				memset(checkSets, 0, sizeof(checkSets));

			}
			sc.SC_Close();
		}
		level_dest->m_StrifeConv.Resize(sizeof(npc_t)*compiled.Size());
		memcpy(&level_dest->m_StrifeConv[0], &compiled[0], sizeof(npc_t)*compiled.Size());
		return 0;
	}
	catch(CRecoverableError * err)
	{
		wxMessageBox(err->GetMessage(), ZED_CAPTION);
		sc.SC_Close();
		return 1;
	}
}
