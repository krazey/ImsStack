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
#ifndef MOCK_I_SOCKET_H_
#define MOCK_I_SOCKET_H_

#include <gmock/gmock.h>

#include "ISocket.h"

class MockISocket : public ISocket
{
public:
    inline MockISocket() {}
    inline virtual ~MockISocket() {}

    MOCK_METHOD(IMS_SINT32, GetSocketId, (), (const, override));
    MOCK_METHOD(SOCKET_ENTYPE, GetSocketType, (), (const, override));
    MOCK_METHOD(SOCKET_RESULT, Open,
            (IN SOCKET_ENTYPE eType, IN ISocketListener* piListener,
                    IN ADDRESS_FAMILY_ENTYPE eAddressFamily),
            (override));
    MOCK_METHOD(SOCKET_RESULT, Open,
            (IN SOCKET_ENTYPE eType, IN ADDRESS_FAMILY_ENTYPE eAddressFamily), (override));
    MOCK_METHOD(void, SetListener, (IN ISocketListener * piListener), (override));
    MOCK_METHOD(SOCKET_RESULT, Close, (), (override));
    MOCK_METHOD(ISocket*, Accept, (), (override));
    MOCK_METHOD(SOCKET_RESULT, Bind,
            (IN const IpAddress& objSocketAddress, IN IMS_UINT32 nSocketPort), (override));
    MOCK_METHOD(SOCKET_RESULT, Connect,
            (IN const IpAddress& objHostAddress, IN IMS_UINT32 nHostPort), (override));
    MOCK_METHOD(SOCKET_RESULT, Listen, (IN IMS_SINT32 nBackLog), (override));
    MOCK_METHOD(IMS_SINT32, Receive, (OUT IMS_BYTE * pBuffer, IN IMS_SINT32 nBuffLen), (override));
    MOCK_METHOD(IMS_SINT32, Send, (IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen), (override));
    MOCK_METHOD(IMS_SINT32, ReceiveFrom,
            (OUT IMS_BYTE * pBuffer, IN IMS_SINT32 nBuffLen, OUT IpAddress& objHostAddress,
                    OUT IMS_UINT32& nHostPort),
            (override));
    MOCK_METHOD(IMS_SINT32, SendTo,
            (IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen, IN const IpAddress& objHostAddress,
                    IN IMS_UINT32 nHostPort),
            (override));
    MOCK_METHOD(SOCKET_RESULT, GetPeerName,
            (OUT IpAddress & objPeerAddress, OUT IMS_UINT32& nPeerPort), (override));
    MOCK_METHOD(SOCKET_RESULT, GetSockName,
            (OUT IpAddress & objSocketAddress, OUT IMS_UINT32& nSocketPort), (override));
    MOCK_METHOD(IMS_BOOL, Equals, (IN const ISocket* piSocket), (override));
    MOCK_METHOD(IMS_SINT32, GetOption, (IN IMS_SINT32 nOption), (override));
    MOCK_METHOD(
            IMS_BOOL, SetOption, (IN IMS_SINT32 nOption, IN IMS_SINT32 nOptionValue), (override));
    MOCK_METHOD(IMS_BOOL, IsClosedOrBeingClosed, (), (const, override));
};

class MockISocketListener : public ISocketListener
{
public:
    inline MockISocketListener() {}
    inline virtual ~MockISocketListener() {}

    MOCK_METHOD(void, Socket_OnDataReceived, (IN ISocket * piSocket), (override));
    MOCK_METHOD(void, Socket_OnSendEnabled, (IN ISocket * piSocket), (override));
    MOCK_METHOD(void, Socket_OnConnectionReceived, (IN ISocket * piSocket), (override));
    MOCK_METHOD(void, Socket_OnConnected, (IN ISocket * piSocket), (override));
    MOCK_METHOD(void, Socket_OnClosed, (IN ISocket * piSocket, IN IMS_SINT32 nReason), (override));
};

#endif
