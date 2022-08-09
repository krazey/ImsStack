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

#ifndef MTC_EMERGENCY_MESSAGE_FORMATTER_H_
#define MTC_EMERGENCY_MESSAGE_FORMATTER_H_

#include "IMtcService.h"
#include "call/IMtcCall.h"
#include "call/message/MessageFormatter.h"

class EmergencyMessageFormatter : public MessageFormatter
{
public:
    EmergencyMessageFormatter(IN IMtcSessionContext& objContext, IN ISession& objSession);
    virtual ~EmergencyMessageFormatter();
    EmergencyMessageFormatter(IN const MessageFormatter&) = delete;
    EmergencyMessageFormatter& operator=(IN const MessageFormatter&) = delete;

public:
    virtual IMS_RESULT FormStartMessage(IN CallType eCallType) override;

private:
    void SetPPreferredIdentityHeader();
    void SetPPreferredIdentityHeaderByUserId();
    void SetPPreferredIdentityHeaderByDeviceId();
    void SetSipInstanceFeature();

    IMS_UINT32 GetAoSRegMode(IN ServiceType eServiceType);
    IMS_RESULT GetLocalIpAddress(OUT AString& strIpAddress);
    IMS_UINT32 GetLocalPort();

private:
    IMS_UINT32 m_eNormalAosRegMode;
    IMS_UINT32 m_eEmergencyAosRegMode;
};

#endif
