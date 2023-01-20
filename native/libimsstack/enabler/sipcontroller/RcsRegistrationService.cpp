/*
 * Copyright (C) 2023 The Android Open Source Project
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
#include "RcsRegistrationService.h"

#include "IJniEnabler.h"
#include "IJniSipControllerServiceThread.h"
#include "IImsAos.h"
#include "ImsAos.h"
#include "ImsAosParameter.h"
#include "ImsAosReason.h"
#include "ImsServiceConfig.h"
#include "IURcsMessageService.h"
#include "config/IMConstants.h"
#include "JniEnablerConnector.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("IMS_SNC");

PUBLIC RcsRegistrationService::RcsRegistrationService(
        IN const AString& strName, IN const IMS_SINT32 nSlotId) :
        ImsService(strName),
        m_nSlotId(nSlotId),
        m_piImsAos(IMS_NULL),
        m_objDefinedFeatures(IMSMap<AString, IMS_UINT32>())
{
    IMS_TRACE_I("+RcsRegistrationService : Service Name = %s\n Slot = %d", GetName().GetStr(),
            m_nSlotId, 0);
    IMS_TRACE_MEM("SNC_MSG", "RCSRegService = %" PFLS_u, sizeof(RcsRegistrationService), 0, 0);
    EnableAoS();
    RcsFeatureTags();
}

PUBLIC VIRTUAL RcsRegistrationService::~RcsRegistrationService()
{
    if (m_piImsAos != IMS_NULL)
    {
        m_piImsAos->SetListener(IMS_NULL);
        m_piImsAos = IMS_NULL;
    }
}
PUBLIC RcsRegistrationService::RcsRegistrationService(
        IN IImsAos* piImsAos, IN const IMS_SINT32 nSlotId) :
        ImsService("RcsRegistrationService"),
        m_nSlotId(nSlotId),
        m_piImsAos(piImsAos),
        m_objDefinedFeatures(IMSMap<AString, IMS_UINT32>())
{
    IMS_TRACE_MEM("SNC_MSG", "RCSRegService = %" PFLS_u, sizeof(RcsRegistrationService), 0, 0);
    RcsFeatureTags();
}

PUBLIC IMS_BOOL RcsRegistrationService::UpdateDelegateRegistration(IN IMS_UINTP nParam)
{
    IMS_TRACE_D("UpdateDelegateRegistration()", 0, 0, 0);
    IUSncFeatureTagsParam* pParam = reinterpret_cast<IUSncFeatureTagsParam*>(nParam);
    if (pParam == IMS_NULL)
    {
        IMS_TRACE_D("UpdateDelegateRegistration() parameter is null", 0, 0, 0);
        return IMS_FALSE;
    }
    if (m_piImsAos == IMS_NULL)
    {
        IMS_TRACE_D("UpdateDelegateRegistration() m_piImsAos is null", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_UINT32 nFeatures = 0;
    IMS_BOOL bDefinedFeature = IMS_TRUE;
    AString strKey;
    for (int i = 0; i < pParam->m_nFeatureCount; i++)
    {
        if (!isPreDefindedFeature(pParam->m_objFeatureTags.GetElementAt(i)))
        {
            bDefinedFeature = IMS_FALSE;
            break;
        }
        nFeatures |= m_objDefinedFeatures.GetValue(pParam->m_objFeatureTags.GetElementAt(i));
    }

    IMS_TRACE_D("UpdateDelegateRegistration() nFeatures: [%d] %d, [%d]", pParam->m_nFeatureCount,
            nFeatures, bDefinedFeature);
    if (bDefinedFeature)
    {
        m_piImsAos->UpdateFeature(nFeatures);
    }
    else
    {
        IMSList<ImsAosFeatureTag*> objTags;
        for (int i = 0; i < pParam->m_nFeatureCount; i++)
        {
            objTags.Append(new ImsAosFeatureTag(
                    strKey.SetNumber(i), pParam->m_objFeatureTags.GetElementAt(i)));
        }
        m_piImsAos->UpdateFeature(objTags);
    }
    return IMS_TRUE;
}

PUBLIC IMS_BOOL RcsRegistrationService::TriggerDelegateDeregistration()
{
    IMS_TRACE_D("TriggerDelegateDeregistration()", 0, 0, 0);
    if (m_piImsAos == IMS_NULL)
    {
        IMS_TRACE_D("TriggerDelegateDeregistration() m_piImsAos is null", 0, 0, 0);
        return IMS_FALSE;
    }
    m_piImsAos->Control(ImsAosControl::TRIGGER_SIP_DELEGATE_DEREGISTRATION);
    return IMS_TRUE;
}

PRIVATE IMS_BOOL RcsRegistrationService::isPreDefindedFeature(IN const AString& feature)
{
    IMS_SLONG nIndex = m_objDefinedFeatures.GetIndexOfKey(feature);
    if (nIndex >= 0)
    {
        return IMS_TRUE;
    }
    return IMS_FALSE;
}

PRIVATE void RcsRegistrationService::EnableAoS()
{
    AString strAppID(ImsServiceConfig::GetAppName(ImsAppId::SIP_DELEGATE));
    m_piImsAos = ImsAos::GetImsAos(
            strAppID, ImsServiceConfig::GetServiceName(ImsServiceId::SIP_DELEGATE), m_nSlotId);

    if (m_piImsAos != IMS_NULL)
    {
        m_piImsAos->SetListener(this);
    }
    else
    {
        IMS_TRACE_E(0, "EnableAoS : m_piImsAos is null", 0, 0, 0);
    }
}

PRIVATE IJniSipControllerServiceThread* RcsRegistrationService::GetJniThread()
{
    IJniEnabler* piJniEnabler =
            JniEnablerConnector::GetInstance().GetJniEnabler(m_nSlotId, EnablerType::SIP_DELEGATE);
    if (piJniEnabler == IMS_NULL)
    {
        return IMS_NULL;
    }

    return reinterpret_cast<IJniSipControllerServiceThread*>(piJniEnabler->GetJniThread());
}

PRIVATE IMSList<AString> RcsRegistrationService::GetFeatureTags(IN const IMS_UINT32 nFeatures)
{
    IMSList<AString> objCurrentFeatures;
    if ((nFeatures & m_objDefinedFeatures.GetValue(IMConstants::TAG_STANDALONE_PAGER)) > 0)
    {
        objCurrentFeatures.Append(IMConstants::TAG_STANDALONE_PAGER);
        objCurrentFeatures.Append(IMConstants::TAG_STANDALONE_LARGE);
        objCurrentFeatures.Append(IMConstants::TAG_STANDALONE_DEFERRED);
        objCurrentFeatures.Append(IMConstants::TAG_STANDALONE_PAGER_LARGE);
    }
    if ((nFeatures & m_objDefinedFeatures.GetValue(IMConstants::TAG_CHAT_IM)) > 0)
    {
        objCurrentFeatures.Append(IMConstants::TAG_CHAT_IM);
    }
    if ((nFeatures & m_objDefinedFeatures.GetValue(IMConstants::TAG_SESSION)) > 0)
    {
        objCurrentFeatures.Append(IMConstants::TAG_SESSION);
    }
    if ((nFeatures & m_objDefinedFeatures.GetValue(IMConstants::TAG_HTTP_FILETRANSFER)) > 0)
    {
        objCurrentFeatures.Append(IMConstants::TAG_HTTP_FILETRANSFER);
    }
    if ((nFeatures & m_objDefinedFeatures.GetValue(IMConstants::TAG_SMS_FILETRANSFER)) > 0)
    {
        objCurrentFeatures.Append(IMConstants::TAG_SMS_FILETRANSFER);
    }
    if ((nFeatures &
                m_objDefinedFeatures.GetValue(IMConstants::TAG_CALL_COMPOSER_ENRICHED_CALLING)) > 0)
    {
        objCurrentFeatures.Append(IMConstants::TAG_CALL_COMPOSER_ENRICHED_CALLING);
    }
    if ((nFeatures & m_objDefinedFeatures.GetValue(IMConstants::TAG_CALL_COMPOSER_VIA_TELEPHONY)) >
            0)
    {
        objCurrentFeatures.Append(IMConstants::TAG_CALL_COMPOSER_VIA_TELEPHONY);
    }
    if ((nFeatures & m_objDefinedFeatures.GetValue(IMConstants::TAG_POST_CALL)) > 0)
    {
        objCurrentFeatures.Append(IMConstants::TAG_POST_CALL);
    }
    if ((nFeatures & m_objDefinedFeatures.GetValue(IMConstants::TAG_SHARED_MAP)) > 0)
    {
        objCurrentFeatures.Append(IMConstants::TAG_SHARED_MAP);
    }
    if ((nFeatures & m_objDefinedFeatures.GetValue(IMConstants::TAG_SHARED_SKETCH)) > 0)
    {
        objCurrentFeatures.Append(IMConstants::TAG_SHARED_SKETCH);
    }
    if ((nFeatures & m_objDefinedFeatures.GetValue(IMConstants::TAG_GEOLOCATIONPUSH)) > 0)
    {
        objCurrentFeatures.Append(IMConstants::TAG_GEOLOCATIONPUSH);
    }
    if ((nFeatures & m_objDefinedFeatures.GetValue(IMConstants::TAG_SMS_GEOLOCATIONPUSH)) > 0)
    {
        objCurrentFeatures.Append(IMConstants::TAG_SMS_GEOLOCATIONPUSH);
    }
    if ((nFeatures &
                m_objDefinedFeatures.GetValue(
                        IMConstants::TAG_CHATBOT_COMMUNICATION_USING_SESSION)) > 0)
    {
        objCurrentFeatures.Append(IMConstants::TAG_CHATBOT_COMMUNICATION_USING_SESSION);
    }
    if ((nFeatures &
                m_objDefinedFeatures.GetValue(
                        IMConstants::TAG_CHATBOT_COMMUNICATION_USING_STANDALONE_MSG)) > 0)
    {
        objCurrentFeatures.Append(IMConstants::TAG_CHATBOT_COMMUNICATION_USING_STANDALONE_MSG);
    }
    if ((nFeatures & m_objDefinedFeatures.GetValue(IMConstants::TAG_CHATBOT_VERSION_SUPPORTED)) > 0)
    {
        objCurrentFeatures.Append(IMConstants::TAG_CHATBOT_VERSION_SUPPORTED);
    }
    if ((nFeatures & m_objDefinedFeatures.GetValue(IMConstants::TAG_CHATBOT_VERSION_V2_SUPPORTED)) >
            0)
    {
        objCurrentFeatures.Append(IMConstants::TAG_CHATBOT_VERSION_V2_SUPPORTED);
    }
    if ((nFeatures & m_objDefinedFeatures.GetValue(IMConstants::TAG_CHATBOT_ROLE)) > 0)
    {
        objCurrentFeatures.Append(IMConstants::TAG_CHATBOT_ROLE);
    }
    return objCurrentFeatures;
}

PRIVATE void RcsRegistrationService::RcsFeatureTags()
{
    m_objDefinedFeatures.Add(IMConstants::TAG_STANDALONE_PAGER, 0x00000040);
    m_objDefinedFeatures.Add(IMConstants::TAG_STANDALONE_LARGE, 0x00000040);
    m_objDefinedFeatures.Add(IMConstants::TAG_STANDALONE_DEFERRED, 0x00000040);
    m_objDefinedFeatures.Add(IMConstants::TAG_STANDALONE_PAGER_LARGE, 0x00000040);
    m_objDefinedFeatures.Add(IMConstants::TAG_CHAT_IM, 0x00000080);
    m_objDefinedFeatures.Add(IMConstants::TAG_SESSION, 0x00000100);
    m_objDefinedFeatures.Add(IMConstants::TAG_HTTP_FILETRANSFER, 0x00000200);
    m_objDefinedFeatures.Add(IMConstants::TAG_SMS_FILETRANSFER, 0x00000400);
    m_objDefinedFeatures.Add(IMConstants::TAG_CALL_COMPOSER_ENRICHED_CALLING, 0x00000800);
    m_objDefinedFeatures.Add(IMConstants::TAG_CALL_COMPOSER_VIA_TELEPHONY, 0x00001000);
    m_objDefinedFeatures.Add(IMConstants::TAG_POST_CALL, 0x00002000);
    m_objDefinedFeatures.Add(IMConstants::TAG_SHARED_MAP, 0x00004000);
    m_objDefinedFeatures.Add(IMConstants::TAG_SHARED_SKETCH, 0x00008000);
    m_objDefinedFeatures.Add(IMConstants::TAG_GEOLOCATIONPUSH, 0x00010000);
    m_objDefinedFeatures.Add(IMConstants::TAG_SMS_GEOLOCATIONPUSH, 0x00020000);
    m_objDefinedFeatures.Add(IMConstants::TAG_CHATBOT_COMMUNICATION_USING_SESSION, 0x00040000);
    m_objDefinedFeatures.Add(
            IMConstants::TAG_CHATBOT_COMMUNICATION_USING_STANDALONE_MSG, 0x00080000);
    m_objDefinedFeatures.Add(IMConstants::TAG_CHATBOT_VERSION_SUPPORTED, 0x00100000);
    m_objDefinedFeatures.Add(IMConstants::TAG_CHATBOT_VERSION_V2_SUPPORTED, 0x00200000);
    m_objDefinedFeatures.Add(IMConstants::TAG_CHATBOT_ROLE, 0x00400000);
}

PROTECTED void RcsRegistrationService::ImsAos_Connected(
        IN IMS_UINT32 nFeatures, IN IMS_UINT32 nIpcan)
{
    (void)nIpcan;
    IMS_TRACE_I("ImsAos_Connected() Features: %d", nFeatures, 0, 0);

    if (!m_objCurrentFeatures.IsEmpty())
    {
        IMS_TRACE_I("ImsAos_Connected() Current Feature is clearing..", 0, 0, 0);
        m_objCurrentFeatures.Clear();
    }
    if (nFeatures > 0)
    {
        m_objCurrentFeatures = GetFeatureTags(nFeatures);
    }
    IUSncFeatureTagsParam* pParam = new IUSncFeatureTagsParam();

    pParam->m_nFeatureCount = m_objCurrentFeatures.GetSize();
    pParam->m_nRegState = static_cast<IMS_SINT32>(RcsRegState::STATE_REGISTERED);

    for (IMS_UINT32 i = 0; i < pParam->m_nFeatureCount; i++)
    {
        pParam->m_objFeatureTags.AddElement(m_objCurrentFeatures.GetAt(i));
    }

    IJniSipControllerServiceThread* piJniThread = GetJniThread();
    if (piJniThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "UpdateDelegateRegistration:piJniThread is null", 0, 0, 0);
        return;
    }
    piJniThread->OnRegistrationUpdated(reinterpret_cast<IMS_UINTP>(pParam));
}

PROTECTED void RcsRegistrationService::ImsAos_Connecting()
{
    IMS_TRACE_I("ImsAos_Connecting()", 0, 0, 0);
    IUSncFeatureTagsParam* pParam = new IUSncFeatureTagsParam();

    pParam->m_nFeatureCount = m_objCurrentFeatures.GetSize();
    pParam->m_nRegState = static_cast<IMS_SINT32>(RcsRegState::STATE_REGISTERING);

    for (IMS_UINT32 i = 0; i < m_objCurrentFeatures.GetSize(); i++)
    {
        pParam->m_objFeatureTags.AddElement(m_objCurrentFeatures.GetAt(i));
    }
    IJniSipControllerServiceThread* piJniThread = GetJniThread();
    if (piJniThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "UpdateDelegateRegistration:piJniThread is null", 0, 0, 0);
        return;
    }
    piJniThread->OnRegistrationUpdated(reinterpret_cast<IMS_UINTP>(pParam));
}

PROTECTED void RcsRegistrationService::ImsAos_Disconnecting(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("ImsAos_Connecting()", 0, 0, 0);
    IUSncFeatureTagsParam* pParam = new IUSncFeatureTagsParam();

    pParam->m_nFeatureCount = m_objCurrentFeatures.GetSize();
    pParam->m_nRegState = static_cast<IMS_SINT32>(RcsRegState::STATE_DEREGISTERING);
    pParam->m_nReason = GetReason(nReason);

    for (IMS_UINT32 i = 0; i < pParam->m_nFeatureCount; i++)
    {
        pParam->m_objFeatureTags.AddElement(m_objCurrentFeatures.GetAt(i));
    }
    IJniSipControllerServiceThread* piJniThread = GetJniThread();
    if (piJniThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "UpdateDelegateRegistration:piJniThread is null", 0, 0, 0);
        return;
    }
    piJniThread->OnRegistrationUpdated(reinterpret_cast<IMS_UINTP>(pParam));
}

PROTECTED void RcsRegistrationService::ImsAos_Disconnected(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("ImsAos_Connecting()", 0, 0, 0);
    IUSncFeatureTagsParam* pParam = new IUSncFeatureTagsParam();

    pParam->m_nFeatureCount = m_objCurrentFeatures.GetSize();
    pParam->m_nRegState = static_cast<IMS_SINT32>(RcsRegState::STATE_DEREGISTERED);
    pParam->m_nReason = GetReason(nReason);

    for (IMS_UINT32 i = 0; i < pParam->m_nFeatureCount; i++)
    {
        pParam->m_objFeatureTags.AddElement(m_objCurrentFeatures.GetAt(i));
    }
    IJniSipControllerServiceThread* piJniThread = GetJniThread();
    if (piJniThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "UpdateDelegateRegistration:piJniThread is null", 0, 0, 0);
        return;
    }
    piJniThread->OnRegistrationUpdated(reinterpret_cast<IMS_UINTP>(pParam));
}

PRIVATE void RcsRegistrationService::ImsAos_Suspended(IN IMS_UINT32 nReason)
{
    (void)nReason;
    IMS_TRACE_I("ImsAos_Suspended : Reason[%d]", nReason, 0, 0);
}

PRIVATE void RcsRegistrationService::ImsAos_Resumed()
{
    IMS_TRACE_I("ImsAos_Resumed ", 0, 0, 0);
}

PRIVATE
IMS_SINT32 RcsRegistrationService::GetReason(IN IMS_UINT32 nReason)
{
    switch (nReason)
    {
        case ImsAosReason::OUT_OF_SERVICE:
        case ImsAosReason::POWER_OFF:
        case ImsAosReason::NO_RAT_COVERAGE:
        case ImsAosReason::SERVICE_POLICY:
        case ImsAosReason::SERVICE_BLOCKED:
        case ImsAosReason::SUSPEND_OUT_OF_SERVICE:
        case ImsAosReason::SUSPEND_NO_RAT_COVERAGE:
            return static_cast<IMS_SINT32>(RcsDeRegReason::REASON_NOT_PROVISIONED);
        case ImsAosReason::DATA_DISCONNECTED:
            return static_cast<IMS_SINT32>(RcsDeRegReason::REASON_DESTROY_PENDING);
        case ImsAosReason::REG_TERMINATED:
            return static_cast<IMS_SINT32>(RcsDeRegReason::REASON_NOT_REGISTERED);
        case ImsAosReason::REG_NEW_REQUIRED:
            return static_cast<IMS_SINT32>(RcsDeRegReason::REASON_PROVISIONING_CHANGE);
        default:  // NOT_SPECIFIED
            return static_cast<IMS_SINT32>(RcsDeRegReason::REASON_UNSPECIFIED);
    }
}
