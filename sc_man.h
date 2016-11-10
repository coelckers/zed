#ifndef __SC_MAN_H__
#define __SC_MAN_H__

#include "doomtype.h"


enum EToken
{
	Tk_Other,
	Tk_Int,
	Tk_Float,
	Tk_String,
	Tk_Bool
};

// Converted the entire Script manager into a class to allow recursive usage of scripts
class ScriptMan
{
	enum 
	{
		MAX_STRING_SIZE=4096
	};

public:
	char *sc_String;
	int sc_Number;
	double sc_Float;
	int sc_Line;
	bool sc_Quote;
	boolean sc_End;
	boolean sc_Crossed;
	boolean sc_FileScripts;
	int sc_Token;

// PRIVATE DATA DEFINITIONS ------------------------------------------------
private:
	char ScriptName[128];
	char *ScriptBuffer;
	char *ScriptPtr;
	char *ScriptEndPtr;
	char StringBuffer[MAX_STRING_SIZE];
	boolean ScriptOpen;
	int ScriptSize;
	boolean AlreadyGot;
	int FreeScript;
	char *SavedScriptPtr;
	int SavedScriptLine;
	int ScriptLump;
	char * tokensep;
public:

	ScriptMan(char * tok="{}|=") { memset(this,0,sizeof(*this)); tokensep=tok;  }

	void SC_Open (const char *name);
	void SC_OpenFile (const char *name);
	void SC_OpenMem (const char *name, char *buffer, int size);
	void SC_OpenLumpNum (QWORD lump, const char *name);
	void SC_PrepareScript (void);
	void SC_Close (void);
	void SC_SavePos (void);
	void SC_RestorePos (void);
	boolean SC_GetString (void);
	void SC_MustGetString (void);
	void SC_MustGetStringName (const char *name);
	boolean SC_GetNumber (void);
	void SC_MustGetNumber (void);
	boolean SC_GetFloat (void);
	void SC_MustGetFloat (void);
	void SC_UnGet (void);
	int SC_MatchString (const char **strings);
	int SC_MustMatchString (const char **strings);
	boolean SC_Compare (const char *text);
	void SC_ScriptError (const char *message, ...);
	void CheckOpen(void);
	bool SC_CheckString (const char *name);
	void SC_MustGetToken (void);

	int tkInt(const char *key = NULL);
	double tkFloat(const char *key = NULL);
	bool tkBool(const char *key = NULL);
	int tkString(const char *key = NULL);
	void tkCopyString(const char *key, char *dest, int len);


	void ChkCom();
	bool TestCom();
	void ChkBraceOpn();
	bool TestBraceOpn();
	void ChkBraceCls();
	bool TestBraceCls();


};

#endif //__SC_MAN_H__
