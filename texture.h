#pragma once

#include "doomtype.h"
#include "TArray.h"
enum tx_namespace
{
	tx_sprite=1,
	tx_pname=2,
	tx_loprio=3,
	tx_flat=4,
	tx_texture=5,	// TEXTUREx
	tx_hiprio=6,
	tx_txlump=7,	// TX_
};

extern const char * TextureTypeNames[];

//--------------------------------------------------------------------------
//
// Texture definition
//
//--------------------------------------------------------------------------

struct column_t
{
	byte topdelta; // -1 is the last post in a column
	byte length;   // length data bytes follows
};

struct patch_t
{ 
	short width, height;  // bounding box size 
	short leftoffset;     // pixels to the left of origin 
	short topoffset;      // pixels below the origin 
	int columnofs[8];     // only [width] used
} ;


//
// Each texture is composed of one or more patches, with patches being lumps
// stored in the WAD. The lumps are referenced by number, and patched into
// the rectangular texture space using origin and possibly other attributes.
//
typedef struct
{
	short	originx;
	short	originy;
	short	patch;
	short	stepdir;
	short	colormap;
} mappatch_t;

//
// A wall texture is a list of patches which are to be combined in a
// predefined order.
//
typedef struct
{
	char				name[8];
	unsigned short		Flags;				// [RH] Was unused
	unsigned char		ScaleX;				// [RH] Scaling (8 is normal)
	unsigned char		ScaleY;				// [RH] Same as above
	short				width;
	short				height;
	unsigned char		columndirectory[4];	// OBSOLETE
	short				patchcount;
	mappatch_t			patches[1];
} maptexture_t;

#define MAPTEXF_WORLDPANNING	0x8000

// Here's what the Strife versions of the above two structures are:

typedef struct
{
	short	originx;
	short	originy;
	short	patch;
} strifemappatch_t;

//
// A wall texture is a list of patches which are to be combined in a
// predefined order.
//
typedef struct
{
	char				name[8];
	unsigned short		Flags;				// [RH] Was nused
	unsigned char		ScaleX;				// [RH] Scaling (8 is normal)
	unsigned char		ScaleY;				// [RH] Same as above
	short				width;
	short				height;
	short				patchcount;
	strifemappatch_t	patches[1];
} strifemaptexture_t;


// Internal data structure
struct TexPatch
{
	int originx, originy;
	QWORD patch; 
};



// this is an intermediate class to keep the scaling and other higher level stuff
// out of the low level class. The low level class merely manages the graphic itself.
class Texture
{
	friend class TextureManager;
	friend class GLTexture;

	char name[9];
	int   next, index;     // killough 1/31/98: used in hashing algorithm

	bool isflat, iswall;

	tx_namespace prio;

	int width;
	int height;


	int patchcount;
	int owner;
	TexPatch * patches;

	void Clean();


public:
	GLTexture * gltexture;

	Texture(QWORD lump, tx_namespace prio);
	Texture(int owner, maptexture_t * mtx,tx_namespace _prio, QWORD * patchlookup, const char * pnames);
	Texture(int owner, strifemaptexture_t * mtx,tx_namespace _prio, QWORD * patchlookup, const char * pnames);
	~Texture()
	{
		Clean();
		delete patches;
	}

	bool Register();

	int TextureHeight() { return height; }
	int TextureWidth() { return width; }
	tx_namespace GetTextureType() { return prio; }
	const char * GetName() const { return name; }
};


class TextureManager
{
	TArray<Texture *> textures;

	void AddPatches (QWORD lumpnum);
	Texture * AddLumpTexture(QWORD lump,tx_namespace prio);
	void AddTextureLump(QWORD lump,QWORD pnlump, tx_namespace prio);

	friend int tccmp(const void * a, const void * b);

public:

	~TextureManager()
	{
		for(unsigned i=0;i<textures.Size();i++) delete(textures[i]);
	}

	void CollectTextures(TArray<int> & list, tx_namespace prio, int mask, int owner=-1);

	void Initialize();
	int CheckTextureNumForName(const char * tex,tx_namespace prio=tx_texture);
	int TextureNumForName(const char * tex,tx_namespace prio);

	inline int TextureHeight(unsigned int texno)
	{
		if (texno<textures.Size()) 
		{
			if (textures[texno]->height<0) textures[texno]->Register();
			return textures[texno]->height;
		}
		return 0;
	}

	inline int TextureWidth(unsigned int texno)
	{
		if (texno<textures.Size()) 
		{
			if (textures[texno]->width<0) textures[texno]->Register();
			return textures[texno]->width;
		}
		return 0;
	}

	inline void GetTextureName(unsigned int texno,char * buffer)
	{
		*buffer=0;
		if (texno<textures.Size()) 
		{
			strncpy(buffer,textures[texno]->name,8);
			buffer[8]=0;
		}
	}

	inline const char * GetTextureName(unsigned int texno)
	{
		if (texno<textures.Size()) 
		{
			return textures[texno]->name;
		}
		return NULL;
	}

	inline int GetTextureType(unsigned int texno)
	{
		if (texno<textures.Size()) 
		{
			return textures[texno]->prio;
		}
		return NULL;
	}

	inline int GetOwner(unsigned int texno)
	{
		if (texno<textures.Size()) 
		{
			return textures[texno]->owner;
		}
		return NULL;
	}


	bool IsTextureQ(int texno, bool wall)
	{
		return wall? textures[texno]->iswall:textures[texno]->isflat;
	}


	Texture * RegisterTexture(unsigned int texno);

	void CleanWorldTextures();
	void CleanMemory();

	wxBitmap * TextureManager::CreateTexture(int tnum);
	wxBitmap * CreateTexture(const char * name,tx_namespace type);

	bool GetWallTextureSize(SIZE * sz,char * texName);
	bool GetFlatTextureSize(SIZE * sz,char * texName);

	int TextureCount() { return textures.Size(); }

	bool IsFlatTexture(const char * str)
	{
		// Todo strict check
		return CheckTextureNumForName(str, tx_flat)>=0;
	}

	bool IsWallTexture(const char * str)
	{
		if (str[0]=='-' && str[1]==0) return true;
		// Todo strict check
		return CheckTextureNumForName(str, tx_texture)>=0;
	}



};


extern TextureManager * texman;


class GLTexture
{
protected:

	int width,height;
	int buffer_size;
	wxBitmap * image;
	Texture * texd;


	void AddPatchToTexture(unsigned char *buffer, const patch_t *patch, int originx, int originy);
	void AddFlatToTexture(unsigned char *buffer, const unsigned char *flat,int pitch);
	void AddRawToTexture(byte * buffer, wxImage & patch, int originx, int originy);
	unsigned char * CreateBuffer()
	{
		unsigned char * buffer = (unsigned char *)malloc(buffer_size);
		for(int i=0;i<buffer_size;i+=3) 
		{
			buffer[i]=0;
			buffer[i+1]=buffer[i+2]=255;
		}
		return buffer;
	}

	void FreeBuffer(unsigned char * buffer)
	{
	}

	void TexInit(int width,int height);

public:
	GLTexture(Texture * tex) 
	{
		image=NULL;
		texd=tex;
		TexInit(tex->TextureWidth(), tex->TextureHeight());
	}
	~GLTexture() 
	{
		if (image) delete image;
		image=NULL;
	}
	wxBitmap * CreateTexBuffer();

};



class TextureBox : public wxWindow
{
	int width;
	int height;
	int type;
	int texnum;
	
public:
	void OnPaint(wxPaintEvent &event);
	void OnClick(wxMouseEvent &event);
	TextureBox(wxWindow *parent, int id, int width, int height);
	~TextureBox();

	void SetTexture(const char * name,tx_namespace txn=tx_texture);
	int GetTextureWidth() const { return width; }
	int GetTextureHeight() const { return height; }
	int GetTextureType() const { return type; }
	bool isValidTexture() const { return texnum>=0; }
	
	DECLARE_EVENT_TABLE()
};

class TextureList : public wxWindow
{
public:
	TextureList(wxWindow * parent, int id);
	virtual ~TextureList();
	void SetTextures(TArray<int> & newlist);
	void SetToName(const wxString & name);
	int GetSelection() const
	{
		return (unsigned)m_Selection<m_texturelist.Size()? m_texturelist[m_Selection] : -1;
	}
	void SetSelection(int newval);

private:
	int m_Selection;
	int m_topleft;
	TArray<int> m_texturelist;

	void OnPaint(wxPaintEvent & event);
	void OnScroll(wxScrollWinEvent & event);
	void OnKeyDown(wxKeyEvent & event);
	void OnSize(wxSizeEvent& event);
	void OnLButtonDown(wxMouseEvent & event);
	void OnLButtonDblClk(wxMouseEvent & event);
	void UpdateScrollPos();


	DECLARE_EVENT_TABLE()
};



void ValidateTextureManager();
