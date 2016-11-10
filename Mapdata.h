#ifndef __MAPDATA_H
#define __MAPDATA_H

#include "tarray.h"
#include "name.h"


#if defined(_MSC_VER)
#define forceinline __forceinline
#else
#define forceinline inline
#endif



//==========================================================================
//
// 
//
//==========================================================================

struct CRect
{
	long left, top, right, bottom;

	forceinline long Width() const { return right-left; }
	forceinline long Height() const { return bottom-top; }

	bool IntersectRect(CRect & other)
	{
		if (other.left>left) left=other.left;
		if (other.right<right) right=other.right;
		if (other.top>top) top=other.top;
		if (other.bottom<bottom) bottom=other.bottom;
		return left<right && top<bottom;
	}

	bool UnionRect(CRect & other)
	{
		if (other.left<left) left=other.left;
		if (other.right>right) right=other.right;
		if (other.top<top) top=other.top;
		if (other.bottom>bottom) bottom=other.bottom;
		return left<right && top<bottom;
	}


};

struct CRectFloat
{
	double left, top, right, bottom;

	forceinline void Set(double a, double b, double c, double d)
	{
		left=a, top=b, right=c, bottom=d;
	}



	bool PointInRect(const wxPoint & p) const
	{
		return p.x>=left && p.x<right && p.y>=top && p.y<bottom;
	}
	bool PointInRect(const wxPoint * p) const
	{
		return p->x>=left && p->x<right && p->y>=top && p->y<bottom;
	}

	forceinline bool PointInRect(const wxRealPoint * p) const
	{
		return p->x>=left && p->x<right && p->y>=top && p->y<bottom;
	}
	forceinline bool PointInRect(const wxRealPoint & p) const
	{
		return p.x>=left && p.x<right && p.y>=top && p.y<bottom;
	}

	bool IntersectRect(CRectFloat & other)
	{
		if (other.left>left) left=other.left;
		if (other.right<right) right=other.right;
		if (other.top>top) top=other.top;
		if (other.bottom<bottom) bottom=other.bottom;
		return left<right && top<bottom;
	}

	wxRealPoint Center() const
	{
		return wxRealPoint((left+right)/2, (top+bottom)/2);
	}

	forceinline double Width() const { return right-left; }
	forceinline double Height() const { return bottom-top; }

};


#define MAX_FLAGS 160	// 160 bits should be enough

class BitField
{
	enum
	{
		BYTES = MAX_FLAGS/8
	};

	BYTE bits[BYTES];

public:
	forceinline void SetBit(int no)
	{
		if (no >=0 && no < 160)
		{
			bits[no>>3]|=(1<<(no&7));
		}
	}

	forceinline void ClearBit(int no)
	{
		if (no >=0 && no < 160)
		{
			bits[no>>3]&=~(1<<(no&7));
		}
	}

	forceinline int GetBit(int no)
	{
		if (no >=0 && no < 160)
		{
			return !!(bits[no>>3]&(1<<(no&7)));
		}
		return 0;
	}

	forceinline int GetShort()
	{
		return bits[0]+256*bits[1];
	}

	forceinline void SetShort(int v)
	{
		bits[0]=(v&255);
		bits[1]=((v>>8)&255);
		for(int i=2;i<BYTES;i++) bits[i]=0;
	}

	forceinline void Clear()
	{
		memset(bits, 0, sizeof(bits));
	}

	forceinline void Set()
	{
		memset(bits, -1, sizeof(bits));
	}

	BitField &operator |=(const BitField &other)
	{
		for(int i=0;i<BYTES;i++) bits[i]|=other.bits[i];
		return *this;
	}

	BitField &operator &=(const BitField &other)
	{
		for(int i=0;i<BYTES;i++) bits[i]&=other.bits[i];
		return *this;
	}

	bool operator ==(const BitField &other)
	{
		for(int i=0;i<BYTES;i++) if (bits[i]!=other.bits[i]) return false;
		return true;
	}

	BitField operator &(const BitField &other)
	{
		BitField ret;
		for(int i=0;i<BYTES;i++) ret.bits[i]=bits[i]&other.bits[i];
		return ret;
	}

	BitField operator ~() const
	{
		BitField other;
		for(int i=0;i<BYTES;i++) other.bits[i]=~bits[i];
		other.bits[1]=~bits[1];
		other.bits[2]=~bits[2];
		other.bits[3]=~bits[3];
		other.bits[4]=~bits[4];
		return other;
	}

	operator bool() const
	{
		for(int i=0;i<BYTES;i++) if (bits[i]) return true;
		return false;
	}

};

//==========================================================================
//
// A common vertex
//
//==========================================================================
class CLevel;

enum EPropType
{
	PROP_Int,
	PROP_Float,
	PROP_String,
	PROP_Bool
};

enum
{
	PROPERTY_COUNT = 100	// should be enough
};

union UPropValue
{
	int intval;
	double floatval;
	bool boolval;
	int stringindex;
};

struct CProperty
{
	FName name;
	EPropType type;
	UPropValue v;
};

void ClearPropertyStrings();
int MakeStringIndex(const char*);
const char *GetStringFromIndex(int);

struct CProperties
{
	CProperty prop[PROPERTY_COUNT];
	int count;

	void Clear()
	{
		memset(this, 0, sizeof(*this));
	}

	bool Add(FName key, int value)
	{
		if (count == PROPERTY_COUNT) return false;
		prop[count].name = key;
		prop[count].type = PROP_Int;
		prop[count].v.intval = value;
		return true;
	}

	bool Add(FName key, double value)
	{
		if (count == PROPERTY_COUNT) return false;
		prop[count].name = key;
		prop[count].type = PROP_Float;
		prop[count].v.floatval = value;
		return true;
	}

	bool Add(FName key, bool value)
	{
		if (count == PROPERTY_COUNT) return false;
		prop[count].name = key;
		prop[count].type = PROP_Bool;
		prop[count].v.boolval = value;
		return true;
	}

	bool Add(FName key, const char *value)
	{
		if (count == PROPERTY_COUNT) return false;
		prop[count].name = key;
		prop[count].type = PROP_Float;
		prop[count].v.stringindex = MakeStringIndex(value);
		return true;
	}
};

struct CMapVertex
{
	short x;
	short y;
};


struct CVertex
{

private:
	double x;
	double y;
public:

	operator wxRealPoint()
	{
		return wxRealPoint(x,y);
	}

	forceinline double X() const { return x; }
	forceinline double Y() const { return y; }

	forceinline CVertex & operator = (CMapVertex p)
	{
		x=p.x;
		y=p.y;
		return *this;
	}

	operator CMapVertex()
	{
		CMapVertex v = {short(x), short(y) };
		return v;
	}

	forceinline void SetX(double _x, bool precise)
	{
		if (!precise) x = floor(_x);
		else x = _x;
	}

	forceinline void SetY(double _x, bool precise)
	{
		if (!precise) y = floor(_x);
		else y = _x;
	}

};

#pragma pack(1)

//==========================================================================
//
// Things in Doom format maps
//
//==========================================================================

struct CThing1
{
	short xpos;
	short ypos;
	short angle;
	unsigned short type;
	short flags;
};

//==========================================================================
//
// Things in Hexen format maps
//
//==========================================================================

struct CThing2
{
	unsigned short thingid;
	short xpos;
	short ypos;
	short zpos;
	short angle;
	unsigned short type;
	short flags;
	unsigned char special;
	unsigned char args[5];
};

#pragma pack()

//==========================================================================
//
// Things in the internal format
//
//==========================================================================

struct CThing
{
	unsigned short thingid;
	float x;
	float y;
	float z;
	short angle;
	unsigned short type;
	BitField Flags;
	int special;
	int args[5];
	BitField notflags;
	bool multi;
	int comment;

	forceinline double X() const { return x; }
	forceinline double Y() const { return y; }
	forceinline double Z() const { return z; }

	forceinline void SetX(double _x, bool precise)
	{
		if (!precise) x = floor(_x);
		else x = _x;
	}

	forceinline void SetY(double _x, bool precise)
	{
		if (!precise) y = floor(_x);
		else y = _x;
	}

	forceinline void SetZ(double _x, bool precise)
	{
		if (!precise) z = floor(_x);
		else z = _x;
	}


	CThing & operator=(const CThing1 & t1)
	{
		thingid=0;
		x = t1.xpos;
		y = t1.ypos;
		z = 0;
		angle=t1.angle;
		type=t1.type;
		Flags.SetShort(t1.flags);
		special=args[0]=args[1]=args[2]=args[3]=args[4]=0;
		return * this;
	}

	CThing & operator=(const CThing2 & t1)
	{
		thingid=t1.thingid;
		x = t1.xpos;
		y = t1.ypos;
		z = t1.zpos;
		angle=t1.angle;
		type=t1.type;
		Flags.SetShort(t1.flags);
		special=t1.special;
		args[0]=t1.args[0];
		args[1]=t1.args[1];
		args[2]=t1.args[2];
		args[3]=t1.args[3];
		args[4]=t1.args[4];
		return * this;
	}

	int Save(char * data, bool extended)
	{
		if (!extended)
		{
			CThing1 * t=(CThing1*)data;

			t->angle=angle;
			t->flags=Flags.GetShort();
			t->type=type;
			t->xpos=short(x);
			t->ypos=short(y);
			return sizeof(CThing1);
		}
		else
		{
			CThing2 * t=(CThing2*)data;

			t->angle=angle;
			t->flags=Flags.GetShort();
			t->type=type;
			t->xpos=short(x);
			t->ypos=short(y);
			t->zpos=short(z);
			t->thingid=thingid;
			t->special=special;
			for(int i=0;i<5;i++) t->args[i]=args[i];
			return sizeof(CThing2);
		}
	}

	operator wxRealPoint()
	{
		return wxRealPoint(x,y);
	}
};

#pragma pack(1)

//==========================================================================
//
// Linedefs in Doom format maps
//
//==========================================================================

struct CLineDef1
{
	short start;
	short end;
	short flags;
	short type;
	short tag;
	unsigned short sidedef1;
	unsigned short sidedef2;
};

//==========================================================================
//
// Linedefs in Hexen format maps
//
//==========================================================================

struct CLineDef2
{
	short start;
	short end;
	short flags;
	unsigned char special;
	unsigned char args[5];
	unsigned short sidedef1;
	unsigned short sidedef2;
};

#pragma pack()

//==========================================================================
//
// Linedefs in the internal format
//
//==========================================================================

enum
{
	NO_SIDE =-1
};

struct CLineDef
{
	int Start;
	int End;
	BitField Flags;
	short type;
	short tag;
	int Sidedef1;
	int Sidedef2;
	int args[5];
	int comment;
	double alpha;

	BitField notflags;

	void Init()
	{
		memset(this, 0, sizeof(*this));
		alpha = 1.;
	}

	CLineDef & operator= (const CLineDef1 & l)
	{
		Init();
		Start=l.start;
		End=l.end;
		Flags.SetShort(l.flags);
		type=l.type;
		tag=l.tag;
		Sidedef1=l.sidedef1==0xffff? NO_SIDE:l.sidedef1;
		Sidedef2=l.sidedef2==0xffff? NO_SIDE:l.sidedef2;
		args[0]=args[1]=args[2]=args[3]=args[4]=0;
		return *this;
	}

	CLineDef & operator= (const CLineDef2 & l)
	{
		Init();
		tag = -1;
		Start=l.start;
		End=l.end;
		Flags.SetShort(l.flags);
		type=l.special;
		Sidedef1=l.sidedef1==0xffff? NO_SIDE:l.sidedef1;
		Sidedef2=l.sidedef2==0xffff? NO_SIDE:l.sidedef2;
		args[0]=l.args[0];
		args[1]=l.args[1];
		args[2]=l.args[2];
		args[3]=l.args[3];
		args[4]=l.args[4];
		return *this;
	}

	int Save(char * data, bool extended)
	{
		if (!extended)
		{
			CLineDef1 * l=(CLineDef1*)data;

			l->end=End;
			l->flags=Flags.GetShort();
			l->sidedef1=Sidedef1;
			l->sidedef2=Sidedef2;
			l->start=Start;
			l->tag=tag;
			l->type=type;
			return sizeof(CLineDef1);
		}
		else
		{
			CLineDef2 * l=(CLineDef2*)data;

			l->end=End;
			l->flags=Flags.GetShort();
			l->sidedef1=Sidedef1;
			l->sidedef2=Sidedef2;
			l->start=Start;
			l->special=(unsigned char)type;
			for(int i=0;i<5;i++) l->args[i]=(byte)args[i];
			return sizeof(CLineDef2);
		}
	}
};

#pragma pack(1)

//==========================================================================
//
// Common sidedef
//
//==========================================================================

struct CSideDef1
{
	short xoff;
	short yoff;
	char texUpper[8];
	char texLower[8];
	char texNormal[8];
	short sector;
};

struct CSideDef
{
	int xoff;
	int yoff;
	char texUpper[8];
	char texLower[8];
	char texNormal[8];
	int sector;
	int comment;
	double texOffset[3][2];
	double texScale[3][2];
	int light;

	BitField Flags;
	BitField notflags;

	void Init()
	{
		memset(this, 0, sizeof(*this));
		comment = 0;
		texOffset[0][0]=texOffset[0][1]=
		texOffset[1][0]=texOffset[1][1]=
		texOffset[2][0]=texOffset[2][1]=0;
		texScale[0][0]=texScale[0][1]=
		texScale[1][0]=texScale[1][1]=
		texScale[2][0]=texScale[2][1]=1;
		light = 0;
	}

	CSideDef &operator=(CSideDef1 &sd)
	{
		Init();
		xoff = sd.xoff;
		yoff = sd.yoff;
		sector = sd.sector;
		memcpy(texUpper, sd.texUpper, 8);
		memcpy(texLower, sd.texLower, 8);
		memcpy(texNormal, sd.texNormal, 8);
		return *this;
	}

	operator CSideDef1()
	{
		CSideDef1 sd;
		sd.xoff = xoff;
		sd.yoff = yoff;
		sd.sector = sector;
		memcpy(sd.texUpper, texUpper, 8);
		memcpy(sd.texLower, texLower, 8);
		memcpy(sd.texNormal, texNormal, 8);
		return sd;
	}
};


//==========================================================================
//
// Sector in the map
//
//==========================================================================

struct CSector1
{
	short floorh;
	short ceilh;
	char floort[8];
	char ceilt[8];
	short light;
	unsigned short special;
	short tag;
};

#pragma pack()

//==========================================================================
//
// Sector in the editor
//
//==========================================================================

enum
{
	CEILING ,
	FLOOR,
};

enum
{
	X, Y
};

struct CSector
{
	int floorh;
	int ceilh;
	char floort[8];
	char ceilt[8];
	int light;
	int special;
	int tag;

	int comment;

	double scale[2][2];
	double offset[2][2];
	double rotation[2];
	int planelight[2];
	int color;
	double desaturation;
	int fade;
	float gravity;


	int flags;
	int notflags;
	BitField Flags;
	BitField notFlags;
	bool multi;
	int buggy;

	void Init()
	{
		memset(this, 0, sizeof(*this));
		scale[0][0] = scale[1][0] = 1.;
		scale[0][1] = scale[1][1] = 1.;
		planelight[0] = planelight[1] = 0;
		color = 0xffffff;
		gravity = 1.0;
	}

	CSector &operator=(CSector1 &sd)
	{
		Init();
		floorh = sd.floorh;
		ceilh = sd.ceilh;
		light = sd.light;
		special = sd.special;
		tag = sd.tag;
		memcpy(floort, sd.floort, 8);
		memcpy(ceilt, sd.ceilt, 8);
		return *this;
	}

	operator CSector1()
	{
		CSector1 sd;
		sd.floorh = floorh;
		sd.ceilh = ceilh;
		sd.light = light;
		sd.special = special;
		sd.tag = tag;
		memcpy(sd.floort, floort, 8);
		memcpy(sd.ceilt, ceilt, 8);
		return sd;
	}

};


//==========================================================================
//
// A line consists of a linedef and 2 sidedefs
// This structure combines them in one place.
//
//==========================================================================

struct CLine
{
	CLineDef line;
	CSideDef sides[2];

	byte buggy;
	bool ispolyobj;
	bool extended;	// make life a little easier. This byte would be padding otherwise so put it to some use.
	bool multi;

	int GetTag()
	{
		if (!extended) 
		{
			return /*CheckTag(line.type)?*/ line.tag /*: 0*/;
		}
		else 
		{
			//int tagpos = GetTagPos(line.special, line.args)0;
			return /*tagpos==-1? 0:*/line.args[0];
		}
	}
};




//==========================================================================
//
// The plain map data
//
//==========================================================================

class Map
{
	friend class CUndoBuffer;	// The undo buffer must have the ability to manipulate the map data directly.

	TArray<CThing> m_Things;
	TArray<CSector> m_Sectors;
	TArray<CVertex> m_Vertices;
	TArray<CLine> m_Lines;

public:
	Map();
	~Map();
	void ShrinkToFit();

	void ClearThings();
	void DeleteThing(int index);
	CThing * AddThing();
	void CopyThing(int index, CThing * thing);
	forceinline CThing * GetThing(unsigned index) { return index<m_Things.Size()? &m_Things[index]:NULL; }
	forceinline int NumThings() { return m_Things.Size(); }

	void ClearSectors();
	void DeleteSector(int index);
	CSector * AddSector();
	void CopySector(int index, CSector * thing);
	forceinline CSector * GetSector(unsigned index) { return index<m_Sectors.Size()? &m_Sectors[index]:NULL; }
	forceinline int NumSectors() { return m_Sectors.Size(); }

	void ClearVertices();
	void StripVertices(int to) { m_Vertices.Resize(to); }
	void DeleteVertex(int index);
	CVertex * AddVertex();
	forceinline CVertex * GetVertex(unsigned index) { return index<m_Vertices.Size()? &m_Vertices[index]:NULL; }
	forceinline int NumVertices() { return m_Vertices.Size(); }

	void ClearLines();
	void DeleteLine(int index);
	CLine * AddLine();
	void CopyLine(int index, CLine * thing);
	forceinline CLine * GetLine(unsigned index) { return index<m_Lines.Size()? &m_Lines[index]:NULL; }
	forceinline int NumLines() { return m_Lines.Size(); }

};

#endif