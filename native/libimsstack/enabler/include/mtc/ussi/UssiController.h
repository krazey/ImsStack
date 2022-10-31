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
class ISipClientConnection;
class ISipMessage;
class ISipServerConnection;

class UssiController
{
public:
    UssiController(IN IMtcCallContext& objContext);
    virtual ~UssiController();

private:
    UssiController(IN const UssiController& objRHS);
    UssiController& operator=(IN const UssiController& objRHS);

public:
    static IMS_BOOL IsNetworkInitiatedUssi(IN IMessage* piMessage);

    IMS_BOOL HasValidXmlBodyForNetworkInitiatedUssi(IN IMessage* piMessage);
    IMS_BOOL IsByeForUssi(IN IMessage* piMessage);
    IMS_BOOL IsUssiInfoReceived(IN ISipServerConnection* piSipServerConnection);
    IMS_BOOL HasXmlBodyInInfo(IN ISipServerConnection* piSipServerConnection);

    UssiResult ParseUssiBodyAndCheckResult(
            IN ISipMessage* piSipMessage, IN IMS_SINT32 nReceivedMethod);

    IMS_RESULT FormStartUssiRequest(IN const AString& strTargetNumber);
    IMS_RESULT FormAcceptUssi();
    IMS_RESULT FormInfoRequest(IN ISipClientConnection* piSipClientConnection,
            IN const AString& strUssdString, IN UssiError eErrorCode);

    void SetNextActionByTerminateUssi();

    UssiResult GetLastResult() const;

private:
    IMS_RESULT FormHeadersForStartUssi(IN IMessage* piMessage);
    IMS_RESULT FormStartUssiBody(IN ISipMessage* piSipMessage, IN const AString& strTarget);

    IMS_RESULT SetRecvInfoHeader(IN IMessage* piMessage);
    IMS_RESULT SetAcceptHeader(IN IMessage* piMessage);

    IMS_RESULT FormHeadersForInfo(IN ISipClientConnection* piSipClientConnection);
    IMS_RESULT FormBodyForInfo(
            IN ISipMessage* piSipMessage, IN const AString& strUssdString, IN UssiError eErrorCode);

    UssiData* GetParsedUssiData(IN ISipMessage* piSipMessage);
    void NotifyUssiEvent(IN AString strUssdString, IN UssiModeType eType, IN UssiError eErrorCode);

    void SetUssiModeTypeForNetworkInitiated(IN UssiModeType eType);
    void SetLastResult(IN UssiResult objResult);

    IMS_BOOL IsUeInitiated();

private:
    IMtcCallContext& m_objContext;
    UssiEventNotifier m_objEventNotifier;
    UssiModeType m_eUssiModeType;
    UssiResult m_objLastResult;
};

#endif
