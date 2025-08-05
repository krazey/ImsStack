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
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ImsFdSet.h"
#include "ImsSocketState.h"
#include "OsPthread.h"
#include "PlatformContext.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "network/OsSocket.h"
#include "network/OsSocketMsg.h"
#include "network/OsSocketService.h"

__IMS_TRACE_TAG_IPL__;

#define IMS_FD_SET ImsFdSet::TYPE_POLL

class SocketFdManager
{
public:
    SocketFdManager();
    virtual ~SocketFdManager();

    SocketFdManager(IN const SocketFdManager&) = delete;
    SocketFdManager& operator=(IN const SocketFdManager&) = delete;

public:
    IMS_SINT32 CopyFdSet(OUT ImsFdSet*& pFdSet);
    void SetFdSet(IN SOCKET hSocket, IN IMS_SINT32 nEvent);
    IMS_SINT32 ResetFdSet(IN SOCKET hSocket, IN IMS_SINT32 nEvent);

    IMS_BOOL AttachSocketHandle(IN IMS_SOCKET hSocket);
    void DetachSocketHandle(IN IMS_SOCKET hSocket);
    void DetachAllSocketHandle();
    IMS_SINT32 GetMaxSocketHandle();
    void GetSocketHandles(OUT ImsList<IMS_SOCKET>& objSocketHandles);

    // __IMS_SOCKET_EVENT__ {
    void CreateControlPipe();
    void DestroyControlPipe();
    IMS_SINT32 GetControlEventFd();
    IMS_SINT32 ResetControlEvent();
    IMS_SINT32 SetControlEvent();
    // }

    static IMS_SINT32 ConvertEvent(IN IMS_SLONG nEvent);

private:
    OsMutex m_objLockFdSet;
    ImsFdSet* m_pFdSetBackup;

    OsMutex m_objLockSocket;
    ImsList<IMS_SOCKET> m_objSockets;

    // __IMS_SOCKET_EVENT__ {
    OsMutex m_objLockCtrlEvent;
    IMS_SINT32 m_nCtrlPipe[2];
    IMS_SINT32 m_nCtrlEvent;
    // }
};

//// class SocketFdManager
PUBLIC
SocketFdManager::SocketFdManager()
{
    IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
    m_pFdSetBackup = piOsFactory->CreateFdSet(IMS_FD_SET);

    // __IMS_SOCKET_EVENT__ {
    m_nCtrlPipe[0] = (-1);
    m_nCtrlPipe[1] = (-1);
    m_nCtrlEvent = 0;
    // }
}

PUBLIC VIRTUAL SocketFdManager::~SocketFdManager()
{
    // __IMS_SOCKET_EVENT__ {
    DestroyControlPipe();
    // }

    DetachAllSocketHandle();

    if (m_pFdSetBackup != IMS_NULL)
    {
        delete m_pFdSetBackup;
    }
}

PUBLIC
IMS_SINT32 SocketFdManager::CopyFdSet(OUT ImsFdSet*& pFdSet)
{
    IMS_SINT32 nHighestFd = 0;

    if (m_pFdSetBackup->IsHighestFdRequired())
    {
        nHighestFd = GetControlEventFd();
        IMS_SINT32 nSocketHandle = GetMaxSocketHandle();

        if (nSocketHandle > nHighestFd)
        {
            nHighestFd = nSocketHandle;
        }
    }

    m_objLockFdSet.Lock();

    m_pFdSetBackup->SetHighestFd(nHighestFd);
    pFdSet->CopyFrom(m_pFdSetBackup);

    m_objLockFdSet.Unlock();

    return nHighestFd;
}

PUBLIC
void SocketFdManager::SetFdSet(IN SOCKET hSocket, IN IMS_SINT32 nEvent)
{
    m_objLockFdSet.Lock();

    m_pFdSetBackup->SetEvent(hSocket, nEvent);

    m_objLockFdSet.Unlock();

    // __IMS_SOCKET_EVENT__ {
    // It will be controlled by the OsSocket
    // SetControlEvent();
    // }
}

PUBLIC
IMS_SINT32 SocketFdManager::ResetFdSet(IN SOCKET hSocket, IN IMS_SINT32 nEvent)
{
    IMS_SINT32 nResetEvent = 0;

    m_objLockFdSet.Lock();

    nResetEvent = m_pFdSetBackup->ClearEvent(hSocket, nEvent);

    m_objLockFdSet.Unlock();

    // __IMS_SOCKET_EVENT__ {
    // It will be controlled by the OsSocket
    // SetControlEvent();
    // }

    return nResetEvent;
}

PUBLIC
IMS_BOOL SocketFdManager::AttachSocketHandle(IN IMS_SOCKET hSocket)
{
    m_objLockSocket.Lock();

    for (IMS_UINT32 i = 0; i < m_objSockets.GetSize(); ++i)
    {
        IMS_SOCKET hTmpSocket = m_objSockets.GetAt(i);

        if (hTmpSocket == hSocket)
        {
            m_objLockSocket.Unlock();
            return IMS_FALSE;
        }
    }

    IMS_BOOL bResult = m_objSockets.Append(hSocket);

    m_objLockSocket.Unlock();

    return bResult;
}

PUBLIC
void SocketFdManager::DetachSocketHandle(IN IMS_SOCKET hSocket)
{
    m_objLockSocket.Lock();

    for (IMS_UINT32 i = 0; i < m_objSockets.GetSize(); ++i)
    {
        IMS_SOCKET hTmpSocket = m_objSockets.GetAt(i);

        if (hTmpSocket == hSocket)
        {
            m_objSockets.RemoveAt(i);
            m_objLockSocket.Unlock();

            if (ResetFdSet(hSocket, ImsFdSet::EVENT_ALL) != 0)
            {
                IMS_TRACE_D("FATAL :: Socket (%d) event is cleared by detach", hSocket, 0, 0);
            }
            return;
        }
    }

    m_objLockSocket.Unlock();
}

PUBLIC
void SocketFdManager::DetachAllSocketHandle()
{
    m_objLockSocket.Lock();

    m_objSockets.Clear();

    m_objLockSocket.Unlock();
}

PUBLIC
IMS_SINT32 SocketFdManager::GetMaxSocketHandle()
{
    IMS_SOCKET hMaxSocket = 0;

    m_objLockSocket.Lock();

    for (IMS_UINT32 i = 0; i < m_objSockets.GetSize(); ++i)
    {
        IMS_SOCKET hSocket = m_objSockets.GetAt(i);

        if (hSocket > hMaxSocket)
        {
            hMaxSocket = hSocket;
        }
    }

    m_objLockSocket.Unlock();

    return static_cast<IMS_SINT32>(hMaxSocket);
}

PUBLIC
void SocketFdManager::GetSocketHandles(OUT ImsList<IMS_SOCKET>& objSocketHandles)
{
    m_objLockSocket.Lock();

    objSocketHandles = m_objSockets;

    m_objLockSocket.Unlock();
}

// __IMS_SOCKET_EVENT__ {
PUBLIC
void SocketFdManager::CreateControlPipe()
{
    DestroyControlPipe();

    m_objLockCtrlEvent.Lock();

    if (pipe2(m_nCtrlPipe, O_CLOEXEC | O_NONBLOCK) < 0)
    {
        IMS_TRACE_E(0, "[SocketCtrlEvent] Creating a control pipe failed (%d)", errno, 0, 0);

        m_objLockCtrlEvent.Unlock();
        return;
    }

    IMS_SINT32 nReadFd = (-1);

    IMS_TRACE_D("[SocketCtrlEvent] READ (%d), WRITE (%d)", m_nCtrlPipe[0], m_nCtrlPipe[1], 0);

    // PIPE - read end
    if (m_nCtrlPipe[0] != (-1))
    {
        nReadFd = m_nCtrlPipe[0];
    }

    m_objLockCtrlEvent.Unlock();

    if (nReadFd >= 0)
    {
        m_pFdSetBackup->SetEvent(nReadFd, ImsFdSet::EVENT_READ);
    }
}

PUBLIC
void SocketFdManager::DestroyControlPipe()
{
    IMS_SINT32 nReadFd = (-1);

    m_objLockCtrlEvent.Lock();

    if (m_nCtrlPipe[0] != (-1))
    {
        nReadFd = m_nCtrlPipe[0];

        close(m_nCtrlPipe[0]);
        m_nCtrlPipe[0] = (-1);
    }

    if (m_nCtrlPipe[1] != (-1))
    {
        close(m_nCtrlPipe[1]);
        m_nCtrlPipe[1] = (-1);
    }

    m_objLockCtrlEvent.Unlock();

    if (nReadFd >= 0)
    {
        m_pFdSetBackup->ClearEvent(nReadFd, ImsFdSet::EVENT_READ);
    }
}

PUBLIC
IMS_SINT32 SocketFdManager::GetControlEventFd()
{
    IMS_SINT32 nFd;

    m_objLockCtrlEvent.Lock();

    // READ FD
    nFd = m_nCtrlPipe[0];

    m_objLockCtrlEvent.Unlock();

    return nFd;
}

PUBLIC
IMS_SINT32 SocketFdManager::ResetControlEvent()
{
    // RECOVER_CONTROL_PIPE
    IMS_SINT32 nError = 0;  // No error
    IMS_CHAR acBuffer[2] = {'\0', '\0'};

    m_objLockCtrlEvent.Lock();

    if (m_nCtrlPipe[0] == -1)
    {
        IMS_TRACE_E(
                0, "[SocketCtrlEvent] Can't reset the control event (PIPE is invalid)", 0, 0, 0);
        goto EXIT_ResetControlEvent;
    }

    if ((m_nCtrlEvent & 0x01) != 0x01)
    {
        // Comment the below trace later
        IMS_TRACE_D("[SocketCtrlEvent] No control event (%d)", m_nCtrlEvent, 0, 0);
        goto EXIT_ResetControlEvent;
    }

    if (read(m_nCtrlPipe[0], acBuffer, 1) <= 0)
    {
        IMS_TRACE_E(0, "[SocketCtrlEvent] Reading a control event failed (%d)", errno, 0, 0);

        if (errno == EBADF)
        {
            nError = -1;
        }

        goto EXIT_ResetControlEvent;
    }

    acBuffer[1] = '\0';

    if (acBuffer[0] != 1)
    {
        IMS_TRACE_E(0, "[SocketCtrlEvent] Control event is not matched (%d, %d)", acBuffer[0],
                acBuffer[1], 0);
        goto EXIT_ResetControlEvent;
    }

#ifdef __IMS_DEBUG__
    IMS_TRACE_D("[SocketCtrlEvent] RESET (%d)", acBuffer[0], 0, 0);
#endif

    m_nCtrlEvent = 0;

EXIT_ResetControlEvent:
    m_objLockCtrlEvent.Unlock();

    return nError;
}

PUBLIC
IMS_SINT32 SocketFdManager::SetControlEvent()
{
    static const IMS_CHAR c = 1;
    // RECOVER_CONTROL_PIPE
    IMS_SINT32 nError = 0;  // No error

    m_objLockCtrlEvent.Lock();

    if ((m_nCtrlEvent & 0x01) == 0x01)
    {
        // Comment the below trace later
        IMS_TRACE_D("[SocketCtrlEvent] Control event (%d) is already set", m_nCtrlEvent, 0, 0);
        goto EXIT_SetControlEvent;
    }

    if (m_nCtrlPipe[1] == -1)
    {
        IMS_TRACE_E(0, "[SocketCtrlEvent] Can't set the control event (PIPE is invalid)", 0, 0, 0);
        goto EXIT_SetControlEvent;
    }

    if (write(m_nCtrlPipe[1], &c, 1) != 1)
    {
        IMS_TRACE_E(0, "[SocketCtrlEvent] Writing a control event failed (%d)", errno, 0, 0);

        if (errno == EBADF)
        {
            nError = -1;
        }

        goto EXIT_SetControlEvent;
    }

    m_nCtrlEvent |= 0x01;

#ifdef __IMS_DEBUG__
    IMS_TRACE_D("[SocketCtrlEvent] SET (%d)", c, 0, 0);
#endif

EXIT_SetControlEvent:
    m_objLockCtrlEvent.Unlock();

    return nError;
}
// __IMS_SOCKET_EVENT__ }

PUBLIC GLOBAL IMS_SINT32 SocketFdManager::ConvertEvent(IN IMS_SLONG nEvent)
{
    IMS_SINT32 nConvertedEvent = 0;

    if ((nEvent & FD_WRITE) != 0)
    {
        nConvertedEvent |= ImsFdSet::EVENT_WRITE;
    }

    if ((nEvent & FD_READ) != 0)
    {
        nConvertedEvent |= ImsFdSet::EVENT_READ;
    }

    if ((nEvent & FD_CLOSE) != 0)
    {
        nConvertedEvent |= ImsFdSet::EVENT_EXCEPT;
    }

    if ((nEvent & FD_CONNECT) != 0)
    {
        nConvertedEvent |= ImsFdSet::EVENT_WRITE;
    }

    if ((nEvent & FD_ACCEPT) != 0)
    {
        nConvertedEvent |= ImsFdSet::EVENT_READ;
    }

    if ((nEvent & FD_TCP_C) != 0)
    {
        nConvertedEvent |= ImsFdSet::EVENT_TCP_C;
    }

    if ((nEvent & FD_TCP) != 0)
    {
        nConvertedEvent |= ImsFdSet::EVENT_TCP;
    }

    return nConvertedEvent;
}

class OsSocketThread : public IThreadImpListener
{
public:
    explicit OsSocketThread(IN OsSocketService* pService);
    ~OsSocketThread() override;

    OsSocketThread(IN const OsSocketThread&) = delete;
    OsSocketThread& operator=(IN const OsSocketThread&) = delete;

public:
    void RunImp() override;

    SocketFdManager& GetSocketFdManager();
    void StartUp();
    void CleanUp();

private:
    OsSocketService* m_pSocketService;
    OsPthread* m_pThread;
    SocketFdManager m_objFdMngr;
};

//// class OsSocketThread
PUBLIC
OsSocketThread::OsSocketThread(IN OsSocketService* pService) :
        m_pSocketService(pService),
        m_pThread(IMS_NULL)
{
}

PUBLIC VIRTUAL OsSocketThread::~OsSocketThread()
{
    CleanUp();
}

PUBLIC VIRTUAL void OsSocketThread::RunImp()
{
    IMS_BOOL bLoop = IMS_TRUE;
    IMS_SINT32 nMaxEvent;
    IMS_SINT32 nSignaledCount = 0;
    IMS_SINT32 nSignaledEvents = 0;
    IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
    ImsFdSet* pFdSet = piOsFactory->CreateFdSet(IMS_FD_SET);
    ImsList<IMS_SOCKET> objSocketHandles;

    while (bLoop)
    {
        // __IMS_SOCKET_EVENT__ {
        if (m_objFdMngr.ResetControlEvent() < 0)
        {
            // RECOVER_CONTROL_PIPE
            IMS_TRACE_I("SocketService :: Control pipe will be re-created", 0, 0, 0);
            m_objFdMngr.DestroyControlPipe();
            m_objFdMngr.CreateControlPipe();
        }

        // Updates FD sets
        m_objFdMngr.CopyFdSet(pFdSet);

        // 4 NOTICE : we can use FD_SETSIZE constant, instead of nMaxSockFD
        nMaxEvent = pFdSet->WaitForEvents();
        // }

        if (m_pSocketService == IMS_NULL)
        {
            IMS_TRACE_E(0, "Socket service is null", 0, 0, 0);
            continue;
        }

        if (nMaxEvent > 0)
        {
            if (!m_pSocketService->IsStarted())
            {
                IMS_TRACE_D("Socket service is not started", 0, 0, 0);
                bLoop = IMS_FALSE;
            }

            // __IMS_SOCKET_EVENT__ {
            nSignaledEvents =
                    pFdSet->GetSignaledEvents(m_objFdMngr.GetControlEventFd(), nSignaledCount);
            nMaxEvent -= nSignaledCount;

            if (ImsFdSet::IsReadEventSignaled(nSignaledEvents))
            {
                // Destroy the dead socket objects
                ImsSocketState::GetInstance()->DestroyDeadSockets();

                if (nMaxEvent <= 0)
                {
                    IMS_TRACE_D("[SocketCtrlEvent] READ only enabled", 0, 0, 0);
                    continue;
                }
                else
                {
#ifdef __IMS_DEBUG__
                    IMS_TRACE_D("[SocketCtrlEvent] READ enabled", 0, 0, 0);
#endif
                }
            }
            // }

            m_objFdMngr.GetSocketHandles(objSocketHandles);

            for (IMS_UINT32 i = 0; i < objSocketHandles.GetSize(); ++i)
            {
                IMS_SOCKET hSocket = objSocketHandles.GetAt(i);

                nSignaledEvents = pFdSet->GetSignaledEvents(hSocket, nSignaledCount);
                nMaxEvent -= nSignaledCount;

                if (ImsFdSet::IsWriteEventSignaled(nSignaledEvents))
                {
                    m_pSocketService->DoNotificationCallback(hSocket, IMS_SOCKET_SEND_ENABLED);
                }

                if (ImsFdSet::IsReadEventSignaled(nSignaledEvents))
                {
                    m_pSocketService->DoNotificationCallback(hSocket, IMS_SOCKET_DATA_RECEIVED);
                }

                if (ImsFdSet::IsExceptEventSignaled(nSignaledEvents))
                {
                    m_pSocketService->DoNotificationCallback(hSocket, IMS_SOCKET_CLOSED);
                }

                if (nMaxEvent <= 0)
                {
                    break;
                }
            }
        }
        else
        {
            IMS_TRACE_D("Socket worker thread :: select (%d), errno (%d)", nMaxEvent, errno, 0);
        }
    }

    delete pFdSet;

    if (m_pSocketService != IMS_NULL)
    {
        m_pSocketService->CleanUp();
    }
}

PUBLIC
SocketFdManager& OsSocketThread::GetSocketFdManager()
{
    return m_objFdMngr;
}

PUBLIC
void OsSocketThread::StartUp()
{
    // __IMS_SOCKET_EVENT__ {
    m_objFdMngr.CreateControlPipe();
    // }

    m_pThread = new OsPthread();

    IMS_ASSERT(m_pThread != IMS_NULL);

    if (m_pThread != IMS_NULL)
    {
        m_pThread->Create("ImsSocketService");
        m_pThread->SetImpListener(this);
        m_pThread->Activate();
    }
}

PUBLIC
void OsSocketThread::CleanUp()
{
    if (m_pThread != IMS_NULL)
    {
        m_pThread->Deactivate();

        delete m_pThread;
        m_pThread = IMS_NULL;
    }

    // __IMS_SOCKET_EVENT__ {
    m_objFdMngr.DestroyControlPipe();
    // }
}

//// class OsSocketService
PUBLIC
OsSocketService::OsSocketService() :
        m_bServiceStarted(IMS_FALSE),
        m_pWorkerThread(IMS_NULL)
{
    m_pWorkerThread = new OsSocketThread(this);
}

PUBLIC VIRTUAL OsSocketService::~OsSocketService()
{
    ExitInstance();

    if (m_pWorkerThread != IMS_NULL)
    {
        delete m_pWorkerThread;
        m_pWorkerThread = IMS_NULL;
    }
}

PUBLIC
IMS_BOOL OsSocketService::StartUp()
{
    if (!InitInstance())
    {
        ExitInstance();
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
void OsSocketService::CleanUp()
{
    IMS_TRACE_D("SocketService :: CleanUp", 0, 0, 0);

    ExitInstance();
}

PUBLIC
void OsSocketService::AddDeadSocket(IN OsSocketBase* pSocket)
{
    ImsSocketState::GetInstance()->AddDeadSocket(pSocket);
}

PUBLIC
void OsSocketService::AttachHandle(IN SOCKET hSocket, IN OsSocketBase* pSocket)
{
    if (LookupHandle(hSocket) != IMS_NULL)
    {
        IMS_TRACE_D("SocketService :: Socket (%d) already exists", hSocket, 0, 0);
        return;
    }

    ImsSocketState* pState = ImsSocketState::GetInstance();
    pState->AttachHandle(static_cast<IMS_SOCKET>(hSocket), pSocket);

    if (m_pWorkerThread == IMS_NULL)
    {
        return;
    }

    m_pWorkerThread->GetSocketFdManager().AttachSocketHandle(hSocket);
}

PUBLIC
void OsSocketService::DetachHandle(IN SOCKET hSocket)
{
    ImsSocketState* pState = ImsSocketState::GetInstance();

    pState->DetachHandle(static_cast<IMS_SOCKET>(hSocket));

    if (m_pWorkerThread == IMS_NULL)
    {
        return;
    }

    m_pWorkerThread->GetSocketFdManager().DetachSocketHandle(hSocket);
}

PUBLIC
OsSocketBase* OsSocketService::LookupHandle(IN SOCKET hSocket)
{
    ImsSocketState* pState = ImsSocketState::GetInstance();

    return DYNAMIC_CAST(OsSocketBase*, pState->LookupHandle(static_cast<IMS_SOCKET>(hSocket)));
}

PUBLIC
void OsSocketService::KillSocket(IN SOCKET hSocket)
{
    if (LookupHandle(hSocket) == IMS_NULL)
    {
        IMS_TRACE_D("KillSocket :: Socket (%d) is already dead", hSocket, 0, 0);
        return;
    }

    DetachHandle(hSocket);
}

PUBLIC
OsSocketService* OsSocketService::GetInstance()
{
    static OsSocketService* s_pService = IMS_NULL;

    if (s_pService == IMS_NULL)
    {
        s_pService = new OsSocketService();
    }

    return s_pService;
}

PRIVATE
IMS_BOOL OsSocketService::InitInstance()
{
    // Start socket worker thread
    if (!StartService())
    {
        IMS_TRACE_E(0, "Initializing a socket service failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
void OsSocketService::ExitInstance()
{
    ImsSocketState::GetInstance()->DetachAll();
    ImsSocketState::GetInstance()->DestroyDeadSockets();
    StopService();
}

PUBLIC
void OsSocketService::SetEvent(IN SOCKET hSocket, IN IMS_SLONG nEvent)
{
    if (m_pWorkerThread == IMS_NULL)
    {
        return;
    }

    SocketFdManager& objFdMngr = m_pWorkerThread->GetSocketFdManager();

    objFdMngr.SetFdSet(hSocket, SocketFdManager::ConvertEvent(nEvent));
}

PUBLIC
void OsSocketService::RemoveEvent(IN SOCKET hSocket, IN IMS_SLONG nEvent)
{
    if (m_pWorkerThread == IMS_NULL)
    {
        return;
    }

    SocketFdManager& objFdMngr = m_pWorkerThread->GetSocketFdManager();

    objFdMngr.ResetFdSet(hSocket, SocketFdManager::ConvertEvent(nEvent));
}

PUBLIC
void OsSocketService::SendControlEvent()
{
    // __IMS_SOCKET_EVENT__ {
    if (m_pWorkerThread == IMS_NULL)
    {
        return;
    }

    if (m_pWorkerThread->GetSocketFdManager().SetControlEvent() < 0)
    {
        // RECOVER_CONTROL_PIPE
        IMS_TRACE_I("SocketService :: Control pipe will be re-created", 0, 0, 0);

        m_pWorkerThread->GetSocketFdManager().DestroyControlPipe();
        m_pWorkerThread->GetSocketFdManager().CreateControlPipe();

        // Try to set the control event again
        m_pWorkerThread->GetSocketFdManager().SetControlEvent();
    }
    // }
}

PRIVATE
void OsSocketService::DoNotificationCallback(IN SOCKET nSocket, IN IMS_SLONG nEvent)
{
    if (nSocket == INVALID_SOCKET)
    {
        IMS_TRACE_E(0, "Invalid Socket", 0, 0, 0);
        return;
    }

    OsSocketBase* pSocket = LookupHandle(nSocket);

    if (pSocket == IMS_NULL)
    {
        // MUST be in the middle of an Accept call
        IMS_TRACE_E(0, "Socket(%d) is null", nSocket, 0, 0);
        return;
    }

    // Set Error Code
    IMS_SINT32 nErrorCode = pSocket->GetLastError();

    if (nErrorCode != 0)
    {
        IMS_TRACE_D("DoNotificationCallback - socket=%d, error=%d", nSocket, nErrorCode, 0);
    }

    switch (nEvent)
    {
        case IMS_SOCKET_DATA_RECEIVED:
        {
            if (pSocket->GetSocketType() == ISocket::TYPE_DGRAM)
            {
                pSocket->NotifyDataReceived(nErrorCode);
            }
            else if (pSocket->GetSocketType() == ISocket::TYPE_STREAM)
            {
                if (pSocket->IsServerSocket())
                {
                    // 2 Clarify the below logic
                    pSocket->NotifyConnectionReceived(nErrorCode);
                }
                else
                {
                    IMS_SINT32 nState = pSocket->GetSocketState();

                    if (nState == OsSocketBase::SOCKET_STATE_CLOSED)
                    {
                        OsSocketBase* pTmpSocket = LookupHandle(nSocket);

                        if (pTmpSocket == IMS_NULL)
                        {
                            IMS_TRACE_E(0,
                                    "TCP socket (%d) is already dead "
                                    "on IMS_SOCKET_DATA_RECEIVED",
                                    nSocket, 0, 0);
                        }
                        else
                        {
                            pTmpSocket->NotifyClosed(nErrorCode);
                        }
                    }
                    else if (nState == OsSocketBase::SOCKET_STATE_DATA_AVAILABLE)
                    {
                        if (nErrorCode != 0)
                        {
                            // hwangoo park
                            pSocket->NotifyDataReceived(0);
                            IMS_TRACE_D("Notify data available (PSH, FIN) "
                                        "even though error occurred (%d)",
                                    nErrorCode, 0, 0);

                            pSocket->NotifyClosed(nErrorCode);
                        }
                        else
                        {
                            pSocket->NotifyDataReceived(nErrorCode);
                        }
                    }
                    else
                    {
                        // just ignore the socket event
                    }
                }
            }
            break;
        }
        case IMS_SOCKET_SEND_ENABLED:
        {
            if (pSocket->GetSocketType() == ISocket::TYPE_DGRAM)
            {
                pSocket->NotifySendEnabled(nErrorCode);
            }
            else if (pSocket->GetSocketType() == ISocket::TYPE_STREAM)
            {
                if (!pSocket->IsServerSocket())
                {
                    if (!pSocket->IsSocketConnected())
                    {
                        pSocket->NotifyConnected(nErrorCode);
                    }
                    else
                    {
                        pSocket->NotifySendEnabled(nErrorCode);
                    }
                }
                else
                {
                    pSocket->NotifySendEnabled(nErrorCode);
                }
            }
            break;
        }
        case IMS_SOCKET_CLOSED:
        {
            OsSocketBase* pTmpSocket = LookupHandle(nSocket);

            if (pTmpSocket == IMS_NULL)
            {
                IMS_TRACE_E(
                        0, "TCP socket (%d) is already dead on IMS_SOCKET_CLOSED", nSocket, 0, 0);
                break;
            }

            pTmpSocket->NotifyClosed(nErrorCode);
            break;
        }
        default:
        {
            break;
        }
    }
}

PRIVATE
IMS_BOOL OsSocketService::StartService()
{
    if (m_bServiceStarted)
    {
        IMS_TRACE_D("Socket service is already started", 0, 0, 0);
        return IMS_TRUE;
    }

    if (m_pWorkerThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "WorkerThread is null", 0, 0, 0);
        return IMS_FALSE;
    }

    m_pWorkerThread->StartUp();
    m_bServiceStarted = IMS_TRUE;

    return IMS_TRUE;
}

PRIVATE
void OsSocketService::StopService()
{
    if (!m_bServiceStarted)
    {
        IMS_TRACE_D("Socket service is not started", 0, 0, 0);
        return;
    }

    m_bServiceStarted = IMS_FALSE;
}
