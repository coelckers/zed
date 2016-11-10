#ifndef __GAMECONFIG_H
#define __GAMECONFIG_H

#include "tarray.h"
#include "sc_man.h"
#include "Mapdata.h"

struct CColor
{
	wxString name;
	wxColour pen;
	wxColour printerpen;

public:
	CColor(char * _name,int r,int g,int b,int rp=-1,int gp=-1,int bp=-1)
	{
		name = _name;
		pen = wxColour(r,g,b);
		if (rp!=-1) printerpen=wxColour(rp, gp, bp);
		else printerpen=pen;
	}
};

struct ColorList
{
	// We need at least one dummy pen in case an invalid name is passed!
	CColor hpDummy;
	TArray<CColor*> colors;

	ColorList() : hpDummy("Dummy",128,128,128,-1,-1,-1)
	{
		colors.Push(&hpDummy);
	}

	void Clear()
	{
		for (unsigned i=1;i<colors.Size();i++) delete colors[i];
		colors.Clear();
	}

	~ColorList()
	{
		Clear();
	}


	CColor * GetPen(const char * name)
	{
		for (unsigned i=0;i<colors.Size();i++) 
		{
			if (!stricmp(colors[i]->name.c_str(),name)) return colors[i];
		}
		return NULL;
	}

	void Push(CColor * newc)
	{
		colors.Push(newc);
	}

	CColor * operator [](int index)
	{
		return colors[index];
	}

	unsigned Size()
	{
		return (unsigned)colors.Size ();
	}

};

extern ColorList Colors;

bool ParseColors();


struct FlagGroupDesc
{
	wxString name;
	int modeflag;
	int firstindex;
	int lastindex;
};

struct FlagDesc
{
	wxString shortname;
	wxString longname;
	wxString textname;
};

enum
{
	STAT_NONE,
	STAT_MONSTER,
	STAT_KEY,
	STAT_ITEM,
	STAT_NCITEM,
	STAT_WEAPON,
	STAT_AMMO,
	STAT_SPECIAL,
	STAT_FRIENDLY,
	STAT_LIGHT,
};

struct ThingDesc
{
	int DoomEdNum;
	int radius;
	wxString name;
	wxString sprite;
	wxString penname;
	CColor * realpens;		// this is filled in later
	int statclass;			// for thing statistics grouping
	wxString argdescript[5];	// only for extended mode

	ThingDesc()
	{
		DoomEdNum=radius=0;
		realpens=0;
		statclass=0;
	}
};

template<class T> 
class Container
{
	TArray <T *> items;
public:
	wxString name;

	void Clear()
	{
		for(unsigned i=0;i<items.Size();i++) delete items[i];
		items.Clear();
	}

	~Container()
	{
		Clear();
	}

	void Push(T * item)
	{
		items.Push(item);
	}

	T * operator [](int index)
	{
		/*if (index>=0 && index<items.size())*/ return items[index];
	}

	int Size()
	{
		return (int)items.Size();
	}

};

class ThingGroup : public Container<ThingDesc>
{
public:
	wxString parent;
};


struct Lock
{
	wxString descript;
	int lockvalue;
	wxString penname;
	CColor * realpen;

	Lock()
	{
		realpen=NULL;
		lockvalue=0;
	}

};

struct LineSpecial
{
	int number;
	wxString descript;
	wxString penname;
	wxString args[5];
	CColor * realpen;
	int param;
	int tagmask;
	int tidmask;
	int lineidmask;

	LineSpecial()
	{
		number=param=0;
		realpen=NULL;
	}
};

struct ArgType
{
	wxString name;
	wxArrayString values;
	wxArrayString flags;
	TArray<int> value_values;
	TArray<int> flag_values;
};

class LineGroup : public Container<LineSpecial>
{
public:
	wxString parent;
};

struct SectorSpecial
{
	int number;
	wxString descript;

	SectorSpecial()
	{
		number=0;
	}
};

class SectorGroup : public Container<SectorSpecial>
{
public:
	wxString parent;
};

struct mobjinfo_t
{
	int parentclassindex;
	int doomednum;
	int flags;
	char * classname;
	char * spriteframe;
	char * penname;
};

class GameConfig
{
	friend void CreateThingOrder();


public:
	GameConfig();
	~GameConfig();

	ThingDesc * ThingMap[32768];	// for quick and easy access
	LineSpecial * LineMap[8192];
	SectorSpecial * SectorMap[32768+256];
	Lock * LockMap[256];
	wxMenu * hmThings;
	wxMenu * hmLines;
	wxMenu * hmSectors;

	wxString namespc;
	wxString thisconfig;
	wxString altconfig;
	wxString defaultconfig;

	FlagDesc ThingFlags[32];
	FlagDesc LineFlags[32];
	FlagDesc TextMapThingFlags[MAX_FLAGS];
	FlagDesc TextMapLineFlags[MAX_FLAGS];
	FlagDesc TextMapSideFlags[MAX_FLAGS];
	FlagDesc TextMapSectorFlags[MAX_FLAGS];
	FlagGroupDesc TextMapThingFlagGroups[10];
	FlagGroupDesc TextMapLineFlagGroups[10];
	FlagGroupDesc TextMapSideFlagGroups[10];
	FlagGroupDesc TextMapSectorFlagGroups[10];
	int ThingGroupCount;
	int LineGroupCount;
	int SideGroupCount;
	int SectorGroupCount;
	Container<ThingGroup>   ThingGroups;
	Container<LineGroup>    LineGroups;
	Container<SectorGroup>  SectorGroups;
	TArray<ArgType*>		ArgTypes;
	bool genericlinedefs;
	bool genericsectors;
	bool hereticsectors;
	bool IsExtended;

	void InitConfig(wxString filename);
	wxString GetThingName(int thing, bool withnumber=true,bool forcedoom=false);
	wxString GetThingSprite(int thing);
	wxString GetThingArg(int thing, int arg);
	wxColour GetThingColor(int thing);
	int GetThingRadius( int type);
	wxString GetLDName(int type);
	const char * GetLDTrigger(int type);
	void SectorType(wxString & buffer, int type);
	void SectorBits(wxString & buffer, int flags);
	wxString GetSectorName(int type, bool wantnumber=true);
	bool IsSectorType(int type);
	wxColour GetLDColor(CLineDef * ld, bool isTextMap);
	wxString GetLDXName(unsigned char type);
	wxString GetLineDefDescription(int type, int trigger, int * args, int spacenumber);
	wxString GetLineDefDescription(CLineDef * ld, int spacenumber);
	int GetTagArg(int type);
	int GetTidArg(int type);
	int GetLineIDArg(int type);
	wxString GetThingFlagText(int flag, int shortt, bool textmap);
	wxString GetLineFlagText(int flag, int shortt, bool textmap);
	wxString GetSideFlagText(int flag, int shortt, bool textmap);
	wxString GetSectorFlagText(int flag, int shortt);
	int CheckTextMapThingFlag(const char *key);
	int CheckTextMapLineFlag(const char *key);
	int CheckTextMapSideFlag(const char *key);
	int CheckTextMapSectorFlag(const char *key);

	wxMenu * GetThingMenu() { return hmThings; }
	wxMenu * GetSpecialMenu() { return hmLines; }
	wxMenu * GetSectorMenu() { return hmSectors; }

private:

	ThingDesc * ParseThing(ScriptMan & sc, ThingDesc & def);
	ThingGroup * ParseThingGroup(ScriptMan & sc, ThingDesc & def, bool discard);
	void ParseThings(ScriptMan & sc, bool discard);
	void ParseKeys(ScriptMan & sc, bool discard);
	LineSpecial * ParseLineSpecial(ScriptMan & sc, bool extended);
	LineGroup * ParseLineGroup(ScriptMan & sc, bool extended, bool discard);
	void ParseLines(ScriptMan & sc, bool extended, bool discard);
	void ParseSides(ScriptMan & sc, bool discard);
	SectorSpecial * ParseSector(ScriptMan & sc);
	SectorGroup * ParseSectorGroup(ScriptMan & sc, bool discard);
	void ParseSectors(ScriptMan & sc, bool discard);
	void ParseArgTypes(ScriptMan & sc, bool discard);
	bool ParseConfig(wxString config, bool things, bool keys, bool lines, bool sides, bool sectors, bool argtypes);

	void AddThingToMenu(wxMenu * hm,ThingDesc * td);
	wxMenu * MakeThingGroupMenu(ThingGroup & group);
	wxMenu * MakeThingMenu();
	bool AddLineToMenu(wxMenu * hm,LineSpecial * td);
	wxMenu * MakeLineGroupMenu(LineGroup & group);
	wxMenu * MakeLineMenu();
	bool AddSectorToMenu(wxMenu * hm,SectorSpecial * td);
	wxMenu * MakeSectorGroupMenu(SectorGroup & group);
	wxMenu * MakeSectorMenu();

	void AddObject(mobjinfo_t * mo);
	mobjinfo_t * FindMobjInfo(const char * name);
	mobjinfo_t * NewMobj(mobjinfo_t * info);
	bool IsKindOf(mobjinfo_t * mo, const char * wxString);
	mobjinfo_t * CreateNewMobj(ScriptMan & sc,mobjinfo_t ** parent=NULL);
	void ProcessStates(ScriptMan & sc,mobjinfo_t * info);
	mobjinfo_t * ProcessActor(ScriptMan & sc,bool subclass);
	void ParseDecorateObject(ScriptMan & sc,int def);
	void ProcessMobjScript(QWORD lump,wxMenu * hm);
	void ParseDefThings();
	wxMenu * InitializeThings();


	TArray <ThingDesc *> CustomThings;
	TArray <mobjinfo_t> Objects;

	wxMenu * hmg;
	int menucount;
	bool dontlist;

};

extern GameConfig * cgc;

class ConfigManager
{

	TArray<GameConfig *> configs;
public:
	GameConfig * GetConfig(wxString name);
	GameConfig * GetRealConfig(bool extended, GameConfig * current);
	GameConfig * FindDefaultConfig(const char *game, bool extended, bool textmap);
	void RefreshConfigs();

	~ConfigManager()
	{
		cgc=NULL;
		RefreshConfigs();
	}
};


#endif