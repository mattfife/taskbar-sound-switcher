// ----------------------------------------------------------------------------
// devicediscovery.h
// Audio device discovery/setting routines (many undocumented)
// @Author: Matt Fife
// @Copyright 2015
// ----------------------------------------------------------------------------
#pragma once
#include "stdafx.h"
#include <assert.h>

// Routines used to enumrate and set current audio device
void DiscoverAllAudioOutputDevices(std::vector<std::wstring>& enumeratedDeviceList);
int DiscoverCurrentAudioOutputDevice(int &deviceSwitchListIndex);
HRESULT SetAudioPlaybackDevice(LPCWSTR devID);
int SetActiveAudioOutputDevice(const int deviceSwitchListIndex);