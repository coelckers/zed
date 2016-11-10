#ifndef __RESOURCEFILE_H
#define __RESOURCEFILE_H

#include "tarray.h"
#include "doomtype.h"
#include "files.h"

struct C7zArchive;
struct FZipCentralDirectoryInfo;

//==========================================================================
//
//
//
//==========================================================================

class CResourceFile
{
	CResourceFile * parent;
protected:
	char * m_Filename;
	int m_NumLumps;

	static void SortMapList(TArray<const char *> & array);
public:
	QWORD m_LumpStart;

	CResourceFile(void);
	virtual ~CResourceFile(void);
	virtual int FindLump(const char * name)=0;
	virtual int FindLumpByFullName(const char * name) { return -1; }
	virtual void FindLumps(const char * name, TArray<QWORD> & list) =0;
	virtual int GetLumpSize(int lump)=0;
	virtual void * ReadLump(int lump)=0;
	virtual void ReadLump(int lump, void * buffer)=0;
	virtual void ReplaceLump(int lump, void * data, int length)=0;
	virtual void Save(const char * filename)=0;
	virtual void * CreateInMemory(int * length)=0;
	virtual void GetMapList(TArray<const char *> & array) {}
	virtual const char * GetLumpName(int lump)=0;
	virtual const char * GetFullLumpName(int lump) { return NULL; }

	virtual void GetAllSprites(TArray<QWORD> & list) {}
	virtual void GetAllFlats(TArray<QWORD> & list) {}
	virtual void GetAllTextures(TArray<QWORD> & list) {}

	void SetParent(CResourceFile * p)
	{
		parent=p;
	}
	CResourceFile * GetParent() const
	{
		return parent;
	}
	const char * GetName() const
	{
		return m_Filename;
	}

	static CResourceFile * GetResourceFile(int no)
	{
		if (no>=0 && no<(int)OpenFiles.Size ()) return OpenFiles[no];
		else return NULL;
	}

	void ChangeIndex(int newindex)
	{
		m_LumpStart=newindex;
	}

	static QWORD GlobalFindLump(const char * name);
	static QWORD GlobalFindLumpByFullName(const char * name);
	static bool GlobalFindLumps(const char * name, TArray<QWORD> & list);
	static const char * GlobalGetLumpName(QWORD lump);
	static const char * GlobalGetFullLumpName(QWORD lump);
	static int GlobalGetLumpSize(QWORD lump);
	static void * GlobalReadLump(QWORD lump);
	static void GlobalReadLump(QWORD lump, void * buffer);
	static void GlobalReplaceLump(QWORD lump, void * data, int length);

	static void GlobalGetAllSprites(TArray<QWORD> & list);
	static void GlobalGetAllFlats(TArray<QWORD> & list);
	static void GlobalGetAllTextures(TArray<QWORD> & list);

	static void AddResourceFile(const char * filename);
	static void CloseResourceFile(const char * filename);
	static void OpenResourceFiles(const TArray<const char *> & filenames);
	static void OpenResourceFiles(const wxArrayString & filenames);
	static void CloseResourceFiles();
	static void SwitchIWAD(const char *newiwad);

	static void GetMapList(const char * wad, TArray<const char *> & array);

	static TArray<CResourceFile *> OpenFiles;
};


//==========================================================================
//
//
//
//==========================================================================

class CSingleLump : public CResourceFile
{
	char * m_Data;
	int m_Length;
	char lumpname[9];
	bool ismap;

public:
	CSingleLump(const char * filename);
	~CSingleLump(void);

	virtual int FindLump(const char * name);
	virtual void FindLumps(const char * name, TArray<QWORD> & list);
	virtual int GetLumpSize(int lump);
	virtual void * ReadLump(int lump);
	virtual void ReadLump(int lump, void * buffer);
	virtual void ReplaceLump(int lump, void * data, int length);
	virtual void Save(const char * filename);
	virtual void * CreateInMemory(int * length);
	virtual const char * GetLumpName(int lump);
	virtual void GetMapList(TArray<const char *> & array);
};

//==========================================================================
//
//
//
//==========================================================================

struct CWADLumpInfo
{
	DWORD offset;
	DWORD length;
	char name[8];
};

struct CWADLump
{
	DWORD offset;
	DWORD length;
	char name[9];
	char * data;
};

class CWADFile : public CResourceFile
{
	TArray<CWADLump> lumps;
	TArray<const char *> mapnames;
	bool IWAD;
	FILE * m_File;

public:
	CWADFile(const char * filename);
	CWADFile(const char * name, const char * memory);
	~CWADFile(void);

	virtual int FindLump(const char * name);
	virtual void FindLumps(const char * name, TArray<QWORD> & list);
	virtual int GetLumpSize(int lump);
	virtual void * ReadLump(int lump);
	virtual void ReadLump(int lump, void * buffer);
	virtual void ReplaceLump(int lump, void * data, int length);
	virtual void Save(const char * filename);
	virtual void * CreateInMemory(int * length);
	virtual void GetMapList(TArray<const char *> & array);
	virtual const char * GetLumpName(int lump);
	virtual void GetAllSprites(TArray<QWORD> & list);
	virtual void GetAllFlats(TArray<QWORD> & list);
	virtual void GetAllTextures(TArray<QWORD> & list);

};

 
//==========================================================================
//
//
//
//==========================================================================

//
// WADFILE I/O related stuff.
//
struct LumpRecord
{
	char *		fullname;		// only valid for files loaded from a .zip file
	char		name[9];
	BYTE		method;			// zip compression method
	WORD		flags;
	WORD		gpflags;
	int			position;
	int			size;
	int			compressedsize;
	void *		data;
};

class CZipFile :
	public CResourceFile
{
protected:
	FileReader *fin;
	TArray<LumpRecord> lumps;
	TArray<CWADFile *> embeddedWADs;
	TArray<const char *> mapnames;

	virtual void ReadLump(LumpRecord *l, void *Ptr);

private:
	static DWORD FindCentralDir(FileReader * fin);
	void ReadDirectory(FileReader *wadinfo);

public:
	CZipFile(const char * filename, bool readdir = true);
	~CZipFile(void);

	virtual int FindLump(const char * name);
	virtual int FindLumpByFullName(const char * name);
	void FindLumps(const char * name, TArray<QWORD> & list);
	virtual int GetLumpSize(int lump);
	virtual void * ReadLump(int lump);
	virtual void ReadLump(int lump, void * buffer);
	virtual void ReplaceLump(int lump, void * data, int length);
	virtual void Save(const char * filename);
	virtual void * CreateInMemory(int * length);
	virtual void GetMapList(TArray<const char *> & array);
	virtual const char * GetLumpName(int lump);
	virtual void GetAllSprites(TArray<QWORD> & list);
	virtual void GetAllFlats(TArray<QWORD> & list);
	virtual void GetAllTextures(TArray<QWORD> & list);
	virtual const char * GetFullLumpName(int lump);

};

class C7zFile : public CZipFile
{
	C7zArchive *Archive;

	void ReadDirectory(FileReader *wadinfo);

protected:

	virtual void ReadLump(LumpRecord *l, void *Ptr);

public:
	C7zFile(const char * filename);
	~C7zFile(void);
};


#endif