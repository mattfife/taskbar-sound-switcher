// ----------------------------------------------------------------------------
// deviceselectdialog.h
// Handles the audio device selection dialog
// @Author: Matt Fife
// @Copyright 2015
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "main.h"
#include "deviceselectdialog.h"
#include "devicediscovery.h"

// DeviceSelectionDialogProc
// This routine handles the events from the device selection dialog box 
//
// Parameters:
//	Standard windows proc parameters
//
// Return values:
//	TRUE	Event handled
//	FALSE	Event not handled
BOOL CALLBACK DeviceSelectionDialogProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	HWND hwndListBox;
	int count = 0;
	int *buf = NULL;

	switch (Message)
	{
		//case WM_CREATE:
	case WM_INITDIALOG:
		// populate the listbox	with all the discovered devices
		for (unsigned int i = 0; i < g_EnumeratedDeviceList.size(); i++)
		{
			SendDlgItemMessage(hwnd, IDC_LIST1, LB_ADDSTRING, 0, (LPARAM)g_EnumeratedDeviceList[i].c_str());
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			// get number of items selected
			hwndListBox = GetDlgItem(hwnd, IDC_LIST1);
			count = (int)SendMessage(hwndListBox, LB_GETSELCOUNT, 0, 0);

			// get the list of selected items
			buf = (int*)malloc(count*sizeof(int));
			assert(nullptr != buf);
			SendMessage(hwndListBox, LB_GETSELITEMS, (WPARAM)count, (LPARAM)buf);

			// clear the recorded list of selected items			
			g_EnumeratedDeviceListSwitchIndexes.clear();

			// store the selected items
			for (int i = 0; i < count; i++)
			{
				g_EnumeratedDeviceListSwitchIndexes.push_back(buf[i]);
			}

			free(buf);

			// figure out if any of these are the current audio device
			DiscoverCurrentAudioOutputDevice(g_DeviceSwitchListIndex);

			// the the audio source off the list
			SetActiveAudioOutputDevice(g_DeviceSwitchListIndex);

			// change the icon to speakers
			ChangeIcon(g_hWnd);

			EndDialog(hwnd, IDOK);
			break;

		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

// SelectDevicesDialog
// Routine discovers all audio output on this machine and puts them into a dialog  
// box for selection.  The user can select as many of the items as they want and
// upon pressing ok, it then writes out which items the user selected to the 
// config file and stores indexes to those devices in this list
// g_EnumeratedDeviceListSwitchIndexes
//
// Parameters:
//	none
//
// Return values:
//	none
void SelectDevicesDialog()
{
	// discover the entire list of devices into a list of strings
	DiscoverAllAudioOutputDevices(g_EnumeratedDeviceList);

	// pop up a dialog box to let user choose what to swap between
	if (0 != g_EnumeratedDeviceList.size())
	{
		if (DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DEVICE_SELECT), g_hWnd, (DLGPROC)DeviceSelectionDialogProc) == IDOK)
		{
			// write out the selected devices to the config file
			WriteDeviceToggleStrings();

			if (0==g_EnumeratedDeviceListSwitchIndexes.size())
			{
				MessageBox(nullptr, L"There were no audio devices selected to toggle.", L"No audio devices selected", MB_OK);
			}
		}
	}
	else
	{
		MessageBox(nullptr, L"Sorry - there were no audio devices found to select between.", L"No audio devices", MB_OK);
	}
}