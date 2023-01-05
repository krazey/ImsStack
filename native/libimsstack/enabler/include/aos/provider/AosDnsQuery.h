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
#ifndef AOS_DNS_QUERY_H_
#define AOS_DNS_QUERY_H_

#include "ImsActivityEx.h"
#include "ImsList.h"
#include "IpAddress.h"
#include "interface/AosInternalMsgDef.h"

class INetworkConnection;
class IAosDnsQueryListener;
class AosDnsQueryPrivate;

class AosDnsQuery : public ImsActivityEx
{
public:
    explicit AosDnsQuery(IN IMS_BOOL bIsTest = IMS_FALSE);
    virtual ~AosDnsQuery();

private:
    AosDnsQuery(IN const AosDnsQuery& objRhs);
    AosDnsQuery& operator=(IN const AosDnsQuery& objRhs);

public:
    void SetListener(IN IAosDnsQueryListener* piListener);
    IMS_BOOL Request(IN const AString& strDomainName, IN INetworkConnection* piConnection);

    // Delete myself
    IMS_BOOL Destroy();

    IMS_BOOL DnsQueryPrivate_Ready();
    IMS_BOOL DnsQueryPrivate_Done(IN IMS_BOOL bResult, IN IMSList<IPAddress> objIps);
    IMS_BOOL DnsQueryPrivate_Terminated();

private:
    // ImsActivityEx
    IMS_BOOL OnMessage(IN IMSMSG& objMsg);

    enum
    {
        MSG_READY = AOSMSG_SERVICE_INTERNAL,
        MSG_REQUEST,
        MSG_DONE,
        MSG_DESTROY,
        MSG_TERMINATED
    };

public:
    static IMS_UINT32 m_nIdentity;

private:
    AosDnsQueryPrivate* m_pPrivate;
    IAosDnsQueryListener* m_piListener;
    AString m_strDomainName;
    INetworkConnection* m_piConnection;
    IMSList<IPAddress> m_objIps;

private:
    // Use only for Unit test
    IMS_BOOL m_bIsTest;
    friend class AosDnsQueryTest;
};

class IAosDnsQueryListener
{
public:
    virtual ~IAosDnsQueryListener(){};

    virtual void DnsQuery_Ready() = 0;
    virtual void DnsQuery_Done(IN IMS_BOOL bResult, IN IMSList<IPAddress> objIps) = 0;
};

#endif  // AOS_DNS_QUERY_H_
