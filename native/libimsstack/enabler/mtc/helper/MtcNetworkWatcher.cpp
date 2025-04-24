/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "IIpcan.h"
#include "INetworkWatcher.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "helper/IMtcNetworkWatcherListener.h"
#include "helper/MtcNetworkWatcher.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcNetworkWatcher::MtcNetworkWatcher(IN IMtcService& objService, IN IMS_SINT32 nSlotId) :
        m_eServiceType(objService.GetServiceType()),
        m_bServiceConnected(objService.IsActive()),
        m_piNetWatcher(PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(nSlotId)),
        m_eIpcanType(
                objService.IsWlanIpCanType() ? IIpcan::CATEGORY_WLAN : IIpcan::CATEGORY_MOBILE),
        m_eMobileRatType(ConvertCellularRatType(m_piNetWatcher->GetNetRadioTechType())),
        m_eOldRatType(INetworkWatcher::RADIOTECH_TYPE_UNKNOWN),
        m_eLastConnectedRatType(GetRatType()),
        m_objListeners(ImsList<IMtcNetworkWatcherListener*>())
{
    m_piNetWatcher->RegisterObserver(this);
}

PUBLIC VIRTUAL MtcNetworkWatcher::~MtcNetworkWatcher()
{
    if (m_piNetWatcher != IMS_NULL)
    {
        m_piNetWatcher->RemoveObserver(this);
        m_piNetWatcher = IMS_NULL;
    }

    m_objListeners.Clear();
}

PUBLIC void MtcNetworkWatcher::AddListener(IN IMtcNetworkWatcherListener& objListener)
{
    if (m_objListeners.Contains(&objListener))
    {
        return;
    }

    m_objListeners.Append(&objListener);
}

PUBLIC void MtcNetworkWatcher::RemoveListener(IN IMtcNetworkWatcherListener& objListener)
{
    for (IMS_UINT32 nIndex = 0; nIndex < m_objListeners.GetSize(); nIndex++)
    {
        if (m_objListeners.GetAt(nIndex) == &objListener)
        {
            m_objListeners.RemoveAt(nIndex);
            return;
        }
    }
}

PUBLIC IMS_SINT32 MtcNetworkWatcher::GetRatType() const
{
    if (!m_bServiceConnected)
    {
        return INetworkWatcher::RADIOTECH_TYPE_INVALID;
    }
    if (m_eIpcanType == IIpcan::CATEGORY_WLAN)
    {
        return INetworkWatcher::RADIOTECH_TYPE_IWLAN;
    }
    return m_eMobileRatType;
}

PUBLIC IMS_SINT32 MtcNetworkWatcher::GetMobileRatType() const
{
    if (!m_bServiceConnected)
    {
        return INetworkWatcher::RADIOTECH_TYPE_INVALID;
    }
    return m_eMobileRatType;
}

PUBLIC void MtcNetworkWatcher::OnConnected(IN IMS_UINT32 eIpcan)
{
    m_eOldRatType = GetRatType();

    m_bServiceConnected = IMS_TRUE;
    m_eMobileRatType = ConvertCellularRatType(m_piNetWatcher->GetNetRadioTechType());
    m_eIpcanType = eIpcan;
    NotifyIfChanged();
}

PUBLIC void MtcNetworkWatcher::OnDisconnected()
{
    m_eOldRatType = GetRatType();

    m_bServiceConnected = IMS_FALSE;
    m_eMobileRatType = INetworkWatcher::RADIOTECH_TYPE_INVALID;
    m_eIpcanType = IIpcan::CATEGORY_MOBILE;
    NotifyIfChanged();
}

PUBLIC VIRTUAL void MtcNetworkWatcher::UpdateMobileRat(IN IMS_SINT32 eRatType)
{
    IMS_TRACE_D("UpdateMobileRat", eRatType, 0, 0);
    m_eOldRatType = GetRatType();

    m_eMobileRatType = eRatType;
    NotifyIfChanged();
}

PUBLIC VIRTUAL void MtcNetworkWatcher::NetworkWatcher_NotifyStatus(
        IN INetworkWatcher* piNetWatcherInfo)
{
    if (m_piNetWatcher != piNetWatcherInfo)
    {
        return;
    }

    IMS_SINT32 eNewMobileRatType = ConvertCellularRatType(m_piNetWatcher->GetNetRadioTechType());

    if (eNewMobileRatType == INetworkWatcher::RADIOTECH_TYPE_INVALID)
    {
        return;
    }

    if (m_eMobileRatType != eNewMobileRatType)
    {
        UpdateMobileRat(eNewMobileRatType);
    }
}

PRIVATE void MtcNetworkWatcher::NotifyIfChanged()
{
    if (m_objListeners.IsEmpty())
    {
        return;
    }

    IMS_SINT32 eCurrentRat = GetRatType();
    if (m_eOldRatType == eCurrentRat)
    {
        return;
    }

    if (eCurrentRat != INetworkWatcher::RADIOTECH_TYPE_INVALID)
    {
        m_eLastConnectedRatType = eCurrentRat;
    }

    IMS_TRACE_D("NotifyIfChanged : serviceType=%d, old RAT=%d, new RAT=%d", m_eServiceType,
            m_eOldRatType, eCurrentRat);

    for (IMS_SINT32 nIndex = static_cast<IMS_SINT32>(m_objListeners.GetSize()) - 1; nIndex >= 0;
            nIndex--)
    {
        m_objListeners.GetAt(nIndex)->OnRatChanged(m_eServiceType, m_eOldRatType, eCurrentRat);
    }
}

PRIVATE GLOBAL IMS_SINT32 MtcNetworkWatcher::ConvertCellularRatType(
        IN const NETRADIO_ENTYPE eRatType)
{
    switch (eRatType)
    {
        case NETRADIO_ENTYPE::NW_REPORT_RADIO_NR:
            return INetworkWatcher::RADIOTECH_TYPE_NR;
        case NETRADIO_ENTYPE::NW_REPORT_RADIO_LTE:
            return INetworkWatcher::RADIOTECH_TYPE_LTE;
        case NETRADIO_ENTYPE::NW_REPORT_RADIO_INVALID:
        case NETRADIO_ENTYPE::NW_REPORT_RADIO_NOSRV:
            return INetworkWatcher::RADIOTECH_TYPE_INVALID;
        default:
            return INetworkWatcher::RADIOTECH_TYPE_UNKNOWN;
    }
}
