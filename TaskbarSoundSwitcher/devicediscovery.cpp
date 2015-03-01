// ----------------------------------------------------------------------------
// devicediscovery.h
// Audio device discovery/setting routines (many undocumented)
// @Author: Matt Fife
// @Copyright 2015
// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "main.h"
#include "devicediscovery.h"

// headers needed for undocumented device discovery routines
#include "windows.h"
#include "Mmdeviceapi.h"
#include "Mmreg.h"
#include "PolicyConfig.h"
#include "Propidl.h"
#include "Functiondiscoverykeys_devpkey.h"

#include "Propvarutil.h"	
#pragma comment(lib, "Propsys.lib")

// Much of this code is derived from the excellent work done by EreTIk on this
// blog entry (under the MIT license):
// http://www.daveamenta.com/2011-05/programmatically-or-command-line-change-the-default-sound-playback-device-in-windows-7/


// DiscoverAllAudioOutputDevices
// Enumerate the list of audio devices and store the string names into 
// the enumeratedDeviceList
//
// Parameters:
//	enumeratedDeviceList	A list of all audio device name strings.  Strings
//							match the name displayed in audio control panel
//
// Return values:
//	none
void DiscoverAllAudioOutputDevices(std::vector<std::wstring>& enumeratedDeviceList)
{
	// clear the list of devices
	enumeratedDeviceList.clear();

	// use undocumented routines to figure out the audio devices
	HRESULT hr = CoInitialize(NULL);
	if (SUCCEEDED(hr))
	{
		IMMDeviceEnumerator *pEnum = NULL;
		// Create a multimedia device enumerator.
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
			CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnum);
		if (SUCCEEDED(hr))
		{
			IMMDeviceCollection *pDevices;

			// Enumerate the output devices.
			hr = pEnum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pDevices);
			if (SUCCEEDED(hr))
			{
				UINT count;
				hr = pDevices->GetCount(&count);
				if (SUCCEEDED(hr))
				{
					for (UINT i = 0; i < count; i++)
					{
						IMMDevice *pDevice;
						hr = pDevices->Item(i, &pDevice);
						if (SUCCEEDED(hr))
						{
							LPWSTR wstrID = NULL;
							hr = pDevice->GetId(&wstrID);
							if (SUCCEEDED(hr))
							{
								IPropertyStore *pStore;
								hr = pDevice->OpenPropertyStore(STGM_READ, &pStore);
								if (SUCCEEDED(hr))
								{
									PROPVARIANT friendlyName;
									PropVariantInit(&friendlyName);
									hr = pStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);
									if (SUCCEEDED(hr))
									{
										// get and store the audio device's friendly string name
										WCHAR szTitle[MAX_DEVICE_STRING_LENGTH];
										HRESULT hr = PropVariantToString(friendlyName, szTitle, ARRAYSIZE(szTitle));
										std::wstring name = szTitle;

										enumeratedDeviceList.push_back(name);

										PropVariantClear(&friendlyName);
									}
									pStore->Release();
								}
							}
							pDevice->Release();
						}
					}
				}
				pDevices->Release();
			}
			pEnum->Release();
		}
	}
}


// DiscoverCurrentAudioOutputDevice
// Figure out the current/active audio output device - should be part of the 
// current discovered list.  
//
// Parameters:
//	deviceSwitchListIndex	Set to the index that matches the current audio 
//							output device
//
// Return values:
//	0	The current audio output device is one of the device switch list items
//	-1	If the current audio output device was not found
int DiscoverCurrentAudioOutputDevice(int &deviceSwitchListIndex)
{
	int ret_value = -1;

	// If the list of devices don't include the currently active device, just go with the 
	// first one by default
	deviceSwitchListIndex = 0;

	HRESULT hr = CoInitialize(NULL);
	if (SUCCEEDED(hr))
	{
		IMMDeviceEnumerator *pEnum = NULL;
		// Create a multimedia device enumerator.
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
			CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnum);
		if (SUCCEEDED(hr))
		{
			EDataFlow dataFlow = eRender;
			ERole role = eMultimedia;
			IMMDevice* pDefaultAudioEndpoint = nullptr;
			bool	defaultDeviceFound = false;

			hr = pEnum->GetDefaultAudioEndpoint(dataFlow, role, &pDefaultAudioEndpoint);
			if (SUCCEEDED(hr) && pDefaultAudioEndpoint)
			{
				defaultDeviceFound = true;

				IPropertyStore *pStore;
				hr = pDefaultAudioEndpoint->OpenPropertyStore(STGM_READ, &pStore);
				if (SUCCEEDED(hr))
				{
					PROPVARIANT friendlyName;
					PropVariantInit(&friendlyName);
					hr = pStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);
					if (SUCCEEDED(hr))
					{
						WCHAR szTitle[1024];
						HRESULT hr = PropVariantToString(friendlyName, szTitle, ARRAYSIZE(szTitle));

						// search the list of enumerated devices and find the current default one
						std::wstring name = szTitle;

						unsigned int i = 0;
						while ((i<g_EnumeratedDeviceListSwitchIndexes.size()) && (0 != ret_value))						
						{
							std::size_t found = name.find(g_EnumeratedDeviceList[g_EnumeratedDeviceListSwitchIndexes[i]]);
							if (found != std::string::npos)
							{
								deviceSwitchListIndex = i;
								ret_value = 0;
							}
							i++;
						}	
						PropVariantClear(&friendlyName);
					}
					pStore->Release();
				}	
				pDefaultAudioEndpoint->Release();
			}
			pEnum->Release();
		}
	}

	return ret_value;
}

// SetAudioPlaybackDevice
// Set the audio playback device to the one defined by the encoded devID string
//
// Parameters:
//	devID		The encoded device ID string of the audio device to set
//				as the current/active audio device
//
// Return values:
//	HRESULT		Indicates success/failure of set operation
HRESULT SetAudioPlaybackDevice(LPCWSTR devID)
{
	IPolicyConfigVista *pPolicyConfig;
	ERole reserved = eConsole;

	HRESULT hr = CoCreateInstance(__uuidof(CPolicyConfigVistaClient), NULL, CLSCTX_ALL, __uuidof(IPolicyConfigVista), (LPVOID *)&pPolicyConfig);
	if (SUCCEEDED(hr))
	{
		hr = pPolicyConfig->SetDefaultEndpoint(devID, reserved);
		pPolicyConfig->Release();
	}
	return hr;
}

// SetActiveAudioOutputDevice
// This sets the audio playback device to the one selected to by deviceSwitchListIndex
//
// Parameters:
//	deviceSwitchListIndex	The index in g_EnumeratedDeviceListSwitchIndexes switch
//				list that should be set as the current audio output device.
//
// Return values:
//	0		The desired audio device was set correctly
//	-1		The desired audio device set operation failed
int SetActiveAudioOutputDevice(const int deviceSwitchListIndex)
{
	int result = -1;
	std::wstring defaultDeviceString;
	
	HRESULT hr = CoInitialize(NULL);
	if (SUCCEEDED(hr))
	{
		IMMDeviceEnumerator *pEnum = NULL;
		// Create a multimedia device enumerator.
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
			CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnum);
		if (SUCCEEDED(hr))
		{
			IMMDeviceCollection *pDevices;

			// Enumerate the output devices.
			hr = pEnum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pDevices);
			if (SUCCEEDED(hr))
			{
				UINT count;
				hr = pDevices->GetCount(&count);
				if (SUCCEEDED(hr))
				{
					for (UINT i = 0; i < count; i++)
					{
						IMMDevice *pDevice;
						hr = pDevices->Item(i, &pDevice);
						if (SUCCEEDED(hr))
						{
							LPWSTR wstrID = NULL;
							hr = pDevice->GetId(&wstrID);
							if (SUCCEEDED(hr))
							{
								IPropertyStore *pStore;
								hr = pDevice->OpenPropertyStore(STGM_READ, &pStore);
								if (SUCCEEDED(hr))
								{
									PROPVARIANT friendlyName;
									PropVariantInit(&friendlyName);
									hr = pStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);
									if (SUCCEEDED(hr))
									{
										// search for the desired one
										assert(deviceSwitchListIndex < (int) g_EnumeratedDeviceListSwitchIndexes.size());
										std::wstring NameToFind = g_EnumeratedDeviceList[g_EnumeratedDeviceListSwitchIndexes[deviceSwitchListIndex]];

										// get the friendly name of the next enumerated device
										WCHAR szTitle[MAX_DEVICE_STRING_LENGTH];
										HRESULT hr = PropVariantToString(friendlyName, szTitle, ARRAYSIZE(szTitle));

										// convert friendly name to lower
										std::wstring friendly_device_name = szTitle;

										// is this the right device?
										if (std::string::npos != friendly_device_name.find(NameToFind))
										{
											// set the playback device - wstrID is an encoded device id
											SetAudioPlaybackDevice(wstrID);

											// cleanup and exit
											PropVariantClear(&friendlyName);
											result = 0;
										}
										// release the prop
										PropVariantClear(&friendlyName);
									}
									pStore->Release();
								}
							}
							pDevice->Release();
						}
					}
				}
				pDevices->Release();
			}
			pEnum->Release();
		}
	}
	return result;
}
