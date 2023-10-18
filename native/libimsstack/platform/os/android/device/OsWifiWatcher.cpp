/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "PlatformContext.h"
#include "ServiceTrace.h"
#include "device/OsWifiWatcher.h"
#include "network/OsNetworkConstants.h"
#include "system-intf/SystemConstants.h"

__IMS_TRACE_TAG_ADAPT__;

PUBLIC
OsWifiWatcher::OsWifiWatcher() :
        m_nWifiState(IWifiWatcher::STATE_DISCONNECTED)
{
    PlatformContext::GetInstance()->GetSystem()->AddListener(
            SystemConstants::CATEGORY_WIFI, this, IMS_SLOT_0);
}

PUBLIC VIRTUAL OsWifiWatcher::~OsWifiWatcher()
{
    PlatformContext::GetInstance()->GetSystem()->RemoveListener(
            SystemConstants::CATEGORY_WIFI, this, IMS_SLOT_0);
}

PUBLIC VIRTUAL IMS_SINT32 OsWifiWatcher::GetState()
{
    IMS_SINT32 nConnectionState =
            PlatformContext::GetInstance()->GetSystem()->GetWifiConnectionState();

    if (nConnectionState == WIFI_CONNECTION_STATE_CONNECTED)
    {
        m_nWifiState = IWifiWatcher::STATE_CONNECTED;
    }
    else
    {
        m_nWifiState = IWifiWatcher::STATE_DISCONNECTED;
    }

    IMS_TRACE_D("GetState :: state=%d", m_nWifiState, 0, 0);

    return m_nWifiState;
}

PUBLIC VIRTUAL void OsWifiWatcher::System_NotifyEvent(
        IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam)
{
    (void)nLParam;

    IMS_TRACE_D("WifiWatcher :: event=%d, wp=%" PFLS_d ", lp=%" PFLS_d, nEvent, nWParam, nLParam);

    switch (nEvent)
    {
        case IMS_SYSTEM_WIFI_STATE_CHANGED:
            UpdateWifiStateChanged(LONG_TO_INT(nWParam));
            break;
        case IMS_SYSTEM_WIFI_CONNECTION_STATE_CHANGED:
            UpdateWifiConnectionStateChanged(LONG_TO_INT(nWParam));
            break;
        default:
            break;
    }
}

/**
 * Wi-Fi connection state
 * WIFI_CONNECTION_STATE_CONNECTED,
 * WIFI_CONNECTION_STATE_DISCONNECTED
 */
PRIVATE
void OsWifiWatcher::UpdateWifiConnectionStateChanged(IN IMS_UINT32 nConnectionState)
{
    if (nConnectionState == WIFI_CONNECTION_STATE_CONNECTED)
    {
        m_nWifiState = IWifiWatcher::STATE_CONNECTED;
    }
    else
    {
        m_nWifiState = IWifiWatcher::STATE_DISCONNECTED;
    }

    PostMsgRegisteredThread();
}

void OsWifiWatcher::UpdateWifiStateChanged(IN IMS_UINT32 nState)
{
    IMS_TRACE_D("UpdateWifiStateChanged :: state=%d", nState, 0, 0);

    switch (nState)
    {
        case WIFI_STATE_DISABLED:
            m_nWifiState = IWifiWatcher::STATE_DISCONNECTED;
            break;

        default:
            break;
    }

    PostMsgRegisteredThread();
}
