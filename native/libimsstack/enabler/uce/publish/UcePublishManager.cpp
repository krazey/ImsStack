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
#include "publish/UcePublishManager.h"

#include "AosAppRequestType.h"
#include "ICoreService.h"
#include "IJniEnabler.h"
#include "IMessage.h"
#include "IPublication.h"
#include "ISipClientConnection.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "IZLib.h"
#include "ImsAosParameter.h"
#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "TextParser.h"
#include "IUce.h"
#include "config/UceConfig.h"
#include "def/UceDef.h"
#include "IUceJniThread.h"
#include "JniEnablerConnector.h"

__IMS_TRACE_TAG_UCE__;

/* -------------------------------------------------------------------------------------------------
    StateMachine
-------------------------------------------------------------------------------------------------
*/
BEGIN_STATE_MAP(UcePublishManager)
STATE_ENTRY(IDLE)
STATE_ENTRY(ON)
STATE_ENTRY(PUBLISHING)
STATE_ENTRY(PUBLISHED)
STATE_ENTRY(REFRESHING)
STATE_ENTRY(TERMINATING)
END_STATE_MAP()

BEGIN_STATE_MSG_MAP(UcePublishManager, IDLE)
STATE_MSG_ENTRY(PUBLISH_REQUESTED, &UcePublishManager::StateIDLE_PublishRequested)
STATE_MSG_ENTRY(AOS_CONNECTED, &UcePublishManager::StateIDLE_AoSConnected)
STATE_MSG_ENTRY(SERVICE_CLOSED, &UcePublishManager::StateALL_Terminated)
END_STATE_MSG_MAP()

BEGIN_STATE_MSG_MAP(UcePublishManager, ON)
STATE_MSG_ENTRY(PUBLISH_REQUESTED, &UcePublishManager::StateON_PublishRequested)
STATE_MSG_ENTRY(AOS_DISCONNECTED, &UcePublishManager::StateON_AoSDisConnected)
STATE_MSG_ENTRY(SERVICE_CLOSED, &UcePublishManager::StateALL_Terminated)
END_STATE_MSG_MAP()

BEGIN_STATE_MSG_MAP(UcePublishManager, PUBLISHING)
STATE_MSG_ENTRY(PUBLISH_REQUESTED, &UcePublishManager::StatePUBLISHING_PublishRequested)
STATE_MSG_ENTRY(AOS_DISCONNECTING, &UcePublishManager::StatePUBLISHING_AoSDisConnecting)
STATE_MSG_ENTRY(AOS_DISCONNECTED, &UcePublishManager::StatePUBLISHING_AoSDisConnected)
STATE_MSG_ENTRY(PUBLISH_SUCCEEDED, &UcePublishManager::StatePUBLISHING_Published)
STATE_MSG_ENTRY(PUBLISH_FAILED, &UcePublishManager::StatePUBLISHING_Failed)
STATE_MSG_ENTRY(PUBLISH_TERMINATED, &UcePublishManager::StateALL_Terminated)
STATE_MSG_ENTRY(SERVICE_CLOSED, &UcePublishManager::StateALL_Terminated)
END_STATE_MSG_MAP()

BEGIN_STATE_MSG_MAP(UcePublishManager, PUBLISHED)
STATE_MSG_ENTRY(PUBLISH_REQUESTED, &UcePublishManager::StatePUBLISHED_PublishRequested)
STATE_MSG_ENTRY(AOS_DISCONNECTING, &UcePublishManager::StatePUBLISHED_AoSDisconnecting)
STATE_MSG_ENTRY(AOS_DISCONNECTED, &UcePublishManager::StatePUBLISHED_AoSDisconnected)
STATE_MSG_ENTRY(PUBLISH_TERMINATED, &UcePublishManager::StateALL_Terminated)
STATE_MSG_ENTRY(PUBLISH_REFRESH_STARTED, &UcePublishManager::StatePUBLISHED_RefreshStarted)
STATE_MSG_ENTRY(SERVICE_CLOSED, &UcePublishManager::StateALL_Terminated)
END_STATE_MSG_MAP()

BEGIN_STATE_MSG_MAP(UcePublishManager, REFRESHING)
STATE_MSG_ENTRY(PUBLISH_REQUESTED, &UcePublishManager::StateREFRESHING_PublishRequested)
STATE_MSG_ENTRY(AOS_DISCONNECTING, &UcePublishManager::StateREFRESHING_AoSDisConnecting)
STATE_MSG_ENTRY(AOS_DISCONNECTED, &UcePublishManager::StateREFRESHING_AoSDisConnected)
STATE_MSG_ENTRY(PUBLISH_SUCCEEDED, &UcePublishManager::StateREFRESHING_Refreshed)
STATE_MSG_ENTRY(PUBLISH_FAILED, &UcePublishManager::StateREFRESHING_RefreshFailed)
STATE_MSG_ENTRY(PUBLISH_TERMINATED, &UcePublishManager::StateALL_Terminated)
STATE_MSG_ENTRY(PUBLISH_REFRESHED, &UcePublishManager::StateREFRESHING_Refreshed)
STATE_MSG_ENTRY(PUBLISH_REFRESH_FAILED, &UcePublishManager::StateREFRESHING_RefreshFailed)
STATE_MSG_ENTRY(PUBLISH_REFRESH_NO_RESPONSE,
        &UcePublishManager::StateREFRESHING_RefreshFailedWithNoResponse)
STATE_MSG_ENTRY(SERVICE_CLOSED, &UcePublishManager::StateALL_Terminated)
END_STATE_MSG_MAP()

BEGIN_STATE_MSG_MAP(UcePublishManager, TERMINATING)
STATE_MSG_ENTRY(PUBLISH_REQUESTED, &UcePublishManager::StateTERMINATING_PublishRequested)
STATE_MSG_ENTRY(AOS_DISCONNECTED, &UcePublishManager::StateTERMINATING_AoSDisconnected)
STATE_MSG_ENTRY(PUBLISH_SUCCEEDED, &UcePublishManager::StateTERMINATING_Published)
STATE_MSG_ENTRY(PUBLISH_FAILED, &UcePublishManager::StateTERMINATING_Failed)
STATE_MSG_ENTRY(PUBLISH_TERMINATED, &UcePublishManager::StateALL_Terminated)
STATE_MSG_ENTRY(SERVICE_CLOSED, &UcePublishManager::StateALL_Terminated)
END_STATE_MSG_MAP()

/* -------------------------------------------------------------------------------------------------
    Constructor, Destructor, Operator Overloading
-------------------------------------------------------------------------------------------------
*/
UcePublishManager::UcePublishManager(
        IN ICoreService* _piCoreService, IN const AString& strAppName, IN IMS_SINT32 nSimSlot) :
        m_eState(IDLE),
        m_nKey(0),
        m_bReceivedUnPublishRequest(IMS_FALSE),
        m_pPendingPublicationData(IMS_NULL),
        m_nConnectedServices(0),
        m_piPublication(IMS_NULL),
        m_pExponentialTimer(IMS_NULL),
        m_pRetryTimer(IMS_NULL),
        m_pRetryAfterTimer(IMS_NULL),
        m_strPidfXml(AString::ConstNull()),
        m_strEtag(AString::ConstNull()),
        m_nCapability(0),
        m_nSimSlot(nSimSlot),
        m_piCoreService(_piCoreService),
        m_strAppName(strAppName),
        m_objExponentialRetryTimeSec(ImsVector<IMS_SINT32>()),
        m_bAoSConnected(IMS_FALSE),
        m_nExtended(1),
        m_bEnablePIDFCompression(IMS_FALSE),
        m_bSetPublishStarted(IMS_FALSE),
        m_bUnpublishSent(IMS_FALSE),
        m_nImmediatelyRetryCount(0),
        m_nRetryCount(0),
        m_nExponentialRetryCount(0)
{
    IMS_TRACE_I("UCE_M : UcePublishManager = %" PFLS_u, sizeof(UcePublishManager), 0, 0);
    if (m_piCoreService == IMS_NULL)
    {
        IMS_TRACE_E(0, "[ERROR]UcePublishManager:ICoreService is NULL", 0, 0, 0);
        // return;
    }
    LoadConfigValue();
    SetState(IDLE);
}

PUBLIC
UcePublishManager::~UcePublishManager()
{
    IMS_TRACE_I("UCE_F : UcePublishManager = %" PFLS_u, sizeof(UcePublishManager), 0, 0);
    StopTimer(TIMER_ALL);
    DestroyPublication();
}
/*-----------------------------------------------------------------------------------/
    Methods
------------------------------------------------------------------------------------*/
PUBLIC
IMS_RESULT UcePublishManager::MessageMediator_AdjustMessage(
        IN_OUT ISipMessage* piSIPMsg, IN IMS_SINT32 nMessage)
{
    IMS_TRACE_D("MessageMediator_AdjustMessage:nMessage [%d]", nMessage, 0, 0);
    if (piSIPMsg == null)
    {
        IMS_TRACE_I("MessageMediator_AdjustMessage:piSIPMsg is null", 0, 0, 0);
        return IMS_SUCCESS;
    }
    if (piSIPMsg->GetMethod().Equals(SipMethod::PUBLISH) == IMS_FALSE)
    {
        IMS_TRACE_I("MessageMediator_AdjustMessage:it is not publish method.", 0, 0, 0);
        return IMS_SUCCESS;
    }
    AString strContactHeader = piSIPMsg->GetHeader(ISipHeader::CONTACT_NORMAL);
    if (UceConfig::GetInstance()->GetBoolValue(
                UceConfig::KEY_USE_CONTACT_HEADER_IN_PUBLISH, m_nSimSlot) == IMS_FALSE)
    {
        IMS_TRACE_I("MessageMediator_AdjustMessage:m_bUseContactHeader is false", 0, 0, 0);
        return IMS_SUCCESS;
    }

    IMS_BOOL bAppendIARITag = IMS_FALSE;
    IMS_BOOL bAppendICSITag = IMS_FALSE;
    IMS_BOOL bNeedToComma = IMS_FALSE;

    AString strContactHeaderValue = IMS_NULL;

    if ((m_nConnectedServices & CONNECTED_SERVICE_CPM_SESSION) != CONNECTED_SERVICE_CPM_SESSION)
    {
        IMS_TRACE_I("MessageMediator_AdjustMessage : CPM is not registered.", 0, 0, 0);
        return IMS_SUCCESS;
    }

    if (strContactHeader.Contains(UceTag::TAG_CHAT) == IMS_FALSE)
    {
        if (strContactHeader.Contains(UceTag::TAG_ICSI) == IMS_FALSE && bAppendICSITag == IMS_FALSE)
        {
            bAppendICSITag = IMS_TRUE;
            strContactHeaderValue.Append(UceTag::TAG_ICSI);
            strContactHeaderValue.Append(TextParser::STR_DQUOTE);
        }
        strContactHeaderValue.Append(UceTag::TAG_CHAT);
        bNeedToComma = IMS_TRUE;
    }
    if (strContactHeader.Contains(UceTag::TAG_CALL_COMPOSER) == IMS_FALSE &&
            (m_nConnectedServices & CONNECTED_SERVICE_CALL_COMPOSER) ==
                    CONNECTED_SERVICE_CALL_COMPOSER)
    {
        if (strContactHeader.Contains(UceTag::TAG_ICSI) == IMS_FALSE && bAppendICSITag == IMS_FALSE)
        {
            bAppendICSITag = IMS_TRUE;
            strContactHeaderValue.Append(UceTag::TAG_ICSI);
            strContactHeaderValue.Append(TextParser::STR_DQUOTE);
        }
        if (bNeedToComma == IMS_TRUE)
        {
            strContactHeaderValue.Append(TextParser::CHAR_COMMA);
        }
        strContactHeaderValue.Append(UceTag::TAG_CALL_COMPOSER);
        bNeedToComma = IMS_TRUE;
    }
    if (strContactHeader.Contains(UceTag::TAG_STANDALONE_PAGER_MESSAGING) == IMS_FALSE &&
            (m_nConnectedServices & CONNECTED_SERVICE_CPM_MSG) == CONNECTED_SERVICE_CPM_MSG)
    {
        if (strContactHeader.Contains(UceTag::TAG_ICSI) == IMS_FALSE && bAppendICSITag == IMS_FALSE)
        {
            bAppendICSITag = IMS_TRUE;
            strContactHeaderValue.Append(UceTag::TAG_ICSI);
            strContactHeaderValue.Append(TextParser::STR_DQUOTE);
        }
        if (bNeedToComma == IMS_TRUE)
        {
            strContactHeaderValue.Append(TextParser::CHAR_COMMA);
        }
        strContactHeaderValue.Append(UceTag::TAG_STANDALONE_PAGER_MESSAGING);
        bNeedToComma = IMS_TRUE;
    }
    if (strContactHeader.Contains(UceTag::TAG_STANDALONE_LARGE_MESSAGING) == IMS_FALSE &&
            (m_nConnectedServices & CONNECTED_SERVICE_CPM_LARGEMSG) ==
                    CONNECTED_SERVICE_CPM_LARGEMSG)
    {
        if (strContactHeader.Contains(UceTag::TAG_ICSI) == IMS_FALSE && bAppendICSITag == IMS_FALSE)
        {
            bAppendICSITag = IMS_TRUE;
            strContactHeaderValue.Append(UceTag::TAG_ICSI);
            strContactHeaderValue.Append(TextParser::STR_DQUOTE);
        }
        if (bNeedToComma == IMS_TRUE)
        {
            strContactHeaderValue.Append(TextParser::CHAR_COMMA);
        }
        strContactHeaderValue.Append(UceTag::TAG_STANDALONE_LARGE_MESSAGING);
    }
    if (bAppendICSITag == IMS_TRUE)
    {
        strContactHeaderValue.Append(TextParser::STR_DQUOTE);
    }
    bNeedToComma = IMS_FALSE;

    if (strContactHeader.Contains(UceTag::TAG_PRESENCE) == IMS_FALSE &&
            (m_nConnectedServices & CONNECTED_SERVICE_PRESENCE) == CONNECTED_SERVICE_PRESENCE)
    {
        if (strContactHeader.Contains(UceTag::TAG_IARI) == IMS_FALSE && bAppendIARITag == IMS_FALSE)
        {
            bAppendIARITag = IMS_TRUE;
            if (bAppendICSITag == IMS_TRUE)
            {
                strContactHeaderValue.Append(TextParser::CHAR_SEMICOLON);
            }
            strContactHeaderValue.Append(UceTag::TAG_IARI);
            strContactHeaderValue.Append(TextParser::STR_DQUOTE);
        }
        strContactHeaderValue.Append(UceTag::TAG_PRESENCE);
        bNeedToComma = IMS_TRUE;
    }
    if (strContactHeader.Contains(UceTag::TAG_FILE_TRANSFER_HTTP) == IMS_FALSE &&
            (m_nConnectedServices & CONNECTED_SERVICE_HTTPFT) == CONNECTED_SERVICE_HTTPFT)
    {
        if (strContactHeader.Contains(UceTag::TAG_IARI) == IMS_FALSE && bAppendIARITag == IMS_FALSE)
        {
            bAppendIARITag = IMS_TRUE;
            if (bAppendICSITag == IMS_TRUE)
            {
                strContactHeaderValue.Append(TextParser::CHAR_SEMICOLON);
            }
            strContactHeaderValue.Append(UceTag::TAG_IARI);
            strContactHeaderValue.Append(TextParser::STR_DQUOTE);
        }
        if (bNeedToComma == IMS_TRUE)
        {
            strContactHeaderValue.Append(TextParser::CHAR_COMMA);
        }
        strContactHeaderValue.Append(UceTag::TAG_FILE_TRANSFER_HTTP);
        bNeedToComma = IMS_TRUE;
    }
    if (strContactHeader.Contains(UceTag::TAG_GEOLOCATION_PUSH) == IMS_FALSE &&
            (m_nConnectedServices & CONNECTED_SERVICE_GEOPUSH) == CONNECTED_SERVICE_GEOPUSH)
    {
        if (strContactHeader.Contains(UceTag::TAG_IARI) == IMS_FALSE && bAppendIARITag == IMS_FALSE)
        {
            bAppendIARITag = IMS_TRUE;
            if (bAppendICSITag == IMS_TRUE)
            {
                strContactHeaderValue.Append(TextParser::CHAR_SEMICOLON);
            }
            strContactHeaderValue.Append(UceTag::TAG_IARI);
            strContactHeaderValue.Append(TextParser::STR_DQUOTE);
        }
        if (bNeedToComma == IMS_TRUE)
        {
            strContactHeaderValue.Append(TextParser::CHAR_COMMA);
        }
        strContactHeaderValue.Append(UceTag::TAG_GEOLOCATION_PUSH);
        bNeedToComma = IMS_TRUE;
    }
    if (strContactHeader.Contains(UceTag::TAG_FT_SMS) == IMS_FALSE &&
            (m_nConnectedServices & CONNECTED_SERVICE_FTSMS) == CONNECTED_SERVICE_FTSMS)
    {
        if (strContactHeader.Contains(UceTag::TAG_IARI) == IMS_FALSE && bAppendIARITag == IMS_FALSE)
        {
            bAppendIARITag = IMS_TRUE;
            if (bAppendICSITag == IMS_TRUE)
            {
                strContactHeaderValue.Append(TextParser::CHAR_SEMICOLON);
            }
            strContactHeaderValue.Append(UceTag::TAG_IARI);
            strContactHeaderValue.Append(TextParser::STR_DQUOTE);
        }
        if (bNeedToComma == IMS_TRUE)
        {
            strContactHeaderValue.Append(TextParser::CHAR_COMMA);
        }
        strContactHeaderValue.Append(UceTag::TAG_FT_SMS);
        bNeedToComma = IMS_TRUE;
    }
    if (strContactHeader.Contains(UceTag::TAG_GEOLOCATIONPUSH_SMS) == IMS_FALSE &&
            (m_nConnectedServices & CONNECTED_SERVICE_GEOSMS) == CONNECTED_SERVICE_GEOSMS)
    {
        if (strContactHeader.Contains(UceTag::TAG_IARI) == IMS_FALSE && bAppendIARITag == IMS_FALSE)
        {
            bAppendIARITag = IMS_TRUE;
            if (bAppendICSITag == IMS_TRUE)
            {
                strContactHeaderValue.Append(TextParser::CHAR_SEMICOLON);
            }
            strContactHeaderValue.Append(UceTag::TAG_IARI);
            strContactHeaderValue.Append(TextParser::STR_DQUOTE);
        }
        if (bNeedToComma == IMS_TRUE)
        {
            strContactHeaderValue.Append(TextParser::CHAR_COMMA);
        }
        strContactHeaderValue.Append(UceTag::TAG_GEOLOCATIONPUSH_SMS);
        bNeedToComma = IMS_TRUE;
    }
    if (strContactHeader.Contains(UceTag::TAG_CHATBOT_SESSION) == IMS_FALSE &&
            (m_nConnectedServices & CONNECTED_SERVICE_CHATBOT) == CONNECTED_SERVICE_CHATBOT)
    {
        if (strContactHeader.Contains(UceTag::TAG_IARI) == IMS_FALSE && bAppendIARITag == IMS_FALSE)
        {
            bAppendIARITag = IMS_TRUE;
            if (bAppendICSITag == IMS_TRUE)
            {
                strContactHeaderValue.Append(TextParser::CHAR_SEMICOLON);
            }
            strContactHeaderValue.Append(UceTag::TAG_IARI);
            strContactHeaderValue.Append(TextParser::STR_DQUOTE);
        }
        if (bNeedToComma == IMS_TRUE)
        {
            strContactHeaderValue.Append(TextParser::CHAR_COMMA);
        }
        strContactHeaderValue.Append(UceTag::TAG_CHATBOT_SESSION);
        bNeedToComma = IMS_TRUE;
    }

    if (strContactHeader.Contains(UceTag::TAG_CHATBOT_STANDALONE_MESSAGE) == IMS_FALSE &&
            (m_nConnectedServices & CONNECTED_SERVICE_CHATBOT_STANDALONE_MSG) ==
                    CONNECTED_SERVICE_CHATBOT_STANDALONE_MSG)
    {
        if (strContactHeader.Contains(UceTag::TAG_IARI) == IMS_FALSE && bAppendIARITag == IMS_FALSE)
        {
            bAppendIARITag = IMS_TRUE;
            if (bAppendICSITag == IMS_TRUE)
            {
                strContactHeaderValue.Append(TextParser::CHAR_SEMICOLON);
            }
            strContactHeaderValue.Append(UceTag::TAG_IARI);
            strContactHeaderValue.Append(TextParser::STR_DQUOTE);
        }
        if (bNeedToComma == IMS_TRUE)
        {
            strContactHeaderValue.Append(TextParser::CHAR_COMMA);
        }
        strContactHeaderValue.Append(UceTag::TAG_CHATBOT_STANDALONE_MESSAGE);
    }
    if (bAppendIARITag == IMS_TRUE)
    {
        strContactHeaderValue.Append(TextParser::STR_DQUOTE);
    }

    if (strContactHeader.Contains(UceTag::TAG_CHATBOT_VERSION_V2) == IMS_FALSE &&
            (m_nConnectedServices & CONNECTED_SERVICE_CHATBOT_V2) == CONNECTED_SERVICE_CHATBOT_V2)
    {
        strContactHeaderValue.Append(TextParser::CHAR_SEMICOLON);
        strContactHeaderValue.Append(UceTag::TAG_CHATBOT_VERSION_V2);
    }
    else if (strContactHeader.Contains(UceTag::TAG_CHATBOT_VERSION_V1) == IMS_FALSE &&
            (m_nConnectedServices & CONNECTED_SERVICE_CHATBOT_V1) == CONNECTED_SERVICE_CHATBOT_V1)
    {
        strContactHeaderValue.Append(TextParser::CHAR_SEMICOLON);
        strContactHeaderValue.Append(UceTag::TAG_CHATBOT_VERSION_V1);
    }

    if (strContactHeader.IsEmpty() == IMS_TRUE)
    {
        piSIPMsg->SetHeader(ISipHeader::CONTACT_NORMAL, strContactHeaderValue);
    }
    else
    {
        strContactHeader = strContactHeader + ";" + strContactHeaderValue;
        piSIPMsg->SetHeader(ISipHeader::CONTACT_NORMAL, strContactHeader);
    }
    return IMS_SUCCESS;
}

IMS_BOOL UcePublishManager::SendPublishRequest(IN IMS_UINT32 key, IN const AString& pidfXml,
        IN const AString& eTag, IN IMS_UINT32 capability, IN IMS_UINT32 extended)
{
    IMS_TRACE_D(
            "SendPublishRequest:key [%d], extended[%d], capability[%d]", key, extended, capability);
    IPublicationData* pPublicationData = new IPublicationData();
    pPublicationData->m_strEtag = eTag;
    pPublicationData->m_nKey = key;
    pPublicationData->m_nCapability = capability;
    pPublicationData->m_strPidfXml = pidfXml;
    pPublicationData->m_nExtended = extended;
    IMSMSG objMsg(PUBLISH_REQUESTED, 0, reinterpret_cast<IMS_UINTP>(pPublicationData));
    return OnStateMessage(objMsg);
}

IMS_BOOL UcePublishManager::AosConnected(IMS_UINT32 conectedService)
{
    LoadConfigValue();
    m_nConnectedServices = conectedService;
    IMSMSG objMsg(AOS_CONNECTED, 0, 0);
    return OnStateMessage(objMsg);
}

IMS_BOOL UcePublishManager::AosDisConnected()
{
    m_nConnectedServices = 0;
    m_bEnablePIDFCompression = IMS_FALSE;
    IMSMSG objMsg(AOS_DISCONNECTED, 0, 0);
    return OnStateMessage(objMsg);
}

IMS_BOOL UcePublishManager::AosDisConnecting()
{
    m_bEnablePIDFCompression = IMS_FALSE;
    IMSMSG objMsg(AOS_DISCONNECTING, 0, 0);
    return OnStateMessage(objMsg);
}

IMS_BOOL UcePublishManager::ClosedService()
{
    IMSMSG objMsg(SERVICE_CLOSED, 0, 0);
    return OnStateMessage(objMsg);
}

void UcePublishManager::UpdateState(IMS_UINT32 _eState)
{
    IMS_TRACE_I("UpdateState - State : [ %s ] -> [ %s ]", StateToString(m_eState),
            StateToString(_eState), 0);
    m_eState = _eState;
    if (m_eState == PUBLISHED || m_eState == REFRESHING)
    {
        SetPublishStateToAoS(PUBLISH_STARTED);
    }
    else
    {
        SetPublishStateToAoS(PUBLISH_STOPPED);
    }
    SetState(_eState);
}

PROTECTED
void UcePublishManager::Timer_TimerExpired(IN ITimer* piTimer)
{
    IMS_TRACE_I("Timer_TimerExpired ", 0, 0, 0);
    if (piTimer == m_pExponentialTimer)
    {
        StopTimer(TIMER_EXPONENTIAL);
        HandleExponentialRetryTimer();
    }
    else if (piTimer == m_pRetryTimer)
    {
        StopTimer(TIMER_RETRY);
        HandleRetryTimer();
    }
    else if (piTimer == m_pRetryAfterTimer)
    {
        StopTimer(TIMER_RETRYAFTER);
        HandleRetryAfterTimer();
    }
}

void UcePublishManager::PublicationDelivered(IN IPublication* piPublication)
{
    IMS_TRACE_I("PublicationDelivered", 0, 0, 0);
    if (piPublication != m_piPublication)
    {
        IMS_TRACE_I("PublicationDelivered:IPublication from Engine is not mine.", 0, 0, 0);
        return;
    }
    IMSMSG objMsg(PUBLISH_SUCCEEDED, 0, 0);
    OnStateMessage(objMsg);
}

void UcePublishManager::PublicationDeliveryFailed(IN IPublication* piPublication)
{
    IMS_TRACE_I("PublicationDeliveryFailed", 0, 0, 0);
    if (piPublication != m_piPublication)
    {
        IMS_TRACE_I("PublicationDeliveryFailed:IPublication from Engine is not mine.", 0, 0, 0);
        return;
    }
    IMSMSG objMsg(PUBLISH_FAILED, 0, 0);
    OnStateMessage(objMsg);
}

void UcePublishManager::PublicationTerminated(IN IPublication* piPublication)
{
    IMS_TRACE_I("PublicationTerminated", 0, 0, 0);
    if (piPublication != m_piPublication)
    {
        IMS_TRACE_I("PublicationTerminated:IPublication from Engine is not mine.", 0, 0, 0);
        return;
    }
    IMSMSG objMsg(PUBLISH_TERMINATED, 0, 0);
    OnStateMessage(objMsg);
}

void UcePublishManager::PublicationRefreshStarted(IN IPublication* piPublication)
{
    IMS_TRACE_I("PublicationRefreshStarted", 0, 0, 0);
    if (piPublication != m_piPublication)
    {
        IMS_TRACE_I("PublicationRefreshStarted:IPublication from Engine is not mine.", 0, 0, 0);
        return;
    }
    IMSMSG objMsg(PUBLISH_REFRESH_STARTED, 0, 0);
    OnStateMessage(objMsg);
}

void UcePublishManager::PublicationRefreshCompleted(IN IPublication* piPublication)
{
    IMS_TRACE_I("PublicationRefreshCompleted", 0, 0, 0);
    if (piPublication != m_piPublication)
    {
        IMS_TRACE_I("PublicationRefreshCompleted:IPublication from Engine is not mine.", 0, 0, 0);
        return;
    }
    IMS_SINT32 nResponseCode = 0;
    const ISipMessage* piSIPMessage = GetISIPMessage(GET_MESSAGE_FROM_RESPONSE);
    if (piSIPMessage != IMS_NULL)
    {
        nResponseCode = piSIPMessage->GetStatusCode();
        IMS_TRACE_D("PublicationRefreshCompleted:StatusCode[%d], reason[%s]", nResponseCode,
                piSIPMessage->GetReasonPhrase().GetStr(), 0);
    }

    if (SipStatusCode::IsFinalSuccess(nResponseCode))
    {
        IMS_TRACE_I("PublicationRefreshCompleted:success refresh PUBLISH.", 0, 0, 0);
        IMSMSG objMsg(PUBLISH_REFRESHED, 0, 0);
        OnStateMessage(objMsg);
    }
    else
    {
        IMS_TRACE_I("PublicationRefreshCompleted:failed refresh PUBLISH.", 0, 0, 0);
        IMSMSG objMsg(PUBLISH_REFRESH_FAILED, 0, 0);
        OnStateMessage(objMsg);
    }
}

void UcePublishManager::Refresh_NotifyCompleted(IN ISipClientConnection* piScc)
{
    if (piScc == IMS_NULL)
    {
        return;
    }
    IMS_SINT32 responseCode = piScc->GetStatusCode();
    IMS_TRACE_D("Refresh_NotifyCompleted:response code[%d]", responseCode, 0, 0);
    // no response
    if (responseCode == 0)
    {
        IMSMSG objMsg(PUBLISH_REFRESH_NO_RESPONSE, 0, 0);
        OnStateMessage(objMsg);
    }
}

void UcePublishManager::Refresh_NotifyTerminated()
{
    IMS_TRACE_D("Refresh_NotifyTerminated", 0, 0, 0);
}

void UcePublishManager::Refresh_NotifyTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh)
{
    IMS_TRACE_D("Refresh_NotifyTimerExpired", 0, 0, 0);
    (void)bDoImplicitRefresh;
}
/*-----------------------------------------------------------------------------------/
    State Machine
------------------------------------------------------------------------------------*/
IMS_BOOL UcePublishManager::StateIDLE_PublishRequested(IN IMSMSG& objMsg)
{
    IMS_TRACE_I("StateIDLE_PublishRequested ", 0, 0, 0);
    IPublicationData* pPublicationData = reinterpret_cast<IPublicationData*>(objMsg.nLparam);
    if (pPublicationData == IMS_NULL)
    {
        IMS_TRACE_I("StateIDLE_PublishRequested - pPublicationData is null", 0, 0, 0);
        return IMS_FALSE;
    }
    SendPublishCommandErrorInd(
            pPublicationData->m_nKey, IUUceService::COMMAND_CODE_GENERIC_FAILURE);
    delete pPublicationData;
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StateIDLE_AoSConnected(IN IMSMSG& objMsg)
{
    (void)objMsg;
    IMS_TRACE_I("StateIDLE_AoSConnected:nConnectedApps [%08x]", m_nConnectedServices, 0, 0);
    m_bAoSConnected = IMS_TRUE;

    m_bEnablePIDFCompression =
            UceConfig::GetInstance()->GetBoolValue(UceConfig::KEY_ENCODE_PUBLISH_BODY, m_nSimSlot);

    UpdateState(ON);
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StateON_PublishRequested(IN IMSMSG& objMsg)
{
    IMS_TRACE_I("StateON_PublishRequested ", 0, 0, 0);
    IPublicationData* pPublicationData = reinterpret_cast<IPublicationData*>(objMsg.nLparam);
    if (pPublicationData == IMS_NULL)
    {
        IMS_TRACE_E(0, "StateON_PublishRequested:IPublicationData is null", 0, 0, 0);
        return IMS_FALSE;
    }
    if (CreatePublication() == IMS_FALSE)
    {
        IMS_TRACE_E(0, "StateON_PublishRequested:create publication failed", 0, 0, 0);
        SendPublishCommandErrorInd(
                pPublicationData->m_nKey, IUUceService::COMMAND_CODE_GENERIC_FAILURE);
        delete pPublicationData;
        return IMS_FALSE;
    }
    StopTimer(TIMER_ALL);
    m_nKey = pPublicationData->m_nKey;
    m_nCapability = pPublicationData->m_nCapability;
    m_strPidfXml = pPublicationData->m_strPidfXml;
    m_strEtag = pPublicationData->m_strEtag;
    m_nExtended = pPublicationData->m_nExtended;

    delete pPublicationData;

    m_nImmediatelyRetryCount = 0;
    m_nRetryCount = 0;
    m_nExponentialRetryCount = 0;

    if (SetPublish(INITIAL) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "StateON_PublishRequested:SetPublish(INITIAL) is failed", 0, 0, 0);
        SendPublishCommandErrorInd(m_nKey, IUUceService::COMMAND_CODE_GENERIC_FAILURE);
        return IMS_FALSE;
    }

    if (Publish() == IMS_TRUE)
    {
        UpdateState(PUBLISHING);
    }
    else
    {
        IMS_TRACE_E(0, "StateON_PublishRequested:Publish() is failed", 0, 0, 0);
        SendPublishCommandErrorInd(m_nKey, IUUceService::COMMAND_CODE_GENERIC_FAILURE);
    }
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StateON_AoSDisConnected(IN IMSMSG& objMsg)
{
    (void)objMsg;
    IMS_TRACE_I("StateON_AoSDisConnected ", 0, 0, 0);
    StopTimer(TIMER_ALL);
    DestroyPublication();
    m_bAoSConnected = IMS_FALSE;
    UpdateState(IDLE);
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StatePUBLISHING_PublishRequested(IN IMSMSG& objMsg)
{
    IMS_TRACE_I("StatePUBLISHING_PublishCmd ", 0, 0, 0);
    IPublicationData* pPublicationData = reinterpret_cast<IPublicationData*>(objMsg.nLparam);
    if (pPublicationData == IMS_NULL)
    {
        IMS_TRACE_I("StatePUBLISHING_PublishCmd - pPublicationData is null", 0, 0, 0);
        return IMS_FALSE;
    }
    SendPublishCommandErrorInd(
            pPublicationData->m_nKey, IUUceService::COMMAND_CODE_GENERIC_FAILURE);
    delete pPublicationData;
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StatePUBLISHING_AoSDisConnecting(IN IMSMSG& objMsg)
{
    (void)objMsg;
    IMS_TRACE_I("StatePUBLISHING_AoSDisConnecting ", 0, 0, 0);
    m_bReceivedUnPublishRequest = IMS_TRUE;
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StatePUBLISHING_AoSDisConnected(IN IMSMSG& objMsg)
{
    (void)objMsg;
    IMS_TRACE_I("StatePUBLISHING_AoSDisConnectedInd ", 0, 0, 0);
    SendPublishCommandErrorInd(m_nKey, IUUceService::COMMAND_CODE_GENERIC_FAILURE);
    m_nKey = 0;
    m_strPidfXml = AString::ConstNull();
    m_strEtag = AString::ConstNull();
    DestroyPublication();
    m_bAoSConnected = IMS_FALSE;
    m_bReceivedUnPublishRequest = IMS_FALSE;
    m_nImmediatelyRetryCount = 0;
    m_nRetryCount = 0;
    m_nExponentialRetryCount = 0;
    UpdateState(IDLE);
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StatePUBLISHING_Published(IN IMSMSG& objMsg)
{
    (void)objMsg;
    IMS_TRACE_I("StatePUBLISHING_Published", 0, 0, 0);
    const ISipMessage* piMessage = GetISIPMessage(GET_MESSAGE_FROM_RESPONSE);
    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_I("StatePUBLISHING_Published:ISipMessage is null", 0, 0, 0);
        SendPublishResponseInd(m_nKey, SipStatusCode::SC_200, "OK", 0, "", "");
    }
    else
    {
        GetEtagAndExpireValue(piMessage);
        const IPublishResponseData* pResponseData = GetPublishResponseData(piMessage);
        SendPublishResponseInd(m_nKey, pResponseData->m_nResponseCode, pResponseData->m_strReason,
                pResponseData->m_nReasonCause, pResponseData->m_strReasonText, m_strEtag);
    }
    m_nKey = 0;
    m_nImmediatelyRetryCount = 0;
    m_nRetryCount = 0;
    m_nExponentialRetryCount = 0;

    if (m_bReceivedUnPublishRequest == IMS_TRUE)
    {
        // case that received aos-disconnecting during publish
        m_bReceivedUnPublishRequest = IMS_FALSE;
        SendUnpublishedInd();
        if (CreatePublication() == IMS_FALSE)
        {
            UpdateState(ON);
            return IMS_TRUE;
        }
        if (Unpublish() == IMS_FALSE)
        {
            DestroyPublication();
            UpdateState(ON);
            return IMS_TRUE;
        }
        UpdateState(TERMINATING);
        return IMS_TRUE;
    }
    UpdateState(PUBLISHED);
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StatePUBLISHING_Failed(IN IMSMSG& objMsg)
{
    (void)objMsg;
    IMS_TRACE_I("StatePUBLISHING_Failed", 0, 0, 0);
    const ISipMessage* piMessage = GetISIPMessage(GET_MESSAGE_FROM_RESPONSE);
    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_I("StatePUBLISHING_Failed:No response case.", 0, 0, 0);
        if (m_nKey != 0)
        {
            SendPublishCommandErrorInd(m_nKey, IUUceService::COMMAND_CODE_GENERIC_FAILURE);
        }
        switch (UceConfig::GetInstance()->GetPublishRetryType(0, m_nSimSlot))
        {
            case UceConfig::IMMEDIATELY:
                if (ProcessImmediatelyRetryResponseScenario())
                {
                    return IMS_TRUE;
                }
                break;
            case UceConfig::RETRY:
                if (ProcessRetryResponseScenario())
                {
                    return IMS_TRUE;
                }
                break;
            case UceConfig::EXPONENTIAL:
                if (ProcessExponentialRetryResponseScenario())
                {
                    return IMS_TRUE;
                }
                break;
            default:
                break;
        }
        StopTimer(TIMER_ALL);
        UpdateState(ON);
        return IMS_TRUE;
    }

    const IPublishResponseData* pResponseData = GetPublishResponseData(piMessage);

    if (m_bEnablePIDFCompression == IMS_TRUE)
    {
        m_bEnablePIDFCompression = IMS_FALSE;
        StopTimer(TIMER_ALL);
        m_strEtag = AString::ConstNull();
        DestroyPublication();
        UpdateState(ON);
        return SendPublishRequest(m_nKey, m_strPidfXml, "", m_nCapability, m_nExtended);
    }

    IMS_BOOL bNeedToRetry = IMS_FALSE;
    if (m_bReceivedUnPublishRequest == IMS_TRUE)
    {
        m_bReceivedUnPublishRequest = IMS_FALSE;
    }
    else
    {
        if (HandleFailResponse(pResponseData->m_nResponseCode) == IMS_TRUE)
        {
            bNeedToRetry = IMS_TRUE;
        }
    }
    SendPublishResponseInd(m_nKey, pResponseData->m_nResponseCode, pResponseData->m_strReason,
            pResponseData->m_nReasonCause, pResponseData->m_strReasonText, m_strEtag, bNeedToRetry);
    m_nKey = 0;
    if (!bNeedToRetry)
    {
        UpdateState(ON);
    }
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StatePUBLISHED_PublishRequested(IN IMSMSG& objMsg)
{
    IMS_TRACE_I("StatePUBLISHED_PublishRequested", 0, 0, 0);
    IPublicationData* pPublicationData = reinterpret_cast<IPublicationData*>(objMsg.nLparam);
    if (pPublicationData == IMS_NULL)
    {
        IMS_TRACE_E(0, "StatePUBLISHED_PublishRequested:IPublicationData is null", 0, 0, 0);
        return IMS_FALSE;
    }
    m_nKey = pPublicationData->m_nKey;
    m_strPidfXml = pPublicationData->m_strPidfXml;
    m_nCapability = pPublicationData->m_nCapability;
    m_nExtended = pPublicationData->m_nExtended;
    if (!pPublicationData->m_strEtag.IsEmpty())
    {
        m_strEtag = pPublicationData->m_strEtag;
    }
    delete pPublicationData;
    if (CreatePublication() == IMS_FALSE)
    {
        IMS_TRACE_E(0, "StatePUBLISHED_PublishRequested:create publication failed", 0, 0, 0);
        SendPublishCommandErrorInd(m_nKey, IUUceService::COMMAND_CODE_GENERIC_FAILURE);
        return IMS_FALSE;
    }

    if (SetPublish(INITIAL) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "StatePUBLISHED_PublishRequested:SetPublish(INITIAL) is failed", 0, 0, 0);
        SendPublishCommandErrorInd(m_nKey, IUUceService::COMMAND_CODE_GENERIC_FAILURE);
        return IMS_FALSE;
    }

    if (Publish() == IMS_TRUE)
    {
        UpdateState(PUBLISHING);
    }
    else
    {
        IMS_TRACE_E(0, "StatePUBLISHED_PublishRequested:Publish() is failed", 0, 0, 0);
        SendPublishCommandErrorInd(m_nKey, IUUceService::COMMAND_CODE_GENERIC_FAILURE);
    }
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StatePUBLISHED_AoSDisconnecting(IN IMSMSG& objMsg)
{
    (void)objMsg;
    IMS_TRACE_I("StatePUBLISHED_AoSDisconnecting ", 0, 0, 0);
    StopTimer(TIMER_ALL);
    SendUnpublishedInd();
    if (CreatePublication() == IMS_TRUE)
    {
        if (Unpublish() == IMS_TRUE)
        {
            UpdateState(TERMINATING);
            return IMS_TRUE;
        }
        IMS_TRACE_E(0, "StatePUBLISHED_AoSDisconnecting:Sending UnPublish Failed.", 0, 0, 0);
    }
    UpdateState(ON);
    DestroyPublication();
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StatePUBLISHED_AoSDisconnected(IN IMSMSG& objMsg)
{
    (void)objMsg;
    IMS_TRACE_I("StatePUBLISHED_AoSDisconnected", 0, 0, 0);
    StopTimer(TIMER_ALL);

    m_bAoSConnected = IMS_FALSE;
    m_bReceivedUnPublishRequest = IMS_FALSE;
    SendUnpublishedInd();
    DestroyPublication();
    UpdateState(IDLE);
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StatePUBLISHED_RefreshStarted(IN IMSMSG& objMsg)
{
    (void)objMsg;
    IMS_TRACE_I("StatePUBLISHED_RefreshStarted ", 0, 0, 0);
    UpdateState(REFRESHING);
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StateREFRESHING_PublishRequested(IN IMSMSG& objMsg)
{
    (void)objMsg;
    IMS_TRACE_I("StateREFRESHING_PublishRequested ", 0, 0, 0);

    IPublicationData* pPublicationData = reinterpret_cast<IPublicationData*>(objMsg.nLparam);
    if (pPublicationData == IMS_NULL)
    {
        IMS_TRACE_I("StateREFRESHING_PublishRequested - pPublicationData is null", 0, 0, 0);
        return IMS_FALSE;
    }
    if (m_bReceivedUnPublishRequest == IMS_TRUE)
    {
        SendPublishCommandErrorInd(
                pPublicationData->m_nKey, IUUceService::COMMAND_CODE_GENERIC_FAILURE);
        return IMS_TRUE;
    }
    m_pPendingPublicationData = new IPublicationData();
    m_pPendingPublicationData->m_nKey = pPublicationData->m_nKey;
    m_pPendingPublicationData->m_nCapability = pPublicationData->m_nCapability;
    m_pPendingPublicationData->m_strEtag = pPublicationData->m_strEtag;
    m_pPendingPublicationData->m_strPidfXml = pPublicationData->m_strPidfXml;
    m_pPendingPublicationData->m_nExtended = pPublicationData->m_nExtended;
    delete pPublicationData;
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StateREFRESHING_AoSDisConnecting(IN IMSMSG& objMsg)
{
    (void)objMsg;
    IMS_TRACE_I("StateREFRESHING_AoSDisConnecting ", 0, 0, 0);
    StopTimer(TIMER_ALL);
    ClearPendingPublishRequest();
    m_bReceivedUnPublishRequest = IMS_TRUE;
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StateREFRESHING_AoSDisConnected(IN IMSMSG& objMsg)
{
    (void)objMsg;
    IMS_TRACE_I("StateREFRESHING_AoSDisConnected ", 0, 0, 0);

    StopTimer(TIMER_ALL);

    m_bAoSConnected = IMS_FALSE;
    m_bReceivedUnPublishRequest = IMS_FALSE;
    ClearPendingPublishRequest();
    SendUnpublishedInd();
    DestroyPublication();
    UpdateState(IDLE);
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StateREFRESHING_Refreshed(IN IMSMSG& objMsg)
{
    (void)objMsg;
    IMS_TRACE_I("StateREFRESHING_Refreshed ", 0, 0, 0);

    StopTimer(TIMER_ALL);

    if (m_bReceivedUnPublishRequest == IMS_TRUE)
    {
        // case that received aos-disconnecting during publish
        m_bReceivedUnPublishRequest = IMS_FALSE;
        ClearPendingPublishRequest();
        SendUnpublishedInd();
        if (CreatePublication() == IMS_FALSE)
        {
            UpdateState(ON);
            return IMS_TRUE;
        }
        if (Unpublish() == IMS_FALSE)
        {
            DestroyPublication();
            UpdateState(ON);
            return IMS_TRUE;
        }
        UpdateState(TERMINATING);
        return IMS_TRUE;
    }
    if (m_pPendingPublicationData != IMS_NULL)
    {
        UpdateState(ON);
        SendPendingPublishRequest();
    }
    else
    {
        UpdateState(PUBLISHED);
    }
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StateREFRESHING_RefreshFailed(IN IMSMSG& objMsg)
{
    (void)objMsg;
    IMS_TRACE_I("StateREFRESHING_RefreshFailed", 0, 0, 0);

    StopTimer(TIMER_ALL);

    const ISipMessage* piMessage = GetISIPMessage(GET_MESSAGE_FROM_RESPONSE);
    IMS_SINT32 nResponseCode = 0;
    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_I("StateREFRESHING_RefreshFailed:ISipMessage is null", 0, 0, 0);
    }
    else
    {
        const IPublishResponseData* pResponseData = GetPublishResponseData(piMessage);
        nResponseCode = pResponseData->m_nResponseCode;
        SendPublishResponseInd(m_nKey, pResponseData->m_nResponseCode, pResponseData->m_strReason,
                pResponseData->m_nReasonCause, pResponseData->m_strReasonText, m_strEtag);
    }

    if (m_bReceivedUnPublishRequest == IMS_TRUE)
    {
        // case that received aos-disconnecting during publish
        m_bReceivedUnPublishRequest = IMS_FALSE;
        ClearPendingPublishRequest();
        SendUnpublishedInd();
        if (CreatePublication() == IMS_FALSE)
        {
            UpdateState(ON);
            return IMS_TRUE;
        }
        if (Unpublish() == IMS_FALSE)
        {
            DestroyPublication();
            UpdateState(ON);
            return IMS_TRUE;
        }
        UpdateState(TERMINATING);
        return IMS_TRUE;
    }

    if (HandleFailResponse(nResponseCode) == IMS_TRUE)
    {
        UpdateState(PUBLISHING);
        return IMS_TRUE;
    }
    if (m_pPendingPublicationData != IMS_NULL)
    {
        SendPendingPublishRequest();
    }
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StateREFRESHING_RefreshFailedWithNoResponse(IN IMSMSG& objMsg)
{
    (void)objMsg;
    IMS_TRACE_I("StateREFRESHING_RefreshFailedWithNoResponse", 0, 0, 0);

    StopTimer(TIMER_ALL);
    IMS_SINT32 nResponseCode = 0;

    if (m_bReceivedUnPublishRequest == IMS_TRUE)
    {
        // case that received aos-disconnecting during publish
        m_bReceivedUnPublishRequest = IMS_FALSE;
        ClearPendingPublishRequest();
        SendUnpublishedInd();
        if (CreatePublication() == IMS_FALSE)
        {
            UpdateState(ON);
            return IMS_TRUE;
        }
        if (Unpublish() == IMS_FALSE)
        {
            DestroyPublication();
            UpdateState(ON);
            return IMS_TRUE;
        }
        UpdateState(TERMINATING);
        return IMS_TRUE;
    }

    switch (UceConfig::GetInstance()->GetPublishRetryType(nResponseCode, m_nSimSlot))
    {
        case UceConfig::IMMEDIATELY:
            if (ProcessImmediatelyRetryResponseScenario())
            {
                return IMS_TRUE;
            }
            break;
        case UceConfig::RETRY:
            if (ProcessRetryResponseScenario())
            {
                return IMS_TRUE;
            }
            break;
        case UceConfig::EXPONENTIAL:
            if (ProcessExponentialRetryResponseScenario())
            {
                return IMS_TRUE;
            }
            break;
        default:
            break;
    }
    StopTimer(TIMER_ALL);
    SendPublishResponseInd(m_nKey, SipStatusCode::SC_504, "Server Time-out", 0, "", "");
    UpdateState(ON);
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StateTERMINATING_PublishRequested(IN IMSMSG& objMsg)
{
    IMS_TRACE_I("StateTERMINATING_PublishRequested", 0, 0, 0);
    IPublicationData* pPublicationData = reinterpret_cast<IPublicationData*>(objMsg.nLparam);
    if (pPublicationData == IMS_NULL)
    {
        IMS_TRACE_I("StateTERMINATING_PublishRequested - pPublicationData is null", 0, 0, 0);
        return IMS_FALSE;
    }
    SendPublishCommandErrorInd(
            pPublicationData->m_nKey, IUUceService::COMMAND_CODE_GENERIC_FAILURE);
    delete pPublicationData;
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StateTERMINATING_AoSDisconnected(IN IMSMSG& objMsg)
{
    (void)objMsg;
    IMS_TRACE_I("StateTERMINATING_AoSDisconnected ", 0, 0, 0);
    DestroyPublication();
    UpdateState(IDLE);
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StateTERMINATING_Published(IN IMSMSG& objMsg)
{
    (void)objMsg;
    IMS_TRACE_I("StateTERMINATING_Published ", 0, 0, 0);
    DestroyPublication();
    UpdateState(ON);
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StateTERMINATING_Failed(IN IMSMSG& objMsg)
{
    (void)objMsg;
    IMS_TRACE_I("StateTERMINATING_Failed ", 0, 0, 0);
    DestroyPublication();
    UpdateState(ON);
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::StateALL_Terminated(IN IMSMSG& objMsg)
{
    (void)objMsg;
    IMS_TRACE_I("StateALL_Terminated:current State[%s]", StateToString(m_eState), 0, 0);
    DestroyPublication();
    switch (GetState())
    {
        case PUBLISHING:
            SendPublishCommandErrorInd(m_nKey, IUUceService::COMMAND_CODE_GENERIC_FAILURE);
            StopTimer(TIMER_ALL);
            break;
        case REFRESHING:
        case TERMINATING:  // FALL-THROUGH
            SendPublishResponseInd(m_nKey, SipStatusCode::SC_504, "Server Time-out", 0, "", "");
            StopTimer(TIMER_ALL);
            break;
        default:
            break;
    }
    ClearPendingPublishRequest();
    UpdateState(ON);
    return IMS_TRUE;
}

PRIVATE
void UcePublishManager::LoadConfigValue()
{
    IMS_TRACE_I("LoadConfigValue : read again config", 0, 0, 0);
    m_objExponentialRetryTimeSec =
            UceConfig::GetInstance()->GetExponentialRetryPublishRespTimeArray(m_nSimSlot);
}

IPublishResponseData* UcePublishManager::GetPublishResponseData(const ISipMessage* piMessage)
{
    IMS_SINT32 nReasonCause = -1;
    AString strReasonText = "";

    IPublishResponseData* pPublishResponseData = new IPublishResponseData();

    IMS_TRACE_D("GetPublishResponseData:StatusCode[%d], reason[%s]", piMessage->GetStatusCode(),
            piMessage->GetReasonPhrase().GetStr(), 0);
    pPublishResponseData->m_nResponseCode = piMessage->GetStatusCode();
    pPublishResponseData->m_strReason = piMessage->GetReasonPhrase();

    ImsList<AString> objReasonHeaders = piMessage->GetHeaders(ISipHeader::REASON);
    if (objReasonHeaders.IsEmpty())
    {
        return pPublishResponseData;
    }
    AString strReasonHdr = SipParsingHelper::GetSipReasonHeader(objReasonHeaders);
    SipParsingHelper::ParseReasonHeader(strReasonHdr, nReasonCause, strReasonText);
    IMS_TRACE_D(
            "GetPublishResponseData:cause[%d], text[%s]", nReasonCause, strReasonText.GetStr(), 0);

    pPublishResponseData->m_nReasonCause = nReasonCause;
    pPublishResponseData->m_strReasonText = strReasonText;

    return pPublishResponseData;
}

ISipMessage* UcePublishManager::GetISIPMessage(IMS_BOOL bRequireResponseMessage)
{
    if (m_piPublication != IMS_NULL)
    {
        const IMessage* piMessage = IMS_NULL;
        if (bRequireResponseMessage == GET_MESSAGE_FROM_RESPONSE)
        {
            piMessage = m_piPublication->GetPreviousResponse(IMessage::PUBLICATION_PUBLISH);
        }
        else
        {  // GET_MESSAGE_FOR_REQUEST
            piMessage = m_piPublication->GetNextRequest();
        }
        if (piMessage == IMS_NULL)
        {
            IMS_TRACE_I("GetISIPMessage:Getting a Message failed", 0, 0, 0);
            return IMS_NULL;
        }
        /* get sip message */
        return piMessage->GetMessage();
    }
    IMS_TRACE_I("GetISIPMessage:m_piPublication is null", 0, 0, 0);
    return IMS_NULL;
}

void UcePublishManager::SendPublishCommandErrorInd(IMS_UINT32 nKey, IMS_UINT32 nCommandError)
{
    IMS_TRACE_I("SendPublishCommandErrorInd:key[%d], error[%d]", nKey, nCommandError, 0);
    IUceJniThread* piJniThread = GetJniThread();
    if (piJniThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "SendPublishCommandErrorInd:piJniThread is null", 0, 0, 0);
        m_nKey = 0;
        return;
    }
    piJniThread->PublishErrorInd(nKey, nCommandError);
    m_nKey = 0;
}

void UcePublishManager::SendPublishResponseInd(IMS_UINT32 nKey, IMS_SINT32 nResponseCode,
        AString strReason, IMS_SINT32 nReasonHeaderCause, AString strReasonHeaderText,
        const AString& eTag, IMS_BOOL bNeedToRetry)
{
    IMS_TRACE_I("SendPublishResponseInd:key[%d], responseCode[%d]", nKey, nResponseCode, 0);
    IMS_TRACE_I("SendPublishResponseInd:reason[%s], Cause[%d], Text[%s]", strReason.GetStr(),
            nReasonHeaderCause, strReasonHeaderText.GetStr());
    m_bUnpublishSent = IMS_FALSE;
    IUceJniThread* piJniThread = GetJniThread();
    if (piJniThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "SendPublishResponseInd:piJniThread is null", 0, 0, 0);
        m_nKey = 0;
        return;
    }

    if (nKey == 0)
    {
        if (nResponseCode != SipStatusCode::SC_200)
        {
            m_nCapability = 0;
        }
        piJniThread->PublishUpdatedInd(m_nCapability, nResponseCode, strReason, nReasonHeaderCause,
                strReasonHeaderText, eTag, 0);
    }
    else
    {
        if (nResponseCode != SipStatusCode::SC_200)
        {
            m_nCapability = 0;
        }
        IMS_UINT32 needToRetry = bNeedToRetry ? 1 : 0;
        piJniThread->PublishResponseInd(nKey, nResponseCode, m_nCapability, strReason,
                nReasonHeaderCause, strReasonHeaderText, eTag, needToRetry);
    }
    m_nKey = 0;
}
void UcePublishManager::SendUnpublishedInd()
{
    IUceJniThread* piJniThread = GetJniThread();
    if (piJniThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "SendUnpublishedInd:piJniThread is null", 0, 0, 0);
        return;
    }
    if (m_bUnpublishSent == IMS_FALSE)
    {
        m_bUnpublishSent = IMS_TRUE;
        IMS_TRACE_I("SendUnpublishedInd", 0, 0, 0);
        piJniThread->UnPublishedInd();
    }
}

IMS_BOOL UcePublishManager::CreatePublication()
{
    if (m_piCoreService == IMS_NULL)
    {
        IMS_TRACE_I("CreatePublication:m_piCoreService is null", 0, 0, 0);
        return IMS_FALSE;
    }
    if (m_piPublication != IMS_NULL)
    {
        return IMS_TRUE;
    }

    m_piPublication = m_piCoreService->CreatePublication(
            AString::ConstNull(), AString::ConstNull(), UceNamespace::PRESENCE);
    if (m_piPublication == IMS_NULL)
    {
        IMS_TRACE_I("CreatePublication:Publication create failed", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_I("CreatePublication:Publication is created", 0, 0, 0);
    m_piPublication->SetListener(this);
    m_piPublication->SetRefreshListener(this);
    if (UceConfig::GetInstance()->GetBoolValue(
                UceConfig::KEY_USE_CONTACT_HEADER_IN_PUBLISH, m_nSimSlot) == IMS_TRUE)
    {
        m_piPublication->SetMessageMediator(this);
    }
    return IMS_TRUE;
}

void UcePublishManager::DestroyPublication()
{
    if (m_piPublication != IMS_NULL)
    {
        m_piPublication->Destroy();
        m_piPublication = IMS_NULL;
        return;
    }
}

IMS_BOOL UcePublishManager::SetPublish(IN IMS_BOOL bIsRefresh, AString strMinExpiryValue)
{
    IMS_TRACE_I("SetPublish:value[%d]-(0)initial, (1)refresh, MinExpiryValue[%s]", bIsRefresh,
            strMinExpiryValue.GetStr(), 0);
    // set header
    ISipMessage* piMessage = GetISIPMessage(GET_MESSAGE_FOR_REQUEST);
    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "InitPublish:ISipMessage is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_strEtag != IMS_NULL && m_strEtag.IsEmpty() == IMS_FALSE)
    {
        piMessage->AddHeader(ISipHeader::SIP_IF_MATCH, m_strEtag);
        IMS_TRACE_I("InitPublish:add SIP IF MATCH header with [%s]", m_strEtag.GetStr(), 0, 0);
    }
    SetRefreshPolicy(piMessage, strMinExpiryValue);
    // check ATT/TMUS live network and equipment test
    piMessage->AddHeader(ISipHeader::ACCEPT_CONTACT, "*;+g.oma.sip-im;explicit;require");
    if (bIsRefresh == INITIAL)
    {
        return SetPidfXmlBody(piMessage);
    }
    return IMS_TRUE;
}

void UcePublishManager::SetRefreshPolicy(ISipMessage* piMessage, AString strMinExpiryValue)
{
    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "SetRefreshPolicy:ISipMessage is NULL", 0, 0, 0);
        return;
    }
    AString strExpireValue;
    if (m_nExtended == 0)
    {
        strExpireValue.SetNumber(UceConfig::GetInstance()->GetIntValue(
                UceConfig::KEY_EXTENDED_EXPIRE_VALUE_PUBLISH, m_nSimSlot));
    }
    else
    {
        strExpireValue.SetNumber(UceConfig::GetInstance()->GetIntValue(
                UceConfig::KEY_EXPIRE_VALUE_PUBLISH, m_nSimSlot));
    }

    if (strMinExpiryValue.IsEmpty() != IMS_TRUE)
    {
        piMessage->AddHeader(ISipHeader::EXPIRES_ANY, strMinExpiryValue, AString::ConstNull());
        IMS_TRACE_I("Min Expire value [ %s ]", strMinExpiryValue.GetStr(), 0, 0);
    }
    else
    {
        if (!strExpireValue.EqualsIgnoreCase("0"))
        {
            piMessage->AddHeader(ISipHeader::EXPIRES_ANY, strExpireValue, AString::ConstNull());
            IMS_TRACE_I("Expire value [ %s ]", strExpireValue.GetStr(), 0, 0);
        }
        else
        {
            strExpireValue = "3600";
        }
    }

    IMS_UINT32 nRefreshRatio =
            UceConfig::GetInstance()->GetIntValue(UceConfig::KEY_PUBLISH_REFRESH_RATIO, m_nSimSlot);
    if (nRefreshRatio == 0)
    {
        IMS_TRACE_I("refresh PUBLISH spec ", 0, 0, 0);
    }
    else
    {
        m_piPublication->SetRefreshPolicy(IPublication::REFRESH_POLICY_RATIO,
                strExpireValue.ToInt32(), nRefreshRatio, nRefreshRatio);
    }
}

IMS_BOOL UcePublishManager::Publish()
{
    if (m_piPublication == IMS_NULL)
    {
        IMS_TRACE_I("Publish:m_piPublication is null.", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_piPublication->Publish(ByteArray::ConstNull(), AString::ConstNull()) != IMS_SUCCESS)
    {
        DestroyPublication();
        IMS_TRACE_I("Publish:sending Publish is failed.", 0, 0, 0);
        return IMS_FALSE;
    }
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::Unpublish()
{
    ISipMessage* piSIPMessage = GetISIPMessage(GET_MESSAGE_FOR_REQUEST);
    if (IMS_NULL == piSIPMessage)
    {
        DestroyPublication();
        IMS_TRACE_I("Unpublish:Getting a ISipMessage failed", 0, 0, 0);
        return IMS_FALSE;
    }
    /* add header */
    piSIPMessage->AddHeader(ISipHeader::EXPIRES_ANY, "0");

    if (m_piPublication->Unpublish() != IMS_SUCCESS)
    {
        DestroyPublication();
        IMS_TRACE_I("Unpublish:Sending a Unpublish failed", 0, 0, 0);
        return IMS_FALSE;
    }
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::SetPidfXmlBody(ISipMessage* piMessage)
{
    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "[ERROR]SetPidfXmlBody:ISipMessage is null.", 0, 0, 0);
        return IMS_FALSE;
    }

    ByteArray objContent;
    ISipMessageBodyPart* piBodyPart = piMessage->CreateBodyPart();
    piMessage->AddHeader(ISipHeader::CONTENT_TYPE, "application/pidf+xml");

    if (m_strPidfXml == IMS_NULL || m_strPidfXml.IsEmpty())
    {
        IMS_TRACE_E(0, "[ERROR]SetPidfXmlBody:m_strPidfXml is null or empty", 0, 0, 0);
        return IMS_FALSE;
    }
    const IMS_BYTE* pszXML = reinterpret_cast<const IMS_BYTE*>(m_strPidfXml.GetStr());
    objContent.Attach(pszXML, m_strPidfXml.GetLength());

    // TMO TRD 2020 4Q - GID-MTRREQ-480586
    if (m_bEnablePIDFCompression == IMS_TRUE)
    {
        ByteArray objCompressedContent;
        if (IMS_UTIL_ZLIB_Compress(objContent, objCompressedContent))
        {
            piBodyPart->SetContent(objCompressedContent);
            piMessage->AddHeader(ISipHeader::CONTENT_ENCODING, "gzip");
        }
        else
        {
            return IMS_FALSE;
        }
    }
    else
    {
        piBodyPart->SetContent(objContent);
    }
    return IMS_TRUE;
}

void UcePublishManager::GetEtagAndExpireValue(const ISipMessage* piMessage)
{
    m_strEtag = AString::ConstEmpty();
    if (piMessage == IMS_NULL)
    {
        return;
    }
    AString szETagHeader = piMessage->GetHeader(ISipHeader::SIP_ETAG);
    if (szETagHeader.IsNULL() != IMS_TRUE)
    {
        ISipHeader* piHeader = SipParsingHelper::CreateHeader(ISipHeader::SIP_ETAG, szETagHeader);
        if (piHeader != IMS_NULL)
        {
            m_strEtag = piHeader->GetValue();
            piHeader->Destroy();
        }
    }
}

IMS_BOOL UcePublishManager::HandleFailResponse(IMS_SINT32 nResponseCode)
{
    IMS_TRACE_I("HandleFailResponse:response Code [%d]", nResponseCode, 0, 0);
    switch (nResponseCode)
    {
        case SipStatusCode::SC_404:  // FALL-THROUGH //rfc3261 20.33 Retry-After
        case SipStatusCode::SC_413:  // FALL-THROUGH
        case SipStatusCode::SC_480:  // FALL-THROUGH
        case SipStatusCode::SC_486:  // FALL-THROUGH
        case SipStatusCode::SC_500:  // FALL-THROUGH
        case SipStatusCode::SC_503:  // FALL-THROUGH
        case SipStatusCode::SC_600:  // FALL-THROUGH
        case SipStatusCode::SC_603:  // FALL-THROUGH
        {
            if (ProcessRetryAfterHeader() == IMS_TRUE)
            {
                return IMS_TRUE;
            }
        }
        break;
        case SipStatusCode::SC_412:  // conditianal request failed - initial full
                                     // publish request
        {
            UpdateState(ON);
            if (Process412Scenario() == IMS_TRUE)
            {
                return IMS_TRUE;
            }
        }
        break;
        case SipStatusCode::SC_423:
        {
            // Interval Too Short :re attempt changing the expiration interval in the
            // expires header
            IMS_BOOL bIsRefresh = INITIAL;
            if (GetState() == REFRESHING)
            {
                bIsRefresh = REFRESH;
            }
            UpdateState(ON);
            if (Process423Scenario(bIsRefresh) == IMS_TRUE)
            {
                return IMS_TRUE;
            }
        }
        break;
        default:
            break;
    }
    if (UceConfig::GetInstance()->IsImsRegistrationRequired(IMS_TRUE, nResponseCode, m_nSimSlot))
    {
        UpdateState(ON);
        Process403Scenario();
        DestroyPublication();
        return IMS_FALSE;
    }

    switch (UceConfig::GetInstance()->GetPublishRetryType(nResponseCode, m_nSimSlot))
    {
        case UceConfig::IMMEDIATELY:
            return ProcessImmediatelyRetryResponseScenario();
        case UceConfig::RETRY:
            return ProcessRetryResponseScenario();
        case UceConfig::EXPONENTIAL:
            return ProcessExponentialRetryResponseScenario();
        default:
            break;
    }
    return IMS_FALSE;
}

void UcePublishManager::SetPublishStateToAoS(IN IMS_UINT32 nState)
{
    if (nState == PUBLISH_STARTED && m_bSetPublishStarted == IMS_FALSE)
    {
        IMSMSG objMsg(AosAppRequest::COMMAND_SET_PUBLISH_STARTED, 0, 0);
        MessageService::PostMessage(m_strAppName, objMsg);
        m_bSetPublishStarted = IMS_TRUE;
        IMS_TRACE_D("SetPublishStateToAoS:send Publish Started Msg to UceApp", 0, 0, 0);
    }
    else if (nState == PUBLISH_STOPPED && m_bSetPublishStarted == IMS_TRUE)
    {
        IMSMSG objMsg(AosAppRequest::COMMAND_SET_PUBLISH_TERMINATED, 0, 0);
        MessageService::PostMessage(m_strAppName, objMsg);
        m_bSetPublishStarted = IMS_FALSE;
        IMS_TRACE_D("SetPublishStateToAoS:send Publish Terminated Msg to UceApp", 0, 0, 0);
    }
}

IMS_BOOL UcePublishManager::ProcessRetryAfterHeader()
{
    if (m_piPublication == IMS_NULL)
    {
        IMS_TRACE_I("ProcessRetryAfterHeader:m_piPublication is null.", 0, 0, 0);
        return IMS_FALSE;
    }
    const ISipMessage* piMessage = GetISIPMessage(GET_MESSAGE_FROM_RESPONSE);
    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_I("ProcessRetryAfterHeader:ISipMessage is null.", 0, 0, 0);
        return IMS_FALSE;
    }
    AString strHeader = piMessage->GetHeader(ISipHeader::RETRY_AFTER_SEC);
    if (strHeader.GetLength() <= 0)
    {
        IMS_TRACE_I("ProcessRetryAfterHeader:RETRY_AFTER_SEC value less than 0.", 0, 0, 0);
        return IMS_FALSE;
    }
    ISipHeader* piHeader = SipParsingHelper::CreateHeader(ISipHeader::RETRY_AFTER_SEC, strHeader);
    if (piHeader == IMS_NULL)
    {
        IMS_TRACE_I("ProcessRetryAfterHeader:ISipHeader is null.", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_SINT32 nValue = piHeader->GetValueInt();
    piHeader->Destroy();
    IMS_TRACE_I("ProcessRetryAfterHeader:Retry After Header value is [%d]", nValue, 0, 0);
    if (nValue <= 0)
    {
        return IMS_FALSE;
    }
    return StartTimer(TIMER_RETRYAFTER, nValue);
}

IMS_BOOL UcePublishManager::Process403Scenario()
{
#define NOT_AUTHORIZED_FOR_PRESENCE "NOT AUTHORIZED FOR PRESENCE"

    IMS_TRACE_I("Process403Scenario", 0, 0, 0);
    StopTimer(TIMER_ALL);
    m_strEtag = AString::ConstNull();

    const ISipMessage* piMessage = GetISIPMessage(GET_MESSAGE_FROM_RESPONSE);
    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_I("Process403Scenario: ISipMessage is null", 0, 0, 0);
        return IMS_FALSE;
    }
    AString strReasonPhrs = piMessage->GetReasonPhrase().MakeUpper();
    IMS_TRACE_D("REASON PHRASE = [%s]", strReasonPhrs.GetStr(), 0, 0);

    if (strReasonPhrs.Contains(NOT_AUTHORIZED_FOR_PRESENCE) == IMS_TRUE)
    {
        IMS_TRACE_I("ReasonPhrase:Not Authorized for presence (ReasonPhrase)", 0, 0, 0);
        return IMS_TRUE;
    }

    ImsList<AString> objReasonList = piMessage->GetHeaders(ISipHeader::REASON);
    if (objReasonList.IsEmpty() == IMS_TRUE)
    {
        IMS_TRACE_D("No Reason header present.Send Register Recovery message to the App", 0, 0, 0);
        IMSMSG objMsg(
                AosAppRequest::COMMAND_REGISTER_RECOVERY, 0, ImsAosControl::REGISTER_REINITIATE);
        MessageService::PostMessage(m_strAppName, objMsg);
        return IMS_TRUE;
    }
    for (IMS_UINT32 i = 0; i < objReasonList.GetSize(); i++)
    {
        AString strReason = objReasonList.GetAt(i).MakeUpper();
        IMS_TRACE_D("REASON header = %d:[%s]", i, strReason.GetStr(), 0);
        if (strReason.Contains(NOT_AUTHORIZED_FOR_PRESENCE) == IMS_TRUE)
        {
            IMS_TRACE_I("ReasonPhrase:Not Authorized for presence(ReasonPhrase)", 0, 0, 0);
            return IMS_TRUE;
        }
    }
    IMSMSG objMsg(AosAppRequest::COMMAND_REGISTER_RECOVERY, 0, ImsAosControl::REGISTER_REINITIATE);
    MessageService::PostMessage(m_strAppName, objMsg);
    IMS_TRACE_D("Send Register Recovery message to the App", 0, 0, 0);
    return IMS_TRUE;
}

IMS_BOOL UcePublishManager::Process412Scenario()
{
    IMS_TRACE_I("Process412Scenario", 0, 0, 0);
    StopTimer(TIMER_ALL);
    m_strEtag = AString::ConstNull();
    DestroyPublication();
    return SendPublishRequest(0, m_strPidfXml, "", m_nCapability, m_nExtended);
}

IMS_BOOL UcePublishManager::Process423Scenario(IMS_BOOL bIsRefresh)
{
    IMS_TRACE_I("Process423Scenario", 0, 0, 0);
    StopTimer(TIMER_ALL);

    const ISipMessage* piMessage = GetISIPMessage(GET_MESSAGE_FROM_RESPONSE);
    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_I("Process423Scenario: ISipMessage is null", 0, 0, 0);
        return IMS_FALSE;
    }
    AString szMinExpire = piMessage->GetHeader(ISipHeader::MIN_EXPIRES);
    if (szMinExpire.GetLength() == 0)
    {
        IMS_TRACE_I("Process423Scenario:NO Min Expire header", 0, 0, 0);
        return IMS_FALSE;
    }
    ISipHeader* piHeader = SipParsingHelper::CreateHeader(ISipHeader::MIN_EXPIRES, szMinExpire);
    if (piHeader == IMS_NULL)
    {
        IMS_TRACE_I("Process423Scenario:ISipHeader is null", 0, 0, 0);
        return IMS_FALSE;
    }
    szMinExpire = piHeader->GetValue();
    piHeader->Destroy();
    IMS_TRACE_I("Process423Scenario:min Expire Value [%s]", szMinExpire.GetStr(), 0, 0);
    DestroyPublication();
    return RetryPublish(bIsRefresh, szMinExpire);
}

IMS_BOOL UcePublishManager::ProcessImmediatelyRetryResponseScenario()
{
    if (m_nImmediatelyRetryCount >=
            UceConfig::GetInstance()->GetIntValue(
                    UceConfig::KEY_IMMEDIATELY_RETRY_PUBLISH_RESPONSE_MAX_COUNT, m_nSimSlot))
    {
        IMS_TRACE_I("ProcessImmediatelyRetryResponseScenario:over max "
                    "ImmediatelyRetryMaxCount",
                0, 0, 0);
        UpdateState(ON);
        return IMS_FALSE;
    }
    if (GetState() == PUBLISHING)
    {
        if (RetryPublish(INITIAL) == IMS_TRUE)
        {
            m_nImmediatelyRetryCount++;
            return IMS_TRUE;
        }
    }
    else if (GetState() == REFRESHING)
    {
        if (RetryPublish(REFRESH) == IMS_TRUE)
        {
            m_nImmediatelyRetryCount++;
            return IMS_TRUE;
        }
    }
    UpdateState(ON);
    return IMS_FALSE;
}

IMS_BOOL UcePublishManager::ProcessRetryResponseScenario()
{
    if (m_nRetryCount >= UceConfig::GetInstance()->GetIntValue(
                                 UceConfig::KEY_RETRY_PUBLISH_RESPONSE_MAX_COUNT, m_nSimSlot))
    {
        IMS_TRACE_I("ProcessRetryResponseScenario:over max RetryMaxCount", 0, 0, 0);
        UpdateState(ON);
        return IMS_FALSE;
    }
    if (StartTimer(TIMER_RETRY,
                UceConfig::GetInstance()->GetIntValue(
                        UceConfig::KEY_RETRY_PUBLISH_RESPONSE_TIME_SEC, m_nSimSlot)))
    {
        m_nRetryCount++;
        return IMS_TRUE;
    }
    UpdateState(ON);
    return IMS_FALSE;
}

IMS_BOOL UcePublishManager::ProcessExponentialRetryResponseScenario()
{
    IMS_UINT32 nExponentialRetryMaxCount = UceConfig::GetInstance()->GetIntValue(
            UceConfig::KEY_VARIABLE_RETRY_PUBLISH_RESPONSE_MAX_COUNT, m_nSimSlot);
    if (nExponentialRetryMaxCount != 0)
    {
        if (m_nExponentialRetryCount >= nExponentialRetryMaxCount)
        {
            IMS_TRACE_I("ProcessExponentialRetryResponseScenario:over max MaxCount", 0, 0, 0);
            UpdateState(ON);
            return IMS_FALSE;
        }
    }
    else
    {
        if (m_nExponentialRetryCount >= m_objExponentialRetryTimeSec.GetSize())
        {
            m_nExponentialRetryCount = m_objExponentialRetryTimeSec.GetSize() - 1;
        }
    }

    IMS_UINT32 nExponentialRetryTimeSec =
            m_objExponentialRetryTimeSec.GetAt(m_nExponentialRetryCount);
    if (StartTimer(TIMER_EXPONENTIAL, nExponentialRetryTimeSec))
    {
        DestroyPublication();
        m_nExponentialRetryCount++;
        if (GetState() == PUBLISHING || GetState() == REFRESHING)
        {
            if (GetState() == REFRESHING)
            {
                IMS_TRACE_D(
                        "ProcessExponentialRetryResponseScenario:reset m_nExponentialRetryCount", 0,
                        0, 0);
                m_nExponentialRetryCount = 0;
            }
            // Set state to ON to invoke the initial PUBLISH.
            UpdateState(ON);
        }
        return IMS_TRUE;
    }
    UpdateState(ON);
    return IMS_FALSE;
}

IMS_BOOL UcePublishManager::StartTimer(INTERNAL_TIMER eTimer, IMS_UINT32 nTime)
{
    switch (eTimer)
    {
        case TIMER_EXPONENTIAL:
        {
            StopTimer(TIMER_EXPONENTIAL);
            m_pExponentialTimer = TimerService::GetTimerService()->CreateTimer();
            if (m_pExponentialTimer == IMS_NULL)
            {
                IMS_TRACE_I("StartTimer(exponential):CreateTimer failed", 0, 0, 0);
                return IMS_FALSE;
            }
            IMS_TRACE_I("StartTimer(exponential):duration[%d]", nTime, 0, 0);
            m_pExponentialTimer->SetTimer(nTime * 1000, this);
            return IMS_TRUE;
        }
        case TIMER_RETRY:
        {
            StopTimer(TIMER_RETRY);
            m_pRetryTimer = TimerService::GetTimerService()->CreateTimer();
            if (m_pRetryTimer == IMS_NULL)
            {
                IMS_TRACE_I("StartTimer(retry):CreateTimer failed", 0, 0, 0);
                return IMS_FALSE;
            }
            IMS_TRACE_I("StartTimer(retry):duration[%d]", nTime, 0, 0);
            m_pRetryTimer->SetTimer(nTime * 1000, this);
            return IMS_TRUE;
        }
        case TIMER_RETRYAFTER:
        {
            StopTimer(TIMER_RETRYAFTER);
            m_pRetryAfterTimer = TimerService::GetTimerService()->CreateTimer();
            if (m_pRetryAfterTimer == IMS_NULL)
            {
                IMS_TRACE_I("StartTimer(retry_after):CreateTimer failed", 0, 0, 0);
                return IMS_FALSE;
            }
            IMS_TRACE_I("StartTimer(retry_after):duration[%d]", nTime, 0, 0);
            m_pRetryAfterTimer->SetTimer(nTime * 1000, this);
            return IMS_TRUE;
        }
        default:
            break;
    }
    return IMS_FALSE;
}

void UcePublishManager::StopTimer(INTERNAL_TIMER eTimer)
{
    ITimer* tempTimer = IMS_NULL;
    IMS_TRACE_I("StopTimer:type[%d]-0(exponential), 1(retry), 2(retry_after),3(all)", eTimer, 0, 0);
    switch (eTimer)
    {
        case TIMER_EXPONENTIAL:
        {
            if (m_pExponentialTimer == IMS_NULL)
            {
                return;
            }
            tempTimer = m_pExponentialTimer;
            m_pExponentialTimer = IMS_NULL;
        }
        break;
        case TIMER_RETRY:
        {
            if (m_pRetryTimer == IMS_NULL)
            {
                return;
            }
            tempTimer = m_pRetryTimer;
            m_pRetryTimer = IMS_NULL;
        }
        break;
        case TIMER_RETRYAFTER:
        {
            if (m_pRetryAfterTimer == IMS_NULL)
            {
                return;
            }
            tempTimer = m_pRetryAfterTimer;
            m_pRetryAfterTimer = IMS_NULL;
        }
        break;
        case TIMER_ALL:
        {
            m_nImmediatelyRetryCount = 0;
            m_nRetryCount = 0;
            m_nExponentialRetryCount = 0;
            StopTimer(TIMER_EXPONENTIAL);
            StopTimer(TIMER_RETRY);
            StopTimer(TIMER_RETRYAFTER);
            return;
        }
        default:
            break;
    }

    tempTimer->KillTimer();
    if (TimerService::GetTimerService() != IMS_NULL)
    {
        TimerService::GetTimerService()->DestroyTimer(tempTimer);
    }
}

void UcePublishManager::HandleExponentialRetryTimer()
{
    IMS_TRACE_I("HandleExponentialRetryTimer:Current State[%s], m_nExponentialRetryCount[%d]",
            StateToString(m_eState), m_nExponentialRetryCount, 0);
    if (GetState() == PUBLISHING || GetState() == ON)
    {
        m_strEtag = AString::ConstEmpty();
        if (RetryPublish(INITIAL) == IMS_TRUE)
        {
            UpdateState(PUBLISHING);
            return;
        }
    }
    else if (GetState() == REFRESHING)
    {
        if (RetryPublish(REFRESH) == IMS_TRUE)
        {
            return;
        }
    }
    UpdateState(ON);
    SendPendingPublishRequest();
}

void UcePublishManager::HandleRetryTimer()
{
    IMS_TRACE_I("HandleRetryTimer:Current State[%s], m_nRetryCount[%d]", StateToString(m_eState),
            m_nRetryCount, 0);
    if (GetState() == PUBLISHING)
    {
        if (RetryPublish(INITIAL) == IMS_TRUE)
        {
            return;
        }
    }
    else if (GetState() == REFRESHING)
    {
        if (RetryPublish(REFRESH) == IMS_TRUE)
        {
            return;
        }
    }
    UpdateState(ON);
    SendPendingPublishRequest();
}

void UcePublishManager::HandleRetryAfterTimer()
{
    IMS_TRACE_I("HandleRetryAfterTimer:Current State[%s]", StateToString(m_eState), 0, 0);
    if (GetState() == PUBLISHING)
    {
        if (RetryPublish(INITIAL) == IMS_TRUE)
        {
            return;
        }
    }
    else if (GetState() == REFRESHING)
    {
        if (RetryPublish(REFRESH) == IMS_TRUE)
        {
            return;
        }
    }
    UpdateState(ON);
    SendPendingPublishRequest();
}

IMS_BOOL UcePublishManager::RetryPublish(IMS_BOOL bRefresh, const AString& strMinExpiryValue)
{
    if (CreatePublication() == IMS_FALSE)
    {
        return IMS_FALSE;
    }
    if (SetPublish(bRefresh, strMinExpiryValue) == IMS_FALSE)
    {
        return IMS_FALSE;
    }
    if (Publish() == IMS_FALSE)
    {
        IMS_TRACE_E(0, "RetryPublish:Publish() is failed", 0, 0, 0);
        return IMS_FALSE;
    }
    return IMS_TRUE;
}

void UcePublishManager::SendPendingPublishRequest()
{
    if (m_pPendingPublicationData != IMS_NULL)
    {
        IMS_UINT32 key = m_pPendingPublicationData->m_nKey;
        AString pidfXml = m_pPendingPublicationData->m_strPidfXml;
        AString eTag = m_pPendingPublicationData->m_strEtag;
        IMS_UINT32 capability = m_pPendingPublicationData->m_nCapability;
        IMS_UINT32 extended = m_pPendingPublicationData->m_nExtended;
        delete m_pPendingPublicationData;
        m_pPendingPublicationData = IMS_NULL;
        SendPublishRequest(key, pidfXml, eTag, capability, extended);
    }
}

void UcePublishManager::ClearPendingPublishRequest()
{
    if (m_pPendingPublicationData != IMS_NULL)
    {
        SendPublishCommandErrorInd(
                m_pPendingPublicationData->m_nKey, IUUceService::COMMAND_CODE_GENERIC_FAILURE);
        delete m_pPendingPublicationData;
        m_pPendingPublicationData = IMS_NULL;
    }
}

IMS_UINT32 UcePublishManager::GetState() const
{
    return m_eState;
}

const IMS_CHAR* UcePublishManager::StateToString(IMS_UINT32 _eState)
{
    static const char* szState[] = {
            "IDLE",
            "ON",
            "PUBLISHING",
            "PUBLISHED",
            "REFRESHING",
            "TERMINATING",
            "ERROR",
    };
    if (sizeof(szState) / sizeof(char*) - 1 <= _eState)
    {
        IMS_TRACE_E(0, "StateToString:State Error", 0, 0, 0);
        return szState[sizeof(szState) / sizeof(char*) - 1];
    }
    return szState[_eState];
}

PRIVATE
IUceJniThread* UcePublishManager::GetJniThread()
{
    const IJniEnabler* piJniEnabler =
            JniEnablerConnector::GetInstance().GetJniEnabler(m_nSimSlot, EnablerType::UCE);
    if (piJniEnabler == IMS_NULL)
    {
        return IMS_NULL;
    }

    return reinterpret_cast<IUceJniThread*>(piJniEnabler->GetJniThread());
}
