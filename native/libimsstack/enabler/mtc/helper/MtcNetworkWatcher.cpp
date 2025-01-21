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
        m_piNetWatcher(PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(nSlotId)),
        m_eIpcanType(
                objService.IsWlanIpCanType() ? IIpcan::CATEGORY_WLAN : IIpcan::CATEGORY_MOBILE),
        m_eMobileRatType(ConvertCellularRatType(m_piNetWatcher->GetNetRadioTechType())),
        m_eOldRatType(INetworkWatcher::RADIOTECH_TYPE_UNKNOWN),
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
    return GetCurrentRat();
}

PUBLIC IMS_SINT32 MtcNetworkWatcher::GetMobileRatType() const
{
    return m_eMobileRatType;
}

PUBLIC void MtcNetworkWatcher::OnServiceConnected(IN IMS_UINT32 eIpcan)
{
    if (m_eIpcanType == eIpcan)
    {
        return;
    }

    m_eOldRatType = GetCurrentRat();
    m_eIpcanType = eIpcan;
    Notify();
}

PUBLIC VIRTUAL void MtcNetworkWatcher::SetTestRatChanged(IN IMS_SINT32 eRatType)
{
    IMS_TRACE_D("SetTestRatChanged", eRatType, 0, 0);
    m_eOldRatType = m_eMobileRatType;
    m_eMobileRatType = eRatType;

    if (m_eIpcanType != IIpcan::CATEGORY_WLAN)
    {
        Notify();
    }
}

PUBLIC VIRTUAL void MtcNetworkWatcher::NetworkWatcher_NotifyStatus(
        IN INetworkWatcher* piNetWatcherInfo)
{
    if (m_piNetWatcher != piNetWatcherInfo)
    {
        return;
    }

    IMS_SINT32 eConvertedMobileRatType =
            ConvertCellularRatType(m_piNetWatcher->GetNetRadioTechType());
    if (m_eMobileRatType == eConvertedMobileRatType)
    {
        return;
    }

    m_eOldRatType = m_eMobileRatType;
    m_eMobileRatType = eConvertedMobileRatType;

    if (m_eIpcanType == IIpcan::CATEGORY_WLAN)
    {
        return;
    }

    Notify();
}

PRIVATE void MtcNetworkWatcher::Notify()
{
    if (m_objListeners.IsEmpty())
    {
        return;
    }

    IMS_SINT32 eCurrentRat = GetCurrentRat();
    IMS_TRACE_D("Notify serviceType=%d, old RAT=%d, new RAT=%d", m_eServiceType, m_eOldRatType,
            eCurrentRat);

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

PRIVATE IMS_SINT32 MtcNetworkWatcher::GetCurrentRat() const
{
    if (m_eIpcanType == IIpcan::CATEGORY_WLAN)
    {
        return INetworkWatcher::RADIOTECH_TYPE_IWLAN;
    }

    return m_eMobileRatType;
}
