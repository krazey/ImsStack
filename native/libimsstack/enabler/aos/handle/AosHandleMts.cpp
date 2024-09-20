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
#include "ServiceTrace.h"
#include "CarrierConfig.h"
#include "IAosService.h"
#include "INetworkWatcher.h"

#include "interface/IAosAppContext.h"
#include "interface/IAosNConfiguration.h"
#include "provider/AosProvider.h"
#include "provider/AosUtil.h"

#include "handle/AosHandleMts.h"

__IMS_TRACE_TAG_AOS__;

#define APPPROFILE m_strTag.GetStr()

PUBLIC
AosHandleMts::AosHandleMts(IN IAosAppContext* piAppContext, IN const AString& strAppId,
        IN const AString& strServiceId, IN const IMS_SINT32 nServiceType) :
        AosHandle(piAppContext, strAppId, strServiceId, nServiceType),
        m_bMtcBlocked(IMS_FALSE),
        m_nSupportedRats(NW_REPORT_RADIO_INVALID)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosHandleMts = %" PFLS_u "/%" PFLS_x, strAppId.GetStr(),
            sizeof(AosHandleMts), this);

    m_objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::SMS));
    m_objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::SMS));
    m_objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::SMS));
    m_objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::UTRAN),
            static_cast<IMS_UINT32>(AosCapability::SMS));
}

PUBLIC VIRTUAL AosHandleMts::~AosHandleMts()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosHandleMts = %" PFLS_u "/%" PFLS_x,
            m_strAppId.GetStr(), sizeof(AosHandleMts), this);
}

PUBLIC VIRTUAL void AosHandleMts::NConfiguration_NotifyConfigChanged()
{
    A_IMS_TRACE_D(APPPROFILE, "NConfiguration_NotifyConfigChanged", 0, 0, 0);

    if (GET_N_CONFIG(m_nSlotId) != IMS_NULL)
    {
        InitializeSupportedRats();
    }

    AosHandle::NConfiguration_NotifyConfigChanged();
}

PUBLIC VIRTUAL void AosHandleMts::NetTracker_StatusChanged()
{
    if (AosUtil::GetInstance()->IsWifiTest())
    {
        return;
    }

    AosHandle::NetTracker_StatusChanged();

    IMS_UINT32 nCurrNetworkType = GetNetworkType();

    if (nCurrNetworkType != m_nNetworkType)
    {
        m_nNetworkType = nCurrNetworkType;
        ProcessNetworkChanged();
    }
}

PROTECTED
void AosHandleMts::InitializeSupportedRats()
{
    ImsVector<IMS_SINT32> objRats = GET_N_CONFIG(m_nSlotId)->GetSmsOverImsSupportedRats();

    m_nSupportedRats = NW_REPORT_RADIO_INVALID;

    for (int i = 0; i < objRats.GetSize(); i++)
    {
        if (objRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN)
        {
            m_nSupportedRats |= NW_REPORT_RADIO_LTE;
        }
        else if (objRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_UTRAN)
        {
            m_nSupportedRats |= NW_REPORT_RADIO_WCDMA | NW_REPORT_RADIO_HSPA;
        }
        else if (objRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_GERAN)
        {
            m_nSupportedRats |= NW_REPORT_RADIO_GSM | NW_REPORT_RADIO_EDGE;
        }
        else if (objRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN)
        {
            m_nSupportedRats |= NW_REPORT_RADIO_NR;
        }
        else if (objRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN)
        {
            m_nSupportedRats |= NW_REPORT_RADIO_WLAN;
        }
    }
}

PROTECTED VIRTUAL void AosHandleMts::Init()
{
    A_IMS_TRACE_D(APPPROFILE, "Init", 0, 0, 0);

    InitializeSupportedRats();
    AosHandle::Init();
}

PROTECTED VIRTUAL void AosHandleMts::InitializeServiceBlock()
{
    m_bBlocked = IsHandleBlocked();

    A_IMS_TRACE_D(
            APPPROFILE, "InitializeServiceBlock :: m_bBlocked(%s))", _TRACE_B_(m_bBlocked), 0, 0);
}

PROTECTED VIRTUAL void AosHandleMts::InitializeServiceFeature()
{
    m_objFeatureTagList.Clear();

    if (!m_bBlocked)
    {
        m_objFeatureTagList.AddFeature(ImsAosFeature::SMSIP);
    }
}

PROTECTED VIRTUAL void AosHandleMts::ProcessCapabilitiesChanged(
        IN const ImsMap<IMS_UINT32, IMS_UINT32>& objNewCapabilities)
{
    if (IsEmergencyService())
    {
        return;
    }

    AosHandle::ProcessCapabilitiesChanged(objNewCapabilities);

    if (!IsSupportedNetworkType(m_nNetworkType) && !Is3G(m_nNetworkType))
    {
        return;
    }

    ProcessBlock(BLOCK_SMS_CAPABILITY,
            !IsCapabilityExistedForNetworkType(m_nNetworkType, AosCapability::SMS), IMS_FALSE);
}

PROTECTED VIRTUAL void AosHandleMts::ProcessNetworkChanged()
{
    if (IsSupportedNetworkType(m_nNetworkType) || Is3G(m_nNetworkType))
    {
        ProcessBlock(BLOCK_SMS_CAPABILITY,
                !IsCapabilityExistedForNetworkType(m_nNetworkType, AosCapability::SMS), IMS_FALSE);
    }
}

PROTECTED VIRTUAL IMS_BOOL AosHandleMts::IsHandleBlocked() const
{
    return (AosHandle::IsHandleBlocked(BLOCK_SMS_CAPABILITY) || m_bMtcBlocked);
}

PROTECTED VIRTUAL IMS_BOOL AosHandleMts::IsSupportedNetworkTypeForCellular(
        IN IMS_UINT32 nType) const
{
    return (m_nSupportedRats & nType);
}

PROTECTED VIRTUAL void AosHandleMts::Handle_Notify(IN IMS_UINT32 nType, IN IMS_BOOL bBlocked)
{
    if (nType != ImsAosService::MTC)
    {
        return;
    }

    A_IMS_TRACE_I(
            APPPROFILE, "Handle_Notify :: nType[%d], bBlocked[%s]", nType, _TRACE_B_(bBlocked), 0);

    m_bMtcBlocked = bBlocked;

    if (bBlocked != m_bBlocked)
    {
        IMS_BOOL bCurrentBlocked = IsHandleBlocked();
        if (m_bBlocked != bCurrentBlocked)
        {
            m_bBlocked = bCurrentBlocked;
            IMSMSG objMSG(HANDLE_MSG_BLOCK_STATUS, 0, 0);
            OnStateMessage(objMSG);
        }
    }
}
