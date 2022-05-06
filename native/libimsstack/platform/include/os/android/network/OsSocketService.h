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
#ifndef OS_SOCKET_SERVICE_H_
#define OS_SOCKET_SERVICE_H_

#include <sys/socket.h>

#include "OsMutex.h"

class OsSocketBase;
class OsSocketThread;

class OsSocketService
{
private:
    OsSocketService();

public:
    ~OsSocketService();

    OsSocketService(IN const OsSocketService&) = delete;
    OsSocketService& operator=(IN const OsSocketService&) = delete;

public:
    IMS_BOOL StartUp();
    void CleanUp();
    inline IMS_BOOL IsStarted() const { return m_bServiceStarted; }

    void AddDeadSocket(IN OsSocketBase* pSocket);
    void AttachHandle(IN SOCKET hSocket, IN OsSocketBase* pSocket);
    void DetachHandle(IN SOCKET hSocket);
    OsSocketBase* LookupHandle(IN SOCKET hSocket);
    void KillSocket(IN SOCKET hSocket);

    void SetEvent(IN SOCKET hSocket, IN IMS_SLONG nEvent);
    void RemoveEvent(IN SOCKET hSocket, IN IMS_SLONG nEvent);
    void SendControlEvent();

    static OsSocketService* GetInstance();
    void DoNotificationCallback(IN SOCKET nSocket, IN IMS_SLONG nEvent);

private:
    IMS_BOOL InitInstance();
    void ExitInstance();

    // Thread-related operations
    // IMS_ULONG RunService();
    IMS_BOOL StartService();
    void StopService();

private:
    IMS_BOOL m_bServiceStarted;
    OsSocketThread* m_pWorkerThread;
};

#endif
