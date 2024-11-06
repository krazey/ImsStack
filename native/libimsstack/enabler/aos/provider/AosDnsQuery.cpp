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
#include <unistd.h>
#include <pthread.h>

#include "ServiceTrace.h"
#include "OsMutex.h"
#include "OsPthread.h"
#include "INetworkConnection.h"
#include "provider/AosDnsQuery.h"

__IMS_TRACE_TAG_AOS__;

extern void JniDetachNativeThread();

PUBLIC GLOBAL IMS_UINT32 AosDnsQuery::m_nIdentity = 0;

class AosDnsQueryPrivate : public IThreadImpListener
{
public:
    explicit AosDnsQueryPrivate(IN AosDnsQuery* pDnsQueryer);
    virtual ~AosDnsQueryPrivate();

private:
    AosDnsQueryPrivate(IN const AosDnsQueryPrivate& objRhs);
    AosDnsQueryPrivate& operator=(IN const AosDnsQueryPrivate& objRhs);

public:
    IMS_BOOL Start();
    IMS_BOOL Terminate();
    IMS_BOOL DoDnsQuery(IN AString& strDomainName, IN INetworkConnection* piConnection);

    void RunImp() override;

    IMS_BOOL ResetEvent(IN IMS_UINT32 nEvent);
    IMS_BOOL SetEvent(IN IMS_UINT32 nEvent);
    IMS_BOOL HasEvent(IN IMS_UINT32 nEvent);
    void SetThread(IN OsPthread* pThread);
    void SetConnection(IN INetworkConnection* piConnection);
    void SetSignaled(IN IMS_BOOL bSignaled);

private:
    void SetDomainName(IN const AString& strDomainName);

private:
    pthread_cond_t m_stSignal;
    OsMutex m_objMutex4Signal;
    IMS_UINT32 m_nEvent;
    OsMutex m_objMutex4Event;
    OsPthread* m_pThread;
    AString m_strDomainName;
    INetworkConnection* m_piConnection;
    AosDnsQuery* m_pQueryer;
    IMS_BOOL m_bSignaled;
};

PUBLIC
AosDnsQueryPrivate::AosDnsQueryPrivate(IN AosDnsQuery* pDnsQueryer) :
        m_nEvent(AosDnsQuery::DNS_QUERY_NONE),
        m_pThread(new OsPthread()),
        m_piConnection(IMS_NULL),
        m_pQueryer(pDnsQueryer),
        m_bSignaled(IMS_FALSE)
{
    if (pthread_cond_init(&m_stSignal, IMS_NULL) == 0)
    {
        // no_op
    }

    if (m_pThread != IMS_NULL)
    {
        AString strName;

        AosDnsQuery::m_nIdentity++;
        strName.Sprintf("AosDnsQueryPrivate%X", AosDnsQuery::m_nIdentity);
        m_pThread->Create(strName);
        m_pThread->SetImpListener(this);
    }
}

PUBLIC VIRTUAL AosDnsQueryPrivate::~AosDnsQueryPrivate()
{
    if (m_pThread != IMS_NULL)
    {
        delete m_pThread;
        m_pThread = IMS_NULL;
    }

    if (pthread_cond_destroy(&m_stSignal) == 0)
    {
        // no_op
    }
}

PUBLIC
IMS_BOOL AosDnsQueryPrivate::Start()
{
    IMS_TRACE_D("Start", 0, 0, 0);

    if (m_pThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_pThread->IsRunning())
    {
        return IMS_TRUE;
    }

    if (m_pQueryer->IsTestMode())
    {
        return IMS_FALSE;
    }

    return m_pThread->Activate();
}

PUBLIC
IMS_BOOL AosDnsQueryPrivate::Terminate()
{
    IMS_TRACE_D("Terminate", 0, 0, 0);

    if (m_pThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!m_pThread->IsRunning())
    {
        IMS_TRACE_D("thread is not running ...", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!SetEvent(AosDnsQuery::DNS_QUERY_TERMINATE))
    {
        return IMS_TRUE;
    }

    m_objMutex4Signal.Lock();
    pthread_cond_signal(&m_stSignal);
    m_objMutex4Signal.Unlock();

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AosDnsQueryPrivate::DoDnsQuery(
        IN AString& strDomainName, IN INetworkConnection* piConnection)
{
    IMS_TRACE_D("DoDnsQuery :: domain = %s", strDomainName.GetStr(), 0, 0);

    if (!SetEvent(AosDnsQuery::DNS_QUERY_EXEC))
    {
        return IMS_FALSE;
    }

    SetDomainName(strDomainName);
    SetConnection(piConnection);

    m_objMutex4Signal.Lock();

    m_bSignaled = IMS_TRUE;

    pthread_cond_signal(&m_stSignal);
    m_objMutex4Signal.Unlock();

    return IMS_TRUE;
}

PUBLIC
VIRTUAL void AosDnsQueryPrivate::RunImp()
{
    IMS_BOOL bLoop = IMS_TRUE;

    if (m_pQueryer != IMS_NULL)
    {
        m_pQueryer->DnsQueryPrivate_Ready();
    }

    while (bLoop)
    {
        m_objMutex4Signal.Lock();

        if (!m_bSignaled && m_pQueryer->IsTestMode() == IMS_FALSE)
        {
            pthread_cond_wait(&m_stSignal,
                    reinterpret_cast<pthread_mutex_t*>(m_objMutex4Signal.GetMutexObj()));
        }

        m_bSignaled = IMS_FALSE;

        m_objMutex4Signal.Unlock();

        m_objMutex4Event.Lock();
        IMS_UINT32 nEventCache = m_nEvent;
        m_objMutex4Event.Unlock();

        IMS_TRACE_D("AosDnsQueryPrivate :: event (%d) before processing", m_nEvent, 0, 0);

        if ((nEventCache & AosDnsQuery::DNS_QUERY_EXEC) != 0)
        {
            ImsList<IpAddress> objIps;

            if (m_piConnection->GetHostByName(m_strDomainName, objIps) == -1)
            {
                if (m_pQueryer != IMS_NULL)
                {
                    m_pQueryer->DnsQueryPrivate_Done(IMS_FALSE, objIps);
                }
            }
            else
            {
                if (m_pQueryer != IMS_NULL)
                {
                    m_pQueryer->DnsQueryPrivate_Done(IMS_TRUE, objIps);
                }
            }

            ResetEvent(AosDnsQuery::DNS_QUERY_EXEC);
        }

        if ((nEventCache & AosDnsQuery::DNS_QUERY_TERMINATE) != 0)
        {
            bLoop = IMS_FALSE;
            ResetEvent(AosDnsQuery::DNS_QUERY_TERMINATE);
        }

        if (m_pQueryer->IsTestMode())
        {
            bLoop = IMS_FALSE;
        }

        IMS_TRACE_D("AosDnsQueryPrivate :: event (%d) after processing", m_nEvent, 0, 0);
    }

    IMS_TRACE_D("AosDnsQueryPrivate :: terminated", 0, 0, 0);

    JniDetachNativeThread();

    if (m_pQueryer != IMS_NULL)
    {
        m_pQueryer->DnsQueryPrivate_Terminated();
    }
}

PUBLIC
void AosDnsQueryPrivate::SetDomainName(IN const AString& strDomainName)
{
    m_strDomainName = strDomainName;
};

PUBLIC
IMS_BOOL AosDnsQueryPrivate::ResetEvent(IN IMS_UINT32 nEvent)
{
    m_objMutex4Event.Lock();

    if ((m_nEvent & nEvent) == 0)
    {
        IMS_TRACE_D("ResetEvent :: Event(%d) is already reset", nEvent, 0, 0);
        m_objMutex4Event.Unlock();

        return IMS_FALSE;
    }

    m_nEvent &= (~nEvent);
    m_objMutex4Event.Unlock();

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AosDnsQueryPrivate::SetEvent(IN IMS_UINT32 nEvent)
{
    m_objMutex4Event.Lock();

    if ((m_nEvent & nEvent) != 0)
    {
        IMS_TRACE_D("SetEvent :: Event(%d) is already set", nEvent, 0, 0);
        m_objMutex4Event.Unlock();

        return IMS_FALSE;
    }

    m_nEvent |= nEvent;
    m_objMutex4Event.Unlock();

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AosDnsQueryPrivate::HasEvent(IN IMS_UINT32 nEvent)
{
    if ((m_nEvent & nEvent) != 0)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
void AosDnsQueryPrivate::SetThread(IN OsPthread* pThread)
{
    m_pThread = pThread;
}

PUBLIC
void AosDnsQueryPrivate::SetConnection(IN INetworkConnection* piConnection)
{
    m_piConnection = piConnection;
}

PROTECTED
void AosDnsQueryPrivate::SetSignaled(IN IMS_BOOL bSignaled)
{
    m_bSignaled = bSignaled;
}

PUBLIC
AosDnsQuery::AosDnsQuery(IN IMS_BOOL bIsTest /*= IMS_FALSE*/) :
        ImsActivityEx(),
        m_pPrivate(IMS_NULL),
        m_piListener(IMS_NULL),
        m_piConnection(IMS_NULL),
        m_bIsTest(bIsTest)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosDnsQuery = %" PFLS_u "/%" PFLS_x, GetName().GetStr(),
            sizeof(AosDnsQuery), this);

    m_pPrivate = new AosDnsQueryPrivate(this);

    if (m_pPrivate != IMS_NULL)
    {
        m_pPrivate->Start();
    }
}

PUBLIC VIRTUAL AosDnsQuery::~AosDnsQuery()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosDnsQuery = %" PFLS_u "/%" PFLS_x, GetName().GetStr(),
            sizeof(AosDnsQuery), this);

    if (m_pPrivate != IMS_NULL)
    {
        delete m_pPrivate;
    }
}

PUBLIC
void AosDnsQuery::SetListener(IN IAosDnsQueryListener* piListener)
{
    m_piListener = piListener;
}

PUBLIC
IMS_BOOL AosDnsQuery::Request(IN const AString& strDomainName, IN INetworkConnection* piConnection)
{
    IMS_TRACE_I("Request", 0, 0, 0);
    m_strDomainName = strDomainName;
    m_piConnection = piConnection;

    PostMessage(MSG_REQUEST, 0, 0);
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AosDnsQuery::Destroy()
{
    IMS_TRACE_I("Destroy", 0, 0, 0);
    PostMessage(MSG_DESTROY, 0, 0);
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AosDnsQuery::DnsQueryPrivate_Ready()
{
    IMS_TRACE_I("DnsQueryPrivate_Ready", 0, 0, 0);
    PostMessage(MSG_READY, 0, 0);
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AosDnsQuery::DnsQueryPrivate_Done(IN IMS_BOOL bResult, IN const ImsList<IpAddress>& objIps)
{
    IMS_TRACE_I("DnsQueryPrivate_Done : %s", (bResult) ? "SUCCESS" : "FAILED", 0, 0);

    m_objIps = objIps;
    PostMessage(MSG_DONE, (bResult) ? 1 : 0, 0);
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AosDnsQuery::DnsQueryPrivate_Terminated()
{
    IMS_TRACE_I("DnsQueryPrivate_Terminated", 0, 0, 0);
    PostMessage(MSG_TERMINATED, 0, 0);
    return IMS_TRUE;
}

// For Unit testing
PUBLIC
IMS_BOOL AosDnsQuery::IsTestMode()
{
    return m_bIsTest;
}

// For Unit testing
PROTECTED
IMS_BOOL AosDnsQuery::ResetEvent(IN IMS_UINT32 nEvent)
{
    return m_pPrivate->ResetEvent(nEvent);
}

// For Unit testing
PROTECTED
IMS_BOOL AosDnsQuery::SetEvent(IN IMS_UINT32 nEvent)
{
    return m_pPrivate->SetEvent(nEvent);
};

// For Unit testing
PROTECTED
IMS_BOOL AosDnsQuery::HasEvent(IN IMS_UINT32 nEvent)
{
    return m_pPrivate->HasEvent(nEvent);
};

// For Unit testing
PROTECTED
IMS_BOOL AosDnsQuery::Start()
{
    return m_pPrivate->Start();
}

// For Unit testing
PROTECTED
IMS_BOOL AosDnsQuery::Terminate()
{
    return m_pPrivate->Terminate();
}

// For Unit testing
PROTECTED
void AosDnsQuery::SetThread(IN OsPthread* pThread)
{
    return m_pPrivate->SetThread(pThread);
};

// For Unit testing
PROTECTED
void AosDnsQuery::SetConnection(IN INetworkConnection* piConnection)
{
    m_pPrivate->SetConnection(piConnection);
};

// For Unit testing
PROTECTED
void AosDnsQuery::SetSignaled(IN IMS_BOOL bSignaled)
{
    m_pPrivate->SetSignaled(bSignaled);
}

// For Unit testing
PROTECTED
void AosDnsQuery::RunImp()
{
    return m_pPrivate->RunImp();
};

PROTECTED
IMS_BOOL AosDnsQuery::OnMessage(IN IMSMSG& objMsg)
{
    IMS_TRACE_I("OnMessage :: (%d)", objMsg.nMSG, 0, 0);

    if (m_piListener == IMS_NULL)
    {
        return IMS_FALSE;
    }

    switch (objMsg.nMSG)
    {
        case MSG_READY:
            m_piListener->DnsQuery_Ready();
            break;

        case MSG_REQUEST:
            if (m_pPrivate != IMS_NULL)
                m_pPrivate->DoDnsQuery(m_strDomainName, m_piConnection);
            break;

        case MSG_DONE:
            m_piListener->DnsQuery_Done(
                    (LONG_TO_INT(objMsg.nWparam) > 0) ? IMS_TRUE : IMS_FALSE, m_objIps);
            break;

        case MSG_DESTROY:
            if (m_pPrivate != IMS_NULL && !m_pPrivate->Terminate() && !m_bIsTest)
                delete this;
            break;

        case MSG_TERMINATED:
            if (!m_bIsTest)
                delete this;
            break;

        default:
            return IMS_FALSE;
    }

    return IMS_TRUE;
}
