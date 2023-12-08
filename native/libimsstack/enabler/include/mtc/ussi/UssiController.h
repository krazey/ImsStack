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

#include "ImsTypeDef.h"
#include "call/IMtcCallContext.h"
#include "ussi/UssiData.h"
#include "ussi/UssiEventNotifier.h"

class IMessage;
class IMessageUtils;
class ISipClientConnection;
class ISipMessage;
class ISipServerConnection;

class UssiController
{
public:
    explicit UssiController(IN IMtcCallContext& objContext, IN UssiDataParser* pParser);
    virtual ~UssiController();
    UssiController(IN const UssiController&) = delete;
    UssiController& operator=(IN const UssiController&) = delete;

    static IMS_BOOL IsNetworkInitiatedUssi(
            IN IMessageUtils& objMessageUtils, IN IMessage* piMessage);

    virtual IMS_BOOL HasValidXmlBodyForNetworkInitiatedUssi(IN IMessage* piMessage);
    virtual IMS_BOOL IsByeForUssi(IN IMessage* piMessage);
    virtual IMS_BOOL IsUssiInfoReceived(IN ISipServerConnection* piSipServerConnection);
    virtual IMS_BOOL HasXmlBodyInInfo(IN ISipServerConnection* piSipServerConnection);
    virtual UssiResult ParseUssiBodyAndCheckResult(
            IN ISipMessage* piSipMessage, IN IMS_SINT32 nReceivedMethod);

    virtual IMS_RESULT FormStartUssiRequest(IN const AString& strTargetNumber);
    virtual IMS_RESULT FormAcceptUssi();
    virtual IMS_RESULT FormInfoRequest(IN ISipClientConnection* piSipClientConnection,
            IN const AString& strUssdString, IN UssiError eErrorCode);

    virtual void SetNextActionByTerminateUssi();

    virtual UssiResult GetLastResult() const;

private:
    IMS_RESULT FormHeadersForStartUssi(IN IMessage* piMessage);
    static IMS_RESULT FormStartUssiBody(IN ISipMessage* piSipMessage, IN const AString& strTarget);

    IMS_RESULT SetRecvInfoHeader(IN IMessage* piMessage);
    IMS_RESULT SetAcceptHeader(IN IMessage* piMessage);

    static IMS_RESULT FormHeadersForInfo(IN ISipClientConnection* piSipClientConnection);
    IMS_RESULT FormBodyForInfo(
            IN ISipMessage* piSipMessage, IN const AString& strUssdString, IN UssiError eErrorCode);

    UssiData* GetParsedUssiData(IN ISipMessage* piSipMessage) const;
    void NotifyUssiEvent(
            IN const AString& strUssdString, IN UssiModeType eType, IN UssiError eErrorCode);

    void SetUssiModeTypeForNetworkInitiated(IN UssiModeType eType);
    void SetLastResult(IN UssiResult objResult);

    IMS_BOOL IsUeInitiated();

private:
    IMtcCallContext& m_objContext;
    std::unique_ptr<UssiDataParser> m_pDataParser;
    UssiEventNotifier m_objEventNotifier;
    UssiModeType m_eUssiModeType;
    UssiResult m_objLastResult;
};

#endif
