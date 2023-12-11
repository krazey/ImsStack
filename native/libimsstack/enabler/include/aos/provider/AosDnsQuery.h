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
#include "OsPthread.h"
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
    IMS_BOOL DnsQueryPrivate_Done(IN IMS_BOOL bResult, IN const ImsList<IpAddress>& objIps);
    IMS_BOOL DnsQueryPrivate_Terminated();

    // For Unit testing
    IMS_BOOL IsTestMode();

public:
    enum
    {
        DNS_QUERY_NONE = 0x0000,
        DNS_QUERY_EXEC = 0x0001,

        DNS_QUERY_TERMINATE = 0x8000
    };

protected:
    // For Unit testing
    IMS_BOOL ResetEvent(IN IMS_UINT32 nEvent);
    // For Unit testing
    IMS_BOOL SetEvent(IN IMS_UINT32 nEvent);
    // For Unit testing
    IMS_BOOL HasEvent(IN IMS_UINT32 nEvent);
    // For Unit testing
    IMS_BOOL Start();
    // For Unit testing
    IMS_BOOL Terminate();
    // For Unit testing
    void SetThread(IN OsPthread* pThread);
    // For Unit testing
    void SetConnection(IN INetworkConnection* piConnection);
    // For Unit testing
    void SetSignaled(IN IMS_BOOL bSignaled);
    // For Unit testing
    void RunImp();

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

protected:
    AosDnsQueryPrivate* m_pPrivate;
    IAosDnsQueryListener* m_piListener;
    AString m_strDomainName;
    INetworkConnection* m_piConnection;
    ImsList<IpAddress> m_objIps;

private:
    // For Unit testing
    IMS_BOOL m_bIsTest;
};

class IAosDnsQueryListener
{
public:
    virtual ~IAosDnsQueryListener(){};

    virtual void DnsQuery_Ready() = 0;
    virtual void DnsQuery_Done(IN IMS_BOOL bResult, IN ImsList<IpAddress> objIps) = 0;
};

#endif  // AOS_DNS_QUERY_H_
