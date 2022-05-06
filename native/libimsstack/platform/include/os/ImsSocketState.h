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
#ifndef IMS_SOCKET_STATE_H_
#define IMS_SOCKET_STATE_H_

#include "IMSList.h"
#include "IMSMap.h"
#include "ImsSocket.h"

class IMutex;

GLOBAL void ImsSocketState_ExitInstance();
GLOBAL IMS_BOOL ImsSocketState_InitInstance();

class ImsSocketState
{
private:
    ImsSocketState();

public:
    ~ImsSocketState();

    ImsSocketState(IN const ImsSocketState&) = delete;
    ImsSocketState& operator=(IN const ImsSocketState&) = delete;

public:
    void AddDeadSocket(IN ImsSocket* pSocket);
    void DestroyDeadSockets();

    void AttachHandle(IN IMS_SOCKET hSocket, IN ImsSocket* pSocket);
    void DetachAll();
    void DetachAll(IN IMS_CONNECTION hConnection);
    void DetachHandle(IN IMS_SOCKET hSocket);
    inline const IMSMap<IMS_SOCKET, ImsSocket*>& GetHandle2ObjectMap() const
    {
        return m_objHandle2Object;
    }
    ImsSocket* LookupHandle(IN IMS_SOCKET hSocket);
    IMS_BOOL IsEmpty() const;

    static ImsSocketState* GetInstance();

private:
    friend void ImsSocketState_ExitInstance();
    friend IMS_BOOL ImsSocketState_InitInstance();

private:
    IMutex* m_piLock;

    // List of map (IMS_SOCKET, ImsSocket)
    IMSMap<IMS_SOCKET, ImsSocket*> m_objHandle2Object;
    // List of ImsSocket
    IMSList<ImsSocket*> m_objDeadSockets;
};

#endif
