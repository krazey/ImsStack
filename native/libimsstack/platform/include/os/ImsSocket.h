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
#ifndef IMS_SOCKET_H_
#define IMS_SOCKET_H_

#include "ImsNetworkConnection.h"
#include "ISocket.h"

typedef IMS_SINT32 IMS_SOCKET;

// Macro definition for socket handle
#define IMS_SOCKET_MAKEPARAM(HIWORD, LOWORD) (IMS_UINT32)(((HIWORD) << 16) | (LOWORD))

#define IMS_SOCKET_HIWORD(LPARAM)            (IMS_UINT16)(((LPARAM) >> 16) & (0xFFFF))

#define IMS_SOCKET_LOWORD(LPARAM)            (IMS_UINT16)((LPARAM)&0xFFFF)

class ImsSocket : public ISocket
{
public:
    inline ImsSocket() :
            m_hConnection(0)
    {
    }
    inline virtual ~ImsSocket() {}

public:
    virtual void Destroy() = 0;
    virtual void DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) = 0;
    virtual SOCKET_RESULT Abort() = 0;
    virtual void ClosedByDataConnection() = 0;

    inline void BindNetworkConnection(IN IMS_CONNECTION hConnection)
    {
        m_hConnection = hConnection;
    }
    inline IMS_CONNECTION GetNetworkConnection() const { return m_hConnection; }

private:
    IMS_CONNECTION m_hConnection;
};

#endif
