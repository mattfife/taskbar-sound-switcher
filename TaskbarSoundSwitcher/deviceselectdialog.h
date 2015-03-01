// ----------------------------------------------------------------------------
// deviceselectdialog.h
// Handles the audio device selection dialog
// @Author: Matt Fife
// @Copyright 2015
// ----------------------------------------------------------------------------
#pragma once
#include "stdafx.h"

// Open the device selection dialog box
void SelectDevicesDialog();

// Device selection dialog proc
BOOL CALLBACK DeviceSelectionDialogProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);


