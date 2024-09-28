#include "PluginDefinition.h"
#include "menuCmdID.h"

#include <stdlib.h>
#include <time.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <iostream>
#include <fstream>

using namespace std;

struct Selection
{
	ui64 beg;
	ui64 end;
	ui64 idx;
};

const TCHAR sectionName[] = TEXT("Insert Extension");
const TCHAR keyName[] = TEXT("doCloseTag");

FuncItem funcItem[nbFunc];
NppData nppData; // The data of Notepad++ that you can use in your plugin commands
HWND scintilla;

Selection *selects;
ShortcutKey blitz_search_this_sh_key;

i64 m2scintilla(UINT id, ui64 wp, ui64 lp);
i64 m2scintilla(UINT id, ui64 wp);
i64 m2scintilla(UINT id);
void updateScintillaHWND();

DWORD __declspec(nothrow) commDbLoadingThread(LPVOID lp);

#pragma warning( suppress : 4100 )
void pluginInit(HANDLE hModule)
{
	CreateThread(NULL, 0, commDbLoadingThread, NULL, 0, NULL);
}

void pluginCleanUp()
{
}

void commandMenuInit()
{
	blitz_search_this_sh_key._isAlt = false;
	blitz_search_this_sh_key._isCtrl = false;
	blitz_search_this_sh_key._isShift = false;
	blitz_search_this_sh_key._key = 0x77; // VK_F8	0x77	F8 key
	
	setCommand(0, TEXT("Blitz Search This"), runSearchThis, &blitz_search_this_sh_key, true);
	setCommand(1, TEXT("Blitz Replace This"), runReplaceThis, NULL, false);
	setCommand(2, TEXT("Plug-in homepage"), visitHomepage, NULL, false);
}

bool setCommand(size_t index, const TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit)
{
	if(index >= nbFunc)
		return false;

	if(!pFunc)
		return false;

	lstrcpy(funcItem[index]._itemName, cmdName);
	funcItem[index]._pFunc = pFunc;
	funcItem[index]._init2Check = check0nInit;
	funcItem[index]._pShKey = sk;

	return true;
}

void commandMenuCleanUp()
{
}


i64 m2scintilla(UINT id, ui64 wp, ui64 lp)
{
	return SendMessage(scintilla, id, (WPARAM)wp, (LPARAM)lp);
}

i64 m2scintilla(UINT id, ui64 wp)
{
	return SendMessage(scintilla, id, (WPARAM)wp, NULL);
}

i64 m2scintilla(UINT id)
{
	return SendMessage(scintilla, id, NULL, NULL);
}

void updateScintillaHWND() // Update the current scintilla
{
	int scintilla_used = -1;
	SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&scintilla_used);
	if(scintilla_used == -1) // Scintilla 0 -> Main View in multivew (Left) | 1 -> Second view (Right)
	{
		return;
	}
	scintilla = (scintilla_used == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
}

#pragma warning( suppress : 4100 )
DWORD __declspec(nothrow) commDbLoadingThread(LPVOID lp)
{
	return 0;
}

void runSearchThis()
{
	static char ext_str[MAX_PATH];
	GetContextForSearch(*ext_str);
	RunBlitzIPCCommand(ext_str, L"SET_SEARCH");
	startBlitz();
}

void runReplaceThis()
{
	static char ext_str[MAX_PATH];
	GetContextForSearch(*ext_str);
	RunBlitzIPCCommand(ext_str, L"SET_REPLACE");
	startBlitz();
}

void GetContextForSearch(char& search_string)
{
	updateScintillaHWND();
	bool wholeWord = false;
	ui64 sel_n = m2scintilla(SCI_GETSELECTIONS); // There is always at least one selection
	selects = (Selection*)VirtualAlloc(0, sel_n * sizeof(Selection), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if (sel_n == 1)
	{
		selects[0].beg = m2scintilla(SCI_GETSELECTIONNSTART, 0);
		selects[0].end = m2scintilla(SCI_GETSELECTIONNEND, 0);
		selects[0].idx = 0;

		if (selects[0].beg == selects[0].end)
		{
			m2scintilla(SCI_WORDLEFT, 0, 0);
			m2scintilla(SCI_WORDRIGHTENDEXTEND, 0, 0);
			wholeWord = true;
		}
	}


	if (wholeWord)
	{
		search_string = '@';
		m2scintilla(SCI_GETSELTEXT, 0, (ui64)(&search_string+1));
	}
	else
	{
		m2scintilla(SCI_GETSELTEXT, 0, (ui64)(&search_string));
	}
}

void RunBlitzIPCCommand(const char * search_string, const wchar_t * ipcID)
{
	wchar_t* app_data = NULL;
	SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &app_data);
	wchar_t npp_poormans_ipc[MAX_PATH];
	wcscpy(npp_poormans_ipc, app_data);
	wcscat(npp_poormans_ipc, L"\\NathanSilvers\\POORMANS_IPC");
	CreateDirectoryW(npp_poormans_ipc, NULL);
	wcscat(npp_poormans_ipc, L"\\");
	wcscat(npp_poormans_ipc, ipcID);
	wcscat(npp_poormans_ipc, L".txt");
	ofstream out_file;
	out_file.open(npp_poormans_ipc);
	out_file << search_string;
	out_file.close();
}

void visitHomepage()
{
	static const char* link = "https://natestah.com";

	ShellExecuteA( // =============================================================================
		NULL,				//  [I|O]  Handle to parent window that will show error or UI messages
		"open",				//  [I|O]  Verb string -> open|edit|explore|find|open|print|runas|NULL(def)
		link,				//    [I]  File or object that the verb is beeing executed on
		NULL,				//  [I|O]  Cmd arguments to pass to the file, if it is exe file
		NULL,				//  [I|O]  Default working directory of the action NULL -> current dir
		SW_SHOWNORMAL);		//    [I]  Parameter that sets default nCmdShow status of the window
	// ============================================================================================
}


void startBlitz()
{
	wchar_t* app_data = NULL;
	SHGetKnownFolderPath(FOLDERID_ProgramFilesX64, 0, NULL, &app_data);
	wchar_t npp_appdata_lang[MAX_PATH];
	wcscpy(npp_appdata_lang, app_data);
	wcscat(npp_appdata_lang, L"\\Blitz\\Blitz.exe");

	if (!PathFileExistsW(npp_appdata_lang))
	{
		visitHomepage();
		return;
	}

	ShellExecuteW( // =============================================================================
		NULL,				//  [I|O]  Handle to parent window that will show error or UI messages
		L"open",				//  [I|O]  Verb string -> open|edit|explore|find|open|print|runas|NULL(def)
		npp_appdata_lang,				//    [I]  File or object that the verb is beeing executed on
		NULL,				//  [I|O]  Cmd arguments to pass to the file, if it is exe file
		NULL,				//  [I|O]  Default working directory of the action NULL -> current dir
		SW_SHOWNORMAL);		//    [I]  Parameter that sets default nCmdShow status of the window
	// ============================================================================================
}