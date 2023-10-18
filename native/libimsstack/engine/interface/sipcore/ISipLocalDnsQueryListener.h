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
#ifndef INTERFACE_SIP_LOCAL_DNS_QUERY_LISTENER_H_
#define INTERFACE_SIP_LOCAL_DNS_QUERY_LISTENER_H_

#include "IpAddress.h"

/**
 * @brief This class provides a listener interface for the local DNS query for a test purpose
 *        or obtaining a real host address by the service enabler.
 */
class ISipLocalDnsQueryListener
{
protected:
    virtual ~ISipLocalDnsQueryListener() = default;

public:
    /**
     * @brief Notifies the application that the host name should be resolved
     *        to send a SIP message.
     *
     * If the method returns IMS_FALSE, J180 engine will keep the original procedure
     * to resolve the host name.
     *
     * @param objLocalIp Local IP address
     * @param strHostname Host name to be resolved
     * @param objHostIp Resolved host IP address
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL LocalDnsQuery_GetHostByName(IN const IpAddress& objLocalIp,
            IN const AString& strHostname, OUT IpAddress& objHostIp) = 0;
};

#endif
