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

#ifndef EMERGENCY_MESSAGE_FORMATTER_H_
#define EMERGENCY_MESSAGE_FORMATTER_H_

#include "ImsTypeDef.h"
#include "call/message/MessageFormatter.h"

class AString;
class IMtcCallContext;
class ISession;
enum class CallType;
enum class ServiceType;
enum class UpdateType;

class EmergencyMessageFormatter : public MessageFormatter
{
public:
    EmergencyMessageFormatter(IN IMtcCallContext& objContext, IN ISession& objSession);
    virtual ~EmergencyMessageFormatter() override;
    EmergencyMessageFormatter(IN const MessageFormatter&) = delete;
    EmergencyMessageFormatter& operator=(IN const MessageFormatter&) = delete;

    IMS_RESULT FormStartMessage(IN CallType eCallType) override;
    IMS_RESULT FormUpdateMessage(IN UpdateType eUpdateType, IN IMS_BOOL bIncludeAlertInfo) override;

protected:
    void SetAcceptHeader() override;
    void SetCallerIdHeader() override;
    void SetPPreferredIdentityHeader() override;

private:
    void SetPPreferredIdentityHeaderByFormat(IN const AString& strFormat);
    void SetPPreferredIdentityHeaderByUserId();
    void SetRecvInfoHeader();
    void SetPEmergencyInfoHeader();
    void SetSipInstanceFeature();
    void SetPComServiceTypeHeader();

    IMS_UINT32 GetAosRegMode(IN ServiceType eServiceType) const;
};

#endif
