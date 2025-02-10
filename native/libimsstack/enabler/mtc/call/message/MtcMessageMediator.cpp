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
    if (m_strOriginalContactHeader.GetLength() <= 0)
    {
        m_strOriginalContactHeader = piSipMessage->GetHeader(ISipHeader::CONTACT_NORMAL);
    }

    if (piSipMessage->IsHeaderPresent(ISipHeader::CONTACT_NORMAL) &&
            m_objContext.GetConfigurationProxy().GetBoolean(ConfigVt::
                            KEY_SET_VIDEO_TEXT_FEATURE_EXCLUSIVELY_IN_CONTACT_HEADER_BY_SESSION_TYPE_BOOL))
    {
        switch (GetCallTypeOfCurrentMessage())
        {
            case CallType::VT:
                piSipMessage->SetHeader(ISipHeader::CONTACT_NORMAL,
                        GetContactHeaderWithoutFeatureTag(MessageUtil::STR_TEXT));
                break;
            case CallType::RTT:
                piSipMessage->SetHeader(ISipHeader::CONTACT_NORMAL,
                        GetContactHeaderWithoutFeatureTag(MessageUtil::STR_VIDEO));
                break;
            default:
                piSipMessage->SetHeader(ISipHeader::CONTACT_NORMAL, m_strOriginalContactHeader);
                break;
        }
    }

    return IMS_SUCCESS;
}

PRIVATE
AString MtcMessageMediator::GetContactHeaderWithoutFeatureTag(IN const AString& strFeatureTag)
{
    IMS_TRACE_D(
            "GetContactHeaderWithoutFeatureTag : Feature tag[%s]", strFeatureTag.GetStr(), 0, 0);

    ISipHeader* piHeader =
            SipParsingHelper::CreateHeader(ISipHeader::CONTACT_NORMAL, m_strOriginalContactHeader);
    if (!piHeader)
    {
        return m_strOriginalContactHeader;
    }

    piHeader->RemoveParameter(strFeatureTag);
    AString strModifiedHeader = piHeader->GetHeaderValue();
    piHeader->Destroy();

    return strModifiedHeader;
}

PRIVATE
CallType MtcMessageMediator::GetCallTypeOfCurrentMessage()
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
