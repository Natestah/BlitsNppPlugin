#pragma once

#include "PluginInterface.h"

#define NPOS 0xFFFFFFFFFFFFFFFF

typedef unsigned long long int ui64;
typedef signed long long int i64;
typedef unsigned int ui32;
typedef signed int i32;
typedef unsigned short int ui16;
typedef signed short int i16;
typedef unsigned char ui8;
typedef signed char i8;

// Notepad++ data
const TCHAR NPP_PLUGIN_NAME[] = TEXT("Blitz Search");
const int nbFunc = 3;

// Utility functions
void pluginInit(HANDLE hModule);
void pluginCleanUp();
void commandMenuInit();
void commandMenuCleanUp();
bool setCommand(size_t index, const TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk = NULL, bool check0nInit = false);
void RunBlitzIPCCommand(const char *, const wchar_t *);
// Command functions
void runSearchThis();
void GetContextForSearch(char& search_string);
void runReplaceThis();
void visitHomepage();
void startBlitz();
