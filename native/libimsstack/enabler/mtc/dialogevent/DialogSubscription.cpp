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

#include "ICoreService.h"
#include "IMessage.h"
#include "IMessageBodyPart.h"
#include "IMtcContext.h"
#include "IMtcService.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISubscription.h"
#include "ImsTypeDef.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "dialogevent/DialogSubscription.h"
#include "dialogevent/IDialogSubscription.h"
#include "helper/sipinterfaceholder/IMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/SubscriptionInterfaceHolder.h"
#include "utility/IMessageUtils.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
DialogSubscription::DialogSubscription(IN IMtcContext& objContext,
        IN IDialogSubscriptionListener& objListener, IN const AString& strTargetUri) :
        m_objContext(objContext),
        m_objListener(objListener),
        m_piSubscription(IMS_NULL),
        m_strTargetUri(strTargetUri),
        m_nExpiresInterval(7200)
{
    IMS_TRACE_I("+DialogSubscription", 0, 0, 0);
}

PUBLIC
DialogSubscription::~DialogSubscription()
{
    IMS_TRACE_I("~DialogSubscription", 0, 0, 0);
    m_objContext.GetSipInterfaceFactory().GetISubscriptionHolder()->ReleaseISubscription(
            m_piSubscription);
}

VIRTUAL PUBLIC IMS_RESULT DialogSubscription::Subscribe()
{
    IMS_TRACE_D("Subscribe", 0, 0, 0);
    if (m_piSubscription)
    {
        IMS_TRACE_E(0, "subscription already started", 0, 0, 0);
        return IMS_FAILURE;
    }

    m_piSubscription =
            m_objContext.GetSipInterfaceFactory().GetISubscriptionHolder()->GetISubscription(
                    m_objContext.GetServiceByType(ServiceType::NORMAL)->GetICoreService(),
                    m_strTargetUri, m_strTargetUri, "dialog");
    if (!m_piSubscription)
    {
        IMS_TRACE_E(0, "creating ISubscription failed.", 0, 0, 0);
        return IMS_FAILURE;
    }

    m_piSubscription->SetListener(this);
    SetHeaders();

    return m_piSubscription->Subscribe();
}

VIRTUAL PUBLIC void DialogSubscription::Unsubscribe()
{
    if (m_piSubscription && m_piSubscription->GetState() == ISubscription::STATE_ACTIVE)
    {
        IMS_TRACE_D("Unsubscribe", 0, 0, 0);
        m_piSubscription->Unsubscribe();
    }
}

VIRTUAL PUBLIC void DialogSubscription::SubscriptionNotify(
        IN ISubscription* /*piSubscription*/, IN IMessage* piNotify)
{
    IMS_TRACE_D("SubscriptionNotify", 0, 0, 0);
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

    m_objListener.OnSubscriptionNotified(strEventPackage);
}

VIRTUAL PUBLIC void DialogSubscription::SubscriptionStarted(IN ISubscription* piSubscription)
{
    IMS_TRACE_D("SubscriptionStarted State[%d]", piSubscription->GetState(), 0, 0);
    m_objListener.OnSubscriptionStarted();
}

VIRTUAL PUBLIC void DialogSubscription::SubscriptionStartFailed(IN ISubscription* piSubscription)
{
    IMS_TRACE_D("SubscriptionStartFailed", 0, 0, 0);
    if (piSubscription)
    {
        const IMessage* piMessage =
                piSubscription->GetPreviousResponse(IMessage::SUBSCRIPTION_SUBSCRIBE);
        if (IsIntervalTooBrief(piMessage) && HandleIntervalTooBrief(*piMessage) == IMS_SUCCESS)
        {
            return;
        }
    }

    m_objListener.OnSubscriptionStartFailed();
}

VIRTUAL PUBLIC void DialogSubscription::SubscriptionTerminated(IN ISubscription* piSubscription)
{
    IMS_TRACE_D("SubscriptionTerminated", 0, 0, 0);
    (void)piSubscription;
    m_objListener.OnSubscriptionTerminated();
    // TODO: re-attempt based on RFC 3265
}

PRIVATE
void DialogSubscription::SetHeaders()
{
    IMessage* piMessage = m_piSubscription->GetNextRequest();
    ISipMessage* piSipMessage = piMessage ? piMessage->GetMessage() : IMS_NULL;
    if (!piSipMessage)
    {
        // keep subscribing.
        return;
    }

    AString strExpires;
    strExpires.SetNumber(m_nExpiresInterval);
    piSipMessage->SetHeader(ISipHeader::EXPIRES_SEC, strExpires);
}

PRIVATE
IMS_BOOL DialogSubscription::IsIntervalTooBrief(IN const IMessage* piMessage) const
{
    if (piMessage && piMessage->GetStatusCode() == SipStatusCode::SC_423)
    {
        return IMS_TRUE;
    }
    return IMS_FALSE;
}

PRIVATE
IMS_RESULT DialogSubscription::HandleIntervalTooBrief(IN const IMessage& objMessage)
{
    IMS_SINT32 nMinExpires =
            m_objContext.GetMessageUtils().GetHeaderValueInt(&objMessage, ISipHeader::MIN_EXPIRES);

    IMS_TRACE_D("HandleIntervalTooBrief [%d]", nMinExpires, 0, 0);
    if (nMinExpires > m_nExpiresInterval)
    {
        m_objContext.GetSipInterfaceFactory().GetISubscriptionHolder()->ReleaseISubscription(
                m_piSubscription);
        m_piSubscription = IMS_NULL;
        m_nExpiresInterval = nMinExpires;
        return Subscribe();
    }

    return IMS_FAILURE;
}
