// ----------------------------------------------------------------------------
// main.h
// Main message handling routines for the app
// @Author: Matt Fife
// @Copyright 2015
// ----------------------------------------------------------------------------
#include "stdafx.h"

#include <stdlib.h>
#include <stdio.h>

#include <shellapi.h>		// for NOTIFYICONDATA
#include <algorithm>		// std::transform
#include <direct.h>			// getenv_s()
#include <sys/stat.h>		// _stat()

#include "resource.h"		// for icon IDI_xxxx identifiers
#include "main.h"
#include "deviceselectdialog.h"
#include "devicediscovery.h"

// App variables
const UINT	WM_APP_TRAY_EVENT = WM_USER;
HINSTANCE	g_hInstance = NULL;				
HICON		g_hSpeakerIcon = NULL;
HICON		g_hHeadphonesIcon = NULL;
HWND		g_hWnd = NULL;							

// audio device lists
int	g_DeviceSwitchListIndex = -1;						
std::vector<std::wstring> g_EnumeratedDeviceList;
std::vector<int> g_EnumeratedDeviceListSwitchIndexes;

// LoadStringSafe
// Helper function to load string resources
//
// Parameters:
//	nStrID	Resource IDS for the string
//	szBuf	Pointer to a string buffer
//	nBufLen	Length of the szBuf buffer in bytes
//
// Return values:
//	none
void LoadStringSafe(UINT nStrID, LPTSTR szBuf, UINT nBufLen)
{
	UINT nLen = LoadString(g_hInstance, nStrID, szBuf, nBufLen);
	if (nLen >= nBufLen)
		nLen = nBufLen - 1;
	szBuf[nLen] = 0;
}

// BuildResourceFilenameString
// Locates the configuration file in %APPDATA%/TaskbarSoundSwitcher/ folder per
// Microsoft's programming guidelines for config files. The config filename is 
// AudioSources.cfg and holds the 'friendly' device string names
//
// Parameters:
//	fullPathFilename	The full file path to the resource file
//
// Return values:
//	0	Success - fullPathFilename will contain the full file path
//	-1	Failure - full path not found
int BuildResourceFilenameString(std::string& fullPathFilename)
{
	size_t requiredSize = 0;

	// empty the string
	fullPathFilename.clear();

	// does %APPDATA% exist?
	errno_t error = getenv_s(&requiredSize, NULL, 0, "APPDATA");
	if (0 == requiredSize)
	{
		return -1;
	}

	// get %APPDATA% string
	char* appDataDir = (char*)malloc(requiredSize * sizeof(char));
	error = getenv_s(&requiredSize, appDataDir, requiredSize, "APPDATA");

	// test if APPDATA dir exists
	std::string dirName = appDataDir;
	dirName += "\\TasbarSoundSwitcher";
	free(appDataDir);

	struct _stat buf;
	int result = _stat(dirName.c_str(), &buf);
	if (0 != result)
	{
		// dir doesn't exist, create it
		result = _mkdir(dirName.c_str());
		if (0 != result)
		{
			// failed to create the output dir
			return -1;
		}
	}

	// attempt to open the config file
	fullPathFilename = dirName + "\\AudioSources.cfg";
	return 0;
}


// ChangeIcon
// Change the app icon to either headphones or speakers based on some name heuristics
//
// Parameters:
//	hWnd	Window handle of the object that needs the icon changed
//
// Return values:
//	0	Success - icon successfully changed
//	-1	Failure - icon not changed
int ChangeIcon(HWND hWnd)
{
	// change the taskbar icon
	NOTIFYICONDATA stData;
	ZeroMemory(&stData, sizeof(stData));
	stData.cbSize = sizeof(stData);
	stData.hWnd = hWnd;
	stData.uFlags = NIF_ICON;

	// do a little bit of heuristic logic here to figure out which icon might be more appropriate
	std::wstring currentDeviceName = g_EnumeratedDeviceList[g_EnumeratedDeviceListSwitchIndexes[g_DeviceSwitchListIndex]];
	std::transform(currentDeviceName.begin(), currentDeviceName.end(), currentDeviceName.begin(), ::tolower);
	if ((std::string::npos != currentDeviceName.find(L"headphone")) || (std::string::npos != currentDeviceName.find(L"headset")))
	{
		stData.hIcon = g_hHeadphonesIcon;
	}
	else
	{
		stData.hIcon = g_hSpeakerIcon;
	}

	if (!Shell_NotifyIcon(NIM_MODIFY, &stData))
		return -1; // oops	

	return 0;
}


// ReadDeviceToggleStrings 
// Open and read the contents of the config file or pop up the device selection 
// dialog if the config file doesn't exist
//
// Parameters:
//	none
//
// Return values:
//	none
void ReadDeviceToggleStrings()
{
	FILE* fp = nullptr;
	
	// Attempt to open the resource file
	std::string fullPathFilename;
	int result = BuildResourceFilenameString(fullPathFilename);
	if (0 == result)
	{
		result = fopen_s(&fp, fullPathFilename.c_str(), "r");
	}

	// no config file found - manually select the audio devices to toggle
	if (NULL == fp)
	{		
		SelectDevicesDialog();
	}
	else
	{
		// check if file has any strings
		fseek(fp, 0L, SEEK_END);
		long fileSize = ftell(fp);
		fseek(fp, 0L, SEEK_SET);

		// If app unexpectedly closed in middle of file write, this file 
		// might be empty.  If file is empty, let user re-select audio devices
		if (0 == fileSize)
		{
			fclose(fp);
			SelectDevicesDialog();
		}
		else
		{			
			// read audio device entries from the file
			int index = 0;
			wchar_t line[MAX_DEVICE_STRING_LENGTH];
			while (fgetws(line, MAX_DEVICE_STRING_LENGTH, fp))
			{
				// strip off anything after newline
				std::wstring s = line;				
				std::wstring stripped = s.substr(0, s.find(L"\n"));

				if (stripped.size())
				{
					g_EnumeratedDeviceList.push_back(stripped);
					g_EnumeratedDeviceListSwitchIndexes.push_back(index);
					index++;
				}
			}
		}
		fclose(fp);
	}

}

//	WriteDeviceToggleStrings 
//	Writes out the current list of selected togglable audio devices to the 
//	resource file.
//
// Parameters:
//	none
//
// Return values:
//	0	Success - device strings written to config file successfully
//	-1	Failure - device strings not written to file
int WriteDeviceToggleStrings()
{
	int result = -1;

	// If there are any selected devices to switch, save them
	if (0!=g_EnumeratedDeviceListSwitchIndexes.size())
	{	
		// open and read the device id strings
		FILE *fp = NULL;
		
		// open the resource file
		std::string fullPathFilename;
		int result = BuildResourceFilenameString(fullPathFilename);
		if (0 == result)
		{
			result = fopen_s(&fp, fullPathFilename.c_str(), "w");
			if (0 == result)
			{
				// save the selected audio device strings to a file
				for (unsigned int i = 0; i < g_EnumeratedDeviceListSwitchIndexes.size(); i++)
				{
					fputws(g_EnumeratedDeviceList[g_EnumeratedDeviceListSwitchIndexes[i]].c_str(), fp);
					fputws(L"\n", fp);
				}
				fclose(fp);
				result = 0;
			}
		}
	}
	return result;
}

//  WndProc
//  Process messages for the main window.
//
// Parameters:
//	Standard windows proc parameters
//
// Return values:
//	0	Event handled
//	-1	Event not handled
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	int index = 0;
	HMENU hMenu = nullptr;

	switch (message)
	{
	// Initialization
	case WM_CREATE:
		// Load icons
		g_hSpeakerIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_SPEAKERS));
		g_hHeadphonesIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_HEADPHONES));

		// Set the app icon, hover tip, and callback message id
		// for this icon-based app
		NOTIFYICONDATA stData;
		ZeroMemory(&stData, sizeof(stData));
		stData.cbSize = sizeof(stData);
		stData.hWnd = hWnd;
		stData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		stData.uCallbackMessage = WM_APP_TRAY_EVENT;
		stData.hIcon = g_hSpeakerIcon;
		LoadStringSafe(IDS_TIP, stData.szTip, _countof(stData.szTip));
		if (!Shell_NotifyIcon(NIM_ADD, &stData))
			return -1; 
		break;

	// handle all events that happen over the tray icon
	case WM_APP_TRAY_EVENT:		
		switch (lParam)
		{
		// left double-click switches to the next audio device in the switch list
		case WM_LBUTTONDBLCLK:			
			if (g_EnumeratedDeviceListSwitchIndexes.size())
			{
				// go to next audio device index
				g_DeviceSwitchListIndex = (g_DeviceSwitchListIndex + 1) % g_EnumeratedDeviceListSwitchIndexes.size();

				// set the playback device
				SetActiveAudioOutputDevice(g_DeviceSwitchListIndex);

				// change the icon based on the device
				ChangeIcon(hWnd);
			}
			break;

		// right-click makes pop-up menu appear
		case WM_RBUTTONDOWN:
			hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_POPUP));
			if (hMenu)
			{
				HMENU hSubMenu = GetSubMenu(hMenu, 0);
				if (hSubMenu)
				{
					// clear any/all items in the list
					int menuItemCount = GetMenuItemCount(hSubMenu);
					for (int i = 0; i < menuItemCount; i++)
					{
						DeleteMenu(hSubMenu, 0, MF_BYPOSITION);
					}

					// add all the selected/loaded items to the menu 
					for (unsigned int i = 0; i < g_EnumeratedDeviceListSwitchIndexes.size(); i++)
					{
						AppendMenu(hSubMenu, MF_STRING, ID_ROOT_ITEM_0 + i, g_EnumeratedDeviceList[g_EnumeratedDeviceListSwitchIndexes[i]].c_str());
					}

					// add quit and re-select items to the list
					AppendMenu(hSubMenu, MF_SEPARATOR, 0, L"");
					AppendMenu(hSubMenu, MF_STRING, ID_ROOT_RESELECT, L"Re-select Devices");
					AppendMenu(hSubMenu, MF_STRING, ID_ROOT_QUIT, L"Quit");

					// track the popup menu
					POINT stPoint;
					GetCursorPos(&stPoint);
					TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, stPoint.x, stPoint.y, 0, hWnd, NULL);
				}
				DestroyMenu(hMenu);
			}
			break;
		}
		break;

	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		// handle pop-up menu item 'quit'
		case ID_ROOT_QUIT:
			DestroyWindow(hWnd);
			return 0;

		// handle pop-up menu item 'reselect audio devices'
		case ID_ROOT_RESELECT:
			SelectDevicesDialog();
			return 0;

		// handle the pop-up menu device selections
		case ID_ROOT_ITEM_0 + 0:
		case ID_ROOT_ITEM_0 + 1:
		case ID_ROOT_ITEM_0 + 2:
		case ID_ROOT_ITEM_0 + 3:
		case ID_ROOT_ITEM_0 + 4:
		case ID_ROOT_ITEM_0 + 5:
		case ID_ROOT_ITEM_0 + 6:
		case ID_ROOT_ITEM_0 + 7:
		case ID_ROOT_ITEM_0 + 8:
		case ID_ROOT_ITEM_0 + 9:
		case ID_ROOT_ITEM_0 + 10:
		case ID_ROOT_ITEM_0 + 11:
		case ID_ROOT_ITEM_0 + 12:
		case ID_ROOT_ITEM_0 + 13:
		case ID_ROOT_ITEM_0 + 14:
		case ID_ROOT_ITEM_0 + 15:
		case ID_ROOT_ITEM_0 + 16:
		case ID_ROOT_ITEM_0 + 17:
		case ID_ROOT_ITEM_0 + 18:
		case ID_ROOT_ITEM_0 + 19:
			g_DeviceSwitchListIndex = wmId - ID_ROOT_ITEM_0;
			
			// set the audio source to the selected item
			SetActiveAudioOutputDevice(g_DeviceSwitchListIndex);
			
			// change the icon (if necessary)
			ChangeIcon(hWnd);
			return 0;
			
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


// _tWinMain
//  Entry point for the application
//
// Parameters:
//	Standard windows main() parameters
//
// Return values:
//	0	Program exited successfully
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	// no command line parameters
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
	// set up struct to create window class
	WNDCLASS wcNotificationAreaClass;
	ZeroMemory(&wcNotificationAreaClass, sizeof(wcNotificationAreaClass));
	wcNotificationAreaClass.lpszClassName = _T("Taskbar_Sound_Switcher");

	// find if window already exists   
	g_hWnd = FindWindow(wcNotificationAreaClass.lpszClassName, NULL);

	if (g_hWnd)
	{
		PostMessage(g_hWnd, WM_APP_TRAY_EVENT, 0, WM_LBUTTONDBLCLK);
	}
	else
	{
		// set up the window class register app
		wcNotificationAreaClass.hInstance = hInstance;
		wcNotificationAreaClass.lpfnWndProc = WndProc;

		ATOM classRC = RegisterClass(&wcNotificationAreaClass);
		if (classRC)
		{
			g_hInstance = hInstance;
			if (g_hWnd = CreateWindow((LPCTSTR)classRC, _T(""), 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL))
			{
				// Get list of devices to toggle between either via dialog box 
				// or via file
				ReadDeviceToggleStrings();

				// Figure out what the current audio device is and set the 
				// the default/current playback device index if it's also  
				// in the current device toggle list
				DiscoverCurrentAudioOutputDevice(g_DeviceSwitchListIndex);

				// Set the audio output to one of the devices in the 
				// selected list
				SetActiveAudioOutputDevice(g_DeviceSwitchListIndex);

				// Change the icon to match the playback device
				ChangeIcon(g_hWnd);


				// handle the message loop
				MSG Msg;
				while (GetMessage(&Msg, NULL, 0, 0) > 0)
				{
					TranslateMessage(&Msg);
					DispatchMessage(&Msg);
				}

				if (IsWindow(g_hWnd))
					DestroyWindow(g_hWnd);
			}
			UnregisterClass((LPCTSTR)classRC, g_hInstance);
		}
	}

	return 0;
}