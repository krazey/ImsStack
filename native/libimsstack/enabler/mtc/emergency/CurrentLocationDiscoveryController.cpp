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

#include "CarrierConfig.h"
#include "ICoreService.h"
#include "IMessage.h"
#include "IPublication.h"
#include "ISession.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipMessageBodyPart.h"
#include "ISipServerConnection.h"
#include "ImsTypeDef.h"
#include "RetryTaskHelper.h"
#include "RetryTimer.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcSession.h"
#include "configuration/MtcConfigurationProxy.h"
#include "emergency/CurrentLocationDiscoveryController.h"
#include "helper/MtcLocationObject.h"
#include "utility/IMessageUtils.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

LOCAL const AString STR_REQUEST_FOR_LOCATION_INFORMATION("requestForLocationInformation");
LOCAL const AString STR_PRESENCE("presence");

PUBLIC
CurrentLocationDiscoveryController::CurrentLocationDiscoveryController(
        IN IMtcCallContext& objContext) :
        m_objContext(objContext),
        m_piPublication(IMS_NULL),
        m_pLocationTransmissionTask(IMS_NULL)
{
    IMS_TRACE_I("+CurrentLocationDiscoveryController", 0, 0, 0);
}

PUBLIC VIRTUAL
CurrentLocationDiscoveryController::~CurrentLocationDiscoveryController()
{
    IMS_TRACE_I("~CurrentLocationDiscoveryController", 0, 0, 0);

    DestroyPublication();

    if (m_pLocationTransmissionTask != IMS_NULL)
    {
        m_pLocationTransmissionTask->Terminate();

        RetryTimer* pTimer = m_pLocationTransmissionTask->SetTimer(IMS_NULL);
        delete pTimer;
    }
}

PUBLIC GLOBAL
IMS_BOOL CurrentLocationDiscoveryController::IsCurrentLocationDiscoveryInfoReceived(
        IN const ISipServerConnection& objSipServerConnection)
{
    const ISipMessage* piSipMessage = objSipServerConnection.GetMessage();
    if (piSipMessage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ImsList<AString> lstHeaders = piSipMessage->GetHeaders(ISipHeader::INFO_PACKAGE);
    for (IMS_UINT32 i = 0; i < lstHeaders.GetSize(); i++)
    {
        if (lstHeaders.GetAt(i).Contains(MessageUtil::STR_PACKAGE_CURRENT_LOCATION_DISCOVERY))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC GLOBAL IMS_BOOL CurrentLocationDiscoveryController::IsPeriodicLocationDiscoveryRequired(
        IN IMS_BOOL bEmergency, IN IMS_SINT32 nMethod)
{
    return bEmergency && nMethod == ConfigEmergency::CALL_PERIODIC_LOCATION_DISCOVERY_METHOD_UPDATE;
}

PUBLIC
void CurrentLocationDiscoveryController::OnCurrentLocationDiscoveryInfoReceived(
        IN ISipServerConnection& objSipServerConnection)
{
    if (!m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigEmergency::KEY_EMERGENCY_CALL_CURRENT_LOCATION_DISCOVERY_SUPPORTED_BOOL) ||
            !m_objContext.GetCallInfo().IsEmergency())
    {
        SendResponseForInfo(objSipServerConnection, SipStatusCode::SC_469);
        return;
    }

    const ISipMessage* piSipMessage = objSipServerConnection.GetMessage();
    IMS_BOOL bNeedToSendPublish = (piSipMessage != IMS_NULL) ?
            HasRequestForCurrentLocation(*piSipMessage) : IMS_FALSE;
    SendResponseForInfo(objSipServerConnection, SipStatusCode::SC_200);

    if (!bNeedToSendPublish)
    {
        return;
    }

    SendCurrentLocationPublish();
}

PUBLIC
void CurrentLocationDiscoveryController::StartPeriodicLocationDiscovery()
{
    if (m_pLocationTransmissionTask != IMS_NULL)
    {
        return;
    }

    m_pLocationTransmissionTask = std::make_unique<RetryTaskHelper>();
    m_pLocationTransmissionTask->SetCommand(this);

    RetryTimer* pTimer = new RetryTimer(IMS_TRUE);
    pTimer->AddValue(m_objContext.GetConfigurationProxy().GetInt(
            ConfigEmergency::KEY_CALL_PERIODIC_LOCATION_DISCOVERY_TIMER_MILLIS_INT));
    m_pLocationTransmissionTask->SetTimer(pTimer);
    m_pLocationTransmissionTask->Start(RetryTaskHelper::START_TIMER);
}

PUBLIC VIRTUAL IMS_RESULT CurrentLocationDiscoveryController::ExecuteCmd()
{
    return m_objContext.GetSession()->Update(UpdateType::LOCATION, IMS_FALSE, SipMethod::UPDATE);
}

PRIVATE
IMS_BOOL CurrentLocationDiscoveryController::HasRequestForCurrentLocation(
        IN const ISipMessage& objSipMessage)
{
    ImsList<ISipMessageBodyPart*> objBodyParts = objSipMessage.GetBodyParts();
    if (objBodyParts.IsEmpty())
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < objBodyParts.GetSize(); i++)
    {
        const ISipMessageBodyPart* piBodyPart = objBodyParts.GetAt(i);
        if (piBodyPart == IMS_NULL)
        {
            continue;
        }

        const ByteArray& objContent = piBodyPart->GetContent();
        if (objContent.ToString().Contains(STR_REQUEST_FOR_LOCATION_INFORMATION))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
void CurrentLocationDiscoveryController::SendResponseForInfo(
        IN ISipServerConnection& objSipServerConnection, IN IMS_UINT32 nResponseCode)
{
    objSipServerConnection.InitResponse(nResponseCode);
    objSipServerConnection.Send();
    objSipServerConnection.Close();
}

PRIVATE
void CurrentLocationDiscoveryController::SendCurrentLocationPublish()
{
    if (CreatePublication() == IMS_FALSE)
    {
        return;
    }

    SetLocationInformation();
    SendPublish();
}

PRIVATE
void CurrentLocationDiscoveryController::SetLocationInformation()
{
    IMessage* piMessage = m_piPublication->GetNextRequest();
    if (piMessage == IMS_NULL)
    {
        return;
    }

    MtcLocationObject(m_objContext).SetLocationToMessage(*piMessage, IMS_TRUE);
}

PRIVATE
void CurrentLocationDiscoveryController::SendPublish()
{
    m_piPublication->Publish(ByteArray::ConstNull(), AString::ConstNull());
}

PRIVATE
IMS_BOOL CurrentLocationDiscoveryController::CreatePublication()
{
    if (m_piPublication != IMS_NULL)
    {
        return IMS_TRUE;
    }

    IMtcSession* pSession = m_objContext.GetSession();
    if (pSession == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ISession& objSession = pSession->GetISession();
    m_piPublication = objSession.CreatePublication(STR_PRESENCE);

    if (m_piPublication == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_D("Publication created", 0, 0, 0);

    return IMS_TRUE;
}

PRIVATE
void CurrentLocationDiscoveryController::DestroyPublication()
{
    if (m_piPublication == IMS_NULL)
    {
        return;
    }

    m_piPublication->Destroy();
    m_piPublication = IMS_NULL;

    IMS_TRACE_D("Publication destroyed", 0, 0, 0);
}
