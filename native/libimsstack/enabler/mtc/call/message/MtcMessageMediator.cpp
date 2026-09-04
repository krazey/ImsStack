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
#include "IMtcService.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ImsTrace.h"
#include "ServiceTrace.h"
#include "SipAddress.h"
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

    MayShapeInitialInvite(piSipMessage);
    MayShapeInitialMtFinalResponse(piSipMessage);

    return IMS_SUCCESS;
}

PRIVATE
void MtcMessageMediator::MayShapeInitialInvite(IN_OUT ISipMessage* pMessage)
{
    const MtcConfigurationProxy& objConfig = m_objContext.GetConfigurationProxy();
    ImsVector<AString> objHeadersToRemove = objConfig.GetStringArray(
            ConfigVoice::KEY_INITIAL_INVITE_HEADERS_TO_REMOVE_STRING_ARRAY);
    ImsVector<AString> objHeadersToSet =
            objConfig.GetStringArray(ConfigVoice::KEY_INITIAL_INVITE_HEADERS_TO_SET_STRING_ARRAY);
    IMS_BOOL bCompactContact =
            objConfig.GetBoolean(ConfigVoice::KEY_INITIAL_INVITE_COMPACT_CONTACT_BOOL);

    if (objHeadersToRemove.IsEmpty() && objHeadersToSet.IsEmpty() && !bCompactContact)
    {
        return;
    }
    if (m_objContext.GetService().GetServiceType() != ServiceType::NORMAL ||
            pMessage->GetType() != ISipMessage::TYPE_REQUEST ||
            !pMessage->GetMethod().Equals(SipMethod::INVITE))
    {
        return;
    }

    AString strTo = pMessage->GetHeader(ISipHeader::TO).MakeLower();
    if (strTo.Contains(";tag="))
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objHeadersToRemove.GetSize(); ++i)
    {
        RemoveAllHeaders(pMessage, objHeadersToRemove.GetAt(i).Trim());
    }

    for (IMS_UINT32 i = 0; i < objHeadersToSet.GetSize(); ++i)
    {
        AString strHeader = objHeadersToSet.GetAt(i);
        IMS_SINT32 nColon = strHeader.GetIndexOf(TextParser::CHAR_COLON);
        if (nColon <= 0)
        {
            IMS_TRACE_E(0, "Invalid initial INVITE header override", 0, 0, 0);
            continue;
        }

        AString strName = strHeader.GetSubStr(0, nColon).Trim();
        AString strValue = strHeader.GetSubStr(nColon + 1).Trim();
        RemoveAllHeaders(pMessage, strName);
        pMessage->SetHeader(ISipHeader::UNKNOWN, strValue, strName);
    }

    if (bCompactContact)
    {
        MayCompactContactHeader(pMessage);
    }
}

PRIVATE
void MtcMessageMediator::MayShapeInitialMtFinalResponse(IN_OUT ISipMessage* pMessage)
{
    const MtcConfigurationProxy& objConfig = m_objContext.GetConfigurationProxy();
    ImsVector<AString> objHeadersToRemove = objConfig.GetStringArray(
            ConfigVoice::KEY_INITIAL_MT_FINAL_RESPONSE_HEADERS_TO_REMOVE_STRING_ARRAY);
    ImsVector<AString> objHeadersToSet = objConfig.GetStringArray(
            ConfigVoice::KEY_INITIAL_MT_FINAL_RESPONSE_HEADERS_TO_SET_STRING_ARRAY);
    IMS_BOOL bCompactContact = objConfig.GetBoolean(
            ConfigVoice::KEY_INITIAL_MT_FINAL_RESPONSE_COMPACT_CONTACT_BOOL);

    if (objHeadersToRemove.IsEmpty() && objHeadersToSet.IsEmpty() && !bCompactContact)
    {
        return;
    }

    IMS_SINT32 nStatusCode = pMessage->GetStatusCode();
    if (m_objContext.GetService().GetServiceType() != ServiceType::NORMAL ||
            pMessage->GetType() != ISipMessage::TYPE_RESPONSE ||
            !pMessage->GetMethod().Equals(SipMethod::INVITE) ||
            nStatusCode < 200 || nStatusCode >= 300 ||
            m_objContext.IsEstablished())
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objHeadersToRemove.GetSize(); ++i)
    {
        RemoveAllHeaders(pMessage, objHeadersToRemove.GetAt(i).Trim());
    }

    for (IMS_UINT32 i = 0; i < objHeadersToSet.GetSize(); ++i)
    {
        AString strHeader = objHeadersToSet.GetAt(i);
        IMS_SINT32 nColon = strHeader.GetIndexOf(TextParser::CHAR_COLON);
        if (nColon <= 0)
        {
            IMS_TRACE_E(0, "Invalid initial MT final-response header override", 0, 0, 0);
            continue;
        }

        AString strName = strHeader.GetSubStr(0, nColon).Trim();
        AString strValue = strHeader.GetSubStr(nColon + 1).Trim();
        RemoveAllHeaders(pMessage, strName);
        pMessage->SetHeader(ISipHeader::UNKNOWN, strValue, strName);
    }

    if (bCompactContact)
    {
        MayCompactContactHeader(pMessage);
    }

    IMS_TRACE_I("Initial MT final response shaped: status=%d, remove=%d, set=%d",
            nStatusCode, objHeadersToRemove.GetSize(), objHeadersToSet.GetSize());
}

PRIVATE
void MtcMessageMediator::RemoveAllHeaders(IN_OUT ISipMessage* pMessage, IN const AString& strName)
{
    IMS_SINT32 nCount = pMessage->GetHeaderCount(ISipHeader::UNKNOWN, strName);
    for (IMS_SINT32 i = 0; i < nCount; ++i)
    {
        pMessage->RemoveHeader(ISipHeader::UNKNOWN, strName);
    }
}

PRIVATE
void MtcMessageMediator::MayCompactContactHeader(IN_OUT ISipMessage* pMessage)
{
    AString strContact = pMessage->GetHeader(ISipHeader::CONTACT_NORMAL);
    IMS_SINT32 nLeftAquot = strContact.GetIndexOf(TextParser::CHAR_LAQUOT);
    IMS_SINT32 nRightAquot = nLeftAquot == AString::NPOS
            ? AString::NPOS
            : strContact.GetIndexOf(TextParser::CHAR_RAQUOT, nLeftAquot + 1);
    if (nRightAquot > 0 && nRightAquot + 1 < strContact.GetLength())
    {
        pMessage->SetHeader(ISipHeader::CONTACT_NORMAL, strContact.GetSubStr(0, nRightAquot + 1));
    }
}

PRIVATE
void MtcMessageMediator::MayAdjustContactHeader(IN_OUT ISipMessage* pMessage)
{
    ISipHeader* pContactHeader = IMS_NULL;
    MaySetVideoTextFeatureExclusively(&pContactHeader, pMessage);
    MayFormatContactAddress(&pContactHeader, pMessage);

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
            m_objContext.GetConfigurationProxy(), GetAosEmergencyRegMode(),
            m_objContext.GetService().GetNetworkRoamingType());

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
    const IMtcAosConnector* pAosConnector = m_objContext.GetAosConnector(ServiceType::EMERGENCY);
    if (pAosConnector == IMS_NULL)
    {
        IMS_TRACE_E(0, "IMtcAosConnector is null", 0, 0, 0);
        return IImsAosInfo::REG_MODE_UNKNOWN;
    }

    return pAosConnector->GetRegistrationMode();
}
