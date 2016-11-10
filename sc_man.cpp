#include "stdafx.h"
#include <limits.h>
#include "sc_man.h"
#include "ResourceFile.h"
#include "doomerrors.h"

//**************************************************************************
//**
//** sc_man.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: sc_man.c,v $
//** $Revision: 1.3 $
//** $Date: 96/01/06 03:23:43 $
//** $Author: bgokey $
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------


// MACROS ------------------------------------------------------------------

#define ASCII_COMMENT (';')
#define CPP_COMMENT ('/')
#define C_COMMENT ('*')
#define ASCII_QUOTE (34)
#define LUMP_SCRIPT 1
#define FILE_ZONE_SCRIPT 2

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// this kind of non-reentrant code REALLY sucks!
// Solution: Just convert the entire stuff into a class to allow recursive usage.
// I didn't have to change a single line of code except the function headers!

// CODE --------------------------------------------------------------------

//==========================================================================
//
// SC_OpenMem
//
// Prepares a script that is already in memory for parsing. The caller is
// responsible for freeing it, if needed.
//
//==========================================================================

void ScriptMan::SC_OpenMem (const char *name, char *buffer, int size)
{
	SC_Close ();
	ScriptSize = size;
	ScriptBuffer = buffer;
	strcpy (ScriptName, name);
	FreeScript = false;
	SC_PrepareScript ();
}


int M_ReadFile (char const *name, char **buffer)

{
	FILE *f = fopen(name, "rb");
	if (!f) 
	{
		*buffer=new char[0];
		return false;
	}
	fseek(f, 0, SEEK_END);
	int length = ftell(f);
	char * buf = new char[length];
	fseek(f, 0, SEEK_SET);
	fread(buf, 1, length, f);
	fclose(f);
	*buffer=buf;
	return length;
}

//==========================================================================
//
// SC_OpenMem
//
// Prepares a script that is already in memory for parsing. The caller is
// responsible for freeing it, if needed.
//
//==========================================================================

void ScriptMan::SC_OpenFile (const char *name)
{
	SC_Close ();
	ScriptSize = M_ReadFile(name, &ScriptBuffer);
	strcpy (ScriptName, name);
	FreeScript = true;
	SC_PrepareScript ();
}

//==========================================================================
//
// SC_OpenLumpNum
//
// Loads a script (from the WAD files).
//
//==========================================================================

void ScriptMan::SC_OpenLumpNum (QWORD lump, const char *name)
{
	SC_Close ();

	ScriptSize = CResourceFile::GlobalGetLumpSize(lump);
	if (ScriptSize<0) ScriptSize=0;
	char * p=new char[ScriptSize];
	CResourceFile::GlobalReadLump(lump, p);
	ScriptBuffer=p;
	FreeScript = 1;
	SC_PrepareScript ();
}

//==========================================================================
//
// SC_PrepareScript
//
// Prepares a script for parsing.
//
//==========================================================================

void ScriptMan::SC_PrepareScript (void)
{
	ScriptPtr = ScriptBuffer;
	ScriptEndPtr = ScriptPtr + ScriptSize;
	sc_Line = 1;
	sc_End = false;
	ScriptOpen = true;
	sc_String = StringBuffer;
	AlreadyGot = false;
	SavedScriptPtr = NULL;
}

//==========================================================================
//
// SC_Close
//
//==========================================================================

void ScriptMan::SC_Close (void)
{
	if (ScriptOpen)
	{
		if (FreeScript==1) delete ScriptBuffer;
		ScriptBuffer = NULL;
		ScriptOpen = false;
	}
}

//==========================================================================
//
// SC_SavePos
//
// Saves the current script location for restoration later
//
//==========================================================================

void ScriptMan::SC_SavePos (void)
{
	CheckOpen ();
	if (sc_End)
	{
		SavedScriptPtr = NULL;
	}
	else
	{
		SavedScriptPtr = ScriptPtr;
		SavedScriptLine = sc_Line;
	}
}

//==========================================================================
//
// SC_RestorePos
//
// Restores the previously saved script location
//
//==========================================================================

void ScriptMan::SC_RestorePos (void)
{
	if (SavedScriptPtr)
	{
		ScriptPtr = SavedScriptPtr;
		sc_Line = SavedScriptLine;
		sc_End = false;
		AlreadyGot = false;
	}
}

//==========================================================================
//
// SC_GetString
//
//==========================================================================

boolean ScriptMan::SC_GetString (void)
{
	char *text;
	boolean foundToken;

	CheckOpen();
	if (AlreadyGot)
	{
		AlreadyGot = false;
		return true;
	}
	foundToken = false;
	sc_Crossed = false;
	if (ScriptPtr >= ScriptEndPtr)
	{
		sc_End = true;
		return false;
	}
	while (foundToken == false)
	{
		if (ScriptPtr >= ScriptEndPtr)
		{
			sc_End = true;
			return false;
		}
		while (*ScriptPtr <= 32)
		{
			if (*ScriptPtr++ == '\n')
			{
				sc_Line++;
				sc_Crossed = true;
			}
			if (ScriptPtr >= ScriptEndPtr)
			{
				sc_End = true;
				return false;
			}
		}

		if (
			!(ScriptPtr[0] == CPP_COMMENT && ScriptPtr < ScriptEndPtr - 1 &&
			  (ScriptPtr[1] == CPP_COMMENT || ScriptPtr[1] == C_COMMENT)))
		{ // Found a token
			foundToken = true;
		}
		else
		{ // Skip comment
			if (ScriptPtr[0] == CPP_COMMENT && ScriptPtr[1] == C_COMMENT)
			{	// C comment
				while (ScriptPtr[0] != C_COMMENT || ScriptPtr[1] != CPP_COMMENT)
				{
					if (ScriptPtr[0] == '\n')
					{
						sc_Line++;
						sc_Crossed = true;
					}
					ScriptPtr++;
					if (ScriptPtr >= ScriptEndPtr - 1)
					{
						sc_End = true;
						return false;
					}
				}
				ScriptPtr += 2;
			}
			else
			{	// C++ comment
				while (*ScriptPtr++ != '\n')
				{
					if (ScriptPtr >= ScriptEndPtr)
					{
						sc_End = true;
						return false;
					}
				}
				sc_Line++;
				sc_Crossed = true;
			}
		}
	}
	text = sc_String;
	sc_Quote=false;
	if (*ScriptPtr == ASCII_QUOTE)
	{ // Quoted string
		sc_Quote=true;
		ScriptPtr++;
		while (*ScriptPtr != ASCII_QUOTE)
		{
			*text++ = *ScriptPtr++;
			if (ScriptPtr == ScriptEndPtr
				|| text == &sc_String[MAX_STRING_SIZE-1])
			{
				break;
			}
		}
		ScriptPtr++;
	}
	else
	{ // Normal string
		if (strchr (tokensep, *ScriptPtr))	// [CO] allow comma as token!
		{
			*text++ = *ScriptPtr++;
		}
		else
		{
			while ((*ScriptPtr > 32) && (strchr (tokensep, *ScriptPtr) == NULL)
				&& !(ScriptPtr[0] == CPP_COMMENT && (ScriptPtr < ScriptEndPtr - 1) &&
					 (ScriptPtr[1] == CPP_COMMENT || ScriptPtr[1] == C_COMMENT)))
			{
				*text++ = *ScriptPtr++;
				if (ScriptPtr == ScriptEndPtr
					|| text == &sc_String[MAX_STRING_SIZE-1])
				{
					break;
				}
			}
		}
	}
	*text = 0;

	return true;
}

//==========================================================================
//
// SC_MustGetString
//
//==========================================================================

void ScriptMan::SC_MustGetString (void)
{
	if (SC_GetString () == false)
	{
		SC_ScriptError ("Missing string (unexpected end of file).");
	}
}

//==========================================================================
//
// SC_MustGetString
//
//==========================================================================

void ScriptMan::SC_MustGetToken (void)
{
	sc_Token = Tk_Other;
	SC_MustGetString();
	if (sc_Quote) 
	{
		sc_Token = Tk_String;
	}
	else if (SC_Compare("true"))
	{
		sc_Token = Tk_Bool;
		sc_Number = 1;
	}
	else if (SC_Compare("false"))
	{
		sc_Token = Tk_Bool;
		sc_Number = 0;
	}
	else
	{
		char *p;
		sc_Number = strtol(sc_String, &p, 0);
		if (*p==0)
		{
			sc_Float = sc_Number;
			sc_Token = Tk_Int;
		}
		else
		{
			sc_Float = strtod(sc_String, &p);
			if (*p==0)
			{
				sc_Token = Tk_Float;
			}
		}
	}
}

//==========================================================================
//
// SC_MustGetStringName
//
//==========================================================================

void ScriptMan::SC_MustGetStringName (const char *name)
{
	SC_MustGetString ();
	if (SC_Compare (name) == false)
	{
		SC_ScriptError ("Expected '%s', got '%s'.", name, sc_String);
	}
}

//==========================================================================
//
// SC_GetNumber
//
//==========================================================================

boolean ScriptMan::SC_GetNumber (void)
{
	char *stopper;

	CheckOpen ();
	if (SC_GetString())
	{
		if (strcmp (sc_String, "MAXINT") == 0)
		{
			sc_Number = INT_MAX;
		}
		else
		{
			sc_Number = strtol (sc_String, &stopper, 0);
			if (*stopper != 0)
			{
				// Push it back and don't generate an error!
				SC_UnGet();
				return false;

				// This is nonsense and severely limits the functionality of the script manager!
				//const char *args;
				//args = sc_String;

				//SC_ScriptError ("SC_GetNumber: Bad numeric constant \"%s\".", &args);
			}
		}
		sc_Float = (float)sc_Number;
		return true;
	}
	else
	{
		return false;
	}
}

//==========================================================================
//
// SC_MustGetNumber
//
//==========================================================================

void ScriptMan::SC_MustGetNumber (void)
{
	if (SC_GetNumber() == false)
	{
		SC_ScriptError ("Missing integer.");
	}
}

//==========================================================================
//
// SC_GetFloat
//
//==========================================================================

boolean ScriptMan::SC_GetFloat (void)
{
	char *stopper;

	CheckOpen ();
	if (SC_GetString())
	{
		sc_Float = (float)strtod (sc_String, &stopper);
		if (*stopper != 0)
		{
			// Push it back and don't generate an error!
			SC_UnGet();
			return false;
		}
		sc_Number = (int)sc_Float;
		return true;
	}
	else
	{
		return false;
	}
}

//==========================================================================
//
// SC_MustGetFloat
//
//==========================================================================

void ScriptMan::SC_MustGetFloat (void)
{
	if (SC_GetFloat() == false)
	{
		SC_ScriptError ("Missing floating-point number.");
	}
}

//==========================================================================
//
// SC_UnGet
//
// Assumes there is a valid string in sc_String.
//
//==========================================================================

void ScriptMan::SC_UnGet (void)
{
	AlreadyGot = true;
}

//==========================================================================
//
// SC_Check
//
// Returns true if another token is on the current line.
//
//==========================================================================

/*
boolean ScriptMan::SC_Check(void)
{
	char *text;

	CheckOpen();
	text = ScriptPtr;
	if(text >= ScriptEndPtr)
	{
		return false;
	}
	while(*text <= 32)
	{
		if(*text == '\n')
		{
			return false;
		}
		text++;
		if(text == ScriptEndPtr)
		{
			return false;
		}
	}
	if(*text == ASCII_COMMENT)
	{
		return false;
	}
	return true;
}
*/

//==========================================================================
//
// SC_MatchString
//
// Returns the index of the first match to sc_String from the passed
// array of strings, or -1 if not found.
//
//==========================================================================

int ScriptMan::SC_MatchString (const char **strings)
{
	int i;

	for (i = 0; *strings != NULL; i++)
	{
		if (SC_Compare (*strings++))
		{
			return i;
		}
	}
	return -1;
}

//==========================================================================
//
// SC_MustMatchString
//
//==========================================================================

int ScriptMan::SC_MustMatchString (const char **strings)
{
	int i;

	i = SC_MatchString (strings);
	if (i == -1)
	{
		SC_ScriptError (NULL);
	}
	return i;
}

//==========================================================================
//
// SC_CheckString
//
// Checks if the next token matches the specified string. Returns true if
// it does. If it doesn't, it ungets it and returns false.
//==========================================================================

bool ScriptMan::SC_CheckString (const char *name)
{
	if (SC_GetString ())
	{
		if (SC_Compare (name))
		{
			return true;
		}
		SC_UnGet ();
	}
	return false;
}

//==========================================================================
//
// SC_Compare
//
//==========================================================================

boolean ScriptMan::SC_Compare (const char *text)
{
	return (stricmp (text, sc_String) == 0);
}

//==========================================================================
//
// SC_ScriptError
//
//==========================================================================

void ScriptMan::SC_ScriptError (const char *message, ...)
{
	char composed[2048];
	if (message == NULL)
	{
		message = "Bad syntax.";
	}
#if !defined(__GNUC__) && !defined(_MSC_VER)
	va_list arglist;
	va_start (arglist, message);
	_vsnprintf (composed, 2048, message, arglist);
	va_end (arglist);
#else
	va_list arglist;
	va_start (arglist, message);
	_vsnprintf (composed, 2048, message, (va_list)arglist);
#endif
	I_Error("%s%s\nLine %d, Token %s\n", composed, ScriptName, sc_Line, sc_String);
}


//==========================================================================
//
// CheckOpen
//
//==========================================================================

void ScriptMan::CheckOpen(void)
{
	if (ScriptOpen == false)
	{
	}
}


//==========================================================================
//
// Some enhancements for easier parsing
//
//==========================================================================

bool ScriptMan::TestCom()
{
	if (SC_GetString())
	{
		if (SC_Compare(",")) return true;
	}
	SC_UnGet();
	return false;
}

void ScriptMan:: ChkCom()
{
	if (!TestCom()) SC_ScriptError("',' expected");
}
	
bool ScriptMan::TestBraceOpn()
{
	SC_MustGetString();
	if (SC_Compare("{")) return true;
	SC_UnGet();
	return false;
}

void ScriptMan:: ChkBraceOpn()
{
	if (!TestBraceOpn()) SC_ScriptError("'{' expected");
}
	
bool ScriptMan::TestBraceCls()
{
	SC_MustGetString();
	if (SC_Compare("}")) return true;
	SC_UnGet();
	return false;
}

void ScriptMan:: ChkBraceCls()
{
	if (!TestBraceCls()) SC_ScriptError("'}' expected");
}
	

int ScriptMan::tkInt(const char *key)
{
	if (sc_Token != Tk_Int)
	{
		SC_ScriptError("Integer value expected for key %s, got '%s'", key, sc_String);
	}
	return sc_Number;
}

double ScriptMan::tkFloat(const char *key)
{
	if (sc_Token != Tk_Int && sc_Token != Tk_Float)
	{
		SC_ScriptError("Float value expected for key %s, got '%s'", key, sc_String);
	}
	return sc_Float;
}

bool ScriptMan::tkBool(const char *key)
{
	if (sc_Token != Tk_Bool)
	{
		SC_ScriptError("Boolean value expected for key %s, got '%s'", key, sc_String);
	}
	return !!sc_Number;
}

int MakeStringIndex(const char*);

int ScriptMan::tkString(const char *key)
{
	if (sc_Token != Tk_String)
	{
		SC_ScriptError("String value expected for key %s, got '%s'", key, sc_String);
	}
	return MakeStringIndex(sc_String);
}

void ScriptMan::tkCopyString(const char *key, char *dest, int len)
{
	if (sc_Token != Tk_String)
	{
		SC_ScriptError("String value expected for key %s, got '%s'", key, sc_String);
	}
	strncpy(dest, sc_String, len);
}


