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
#ifndef INTERFACE_SIP_STREAM_SOCKET_LISTENER_H_
#define INTERFACE_SIP_STREAM_SOCKET_LISTENER_H_

#include "ByteArray.h"

class SipSocket;

class ISipStreamSocketListener
{
protected:
    virtual ~ISipStreamSocketListener() = default;

public:
    virtual void StreamSocket_ConnectionReceived(IN SipSocket* pSocket) = 0;
    virtual void StreamSocket_DataReceived(IN SipSocket* pSocket, IN_OUT ByteArray& objBuffer) = 0;
    virtual void StreamSocket_KeepAliveExpired(IN SipSocket* pSocket) = 0;
    virtual void StreamSocket_PassiveClosed(IN SipSocket* pSocket) = 0;
};

#endif
