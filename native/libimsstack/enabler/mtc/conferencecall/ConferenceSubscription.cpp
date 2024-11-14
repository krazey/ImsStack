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

#include "AString.h"
#include "CarrierConfig.h"
#include "ICoreService.h"
#include "IFeatureCaps.h"
#include "IMessage.h"
#include "IMessageBodyPart.h"
#include "IMtcContext.h"
#include "ISession.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISubscription.h"
#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "ServiceMemory.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/IMtcSession.h"
#include "conferencecall/ConferenceConfigurationHelper.h"
#include "conferencecall/ConferenceConst.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/ConferenceFactory.h"
#include "conferencecall/ConferenceInfoUpdater.h"
#include "conferencecall/ConferenceSubscription.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/sipinterfaceholder/IMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/SubscriptionInterfaceHolder.h"
#include "utility/IMessageUtils.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
ConferenceSubscription::ConferenceSubscription(IN IMtcContext& objContext, IN CallKey nConfCallKey,
        IN ConferenceParticipantList& objList, IN IConferenceSubscriptionListener& objListener,
        IN ConferenceFactory& objFactory) :
        m_objContext(objContext),
        m_nConfCallKey(nConfCallKey),
        m_objFactory(objFactory),
        m_objList(objList),
        m_objListener(objListener),
        m_strTo(AString::ConstNull()),
        m_piSubscription(IMS_NULL),
        m_nState(SubscriptionState::IDLE),
        m_nDialogType(CONF_SUBSCRIPTION_DIALOG_TYPE_OUT),
        m_nExpires(-1),
        m_nReSubsCount(0)
{
    IMS_TRACE_I("+ConferenceSubscription", 0, 0, 0);

    m_nDialogType = ConferenceConfigurationHelper::IsSubscriptionOutDialog(
                            m_objContext.GetConfigurationProxy())
            ? CONF_SUBSCRIPTION_DIALOG_TYPE_OUT
            : CONF_SUBSCRIPTION_DIALOG_TYPE_IN;
}

PUBLIC
ConferenceSubscription::~ConferenceSubscription()
{
    IMS_TRACE_I("~ConferenceSubscription", 0, 0, 0);
    ReleaseISubscription();
}

PUBLIC VIRTUAL void ConferenceSubscription::SubscriptionNotify(
        IN ISubscription* piSubscription, IN IMessage* piNotify)
{
    (void)piSubscription;
    UpdateConferenceInfo(piNotify);
}

PUBLIC VIRTUAL void ConferenceSubscription::SubscriptionStarted(IN ISubscription* piSubscription)
{
    (void)piSubscription;
    IMS_TRACE_I("SubscriptionStarted", 0, 0, 0);

    if (GetState() == SubscriptionState::SUBSCRIBING)
    {
        SetState(SubscriptionState::ACTIVE);
        m_objListener.OnSubscriptionUpdated(SubscriptionUpdateType::SUCCEEDED);

        if (m_nDialogType == CONF_SUBSCRIPTION_DIALOG_TYPE_FALLBACK)
        {
            m_objContext.GetConfigurationProxy().PutCache(
                    ConfigVoice::KEY_CONFERENCE_SUBSCRIBE_TYPE_INT,
                    ConfigVoice::CONFERENCE_SUBSCRIBE_TYPE_IN_DIALOG);
        }
    }
    else if (GetState() == SubscriptionState::UNSUBSCRIBING)
    {
        SetState(SubscriptionState::IDLE);
        m_objListener.OnSubscriptionUpdated(SubscriptionUpdateType::UNSUBSCRIBED);
    }
}

PUBLIC VIRTUAL void ConferenceSubscription::SubscriptionStartFailed(
        IN ISubscription* piSubscription)
{
    IMS_TRACE_I("SubscriptionStartFailed", 0, 0, 0);
    IMessage* piMessage = piSubscription->GetPreviousResponse(IMessage::SUBSCRIPTION_SUBSCRIBE);

    if (piMessage == IMS_NULL)
    {
        return;
    }

    if (GetState() == SubscriptionState::UNSUBSCRIBING)
    {
        ReleaseISubscription();
        m_objListener.OnSubscriptionUpdated(SubscriptionUpdateType::UNSUBSCRIBED);
        return;
    }

    switch (piMessage->GetStatusCode())
    {
        case SipStatusCode::SC_403:
            if (OnReceiving403(piSubscription))
            {
                return;
            }
            break;
        case SipStatusCode::SC_423:
            if (OnReceiving423(piSubscription))
            {
                return;
            }
            break;
        default:
            // TODO: Other responses
            break;
    }

    ReleaseISubscription();
    m_objListener.OnSubscriptionUpdated(SubscriptionUpdateType::FAILED);
}

PUBLIC VIRTUAL void ConferenceSubscription::SubscriptionTerminated(IN ISubscription* piSubscription)
{
    (void)piSubscription;

    SetState(SubscriptionState::IDLE);
    m_objListener.OnSubscriptionUpdated(SubscriptionUpdateType::TERMINATED);
}

PUBLIC
IMS_RESULT ConferenceSubscription::Subscribe(IN const AString& strTo)
{
    IMS_TRACE_I("Subscribe : (%s)",
            m_nDialogType == CONF_SUBSCRIPTION_DIALOG_TYPE_OUT ? "OUT" : "IN", 0, 0);

    if (m_piSubscription == IMS_NULL)
    {
        m_strTo = strTo;
        CreateSubscription();
    }

    return Subscribe();
}

PUBLIC
void ConferenceSubscription::UnSubscribe()
{
    IMS_TRACE_I("UnSubscribe", 0, 0, 0);

    if (m_piSubscription == IMS_NULL)
    {
        return;
    }

    if (GetState() == SubscriptionState::IDLE)
    {
        return;
    }

    m_piSubscription->Unsubscribe();
    SetState(SubscriptionState::UNSUBSCRIBING);
}

PUBLIC
SubscriptionState ConferenceSubscription::GetState()
{
    IMS_TRACE_I("GetState : [%d]", m_nState, 0, 0);
    return m_nState;
}

PRIVATE
void ConferenceSubscription::SetState(IN SubscriptionState nState)
{
    IMS_TRACE_I("SetState : [%d]", nState, 0, 0);
    m_nState = nState;
}

PRIVATE
void ConferenceSubscription::CreateSubscription()
{
    IMS_TRACE_I("CreateSubscription : (%s)",
            m_nDialogType == CONF_SUBSCRIPTION_DIALOG_TYPE_OUT ? "OUT" : "IN", 0, 0);

    if (m_nDialogType == CONF_SUBSCRIPTION_DIALOG_TYPE_OUT)
    {
        m_piSubscription =
                m_objContext.GetSipInterfaceFactory().GetISubscriptionHolder()->GetISubscription(
                        m_objContext.GetServiceByType(ServiceType::NORMAL)->GetICoreService(),
                        IMS_NULL, m_strTo, "conference");
    }
    else
    {
        m_piSubscription =
                m_objContext.GetSipInterfaceFactory().GetISubscriptionHolder()->GetISubscription(
                        &m_objContext.GetCallManager()
                                 .GetCallByCallKey(m_nConfCallKey)
                                 ->GetCallContext()
                                 .GetSession()
                                 ->GetISession(),
                        "conference");
    }
}

PRIVATE
IMS_RESULT ConferenceSubscription::Subscribe()
{
    IMS_TRACE_I("Subscribe", 0, 0, 0);

    if (m_piSubscription == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    Initialize();

    m_piSubscription->SetListener(this);
    if (m_piSubscription->Subscribe() == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    SetState(SubscriptionState::SUBSCRIBING);
    return IMS_SUCCESS;
}

PRIVATE
void ConferenceSubscription::ReSubscribe()
{
    IMS_TRACE_I("ReSubscribe : [%d]th", ++m_nReSubsCount, 0, 0);

    IMS_RESULT nResult = IMS_FAILURE;
    if (m_nReSubsCount <= MAX_RESUBS_COUNT)
    {
        nResult = Subscribe();
    }

    if (nResult == IMS_FAILURE)
    {
        ReleaseISubscription();
        m_objListener.OnSubscriptionUpdated(SubscriptionUpdateType::FAILED);
    }
}

PRIVATE
void ConferenceSubscription::Initialize()
{
    IMS_TRACE_I("Initialize", 0, 0, 0);

    SetHeaders();
}

PRIVATE
void ConferenceSubscription::SetHeaders()
{
    IMessage* piMessage = m_piSubscription->GetNextRequest();
    if (!piMessage)
    {
        return;
    }

    ISipMessage* piSipMessage = piMessage->GetMessage();
    if (!piSipMessage)
    {
        return;
    }

    piSipMessage->AddHeader(ISipHeader::ACCEPT, ConferenceConst::APPLICATION_CONFERENCEINFO);

    // TODO: messageformatter.
    // TODO: null check?
    IFeatureCaps* piFeatureCaps =
            m_objContext.GetServiceByType(ServiceType::NORMAL)->GetICoreService()->GetFeatureCaps();
    if (piFeatureCaps != IMS_NULL)
    {
        piFeatureCaps->AddFeature("+g.3gpp.mid-call", AString::ConstEmpty(), SipMethod::SUBSCRIBE,
                ISipMessage::TYPE_REQUEST);
    }

    IMS_TRACE_I("SetHeaders : [%d]", m_nExpires, 0, 0);
    if (m_nExpires >= 0)
    {
        AString strExpires;
        strExpires.SetNumber(m_nExpires);
        piSipMessage->SetHeader(ISipHeader::EXPIRES_SEC, strExpires);
    }
}

PRIVATE
void ConferenceSubscription::UpdateConferenceInfo(IN IMessage* piNotify)
{
    IMS_TRACE_I("UpdateConferenceInfo", 0, 0, 0);
    AString strSubState =
            m_objContext.GetMessageUtils().GetHeaderValue(piNotify, ISipHeader::SUBSCRIPTION_STATE);
    if (strSubState.Equals("terminated"))
    {
        // TODO: needed? static final const value.
        return;
    }

    ImsList<IMessageBodyPart*> objBodyParts = piNotify->GetBodyParts();

    if (objBodyParts.IsEmpty())
    {
        IMS_TRACE_E(0, "objBodyParts IsEmpty", 0, 0, 0);
        return;
    }

    AString strEventPackage;
    for (IMS_UINT32 nIndex = 0; nIndex < objBodyParts.GetSize(); nIndex++)
    {
        IMessageBodyPart* piBodyPart = objBodyParts.GetAt(nIndex);
        if (piBodyPart != IMS_NULL)
        {
            const ByteArray& objEventPackage = piBodyPart->GetContent();
            strEventPackage = objEventPackage.ToString();
            break;
        }
    }

    if (strEventPackage.GetLength() <= 0)
    {
        HandleUpdateResult(ConferenceInfoUpdater::RESULT_AMBIGUOUS);
        return;
    }

    ConferenceInfoUpdater* pInfoUpdater = m_objFactory.CreateInfoUpdater();
    IMS_UINT32 nResult = pInfoUpdater->Update(&m_objList, strEventPackage);

    HandleUpdateResult(nResult);
    delete pInfoUpdater;
}

PRIVATE
void ConferenceSubscription::HandleUpdateResult(IN IMS_UINT32 nResult)
{
    IMS_TRACE_I("HandleUpdateResult : [%d]", nResult, 0, 0);
    switch (nResult)
    {
        case ConferenceInfoUpdater::RESULT_UPDATED:
        case ConferenceInfoUpdater::RESULT_NOTHING_UPDATED:
            Notify();
            break;
        case ConferenceInfoUpdater::RESULT_INVALID_VERSION:
            // re-send Subscription
            ReSubscribe();
            break;
        case ConferenceInfoUpdater::RESULT_MALFORMED_XML:
            // stop subscription
        case ConferenceInfoUpdater::RESULT_INFO_DELETED:
            // terminate conference call?
            // or stop Subscription?
        default:  // ConferenceInfoUpdater::RESULT_AMBIGUOUS:
            // re-sned Subscription
            break;
    }
}

PRIVATE
void ConferenceSubscription::Notify()
{
    m_objListener.OnSubscriptionUpdated(SubscriptionUpdateType::NOTIFY_RECEIVED);
}

PRIVATE
IMS_BOOL ConferenceSubscription::OnReceiving403(IN ISubscription* piSubscription)
{
    (void)piSubscription;
    IMS_TRACE_I("OnReceiving403", 0, 0, 0);

    if (m_nDialogType == CONF_SUBSCRIPTION_DIALOG_TYPE_FALLBACK ||
            m_nDialogType == CONF_SUBSCRIPTION_DIALOG_TYPE_IN)
    {
        // failed.
        return IMS_FALSE;
    }

    ReleaseISubscription();

    m_nDialogType = CONF_SUBSCRIPTION_DIALOG_TYPE_FALLBACK;
    CreateSubscription();
    Subscribe();

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL ConferenceSubscription::OnReceiving423(IN ISubscription* piSubscription)
{
    IMS_SINT32 nExpires = m_objContext.GetMessageUtils().GetHeaderValueInt(
            piSubscription->GetPreviousResponse(IMessage::SUBSCRIPTION_SUBSCRIBE),
            ISipHeader::MIN_EXPIRES);

    IMS_TRACE_I("OnReceiving423 : [%d]", nExpires, 0, 0);

    if (nExpires == -1)
    {
        return IMS_FALSE;
    }

    ReleaseISubscription();

    m_nExpires = (nExpires + nExpires / 2);  // where is this from?
    CreateSubscription();
    Subscribe();

    return IMS_TRUE;
}

PRIVATE
void ConferenceSubscription::ReleaseISubscription()
{
    m_objContext.GetSipInterfaceFactory().GetISubscriptionHolder()->ReleaseISubscription(
            m_piSubscription);
    m_piSubscription = IMS_NULL;
}
