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

#include "AStringBuffer.h"
#include "IMessage.h"
#include "ISession.h"
#include "ISipClientConnection.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipMessageBodyPart.h"
#include "ISipServerConnection.h"
#include "ImsList.h"
#include "ServiceTrace.h"
#include "SipHeaderName.h"
#include "SipMethod.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/IMtcSession.h"
#include "ussi/UssiConstants.h"
#include "ussi/UssiController.h"
#include "ussi/UssiData.h"
#include "ussi/UssiDataCreator.h"
#include "utility/IMessageUtils.h"
#include <memory>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
UssiController::UssiController(IN IMtcCallContext& objContext, IN UssiDataParser* pParser) :
        m_objContext(objContext),
        m_pDataParser(pParser),
        m_objEventNotifier(UssiEventNotifier(objContext)),
        m_eUssiModeType(UssiModeType::NONE),
        m_objLastResult(UssiResult(UssiNextAction::NOTHING, UssiError::CODE_NONE))
{
    IMS_TRACE_I("+UssiController", 0, 0, 0);
}

PUBLIC VIRTUAL UssiController::~UssiController()
{
    IMS_TRACE_I("~UssiController", 0, 0, 0);
}

PUBLIC GLOBAL IMS_BOOL UssiController::IsNetworkInitiatedUssi(
        IN IMessageUtils& objMessageUtils, IN const IMessage* piMessage)
{
    IMS_BOOL bResult = IMS_TRUE;

    if (!objMessageUtils.ContainsValue(
                piMessage, UssiConstants::HEADER_USSD_PACKAGE, ISipHeader::RECV_INFO))
    {
        bResult = IMS_FALSE;
    }

    if (!objMessageUtils.ContainsValue(
                piMessage, UssiConstants::HEADER_APPLICATION_USSDXML, ISipHeader::ACCEPT))
    {
        bResult = IMS_FALSE;
    }

    IMS_TRACE_D("IsNetworkInitiatedUssi : %s", _TRACE_B_(bResult), 0, 0);
    return bResult;
}

PUBLIC
IMS_BOOL UssiController::HasValidXmlBodyForNetworkInitiatedUssi(IN const IMessage* piMessage)
{
    IMS_BOOL bResult = IMS_FALSE;
    if (piMessage)
    {
        std::unique_ptr<UssiData> pParsedData =
                std::unique_ptr<UssiData>(GetParsedUssiData(piMessage->GetMessage()));

        if (pParsedData && pParsedData->GetAnyExtension().GetUssiModeType() != UssiModeType::NONE)
        {
            bResult = IMS_TRUE;
        }
    }

    IMS_TRACE_D("HasValidXmlBodyForNetworkInitiatedUssi : %s", _TRACE_B_(bResult), 0, 0);
    return bResult;
}

PUBLIC
IMS_BOOL UssiController::IsByeForUssi(IN const IMessage* piMessage)
{
    IMS_BOOL bResult = IMS_FALSE;
    if (m_objContext.GetMessageUtils().ContainsValue(
                piMessage, UssiConstants::HEADER_APPLICATION_USSDXML, ISipHeader::CONTENT_TYPE))
    {
        bResult = IMS_TRUE;
    }

    IMS_TRACE_D("IsByeForUssi : %s", _TRACE_B_(bResult), 0, 0);
    return bResult;
}

PUBLIC
IMS_BOOL UssiController::IsUssiInfoReceived(IN const ISipServerConnection* piSipServerConnection)
{
    IMS_BOOL bResult = IMS_FALSE;

    if (piSipServerConnection)
    {
        const ISipMessage* piSipMessage = piSipServerConnection->GetMessage();
        if (piSipMessage)
        {
            ImsList<AString> lstHeaders = piSipMessage->GetHeaders(ISipHeader::INFO_PACKAGE);
            for (IMS_UINT32 i = 0; i < lstHeaders.GetSize(); i++)
            {
                if (lstHeaders.GetAt(i).Contains(UssiConstants::HEADER_USSD_PACKAGE))
                {
                    bResult = IMS_TRUE;
                    break;
                }
            }
        }
    }

    IMS_TRACE_D("IsUssiInfoReceived : %s", _TRACE_B_(bResult), 0, 0);
    return bResult;
}

PUBLIC
IMS_BOOL UssiController::HasXmlBodyInInfo(IN const ISipServerConnection* piSipServerConnection)
{
    IMS_BOOL bResult = IMS_FALSE;
    if (piSipServerConnection)
    {
        std::unique_ptr<UssiData> pParsedData =
                std::unique_ptr<UssiData>(GetParsedUssiData(piSipServerConnection->GetMessage()));
        if (pParsedData)
        {
            bResult = IMS_TRUE;
        }
    }

    IMS_TRACE_D("HasXmlBodyInInfo : %s", _TRACE_B_(bResult), 0, 0);
    return bResult;
}

PUBLIC
UssiResult UssiController::HandleUssiBody(
        IN const ISipMessage* piSipMessage, IN IMS_SINT32 nReceivedMethod)
{
    UssiResult objResult(UssiNextAction::NOTHING, UssiError::CODE_NONE);
    std::unique_ptr<UssiData> pParsedData =
            std::unique_ptr<UssiData>(GetParsedUssiData(piSipMessage));

    if (!pParsedData)
    {
        IMS_TRACE_D("HandleUssiBody : there's no xml body.", 0, 0, 0);
        return objResult;
    }

    IMS_TRACE_D("HandleUssiBody : Method[%d] received.", nReceivedMethod, 0, 0);
    UssiModeType eReceivedType = pParsedData->GetAnyExtension().GetUssiModeType();

    if (nReceivedMethod == SipMethod::BYE)
    {
        eReceivedType = UssiModeType::NOTIFY;
    }
    else if (pParsedData->GetUssdString().GetLength() <= 0)
    {
        // if there's no ussd-string in the xml body.
        objResult = UssiResult(UssiNextAction::SEND_INFO_WITH_ERROR_CODE, UssiError::CODE_1);
    }
    else if (nReceivedMethod == SipMethod::ACK &&
            m_objContext.GetCallManager().GetCalls().GetSize() > 1)
    {
        // if there's another call receiving network initiated ussi.
        objResult = UssiResult(
                UssiNextAction::SEND_INFO_WITH_ERROR_CODE_AND_TERMINATE, UssiError::CODE_4);
    }
    else if (nReceivedMethod == SipMethod::INFO && IsUeInitiated())
    {
        // if UE receives INFO request during UE initiated ussi.
        eReceivedType = UssiModeType::REQUEST;
    }
    else if (!IsUeInitiated() && eReceivedType == UssiModeType::NOTIFY)
    {
        // if received INFO or INVITE has USS-Notify element during network initiated ussi.
        objResult = UssiResult(UssiNextAction::SEND_INFO_WITH_NOTIFY_ELEMENT, UssiError::CODE_NONE);
    }

    if (objResult.eErrorCode == UssiError::CODE_NONE)
    {
        NotifyUssiEvent(pParsedData->GetUssdString(), eReceivedType, pParsedData->GetErrorCode());
    }

    SetUssiModeTypeForNetworkInitiated(eReceivedType);
    SetLastResult(objResult);

    IMS_TRACE_D("HandleUssiBody : action[%d] error-code[%d] ussd mode[%d]", m_objLastResult.eAction,
            m_objLastResult.eErrorCode, m_eUssiModeType);

    return objResult;
}

PUBLIC
IMS_RESULT UssiController::FormStartUssiRequest(IN const AString& strTargetNumber)
{
    IMS_TRACE_D("FormStartUssiRequest", 0, 0, 0);
    IMessage* piMessage = m_objContext.GetSession()->GetISession().GetNextRequest();

    if (FormHeadersForStartUssi(piMessage) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    if (FormStartUssiBody(piMessage->GetMessage(), strTargetNumber) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT UssiController::FormAcceptUssi()
{
    IMS_TRACE_D("FormAcceptUssi", 0, 0, 0);
    IMessage* piMessage = m_objContext.GetSession()->GetISession().GetNextRequest();

    if (SetRecvInfoHeader(piMessage) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    return SetAcceptHeader(piMessage);
}

PUBLIC
IMS_RESULT UssiController::FormInfoRequest(IN ISipClientConnection* piSipClientConnection,
        IN const AString& strUssdString, IN UssiError eErrorCode)
{
    IMS_TRACE_D("FormInfoRequest", 0, 0, 0);

    if (FormHeadersForInfo(piSipClientConnection) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    if (FormBodyForInfo(piSipClientConnection->GetMessage(), strUssdString, eErrorCode) ==
            IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PUBLIC
void UssiController::SetNextActionByTerminateUssi()
{
    IMS_TRACE_D("SetNextActionByTerminateUssi", 0, 0, 0);
    SetLastResult(
            UssiResult(UssiNextAction::SEND_INFO_WITH_ERROR_CODE_AND_TERMINATE, UssiError::CODE_1));
}

PUBLIC
UssiResult UssiController::GetLastResult() const
{
    return m_objLastResult;
}

PRIVATE
IMS_RESULT UssiController::FormHeadersForStartUssi(IN IMessage* piMessage)
{
    IMS_TRACE_D("FormHeadersForStartUssi", 0, 0, 0);
    if (SetRecvInfoHeader(piMessage) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    if (SetAcceptHeader(piMessage) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    if (m_objContext.GetMessageUtils().AddValueIfNotExists(piMessage,
                UssiConstants::HEADER_MULTIPART_MIXED, ISipHeader::CONTENT_TYPE) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT UssiController::FormStartUssiBody(
        IN ISipMessage* piSipMessage, IN const AString& strTarget)
{
    IMS_TRACE_D("FormStartUssiBody", 0, 0, 0);
    if (!piSipMessage)
    {
        return IMS_FAILURE;
    }

    ISipMessageBodyPart* piBodyPart = piSipMessage->CreateBodyPart();
    if (!piBodyPart)
    {
        return IMS_FAILURE;
    }

    piBodyPart->SetHeader(
            ISipMessageBodyPart::CONTENT_TYPE, UssiConstants::HEADER_APPLICATION_USSDXML);
    piBodyPart->SetHeader(
            ISipMessageBodyPart::CONTENT_DISPOSITION, UssiConstants::HEADER_RENDER_HANDLING);

    AStringBuffer objXml(UssiConstants::XML_BUFFER_SIZE);
    UssiDataCreator::GetXmlBody(strTarget, objXml, UssiModeType::NONE);
    piBodyPart->SetContent(objXml.GetString());

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT UssiController::SetRecvInfoHeader(IN IMessage* piMessage)
{
    IMS_TRACE_D("SetRecvInfoHeader", 0, 0, 0);
    return m_objContext.GetMessageUtils().AddValueIfNotExists(
            piMessage, UssiConstants::HEADER_USSD_PACKAGE, ISipHeader::RECV_INFO);
}

PRIVATE
IMS_RESULT UssiController::SetAcceptHeader(IN IMessage* piMessage)
{
    IMS_TRACE_D("SetAcceptHeader", 0, 0, 0);
    IMessageUtils& objMessageUtils = m_objContext.GetMessageUtils();
    if (objMessageUtils.AddValueIfNotExists(piMessage, UssiConstants::HEADER_APPLICATION_SDP,
                ISipHeader::ACCEPT) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    if (objMessageUtils.AddValueIfNotExists(piMessage, UssiConstants::HEADER_APPLICATION_IMSXML,
                ISipHeader::ACCEPT) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    if (objMessageUtils.AddValueIfNotExists(piMessage, UssiConstants::HEADER_APPLICATION_USSDXML,
                ISipHeader::ACCEPT) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    if (objMessageUtils.AddValueIfNotExists(piMessage, UssiConstants::HEADER_MULTIPART_MIXED,
                ISipHeader::ACCEPT) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT UssiController::FormHeadersForInfo(IN ISipClientConnection* piSipClientConnection)
{
    IMS_TRACE_D("FormHeadersForInfo", 0, 0, 0);
    if (!piSipClientConnection)
    {
        return IMS_FAILURE;
    }

    if (piSipClientConnection->SetHeader(UssiConstants::HEADER_INFO_PACKAGE,
                UssiConstants::HEADER_USSD_PACKAGE) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    if (piSipClientConnection->SetHeader(SipHeaderName::CONTENT_TYPE,
                UssiConstants::HEADER_APPLICATION_USSDXML) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    if (piSipClientConnection->SetHeader(SipHeaderName::CONTENT_DISPOSITION,
                UssiConstants::HEADER_INFO_PACKAGE) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT UssiController::FormBodyForInfo(
        IN ISipMessage* piSipMessage, IN const AString& strUssdString, IN UssiError eErrorCode)
{
    IMS_TRACE_D("FormBodyForInfo", 0, 0, 0);
    if (!piSipMessage)
    {
        return IMS_FAILURE;
    }

    ISipMessageBodyPart* piBodyPart = piSipMessage->CreateBodyPart();
    if (!piBodyPart)
    {
        return IMS_FAILURE;
    }

    AStringBuffer objXml(UssiConstants::XML_BUFFER_SIZE);
    UssiDataCreator::GetXmlBody(strUssdString, objXml, m_eUssiModeType, eErrorCode);
    piBodyPart->SetContent(objXml.GetString());

    return IMS_SUCCESS;
}

PRIVATE
UssiData* UssiController::GetParsedUssiData(IN const ISipMessage* piSipMessage) const
{
    IMS_TRACE_D("GetParsedUssiData", 0, 0, 0);
    if (!piSipMessage)
    {
        return IMS_NULL;
    }

    ImsList<ISipMessageBodyPart*> objBodyParts = piSipMessage->GetBodyParts();
    if (objBodyParts.IsEmpty())
    {
        return IMS_NULL;
    }

    AString strUssiBody;

    for (IMS_UINT32 i = 0; i < objBodyParts.GetSize(); i++)
    {
        const ISipMessageBodyPart* piBodyPart = objBodyParts.GetAt(i);
        if (piBodyPart != IMS_NULL)
        {
            const ByteArray& objUssiBody = piBodyPart->GetContent();
            strUssiBody = objUssiBody.ToString();
            break;
        }
    }

    return m_pDataParser->Parse(strUssiBody);
}

PRIVATE
void UssiController::NotifyUssiEvent(
        IN const AString& strUssdString, IN UssiModeType eType, IN UssiError eErrorCode)
{
    IMS_TRACE_D("NotifyUssiEvent", 0, 0, 0);

    if (eErrorCode != UssiError::CODE_NONE)
    {
        m_objEventNotifier.NotifyUssiError(strUssdString);
    }
    else
    {
        m_objEventNotifier.NotifyUssiResult(strUssdString, eType);
    }
}

PRIVATE
void UssiController::SetUssiModeTypeForNetworkInitiated(IN UssiModeType eType)
{
    if (!IsUeInitiated())
    {
        m_eUssiModeType = eType;
    }
}

PRIVATE
void UssiController::SetLastResult(IN UssiResult objResult)
{
    m_objLastResult = objResult;
}

PRIVATE
IMS_BOOL UssiController::IsUeInitiated()
{
    return (m_objContext.GetCallInfo().ePeerType == PeerType::MO);
}
