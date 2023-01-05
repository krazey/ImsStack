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
#ifndef INTERFACE_SESSION_STATE_H_
#define INTERFACE_SESSION_STATE_H_

#include "AString.h"

class SdpSessionParameter;

class ISessionState
{
protected:
    virtual ~ISessionState() = default;

public:
    virtual const AString& GetConnectionAddress() const = 0;
    virtual IMS_SINT32 GetSessionState() const = 0;
    virtual SdpSessionParameter* GetSessionParameter() const = 0;
    virtual const AString& GetPeerConnectionAddress() const = 0;
    virtual SdpSessionParameter* GetPeerSessionParameter() const = 0;
    virtual SdpSessionParameter* GetProposalSessionParameter() = 0;

public:
    /// Types of main session state
    enum
    {
        SESSION_STATE_CREATED = 0,
        SESSION_STATE_INITIATED = 1,
        SESSION_STATE_NEGOTIATING = 2,
        SESSION_STATE_ESTABLISHING = 3,
        SESSION_STATE_ESTABLISHED = 4,
        SESSION_STATE_RENEGOTIATING = 5,
        SESSION_STATE_REESTABLISHING = 6,
        SESSION_STATE_TERMINATING = 7,
        SESSION_STATE_TERMINATED = 8
    };
};

#endif
