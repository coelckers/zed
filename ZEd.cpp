#include "stdafx.h"
#include "ZEd.h"
#include "ResourceFile.h"
#include "texture.h"
#include "cmdlib.h"


// the global application state
FConfigFile config;					
wxString CurrentConfig;
ConfigManager confman;
CLevel * CurrentLevel;	// the level that is currently being edited
GameConfig * cgc=NULL;



static char * commandnames[]={
	"file_new",
	"file_wadmanager",
	"file_selectiwad",
	"file_preferences",

};

//==========================================================================
//
//
//
//==========================================================================

void I_Error (const char *error, ...)
{
	static char errmsg[2048];	// Using wxString here causes a crash.
    va_list argptr;

    va_start (argptr, error);
	_vsnprintf(errmsg, 2048, error, argptr);
    va_end (argptr);
	OutputDebugString(errmsg);
	OutputDebugString("\n");
	if (IsDebuggerPresent())
	{
		DebugBreak();
	}
    throw CRecoverableError (errmsg);
}


wxString GetConfigDir()
{
	wxString path, name;
	bool isdir = false;

	if (DirEntryExists("configs/colors.cfg", &isdir) && !isdir)
	{
		return "configs/";
	}
	else
	{
		wxFileName::SplitPath(wxString(wxGetApp().argv[0]), &path, &name, NULL);
		return path + "/configs/";
	}
}

//==========================================================================
//
//
//
//==========================================================================

DirectorySaver::DirectorySaver(const char * key)
{
	m_configkey = key;
	config.SetSection("Directorys", true);
	m_thisdir = config.GetValueForKey(key);

	m_olddir = wxGetCwd();
	if (m_thisdir) wxSetWorkingDirectory(m_thisdir);
}

//==========================================================================
//
//
//
//==========================================================================

DirectorySaver::~DirectorySaver()
{
	wxString cwd = wxGetCwd();
	config.SetSection("Directorys", true);
	config.SetValueForKey(m_configkey.c_str(), cwd.c_str());
	wxSetWorkingDirectory(m_olddir.c_str());
}



IMPLEMENT_APP(ZEdApp)

//==========================================================================
//
//
//
//==========================================================================

void ZEdApp::LoadConfig()
{
	wxString path, name;
	wxFileName ini;
	bool isdir = false;

	if (DirEntryExists("ZEd.ini", &isdir) && !isdir)
	{
		config.ChangePathName("ZEd.ini");
	}
	else
	{
		wxFileName::SplitPath(wxString(argv[0]), &path, &name, NULL);
		ini.Assign(path, name, "ini");
		config.ChangePathName(ini.GetLongPath().c_str());
	}

	config.LoadConfigFile(NULL, NULL);

	if (config.SetSection("config"))
	{
		CurrentConfig = config.GetValueForKey("doomconfig");
	}
	CLevel::LoadSettings();
	ParseColors();
	SetDefaultBindings();
}

//==========================================================================
//
//
//
//==========================================================================

void ZEdApp::SaveConfig()
{
	if (config.SetSection("config", true))
	{
		config.SetValueForKey("doomconfig", CurrentConfig.c_str());
	}
	config.WriteConfigFile();
	config.ClearConfig();
}


//==========================================================================
//
//
//
//==========================================================================

bool ZEdApp::OnInit()
{
	try
	{
		wxString path;

/*
#ifndef _DEBUG
		wxFileName::SplitPath(wxString(wxGetApp().argv[0]), &path, NULL, NULL);
		wxSetWorkingDirectory(path);
#endif
*/
		TArray<const char *> iwads;

		wxSystemOptions::SetOption(wxT("msw.remap"), 0);
		wxSystemOptions::SetOption(wxT("msw.notebook.themed-background"), 0);
		wxImage::AddHandler(new wxPNGHandler);
		wxImage::AddHandler(new wxJPEGHandler);

		frame = new ZEdFrame((wxFrame *)NULL, wxID_ANY, _T("ZEd v2.2"),
							wxDefaultPosition, wxSize(500, 400),
							wxDEFAULT_FRAME_STYLE | wxMAXIMIZE);

		frame->Show(true);
		frame->Maximize(true);
		SetTopWindow(frame);
		LoadConfig();

		if (config.SetSection("IWADs"))
		{
			const char * iwad = config.GetValueForKey(CurrentConfig.c_str());
			if (iwad!=NULL && *iwad!=0) iwads.Push(iwad);
			for(int i=1;i<argc;i++)
			{
				iwads.Push(argv[i]);
			}
		}
		CResourceFile::OpenResourceFiles(iwads);
		return true;
	}
	catch(CDoomError * err)
	{
		wxMessageBox(err->GetMessage(), ZED_CAPTION);
		return false;
	}
}

//==========================================================================
//
//
//
//==========================================================================

int ZEdApp::OnExit()
{
	delete texman;
	texman=NULL;
	SaveConfig();
	CResourceFile::CloseResourceFiles();
	cgc=NULL;
	confman.RefreshConfigs();	// This deletes the internal menus which must happen before wxApp::OnExit
	return wxApp::OnExit();
}