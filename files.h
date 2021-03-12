#ifndef FILES_H
#define FILES_H

#include <stdio.h>
#include "zip/zlib/zlib.h"
#include "zip/bzip2/bzlib.h"
#include "zip/lzma/c/LzmaDec.h"
#include "doomtype.h"
#include "tarray.h"
#include "m_swap.h"

class FileReader
{
public:
	FileReader ();
	FileReader (const char *filename);
	FileReader (FILE *file);
	FileReader (FILE *file, long length);
	bool Open (const char *filename);
	virtual ~FileReader ();

	virtual long Tell () const;
	virtual long Seek (long offset, int origin);
	virtual long Read (void *buffer, long len);
	virtual char *Gets(char *strbuf, int len);
	long GetLength () const { return Length; }

	// If you use the underlying FILE without going through this class,
	// you must call ResetFilePtr() before using this class again.
	void ResetFilePtr ();

	FILE *GetFile () const { return File; }

	FileReader &operator>> (BYTE &v)
	{
		Read (&v, 1);
		return *this;
	}

	FileReader &operator>> (SBYTE &v)
	{
		Read (&v, 1);
		return *this;
	}

	FileReader &operator>> (WORD &v)
	{
		Read (&v, 2);
		v = LittleShort(v);
		return *this;
	}

	FileReader &operator>> (SWORD &v)
	{
		Read (&v, 2);
		v = LittleShort(v);
		return *this;
	}

	FileReader &operator>> (DWORD &v)
	{
		Read (&v, 4);
		v = LittleLong(v);
		return *this;
	}

	uint8_t ReadUInt8()
	{
		uint8_t v = 0;
		Read(&v, 1);
		return v;
	}

	int8_t ReadInt8()
	{
		int8_t v = 0;
		Read(&v, 1);
		return v;
	}

	uint16_t ReadUInt16()
	{
		uint16_t v = 0;
		Read(&v, 2);
		return LittleShort(v);
	}

	int16_t ReadInt16()
	{
		int16_t v = 0;
		Read(&v, 2);
		return LittleShort(v);
	}

	int16_t ReadInt16BE()
	{
		int16_t v = 0;
		Read(&v, 2);
		return BigShort(v);
	}

	uint32_t ReadUInt32()
	{
		uint32_t v = 0;
		Read(&v, 4);
		return LittleLong(v);
	}

	int32_t ReadInt32()
	{
		int32_t v = 0;
		Read(&v, 4);
		return LittleLong(v);
	}

	uint32_t ReadUInt32BE()
	{
		uint32_t v = 0;
		Read(&v, 4);
		return BigLong(v);
	}

	int32_t ReadInt32BE()
	{
		int32_t v = 0;
		Read(&v, 4);
		return BigLong(v);
	}

	uint64_t ReadUInt64()
	{
		uint64_t v = 0;
		Read(&v, 8);
		// Prove to me that there's a relevant 64 bit Big Endian architecture and I fix this! :P
		return v;
	}

protected:
	FileReader (const FileReader &other, long length);

	char *GetsFromBuffer(const char * bufptr, char *strbuf, int len);

	FILE *File;
	long Length;
	long StartPos;
	long FilePos;

private:
	long CalcFileLen () const;
protected:
	bool CloseOnDestruct;
};

// Wraps around a FileReader to decompress a zlib stream
class FileReaderZ
{
public:
	FileReaderZ (FileReader &file, bool zip=false);
	~FileReaderZ ();

	long Read (void *buffer, long len);

	FileReaderZ &operator>> (BYTE &v)
	{
		Read (&v, 1);
		return *this;
	}

	FileReaderZ &operator>> (SBYTE &v)
	{
		Read (&v, 1);
		return *this;
	}

	FileReaderZ &operator>> (WORD &v)
	{
		Read (&v, 2);
		v = LittleShort(v);
		return *this;
	}

	FileReaderZ &operator>> (SWORD &v)
	{
		Read (&v, 2);
		v = LittleShort(v);
		return *this;
	}

	FileReaderZ &operator>> (DWORD &v)
	{
		Read (&v, 4);
		v = LittleLong(v);
		return *this;
	}

	FileReaderZ &operator>> (fixed_t &v)
	{
		Read (&v, 4);
		v = LittleLong(v);
		return *this;
	}

private:
	enum { BUFF_SIZE = 4096 };

	FileReader &File;
	bool SawEOF;
	z_stream Stream;
	BYTE InBuff[BUFF_SIZE];

	void FillBuffer ();

	FileReaderZ &operator= (const FileReaderZ &) { return *this; }
};

// Wraps around a FileReader to decompress a bzip2 stream
class FileReaderBZ2
{
public:
	FileReaderBZ2 (FileReader &file);
	~FileReaderBZ2 ();

	long Read (void *buffer, long len);

	FileReaderBZ2 &operator>> (BYTE &v)
	{
		Read (&v, 1);
		return *this;
	}

	FileReaderBZ2 &operator>> (SBYTE &v)
	{
		Read (&v, 1);
		return *this;
	}

	FileReaderBZ2 &operator>> (WORD &v)
	{
		Read (&v, 2);
		v = LittleShort(v);
		return *this;
	}

	FileReaderBZ2 &operator>> (SWORD &v)
	{
		Read (&v, 2);
		v = LittleShort(v);
		return *this;
	}

	FileReaderBZ2 &operator>> (DWORD &v)
	{
		Read (&v, 4);
		v = LittleLong(v);
		return *this;
	}

	FileReaderBZ2 &operator>> (fixed_t &v)
	{
		Read (&v, 4);
		v = LittleLong(v);
		return *this;
	}

private:
	enum { BUFF_SIZE = 4096 };

	FileReader &File;
	bool SawEOF;
	bz_stream Stream;
	BYTE InBuff[BUFF_SIZE];

	void FillBuffer ();

	FileReaderBZ2 &operator= (const FileReaderBZ2 &) { return *this; }
};

// Wraps around a FileReader to decompress a lzma stream
class FileReaderLZMA
{
public:
	FileReaderLZMA (FileReader &file, size_t uncompressed_size, bool zip);
	~FileReaderLZMA ();

	long Read (void *buffer, long len);

	FileReaderLZMA &operator>> (BYTE &v)
	{
		Read (&v, 1);
		return *this;
	}

	FileReaderLZMA &operator>> (SBYTE &v)
	{
		Read (&v, 1);
		return *this;
	}

	FileReaderLZMA &operator>> (WORD &v)
	{
		Read (&v, 2);
		v = LittleShort(v);
		return *this;
	}

	FileReaderLZMA &operator>> (SWORD &v)
	{
		Read (&v, 2);
		v = LittleShort(v);
		return *this;
	}

	FileReaderLZMA &operator>> (DWORD &v)
	{
		Read (&v, 4);
		v = LittleLong(v);
		return *this;
	}

	FileReaderLZMA &operator>> (fixed_t &v)
	{
		Read (&v, 4);
		v = LittleLong(v);
		return *this;
	}

private:
	enum { BUFF_SIZE = 4096 };

	FileReader &File;
	bool SawEOF;
	CLzmaDec Stream;
	size_t Size;
	size_t InPos, InSize;
	size_t OutProcessed;
	BYTE InBuff[BUFF_SIZE];

	void FillBuffer ();

	FileReaderLZMA &operator= (const FileReaderLZMA &) { return *this; }
};

class MemoryReader : public FileReader
{
public:
	MemoryReader (const char *buffer, long length);
	~MemoryReader ();

	virtual long Tell () const;
	virtual long Seek (long offset, int origin);
	virtual long Read (void *buffer, long len);
	virtual char *Gets(char *strbuf, int len);

protected:
	const char * bufptr;
};

class FileWriter
{
protected:
	bool OpenDirect(const char* filename);

public:
	FileWriter(FILE* f = nullptr) // if passed, this writer will take over the file.
	{
		File = f;
	}
	virtual ~FileWriter()
	{
		Close();
	}

	static FileWriter* Open(const char* filename);

	virtual size_t Write(const void* buffer, size_t len);
	virtual long Tell();
	virtual long Seek(long offset, int mode);
	size_t Printf(const char* fmt, ...) GCCPRINTF(2, 3);
	virtual void Close()
	{
		if (File != NULL) fclose(File);
		File = nullptr;
	}

protected:

	FILE* File;

protected:
	bool CloseOnDestruct;
};

class BufferWriter : public FileWriter
{
protected:
	TArray<unsigned char> mBuffer;
public:

	BufferWriter() {}
	virtual size_t Write(const void* buffer, size_t len) override;
	TArray<unsigned char>* GetBuffer() { return &mBuffer; }
	TArray<unsigned char>&& TakeBuffer() { return std::move(mBuffer); }
};



#endif
