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

#include "CarrierConfig.h"
#include "ICoreService.h"
#include "IImsAosInfo.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ImsTrace.h"
#include "ServiceTrace.h"
#include "SipParsingHelper.h"
#include "TextParser.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcSession.h"
#include "call/message/MtcMessageMediator.h"
#include "call/message/TemplateFormatter.h"
#include "configuration/MtcConfigurationProxy.h"
#include "configuration/MtcConfigurationResolver.h"
#include "helper/IMtcAosConnector.h"
#include "utility/IMessageUtils.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcMessageMediator::MtcMessageMediator(IN IMtcCallContext& objContext) :
        m_objContext(objContext),
        m_strOriginalContactHeader(AString::ConstEmpty())
{
}

PUBLIC
MtcMessageMediator::~MtcMessageMediator() {}

PUBLIC IMS_RESULT MtcMessageMediator::MessageMediator_AdjustMessage(
        IN_OUT ISipMessage* piSipMessage, IN IMS_SINT32 /* nMessage */)
{
    if (piSipMessage->IsHeaderPresent(ISipHeader::CONTACT_NORMAL))
    {
        MayAdjustContactHeader(piSipMessage);
    }

    return IMS_SUCCESS;
}

PRIVATE
void MtcMessageMediator::MayAdjustContactHeader(IN_OUT ISipMessage* pMessage)
{
    ISipHeader* pContactHeader = IMS_NULL;
    MaySetVideoTextFeatureExclusively(&pContactHeader, pMessage);
    MayFormatContactAddress(&pContactHeader, pMessage);
    MayRemoveSosParameter(&pContactHeader, pMessage);

    if (pContactHeader)
    {
        pMessage->SetHeader(ISipHeader::CONTACT_NORMAL, pContactHeader->ToStringWithoutName());
        pContactHeader->Destroy();
    }
}

PRIVATE
void MtcMessageMediator::MaySetVideoTextFeatureExclusively(
        IN_OUT ISipHeader** pContactHeader, IN const ISipMessage* pMessage)
{
    if (!m_objContext.GetConfigurationProxy().GetBoolean(ConfigVt::
                        KEY_SET_VIDEO_TEXT_FEATURE_EXCLUSIVELY_IN_CONTACT_HEADER_BY_SESSION_TYPE_BOOL))
    {
        return;
    }

    if (*pContactHeader == IMS_NULL)
    {
        *pContactHeader = CreateContactHeader(pMessage);
    }
    switch (GetCallType())
    {
        case CallType::VT:
            return (*pContactHeader)->RemoveParameter(MessageUtil::STR_TEXT);
        case CallType::RTT:
            return (*pContactHeader)->RemoveParameter(MessageUtil::STR_VIDEO);
        default:
            return;
    }
}

PRIVATE
void MtcMessageMediator::MayFormatContactAddress(
        IN_OUT ISipHeader** pContactHeader, IN const ISipMessage* pMessage)
{
    if (m_objContext.GetService().GetServiceType() != ServiceType::EMERGENCY)
    {
        return;
    }
    AString strFormat = MtcConfigurationResolver::GetContactHeaderAddressInInviteForEmergency(
            m_objContext.GetConfigurationProxy(), GetAosEmergencyRegMode());
    if (strFormat.GetLength() <= 0)
    {
        return;
    }

    if (*pContactHeader == IMS_NULL)
    {
        *pContactHeader = CreateContactHeader(pMessage);
    }
    (*pContactHeader)->GetSipAddress()->SetUri(TemplateFormatter::Format(strFormat, m_objContext));
}

PRIVATE
void MtcMessageMediator::MayRemoveSosParameter(
        IN_OUT ISipHeader** pContactHeader, IN const ISipMessage* pMessage)
{
    if (m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigVoice::KEY_ALLOW_SOS_PARAM_IN_CONTACT_BOOL))
    {
        return;
    }

    if (*pContactHeader == IMS_NULL)
    {
        *pContactHeader = CreateContactHeader(pMessage);
    }
    (*pContactHeader)->GetSipAddress()->RemoveParameter("sos");
}

PRIVATE
ISipHeader* MtcMessageMediator::CreateContactHeader(IN const ISipMessage* pMessage) const
{
    ISipHeader* pContactHeader = SipParsingHelper::CreateHeader(
            ISipHeader::CONTACT_NORMAL, pMessage->GetHeader(ISipHeader::CONTACT_NORMAL));
    if (!pContactHeader)
    {
        IMS_TRACE_E(0, "Failed to create a Contact header", 0, 0, 0);
    }

    return pContactHeader;
}

PRIVATE
CallType MtcMessageMediator::GetCallType() const
{
    // VZ_REQ_5GNRSAVOICEVIDEO_4105999311948863
    // The device shall treat a "downgraded video call" as a video call, ...
    CallType eCallType = m_objContext.GetMessageUtils().GetCallTypeFromSdp(
            &m_objContext.GetSession()->GetISession(), IMS_FALSE, IMS_TRUE, IMS_FALSE);
    if (eCallType != CallType::UNKNOWN)
    {
        return eCallType;
    }

    return m_objContext.GetSession()->GetCallType();
}

PRIVATE
IMS_UINT32 MtcMessageMediator::GetAosEmergencyRegMode() const
{
    IMtcAosConnector* pAosConnector = m_objContext.GetAosConnector(ServiceType::EMERGENCY);
    if (pAosConnector == IMS_NULL)
    {
        IMS_TRACE_E(0, "IMtcAosConnector is null", 0, 0, 0);
        return IImsAosInfo::REG_MODE_UNKNOWN;
    }

    return pAosConnector->GetRegistrationMode();
}
