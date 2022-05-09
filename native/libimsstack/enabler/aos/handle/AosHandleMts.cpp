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
#include "ServiceEvent.h"
#include "ServiceTrace.h"

#include "interface/IAosAppContext.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosService.h"
#include "provider/AosProvider.h"
#include "handle/AosHandleMts.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define APPPROFILE m_strTag.GetStr()

/*

Remarks

*/
PUBLIC
AosHandleMts::AosHandleMts(IN IAosAppContext* piAppContext, IN const AString& strAppId,
        IN const AString& strServiceId, IN const IMS_SINT32 nServiceType) :
        AosHandle(piAppContext, strAppId, strServiceId, nServiceType),
        m_bSmsOverIp(IMS_TRUE)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosHandleMts = %" PFLS_u "/%" PFLS_x, strAppId.GetStr(),
            sizeof(AosHandleMts), this);

    m_objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::SMS));
    m_objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::SMS));
    m_objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::SMS));

    m_objServiceFeatures.Append(ImsAosFeature::SMSIP);
}

/*

Remarks

*/
PUBLIC VIRTUAL AosHandleMts::~AosHandleMts()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosHandleMts = %" PFLS_u "/%" PFLS_x,
            m_strAppId.GetStr(), sizeof(AosHandleMts), this);
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosHandleMts::Init()
{
    A_IMS_TRACE_D(APPPROFILE, "Init", 0, 0, 0);

    AosHandle::Init();

    IMS_EVENT_AddListenerForSlotId(IMS_EVENT_CONFIG_UPDATE, this, m_nSlotId);
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosHandleMts::CleanUp()
{
    A_IMS_TRACE_D(APPPROFILE, "CleanUp", 0, 0, 0);

    AosHandle::CleanUp();

    IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_CONFIG_UPDATE, this, m_nSlotId);
}

/* jryou::TODO
PROTECTED VIRTUAL
void AosHandleMts::EnableAoS()
{
    IMS_BOOL bSmsOverIpNetwork = IMS_FALSE;
    A_IMS_TRACE_I(APPPROFILE, "EnableAos()", 0, 0, 0);

    IConfigBuffer* piConfigBuffer =
            Configuration::GetInstance()->CreateConfig("ims.service.mts", m_nSlotId);
    if (piConfigBuffer != IMS_NULL)
    {
        if (piConfigBuffer->CaptureSection("sdm") != IMS_FALSE)
        {
            bSmsOverIpNetwork = piConfigBuffer->ReadValueBoolean("sms_over_ip_network");
            piConfigBuffer->ReleaseSection();
        }
        piConfigBuffer->Destroy();
    }

    if(bSmsOverIpNetwork == IMS_TRUE )
    {
        A_IMS_TRACE_I(APPPROFILE,
                "AosHandleMts::EnableAos() enableAos() due to sms_over_ip_network is true",
                0, 0, 0);
        AosHandle::EnableAoS();
    }
    else
    {
        A_IMS_TRACE_I(APPPROFILE,
                "AosHandleMts::EnableAos() disableAos() due to sms_over_ip_network is false",
                0, 0, 0);
        AosHandle::DisableAoS();
    }
}
*/

/*

Remarks

*/
PROTECTED VIRTUAL void AosHandleMts::InitializeServiceBlock()
{
    A_IMS_TRACE_I(APPPROFILE, "InitializeServiceBlock :: m_bSmsOverIp(%s)", _TRACE_B_(m_bSmsOverIp),
            0, 0);

    if (!GET_N_CONFIG(m_nSlotId)->IsSmsOverIpEnabled())
    {
        AddBlock(BLOCK_SMS_OVER_IP_NETWORK_INDICATION, m_nBlocks);
        m_bBlocked = IMS_TRUE;
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosHandleMts::InitializeServiceFeature()
{
    if (!m_bBlocked)
    {
        m_objFeatureTagList.AddFeature(ImsAosFeature::SMSIP);
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosHandleMts::ProcessCapabilitiesChanged(
        IN const IMSMap<IMS_UINT32, IMS_UINT32>& /*objCapabilities*/)
{
    /* jryou:: Temp blocked until GII is changed to consider SMS capability.
    A_IMS_TRACE_I(APPPROFILE, "ProcessCapabilitiesChanged :: Size[%d]",
            objCapabilities.GetSize(), 0, 0);

    for (IMS_UINT32 i = 0; i < objCapabilities.GetSize(); i++)
    {
        IMS_UINT32 nType = objCapabilities.GetKeyAt(i);
        IMS_UINT32 nCapabilities = objCapabilities.GetValueAt(i);
        IMS_UINT32 nChangedCapabilities = m_objCapabilities.GetValue(nType) ^ nCapabilities;
        IMS_UINT32 nCurrentRat = GetNetworkType();

        if ((nType == static_cast<IMS_UINT32>(AosNetworkType::LTE) &&
                nCurrentRat == NW_REPORT_RADIO_LTE) ||
                (nType == static_cast<IMS_UINT32>(AosNetworkType::NR) &&
                nCurrentRat == NW_REPORT_RADIO_NR))
        {
            if (nChangedCapabilities & static_cast<IMS_UINT32>(AosCapability::SMS))
            {
                ProcessBlock(BLOCK_SMS_CAPABILITY,
                        (nCapabilities & static_cast<IMS_UINT32>(AosCapability::SMS)) == 0);
            }
        }

        m_objCapabilities.SetValue(nType, nCapabilities);
    }
    */
}