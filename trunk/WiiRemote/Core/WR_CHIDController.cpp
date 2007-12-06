////////////////////////////////////////////////////
// Wii Remote Core File
// Copyright (C), RenEvo Software & Designs, 2007
//
// WR_CHIDController.cpp
//
// Purpose: HID Bluetooth controller which
//			detects the remote's device
//
// History:
//	- 11/1/07 : File created - KAK
////////////////////////////////////////////////////

#include "stdafx.h"
#include "WR_Implementation.h"
#include "WR_CHIDController.h"
#include "WR_CWiiRemote.h"

// HID API
extern "C"
{
	#include <setupapi.h>
	#include <hidsdi.h>
}
#pragma comment (lib, "setupapi.lib")
#pragma comment (lib, "hid.lib")

REGISTER_WR_MODULE(CWR_HIDController, HIDCONTROLLER);

////////////////////////////////////////////////////
CWR_HIDController::CWR_HIDController(void)
{
	memset(&m_GUID, 0, sizeof(GUID));
	memset(&m_Remotes, 0, sizeof(RemoteMap));
}

////////////////////////////////////////////////////
CWR_HIDController::~CWR_HIDController(void)
{
	Shutdown();
}

////////////////////////////////////////////////////
int CWR_HIDController::Initialize(void)
{
	// Get the HID GUID
	HidD_GetHidGuid(&m_GUID);

	ClearFoundRemotes();

	return WR_HIDCONTROLLER_OK;
}

////////////////////////////////////////////////////
int CWR_HIDController::PoolRemoteDevices(int nCount, float fTimeout)
{
	// Timeout controlls
	float fStart = 0.0f, fTick = 0.0f;
#define STOP_TICK() fTick += (g_pWR->pTimer->GetPreciseTime() - fStart);
#define START_TICK() fStart = g_pWR->pTimer->GetPreciseTime();

	// Loop until we timeout
	do
	{
		START_TICK();

		// Get list of connnected devices
		HDEVINFO devices = SetupDiGetClassDevs(&m_GUID, NULL, NULL, DIGCF_DEVICEINTERFACE);
		if (NULL == devices) WR_RAISEERROR(WR_HIDCONTROLLER_NODEVICES);

		// Investigate each returned device
		SP_DEVICE_INTERFACE_DATA device;
		int nDeviceIndex = 0;
		memset(&device, 0, sizeof(SP_DEVICE_INTERFACE_DATA));
		device.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		while (TRUE == SetupDiEnumDeviceInterfaces(devices, 0, &m_GUID, nDeviceIndex++, &device))
		{
			// Get the info about it
			DWORD dwLength = 0;
			PSP_DEVICE_INTERFACE_DETAIL_DATA pDeviceData = NULL;
			SetupDiGetDeviceInterfaceDetail(devices, &device, NULL, 0, &dwLength, 0);
			pDeviceData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) new BYTE[dwLength];
			assert(pDeviceData);
			pDeviceData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			if (TRUE == SetupDiGetDeviceInterfaceDetail(devices, &device, pDeviceData, dwLength,
				&dwLength, 0))
			{
				// Open a handle to the device
				HANDLE deviceHandle = CreateFile(pDeviceData->DevicePath, 0, (FILE_SHARE_READ|FILE_SHARE_WRITE),
					NULL, OPEN_EXISTING, 0, NULL);
				if (INVALID_HANDLE_VALUE != deviceHandle)
				{
					// Get vendor and product IDs
					HIDD_ATTRIBUTES deviceAttrs;
					memset(&deviceAttrs, 0, sizeof(HIDD_ATTRIBUTES));
					deviceAttrs.Size = sizeof(HIDD_ATTRIBUTES);
					if (TRUE == HidD_GetAttributes(deviceHandle, &deviceAttrs) &&
						WR_VENDORID == deviceAttrs.VendorID &&
						WR_PRODUCTID == deviceAttrs.ProductID)
					{
						// Check for duplicate
						int temp = 0;
						for (temp; temp < m_nFoundCount; temp++)
						{
							if (m_FoundRemotes[temp] == pDeviceData->DevicePath)
								temp = MAX_REMOTES; // Skip it
						}
						if (temp < MAX_REMOTES)
						{
							// Add it
							m_FoundRemotes[m_nFoundCount++] = pDeviceData->DevicePath;

							// Call the callback
							for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
								(*itI)->OnFoundRemoteDevice(pDeviceData->DevicePath, deviceHandle);
						}
					}
		
					// Clean up
					CloseHandle(deviceHandle);
				}
			}

			// Clean up
			SAFE_DELETE_ARRAY(pDeviceData);
		}

		// Clean up
		SetupDiDestroyDeviceInfoList(devices);

		STOP_TICK();
	}
	while (fTimeout > fTick && m_nFoundCount < nCount);
#undef STOP_TICK
#undef START_TICK

	// Return how many were found
	return m_nFoundCount;
}

////////////////////////////////////////////////////
void CWR_HIDController::Shutdown(void)
{
	ShutdownRemotes();

	// Clean out listeners
	m_Listeners.clear();
}

////////////////////////////////////////////////////
void CWR_HIDController::ClearFoundRemotes(void)
{
	// Close all valid handles
	for (int i = 0; i < MAX_REMOTES; i++)
	{
		m_FoundRemotes[i].clear();
	}
	m_nFoundCount = 0;
}

////////////////////////////////////////////////////
int CWR_HIDController::GetFoundRemoteCount(void)
{
	return m_nFoundCount;
}

////////////////////////////////////////////////////
IWR_WiiRemote* CWR_HIDController::InitializeRemote(int nID)
{
	nID--;
	if (nID < 0 || nID >= MAX_REMOTES) return NULL;

	// Must be valid
	if (true == m_FoundRemotes[nID].empty() ||
		NULL != m_Remotes[nID])
		return NULL;

	// Create entry
	IWR_WiiRemote *pRemote = new CWR_WiiRemote;
	assert(pRemote);
	if (WR_SUCCESS(pRemote->Initialize(m_FoundRemotes[nID].c_str(), nID+1)))
	{
		m_Remotes[nID] = pRemote;

		// Call the callback
		for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
			(*itI)->OnRemoteInitialized(pRemote, pRemote->GetID());

		return pRemote;
	}
	else
	{
		// Don't hold on to it if it failed
		SAFE_DELETE(pRemote);
	}
	return NULL;
}

////////////////////////////////////////////////////
void CWR_HIDController::ShutdownRemotes(void)
{
	// Shutdown all remotes
	for (int i = 0; i < MAX_REMOTES; i++)
	{
		if (NULL != m_Remotes[i])
		{
			m_Remotes[i]->Shutdown();
			SAFE_DELETE(m_Remotes[i]);
		}
	}
	memset(m_Remotes, 0, sizeof(RemoteMap));
}

////////////////////////////////////////////////////
void CWR_HIDController::UpdateRemotes(void)
{
	for (int i = 0; i < MAX_REMOTES; i++)
	{
		if (NULL != m_Remotes[i])
			m_Remotes[i]->Update();
	}
}

////////////////////////////////////////////////////
IWR_WiiRemote *CWR_HIDController::GetRemote(RemoteID const& nID) const
{
	if (nID < 1 || nID >= MAX_REMOTES) return NULL;
	return m_Remotes[nID-1];
}

////////////////////////////////////////////////////
bool CWR_HIDController::GetRemotes(RemoteMap &remotes) const
{
	memcpy(remotes, m_Remotes, sizeof(RemoteMap));
	return true;
}

////////////////////////////////////////////////////
int CWR_HIDController::GetRemoteCount(void) const
{
	int nCount = 0;
	for (int i = 0; i < MAX_REMOTES; i++)
		if (m_Remotes[i]) nCount++;
	return nCount;
}

////////////////////////////////////////////////////
void CWR_HIDController::AddListener(IWR_HIDControllerListener *pListener)
{
	if (NULL == pListener) return;
	for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
		if (*itI == pListener)
			return;
	m_Listeners.push_back(pListener);
}

////////////////////////////////////////////////////
void CWR_HIDController::RemoveListener(IWR_HIDControllerListener *pListener)
{
	if (NULL == pListener) return;
	for (Listeners::iterator itI = m_Listeners.begin(); itI != m_Listeners.end(); itI++)
		if (*itI == pListener)
		{
			m_Listeners.erase(itI);
			return;
		}
}