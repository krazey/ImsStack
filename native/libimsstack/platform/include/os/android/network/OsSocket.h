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
#ifndef OS_SOCKET_H_
#define OS_SOCKET_H_

#include "OsSocketDef.h"
#include "OsSocketBase.h"

class IThread;

class OsSocket : public OsSocketBase
{
public:
    OsSocket();
    virtual ~OsSocket();

    OsSocket(IN const OsSocket&) = delete;
    OsSocket& operator=(IN const OsSocket&) = delete;

public:
    IMS_SINT32 GetLastError() const override;
    IMS_SINT32 GetSocketState() const override;

    // FIX_TIMING_ISSUE
    inline IMS_UINT16 GetInternalSocketId() const { return m_nInternalSocketId; }

    static IMS_BOOL StartUp();
    static void CleanUp();
    static IMS_BOOL CheckIpAndPortAvailability(
            IN const IpAddress& objIpAddr, IN IMS_SINT32 nPort, IN SOCKET_ENTYPE enType);

protected:
    // ISocket class
    inline IMS_SINT32 GetSocketId() const override { return m_hSocket; }
    SOCKET_RESULT Open(IN SOCKET_ENTYPE eType, IN ISocketListener* piListener,
            IN ADDRESS_FAMILY_ENTYPE eAddressFamily = ADDRESS_FAMILY_INET) override;
    SOCKET_RESULT Open(IN SOCKET_ENTYPE eType,
            IN ADDRESS_FAMILY_ENTYPE eAddressFamily = ADDRESS_FAMILY_INET) override;
    SOCKET_RESULT Close() override final;
    void SetListener(IN ISocketListener* piListener) override;
    ISocket* Accept() override;
    SOCKET_RESULT Bind(IN const IpAddress& objSocketAddress, IN IMS_UINT32 nSocketPort) override;
    SOCKET_RESULT Connect(IN const IpAddress& objHostAddress, IN IMS_UINT32 nHostPort) override;
    SOCKET_RESULT Listen(IN IMS_SINT32 nBackLog = MAX_BACKLOG
            /*Maximum length of the queue of pending connections*/) override;
    IMS_SINT32 Receive(OUT IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen) override;
    IMS_SINT32 Send(IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen) override;
    IMS_SINT32 ReceiveFrom(OUT IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen,
            OUT IpAddress& objHostAddress, OUT IMS_UINT32& nHostPort) override;
    IMS_SINT32 SendTo(IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen,
            IN const IpAddress& objHostAddress, IN IMS_UINT32 nHostPort) override;
    SOCKET_RESULT GetPeerName(OUT IpAddress& objPeerAddress, OUT IMS_UINT32& nPeerPort) override;
    SOCKET_RESULT GetSockName(
            OUT IpAddress& objSocketAddress, OUT IMS_UINT32& nSocketPort) override;
    IMS_BOOL Equals(IN const ISocket* piSocket) override;
    IMS_SINT32 GetOption(IN IMS_SINT32 nOption) override;
    IMS_BOOL SetOption(IN IMS_SINT32 nOption, IN IMS_SINT32 nOptionValue) override;
    IMS_BOOL IsClosedOrBeingClosed() const override
    {
        return GetCloseReason() != CLOSE_REASON_UNKNOWN;
    }

    // ImsSocket class
    void Destroy() override;
    void DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) override;
    SOCKET_RESULT Abort() override;
    void ClosedByDataConnection() override;

    // OsSocketBase class
    void NotifyDataReceived(IN IMS_SINT32 nErrorCode) override;
    void NotifySendEnabled(IN IMS_SINT32 nErrorCode) override;
    void NotifyConnectionReceived(IN IMS_SINT32 nErrorCode) override;
    void NotifyConnected(IN IMS_SINT32 nErrorCode) override;
    void NotifyClosed(IN IMS_SINT32 nErrorCode) override;
    void NotifyAcceptCompleted(IN IMS_SOCKET hSocket) override;

    virtual OsSocket* CreateSocket();
    virtual IMS_BOOL ShutDown(IN IMS_SINT32 nHow = SHUTDOWN_BOTH);

    inline IMS_SINT32 GetOptionForShutdown() const { return m_nOptionForShutdown; }
    IMS_BOOL SelectEvent(IN IMS_SLONG nEvent = EVENT_FD_ALL);
    IMS_BOOL SelectEventEx(IN IMS_SLONG nEvent);
    IMS_BOOL DeselectEventEx(IN IMS_SLONG nEvent);
    void NotifyMessage(IN IMS_SINT32 nMsgParam);
    IMS_SINT32 GetCloseReason() const;
    void SetCloseReason(IN IMS_SINT32 nReason);
    IMS_SOCKET GetSocket() const;
    SOCKET_RESULT DoSocketRecovery();
    void BindSocketToIpSecTransform();
    void UnbindSocketFromIpSecTransform(IN IMS_SOCKET hSocket);

#ifdef _DEBUG
    void OutputDebugString(
            IN IMS_SINT32 nErrorCode, IN const IMS_CHAR* pszModule, IN IMS_SINT32 nLine);
#endif

private:
    // FIX_TIMING_ISSUE
    static IMS_UINT16 s_nInternalSocketId;

    IMS_SOCKET m_hSocket;
    ADDRESS_FAMILY_ENTYPE m_eAddressFamily;
    IMS_SLONG m_nSocketEvent;

    IThread* m_piOwnerThread;
    ISocketListener* m_piListener;

    // For close reason
    IMS_SINT32 m_nCloseReason;
    IMS_SINT32 m_nOptionForShutdown;

    // PATCH_FOR_NON_SOCKET
    IpAddress m_objSocketAddress;
    IMS_UINT32 m_nSocketPort;

    // FIX_TIMING_ISSUE
    IMS_UINT16 m_nInternalSocketId;
};

#endif
