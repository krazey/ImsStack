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
#ifndef SIP_PROTOCOL_H_
#define SIP_PROTOCOL_H_

#include "Protocol.h"

class SipProtocol : public Protocol
{
public:
    inline SipProtocol() :
            Protocol()
    {
    }

    ~SipProtocol() override = default;

    SipProtocol(IN const SipProtocol&) = delete;
    SipProtocol& operator=(IN const SipProtocol&) = delete;

private:
    // Protocol class
    IConnection* OpenPrim(IN const AString& strName) override;
    IConnection* OpenPrim(IN const AString& strScheme, IN const AString& strTarget,
            IN const AString& strParams) override;

    IConnection* CreateConnectionNotifier(IN IMS_SINT32 nScheme, IN IMS_SINT32 nPort,
            IN const AString& strParams, IN IMS_BOOL bSharedMode = IMS_FALSE);
};

#endif
