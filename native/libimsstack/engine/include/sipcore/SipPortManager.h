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
#ifndef SIP_PORT_MANAGER_H_
#define SIP_PORT_MANAGER_H_

#include "IpAddress.h"

/**
 * @brief This class provides the port information for SIP TCP/TLS transport protocol.
 */
class SipPortManager
{
private:
    SipPortManager();
    ~SipPortManager();

public:
    SipPortManager(IN const SipPortManager&) = delete;
    SipPortManager& operator=(IN const SipPortManager&) = delete;

public:
    void Clear();
    inline IMS_SINT32 GetPortC(IN const IPAddress& objIp) const { return SelectNextPortC(objIp); }
    inline IMS_BOOL IsPortCProvisioned() const
    {
        return (m_nPortCStart >= CLIENT_PORT_MIN) && (m_nPortCEnd <= CLIENT_PORT_MAX);
    }
    void SetPortC(IN IMS_SINT32 nPortStart, IN IMS_SINT32 nPortEnd);

    static SipPortManager* GetInstance();

private:
    inline IMS_SINT32 GetNextPortC() const { return m_nNextPortC; }
    IMS_BOOL IsPortAvailable(IN const IPAddress& objIp, IN IMS_SINT32 nPort) const;
    IMS_SINT32 SelectNextPortC(IN const IPAddress& objIp) const;
    void SetNextPortC(IN IMS_SINT32 nPort) const;

private:
    /// Port range as a default (not-inclusive).
    enum
    {
        CLIENT_PORT_MIN = 1024,
        CLIENT_PORT_MAX = 65535,

        CLIENT_PORT_START = 40000,
        CLIENT_PORT_END = CLIENT_PORT_MAX
    };

    /// Minimum Round-Robin Gap
    /// Do not re-use a source port that has been used in any of the previous 32 TCP sockets.
    enum
    {
        MIN_RR_GAP = 32
    };

    // Port range (not-inclusive)
    IMS_SINT32 m_nPortCStart;
    IMS_SINT32 m_nPortCEnd;

    // Next client port number
    mutable IMS_SINT32 m_nNextPortC;
};

#endif
