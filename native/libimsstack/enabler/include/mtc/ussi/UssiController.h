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

#ifndef USSI_CONTROLLER_H_
#define USSI_CONTROLLER_H_

#include "ISipClientConnectionListener.h"
#include "ISipErrorListener.h"
#include "ImsTypeDef.h"
#include "ussi/UssiData.h"
#include "ussi/UssiEventNotifier.h"

class AString;
class IMessage;
class IMessageUtils;
class IMtcCallContext;
class ISession;
class ISipClientConnection;
class ISipConnection;
class ISipMessage;
class ISipServerConnection;

class UssiController : public ISipClientConnectionListener, public ISipErrorListener
{
public:
    UssiController(IN IMtcCallContext& objContext, IN UssiDataParser* pParser);
    ~UssiController() override;
    UssiController(IN const UssiController&) = delete;
    UssiController& operator=(IN const UssiController&) = delete;

    static IMS_BOOL IsNetworkInitiatedUssi(
            IN IMessageUtils& objMessageUtils, IN const IMessage* piMessage);

    void ClientConnection_NotifyResponse(IN ISipClientConnection* piScc,
            IN ISipClientConnection* piForkedScc = IMS_NULL) override;
    void Error_NotifyError(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) override;

    virtual IMS_BOOL HasValidXmlBodyForNetworkInitiatedUssi(IN const IMessage* piMessage);
    virtual IMS_BOOL IsUssiInfoReceived(IN const ISipServerConnection* piSipServerConnection);
    virtual IMS_BOOL HasXmlBodyInInfo(IN const ISipServerConnection* piSipServerConnection);
    virtual UssiResult HandleUssiBody(
            IN const ISipMessage* piSipMessage, IN IMS_SINT32 nReceivedMethod);

    virtual IMS_RESULT FormStartUssiRequest(IN const AString& strTargetNumber);
    virtual IMS_RESULT FormAcceptUssi();

    virtual IMS_RESULT SendInfo(
            IN ISession& objSession, IN const AString& strUssdString, IN UssiError eErrorCode);

    virtual void SetNextActionByTerminateUssi();

private:
    IMS_RESULT FormHeadersForStartUssi(IN IMessage* piMessage);
    static IMS_RESULT FormStartUssiBody(IN ISipMessage* piSipMessage, IN const AString& strTarget);

    IMS_RESULT SetRecvInfoHeader(IN IMessage* piMessage);
    IMS_RESULT SetAcceptHeader(IN IMessage* piMessage);

    IMS_RESULT FormInfoRequest(IN const AString& strUssdString, IN UssiError eErrorCode);
    IMS_RESULT FormHeadersForInfo();
    IMS_RESULT FormBodyForInfo(
            IN ISipMessage* piSipMessage, IN const AString& strUssdString, IN UssiError eErrorCode);

    UssiData* GetParsedUssiData(IN const ISipMessage* piSipMessage) const;
    void NotifyUssiEvent(
            IN const AString& strUssdString, IN UssiModeType eType, IN UssiError eErrorCode);

    void SetUssiModeTypeForNetworkInitiated(IN UssiModeType eType);
    void SetLastResult(IN UssiResult objResult);

    void CompleteClientConnection();

    IMS_BOOL IsUeInitiated();

private:
    IMtcCallContext& m_objContext;
    std::unique_ptr<UssiDataParser> m_pDataParser;
    UssiEventNotifier m_objEventNotifier;
    ISipClientConnection* m_pCurrentInfoClientConnection;
    UssiModeType m_eUssiModeType;
    UssiResult m_objLastResult;
};

#endif
