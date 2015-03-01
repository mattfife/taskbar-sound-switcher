// ----------------------------------------------------------------------------
// main.h
// Main message handling routines for the app
// @Author: Matt Fife
// @Copyright 2015
// ----------------------------------------------------------------------------
#pragma once
#include "resource.h"

#include <fstream>
#include <vector>
#include "Propvarutil.h"	
#pragma comment(lib, "Propsys.lib")

#define MAX_LOADSTRING 100
#define MAX_DEVICE_STRING_LENGTH 4096
extern const UINT WM_APP_TRAY_EVENT;
extern HWND g_hWnd;									// app window
extern HINSTANCE g_hInstance;						// app instance
extern HICON g_hSpeakerIcon;						// tray icon
extern HICON g_hHeadphonesIcon;						// tray icon

// Global variables					
extern int	g_DeviceSwitchListIndex;							// the index of the current output device
extern std::vector<std::wstring> g_EnumeratedDeviceList;		// list of all devices (strings)
extern std::vector<int> g_EnumeratedDeviceListSwitchIndexes;	// list of indexes into the device list 

// function definitions
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void LoadStringSafe(UINT nStrID, LPTSTR szBuf, UINT nBufLen);	
int	 ChangeIcon(HWND hWnd);
void ReadDeviceToggleStrings();
int WriteDeviceToggleStrings();

