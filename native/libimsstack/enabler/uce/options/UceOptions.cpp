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
#include "options/UceOptions.h"

#include "ICapabilities.h"
#include "ICoreService.h"
#include "IJniEnabler.h"
#include "IMessage.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "IUce.h"
#include "ServiceMessage.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "Sip.h"
#include "SipStatusCode.h"
#include "TextParser.h"
#include "IUceJniThread.h"
#include "JniEnablerConnector.h"

__IMS_TRACE_TAG_UCE__;

PUBLIC
UceOptions::UceOptions(IN const AString& strManagerName, IN ICoreService* piCoreService,
        IN ICapabilities* piCapabilities, IN IMS_UINT32 nKey, IN IMS_BOOL isSendingRequest,
        IN IMS_SINT32 nSimSlot) :
        m_nKey(nKey),
        m_bIsSendingRequest(isSendingRequest),
        m_strManagerName(strManagerName),
        m_piCoreService(piCoreService),
        m_piCapabilities(piCapabilities),
        m_nSimSlot(nSimSlot)
{
    m_nSimSlot = nSimSlot;
    IMS_TRACE_D("UCE_M : UceOptions = %" PFLS_u, sizeof(UceOptions), 0, 0);
    IMS_TRACE_I("UceOptions", 0, 0, 0);
}

PUBLIC VIRTUAL UceOptions::~UceOptions()
{
    IMS_TRACE_D("UCE_F : UceOptions = %" PFLS_u, sizeof(UceOptions), 0, 0);
    IMS_TRACE_I("~UceOptions", 0, 0, 0);
    DestroyCapabilities();
}

PUBLIC
IMS_BOOL UceOptions::SendOptionsRequest(IN AString strRemoteURI, IN IMS_UINT32 ownCapabilities)
{
    IMS_TRACE_D("SendOptionsRequest: remoteUri [%s] ", strRemoteURI.GetStr(), 0, 0);
    if (m_piCoreService == IMS_NULL)
    {
        IMS_TRACE_I("SendOptionsRequest:m_piCoreService is null", 0, 0, 0);
        SendOptionsCommandError(IUUceService::COMMAND_CODE_GENERIC_FAILURE);
        OptionsTerminated();
        return IMS_FALSE;
    }
    AString strFrom = m_piCoreService->GetUserIdentity(Sip::URI_SCHEME_TEL);
    DestroyCapabilities();
    m_piCapabilities = m_piCoreService->CreateCapabilities(strFrom, strRemoteURI);
    if (m_piCapabilities == IMS_NULL)
    {
        IMS_TRACE_I("SendOptionsRequest:piCapabilities is null", 0, 0, 0);
        SendOptionsCommandError(IUUceService::COMMAND_CODE_GENERIC_FAILURE);
        OptionsTerminated();
        return IMS_FALSE;
    }
    return HandleOptionsRequest(m_piCapabilities, ownCapabilities);
}

IMS_BOOL UceOptions::SendOptionsResponse(
        IN IMS_UINT32 nResponse, IN const AString& reason, IN IMS_UINT32 ownCapabilities)
{
    (void)reason;
    if (m_piCapabilities == IMS_NULL)
    {
        IMS_TRACE_I("SendOptionsResponse:m_piCapabilities is null", 0, 0, 0);
        return IMS_FALSE;
    }

    if (nResponse == 200)
    {
        IMessage* piMessage = m_piCapabilities->GetNextResponse();
        if (piMessage == IMS_NULL)
        {
            IMS_TRACE_I("SendOptionsResponse:IMessage is null", 0, 0, 0);
            return IMS_FALSE;
        }

        ISipMessage* piSIPMessage = piMessage->GetMessage();
        if (piSIPMessage == IMS_NULL)
        {
            IMS_TRACE_I("SendOptionsResponse:ISipMessage is null", 0, 0, 0);
            return IMS_FALSE;
        }
        SetContactHeader(ownCapabilities, piSIPMessage);

        if (m_piCapabilities->Accept(nResponse) == IMS_FAILURE)
        {
            IMS_TRACE_I("SendOptionsResponse:Accept is failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }
    else
    {
        if (m_piCapabilities->Reject(nResponse) == IMS_FAILURE)
        {
            IMS_TRACE_I("SendOptionsResponse:Reject is failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }
    return IMS_TRUE;
}

void UceOptions::AoSDisconnected()
{
    if (m_bIsSendingRequest)
    {
        SendOptionsCommandError(IUUceService::COMMAND_CODE_GENERIC_FAILURE);
    }
    OptionsTerminated();
}

IMS_UINT32 UceOptions::GetCapability(ImsList<AString> objContactList)
{
    IMS_UINT32 capabilities = 0;
    for (IMS_UINT32 i = 0; i < objContactList.GetSize(); i++)
    {
        AString featureTag = objContactList.GetAt(i);
        if (featureTag.Contains("urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.dp"))
        {
            capabilities |= FEATURE_TAG_DP;
        }
        else if (featureTag.Contains("urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel"))
        {
            capabilities |= FEATURE_TAG_IPCALL_VOICE;
        }
        else if (featureTag.Contains("video"))
        {
            capabilities |= FEATURE_TAG_IPCALL_VIDEO;
        }
        else if (featureTag.Contains("urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.msg"))
        {
            capabilities |= FEATURE_TAG_PAGER_MESSAGING;
        }
        else if (featureTag.Contains("urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.largemsg"))
        {
            capabilities |= FEATURE_TAG_LARGE_MESSAGING;
        }
        else if (featureTag.Contains("urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session"))
        {
            capabilities |= FEATURE_TAG_CPM_CHAT;
        }
        else if (featureTag.Contains("urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.im"))
        {
            capabilities |= FEATURE_TAG_SIMPLE_IM;
        }
        else if (featureTag.Contains("urn%3Aurn-7%3A3gpp-application.ims.iari."
                                     "rcs.fullsfgroupchat"))
        {
            capabilities |= FEATURE_TAG_STORE_FORWARD_GROUP_CHAT;
        }
        else if (featureTag.Contains("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftthumb"))
        {
            capabilities |= FEATURE_TAG_FT_THUMBNAIL;
        }
        else if (featureTag.Contains("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftstandfw"))
        {
            capabilities |= FEATURE_TAG_FT_STORE_FORWARD;
        }
        else if (featureTag.Contains("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fthttp"))
        {
            capabilities |= FEATURE_TAG_FT_HTTP;
        }
        else if (featureTag.Contains("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geopush"))
        {
            capabilities |= FEATURE_TAG_GEOLOCATION_PUSH;
        }
        else if (featureTag.Contains("urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.ft"))
        {
            capabilities |= FEATURE_TAG_FT;
        }
        else if (featureTag.Contains("urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.sharedmap"))
        {
            capabilities |= FEATURE_TAG_SHARED_MAP;
        }
        else if (featureTag.Contains("urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.sharedsketch"))
        {
            capabilities |= FEATURE_TAG_SHARED_SKETCH;
        }
        else if (featureTag.Contains("urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.callcomposer"))
        {
            capabilities |= FEATURE_TAG_CALL_COMPOSER;
        }
        else if (featureTag.Contains("+g.gsma.callcomposer"))
        {
            capabilities |= FEATURE_TAG_CALL_COMPOSER_MMTEL;
        }
        else if (featureTag.Contains("urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.callunanswered"))
        {
            capabilities |= FEATURE_TAG_POST_CALL;
        }
        else if (featureTag.Contains("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftsms"))
        {
            capabilities |= FEATURE_TAG_FT_SMS;
        }
        else if (featureTag.Contains("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geosms"))
        {
            capabilities |= FEATURE_TAG_GEOLOCATION_SMS;
        }
        else if (featureTag.EqualsIgnoreCase("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot"))
        {
            capabilities |= FEATURE_TAG_CHATBOT_SESSION;
        }
        else if (featureTag.EqualsIgnoreCase(
                         "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot.sa"))
        {
            capabilities |= FEATURE_TAG_CHATBOT_SA;
        }
        else if (featureTag.Contains("+g.gsma.rcs.isbot"))
        {
            capabilities |= FEATURE_TAG_IS_BOT;
        }
        else if (featureTag.Contains("+g.gsma.rcs.botversion=\"#=1\""))
        {
            capabilities |= FEATURE_TAG_CHATBOT_VERSION_V1;
        }
        else if (featureTag.Contains("+g.gsma.rcs.botversion=\"#=1,#=2\""))
        {
            capabilities |= FEATURE_TAG_CHATBOT_VERSION_V2;
        }
        else
        {
            IMS_TRACE_I("GetCapability:Uuknow Feature tag[%s]", featureTag.GetStr(), 0, 0);
        }
    }
    return capabilities;
}

void UceOptions::SetIARIFeatureTag(IN IMS_UINT32 capabilities, OUT AString& strIARITag)
{
    if ((capabilities & FEATURE_TAG_DP) > 0)
    {
        strIARITag.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.dp");
        strIARITag.Append(TextParser::CHAR_COMMA);
    }
    if ((capabilities & FEATURE_TAG_SIMPLE_IM) > 0)
    {
        strIARITag.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.im");
        strIARITag.Append(TextParser::CHAR_COMMA);
    }
    if ((capabilities & FEATURE_TAG_STORE_FORWARD_GROUP_CHAT) > 0)
    {
        strIARITag.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fullsfgroupchat");
        strIARITag.Append(TextParser::CHAR_COMMA);
    }
    if ((capabilities & FEATURE_TAG_FT_THUMBNAIL) > 0)
    {
        strIARITag.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftthumb");
        strIARITag.Append(TextParser::CHAR_COMMA);
    }
    if ((capabilities & FEATURE_TAG_FT_STORE_FORWARD) > 0)
    {
        strIARITag.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftstandfw");
        strIARITag.Append(TextParser::CHAR_COMMA);
    }
    if ((capabilities & FEATURE_TAG_FT_HTTP) > 0)
    {
        strIARITag.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fthttp");
        strIARITag.Append(TextParser::CHAR_COMMA);
    }
    if ((capabilities & FEATURE_TAG_GEOLOCATION_PUSH) > 0)
    {
        strIARITag.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geopush");
        strIARITag.Append(TextParser::CHAR_COMMA);
    }
    if ((capabilities & FEATURE_TAG_FT) > 0)
    {
        strIARITag.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.ft");
        strIARITag.Append(TextParser::CHAR_COMMA);
    }
    if ((capabilities & FEATURE_TAG_FT_SMS) > 0)
    {
        strIARITag.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftsms");
        strIARITag.Append(TextParser::CHAR_COMMA);
    }
    if ((capabilities & FEATURE_TAG_GEOLOCATION_SMS) > 0)
    {
        strIARITag.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geosms");
        strIARITag.Append(TextParser::CHAR_COMMA);
    }
    if ((capabilities & FEATURE_TAG_CHATBOT_SESSION) > 0)
    {
        strIARITag.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot");
        strIARITag.Append(TextParser::CHAR_COMMA);
    }
    if ((capabilities & FEATURE_TAG_CHATBOT_SA) > 0)
    {
        strIARITag.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot.sa");
        strIARITag.Append(TextParser::CHAR_COMMA);
    }
    if (!strIARITag.IsEmpty())
    {
        strIARITag.Erase(strIARITag.GetLength() - 1, 1);
    }
}

void UceOptions::SetICSIFeatureTag(IN IMS_UINT32 capabilities, OUT AString& strICSITag)
{
    if ((capabilities & FEATURE_TAG_IPCALL_VOICE) > 0 ||
            (capabilities & FEATURE_TAG_IPCALL_VIDEO) > 0)
    {
        strICSITag.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel");
        strICSITag.Append(TextParser::CHAR_COMMA);
    }
    if ((capabilities & FEATURE_TAG_PAGER_MESSAGING) > 0)
    {
        strICSITag.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.msg");
        strICSITag.Append(TextParser::CHAR_COMMA);
    }
    if ((capabilities & FEATURE_TAG_LARGE_MESSAGING) > 0)
    {
        strICSITag.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.largemsg");
        strICSITag.Append(TextParser::CHAR_COMMA);
    }
    if ((capabilities & FEATURE_TAG_CPM_CHAT) > 0)
    {
        strICSITag.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session");
        strICSITag.Append(TextParser::CHAR_COMMA);
    }
    if ((capabilities & FEATURE_TAG_SHARED_MAP) > 0)
    {
        strICSITag.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.sharedmap");
        strICSITag.Append(TextParser::CHAR_COMMA);
    }
    if ((capabilities & FEATURE_TAG_SHARED_SKETCH) > 0)
    {
        strICSITag.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.sharedsketch");
        strICSITag.Append(TextParser::CHAR_COMMA);
    }
    if ((capabilities & FEATURE_TAG_CALL_COMPOSER) > 0)
    {
        strICSITag.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.callcomposer");
        strICSITag.Append(TextParser::CHAR_COMMA);
    }
    if ((capabilities & FEATURE_TAG_POST_CALL) > 0)
    {
        strICSITag.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.callunanswered");
        strICSITag.Append(TextParser::CHAR_COMMA);
    }
    if (!strICSITag.IsEmpty())
    {
        strICSITag.Erase(strICSITag.GetLength() - 1, 1);
    }
}

void UceOptions::SetNoTypeFeatureTag(IN IMS_UINT32 capabilities, OUT AString& strTag)
{
    if ((capabilities & FEATURE_TAG_IPCALL_VIDEO) > 0)
    {
        strTag.Append("video");
        strTag.Append(TextParser::CHAR_SEMICOLON);
    }
    if ((capabilities & FEATURE_TAG_CALL_COMPOSER_MMTEL) > 0)
    {
        strTag.Append("+g.gsma.callcomposer");
        strTag.Append(TextParser::CHAR_SEMICOLON);
    }
    if ((capabilities & FEATURE_TAG_IS_BOT) > 0)
    {
        strTag.Append("+g.gsma.rcs.isbot");
        strTag.Append(TextParser::CHAR_SEMICOLON);
    }
    if ((capabilities & FEATURE_TAG_CHATBOT_VERSION_V1) > 0)
    {
        strTag.Append("+g.gsma.rcs.botversion=\"#=1\"");
        strTag.Append(TextParser::CHAR_SEMICOLON);
    }
    if ((capabilities & FEATURE_TAG_CHATBOT_VERSION_V2) > 0)
    {
        strTag.Append("+g.gsma.rcs.botversion=\"#=1,#=2\"");
        strTag.Append(TextParser::CHAR_SEMICOLON);
    }
    if (!strTag.IsEmpty())
    {
        strTag.Erase(strTag.GetLength() - 1, 1);
    }
}

IMS_BOOL UceOptions::HandleOptionsRequest(
        IN ICapabilities* piCapabilities, IN IMS_UINT32 ownCapabilities)
{
    piCapabilities->SetListener(this);

    IMessage* piMessage = piCapabilities->GetNextRequest();
    if (piMessage == IMS_NULL)
    {
        SendOptionsCommandError(IUUceService::COMMAND_CODE_GENERIC_FAILURE);
        OptionsTerminated();
        IMS_TRACE_I("SendOptionsRequest:IMessage is null", 0, 0, 0);
        return IMS_FALSE;
    }

    ISipMessage* piSIPMessage = piMessage->GetMessage();
    if (piSIPMessage == IMS_NULL)
    {
        SendOptionsCommandError(IUUceService::COMMAND_CODE_GENERIC_FAILURE);
        OptionsTerminated();
        IMS_TRACE_I("SendOptionsRequest:ISipMessage is null", 0, 0, 0);
        return IMS_FALSE;
    }

    SetContactHeader(ownCapabilities, piSIPMessage);

    if (piCapabilities->QueryCapabilities(ICapabilities::FLAG_ADD_CONTACT_HEADER) == IMS_FAILURE)
    {
        IMS_TRACE_I("SendOptionsRequest:QueryCapabilities is failed", 0, 0, 0);
        SendOptionsCommandError(IUUceService::COMMAND_CODE_GENERIC_FAILURE);
        OptionsTerminated();
        return IMS_FALSE;
    }
    return IMS_TRUE;
}

PROTECTED VIRTUAL void UceOptions::CapabilityQueryDelivered(IN ICapabilities* piCapabilities)
{
    // received success response to options request
    if (piCapabilities == IMS_NULL || (m_piCapabilities != piCapabilities))
    {
        IMS_TRACE_I("CapabilityQueryDelivered:piCapabilities_ is null", 0, 0, 0);
        SendOptionsCommandError(IUUceService::COMMAND_CODE_GENERIC_FAILURE);
        OptionsTerminated();
        return;
    }

    const IMessage* piMessage = m_piCapabilities->GetPreviousResponse(IMessage::CAPABILITIES_QUERY);
    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_I("CapabilityQueryDelivered:piMessage is null", 0, 0, 0);
        SendOptionsCommandError(IUUceService::COMMAND_CODE_GENERIC_FAILURE);
        OptionsTerminated();
        return;
    }

    const ISipMessage* piSIPMessage = piMessage->GetMessage();
    if (piSIPMessage == IMS_NULL)
    {
        IMS_TRACE_I("CapabilityQueryDelivered:piSIPMessage is null", 0, 0, 0);
        SendOptionsCommandError(IUUceService::COMMAND_CODE_GENERIC_FAILURE);
        OptionsTerminated();
        return;
    }

    ImsList<AString> objContactList = piSIPMessage->GetHeaders(ISipHeader::CONTACT_NORMAL);
    IMS_UINT32 capabilities = GetCapability(objContactList);
    SendOptionsResponseInd(piMessage->GetStatusCode(), piMessage->GetReasonPhrase(), capabilities);
    OptionsTerminated();
}

PROTECTED VIRTUAL void UceOptions::CapabilityQueryDeliveryFailed(IN ICapabilities* piCapabilities)
{
    // received failure response to options request
    if (piCapabilities == IMS_NULL || (m_piCapabilities != piCapabilities))
    {
        IMS_TRACE_I("CapabilityQueryDeliveryFailed:piCapabilities_ is null", 0, 0, 0);
        SendOptionsResponseInd(SipStatusCode::SC_408, "Request Timeout", 0);
        OptionsTerminated();
        return;
    }

    const IMessage* piMessage = m_piCapabilities->GetPreviousResponse(IMessage::CAPABILITIES_QUERY);
    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_I("CapabilityQueryDeliveryFailed:piMessage is null", 0, 0, 0);
        SendOptionsResponseInd(SipStatusCode::SC_408, "Request Timeout", 0);
        OptionsTerminated();
        return;
    }
    /*
    ISipMessage* piSIPMessage = piMessage->GetMessage();
    if (piSIPMessage == IMS_NULL)
    {
        IMS_TRACE_I("CapabilityQueryDeliveryFailed:piSIPMessage is null", 0, 0, 0);
        SendOptionsResponseInd(SipStatusCode::SC_408, "Request Timeout", 0);
        OptionsTerminated();
        return;
    }
    */
    SendOptionsResponseInd(piMessage->GetStatusCode(), piMessage->GetReasonPhrase(), 0);
    OptionsTerminated();
}

PRIVATE
void UceOptions::SetContactHeader(IN IMS_UINT32 capabilities, ISipMessage* piSIPMessage) const
{
    const IService* piService = m_piCoreService;
    AString strMyContact = AString::ConstEmpty();

    if (piService->GetPublicGruu() == IMS_NULL)
    {
        strMyContact.Append(piService->GetContactAddress().ToString());
    }
    else
    {
        strMyContact.Append(piService->GetPublicGruu()->ToString());
        piSIPMessage->AddHeader(ISipHeader::SUPPORTED, "gruu");
    }

    if (!strMyContact.Contains(TextParser::CHAR_LAQUOT))
    {
        strMyContact.Prepend(TextParser::CHAR_LAQUOT);
        strMyContact.Append(TextParser::CHAR_RAQUOT);
    }

    AString strIari = AString::ConstEmpty();
    SetIARIFeatureTag(capabilities, strIari);
    if (!strIari.IsEmpty())
    {
        strMyContact.Append(TextParser::CHAR_SEMICOLON);
        strMyContact.Append("+g.3gpp.iari-ref=");  // iari service start
        strMyContact.Append(TextParser::STR_DQUOTE);
        strMyContact.Append(strIari);
        strMyContact.Append(TextParser::STR_DQUOTE);  // iari service end
    }

    AString strIcsi = AString::ConstEmpty();
    SetICSIFeatureTag(capabilities, strIcsi);
    if (!strIcsi.IsEmpty())
    {
        strMyContact.Append(TextParser::CHAR_SEMICOLON);
        strMyContact.Append("+g.3gpp.icsi-ref=");  // icsi service start
        strMyContact.Append(TextParser::STR_DQUOTE);
        strMyContact.Append(strIcsi);
        strMyContact.Append(TextParser::STR_DQUOTE);  // icsi service end
    }

    AString strNoType = AString::ConstEmpty();
    SetNoTypeFeatureTag(capabilities, strNoType);

    if (!strNoType.IsEmpty())
    {
        strMyContact.Append(TextParser::CHAR_SEMICOLON);
        strMyContact.Append(strNoType);
    }

    piSIPMessage->SetHeader(ISipHeader::CONTACT_NORMAL, strMyContact);
}

void UceOptions::SendOptionsResponseInd(
        IN IMS_SINT32 nResponseCode, IN AString reason, IN IMS_UINT32 capabilities)
{
    IMS_TRACE_I("SendOptionsResponseInd:key[%d], code[%d], reason[%s]", m_nKey, nResponseCode,
            reason.GetStr());
    IUceJniThread* piJniThread = GetJniThread();
    if (piJniThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "SendOptionsResponseInd:piJniThread is null", 0, 0, 0);
        m_nKey = 0;
        return;
    }
    piJniThread->OptionsResponseInd(m_nKey, nResponseCode, reason, capabilities);
    m_nKey = 0;
}

void UceOptions::SendOptionsCommandError(IN IMS_UINT32 code)
{
    IMS_TRACE_I("SendOptionsCommandError:key[%d], error[%d]", m_nKey, code, 0);
    IUceJniThread* piJniThread = GetJniThread();
    if (piJniThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "SendOptionsCommandError:piJniThread is null", 0, 0, 0);
        m_nKey = 0;
        return;
    }
    piJniThread->OptionsErrorInd(m_nKey, code);
    m_nKey = 0;
}

void UceOptions::OptionsTerminated()
{
    IMS_TRACE_I("OptionsTerminated:send UCE_OPTIONS_DELETED_IND to manager", 0, 0, 0);
    m_bIsSendingRequest = IMS_FALSE;

    IMSMSG objUIMsg(IUUceService::UCE_OPTIONS_DELETED_IND, 0, m_nKey);
    MessageService::PostMessage(m_strManagerName, objUIMsg);
}

void UceOptions::DestroyCapabilities()
{
    if (m_piCapabilities != IMS_NULL)
    {
        m_piCapabilities->Destroy();
        m_piCapabilities = IMS_NULL;
    }
}

IUceJniThread* UceOptions::GetJniThread()
{
    const IJniEnabler* piJniEnabler =
            JniEnablerConnector::GetInstance().GetJniEnabler(m_nSimSlot, EnablerType::UCE);
    if (piJniEnabler == IMS_NULL)
    {
        return IMS_NULL;
    }

    return reinterpret_cast<IUceJniThread*>(piJniEnabler->GetJniThread());
}
