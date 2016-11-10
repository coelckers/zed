// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#pragma inline_depth(1)


#define _CRT_SECURE_NO_WARNINGS

#include <wx/wxprec.h>
#include <wx/filename.h>
#include <wx/listctrl.h>
#include <wx/dir.h>
#include <wx/mstream.h>
#include <wx/image.h>
#include <wx/sysopt.h>
#include <wx/spinctrl.h>
#include <wx/progdlg.h>
#include <wx/statline.h>
#include <wx/tglbtn.h>
#include <wx/html/htmprint.h>
#include <wx/stc/stc.h>
#include <wx/fdrepdlg.h>
#include <wx/generic/numdlgg.h>
#include <wx/notebook.h>
#include <wx/cmndata.h>
#include <wx/colordlg.h>

#include <CommCtrl.h>

/*
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
*/


// TODO: reference additional headers your program requires here

// MSVC: Disable exception warning. ZEd uses exceptions but doesn't need the unwind
// handling which only serves to massively inflate the code.
#pragma warning(disable:4530)
#pragma warning(disable:4996)

#include "mystdint.h"
