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
#include "configuration/MtcConfigurationProxy.h"
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
    IMS_BOOL bSetVideoTextFeatureExclusively = m_objContext.GetConfigurationProxy().GetBoolean(
            ConfigVt::
                    KEY_SET_VIDEO_TEXT_FEATURE_EXCLUSIVELY_IN_CONTACT_HEADER_BY_SESSION_TYPE_BOOL);
    IMS_BOOL bAllowSosParam = m_objContext.GetConfigurationProxy().GetBoolean(
            ConfigVoice::KEY_ALLOW_SOS_PARAM_IN_CONTACT_BOOL);
    if (bSetVideoTextFeatureExclusively == IMS_FALSE && bAllowSosParam == IMS_TRUE)
    {
        return;  // No need to adjust
    }

    ISipHeader* pContactHeader = SipParsingHelper::CreateHeader(
            ISipHeader::CONTACT_NORMAL, pMessage->GetHeader(ISipHeader::CONTACT_NORMAL));
    if (!pContactHeader)
    {
        return;
    }

    if (bSetVideoTextFeatureExclusively)
    {
        SetVideoTextFeatureExclusively(pContactHeader);
    }
    if (!bAllowSosParam)
    {
        RemoveSosParameter(pContactHeader);
    }

    pMessage->SetHeader(ISipHeader::CONTACT_NORMAL, pContactHeader->ToStringWithoutName());
    pContactHeader->Destroy();
}

PRIVATE
void MtcMessageMediator::SetVideoTextFeatureExclusively(IN_OUT ISipHeader* pContactHeader)
{
    switch (GetCallType())
    {
        case CallType::VT:
            return pContactHeader->RemoveParameter(MessageUtil::STR_TEXT);
        case CallType::RTT:
            return pContactHeader->RemoveParameter(MessageUtil::STR_VIDEO);
        default:
            return;
    }
}

PRIVATE
void MtcMessageMediator::RemoveSosParameter(IN_OUT ISipHeader* pContactHeader)
{
    SipAddress* pAddress = pContactHeader->GetSipAddress();
    pAddress->RemoveParameter("sos");
}

PRIVATE
CallType MtcMessageMediator::GetCallType()
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
