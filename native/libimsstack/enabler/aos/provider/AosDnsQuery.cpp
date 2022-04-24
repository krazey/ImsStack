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

__IMS_TRACE_TAG_USER_DECL__("AOS");


extern void JNI_DetachNativeThread(void);

PUBLIC GLOBAL
IMS_UINT32 AosDnsQuery::nIdentity = 0;

class AosDnsQueryPrivate
    : public IThreadImpListener
{
public:
    AosDnsQueryPrivate(IN AosDnsQuery *pDnsQueryer);
    virtual ~AosDnsQueryPrivate();

private:
    AosDnsQueryPrivate(IN CONST AosDnsQueryPrivate &objRHS);
    AosDnsQueryPrivate& operator=(IN CONST AosDnsQueryPrivate &objRHS);

public:
    void DoDnsQuery(IN AString &strDomainName_, IN INetworkConnection *piConnection_);
    IMS_BOOL Start();
    IMS_BOOL Terminate();

private:
    virtual void RunImp();

    void ResetEvent(IN IMS_UINT32 nEvent);
    IMS_BOOL SetEvent(IN IMS_UINT32 nEvent);

    void SetDomainName(IN AString &strDomainName_) { strDomainName = strDomainName_; };
    void SetNetConnection(IN INetworkConnection *piConnection_) { piConnection = piConnection_; };

private:
    // Dns Query Event
    enum
    {
        DNS_QUERY_NONE = 0x0000,
        DNS_QUERY_EXEC = 0x0001,

        DNS_QUERY_TERMINATE = 0x8000
    };

    pthread_cond_t stSignal;
    OsMutex objMutex4Signal;
    IMS_SINT32 nEvent;
    OsMutex objMutex4Event;
    OsPthread *pThread;
    AString strDomainName;
    INetworkConnection *piConnection;
    AosDnsQuery *pQueryer;
    IMS_BOOL bSignaled;
};

/*

Remarks

*/
PUBLIC
AosDnsQueryPrivate::AosDnsQueryPrivate(IN AosDnsQuery *pDnsQueryer)
    : nEvent(DNS_QUERY_NONE)
    , pThread(new OsPthread())
    , piConnection(IMS_NULL)
    , pQueryer(pDnsQueryer)
    , bSignaled(IMS_FALSE)
{
    if (pthread_cond_init(&stSignal, IMS_NULL) == 0)
    {
        // no_op
    }

    if (pThread != IMS_NULL)
    {
        AString strName;

        AosDnsQuery::nIdentity++;
        strName.Sprintf("AosDnsQueryPrivate%X", AosDnsQuery::nIdentity);
        pThread->Create(strName);
        pThread->SetImpListener(this);
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
AosDnsQueryPrivate::~AosDnsQueryPrivate()
{
    if (pThread != IMS_NULL)
    {
        delete pThread;
        pThread = IMS_NULL;
    }

    if (pthread_cond_destroy(&stSignal) == 0)
    {
        // no_op
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL
void AosDnsQueryPrivate::RunImp()
{
    IMS_BOOL bLoop = IMS_TRUE;
    IMS_SINT32 nWaitResult;
    IMS_SINT32 nEventCache;

    if (pQueryer != IMS_NULL)
    {
        pQueryer->DnsQueryPrivate_Ready();
    }

    while (bLoop)
    {
        objMutex4Signal.Lock();

        if (bSignaled)
        {
            IMS_TRACE_D("AosDnsQueryPrivate :: signal is already triggered", 0, 0, 0);
            nWaitResult = 1;
        }
        else
        {
            nWaitResult = pthread_cond_wait(&stSignal,
            reinterpret_cast<pthread_mutex_t*>(objMutex4Signal.GetMutexObj()));

            if (nWaitResult == 0)
            {
                // no_op
            }
        }

        bSignaled = IMS_FALSE;

        objMutex4Signal.Unlock();

        objMutex4Event.Lock();
        nEventCache = nEvent;
        objMutex4Event.Unlock();

        IMS_TRACE_D("AosDnsQueryPrivate :: wait_result (%d) , event (%d) before processing",
            nWaitResult, nEvent, 0);

        if ((nEventCache & DNS_QUERY_EXEC) != 0)
        {
            IMSList<IPAddress> objIPs;

            if(piConnection->GetHostByName(strDomainName, objIPs) == -1)
            {
                IMS_TRACE_I("GetHostByName is failed", 0, 0, 0);

                if (pQueryer != IMS_NULL)
                {
                    pQueryer->DnsQueryPrivate_Done(IMS_FALSE, objIPs);
                }
            }
            else
            {
                IMS_TRACE_I("GetHostByName is success", 0, 0, 0);

                if (pQueryer != IMS_NULL)
                {
                    pQueryer->DnsQueryPrivate_Done(IMS_TRUE, objIPs);
                }
            }

            ResetEvent(DNS_QUERY_EXEC);
        }

        if ((nEventCache & DNS_QUERY_TERMINATE) != 0)
        {
            bLoop = IMS_FALSE;
            ResetEvent(DNS_QUERY_TERMINATE);
        }

        IMS_TRACE_D("AosDnsQueryPrivate :: event (%d) after processing", nEvent, 0, 0);
    }

    IMS_TRACE_D("AosDnsQueryPrivate :: terminated", 0, 0, 0);

    JNI_DetachNativeThread();

    if (pQueryer != IMS_NULL)
    {
        pQueryer->DnsQueryPrivate_Terminated();
    }
}

/*

Remarks

*/
PRIVATE
void AosDnsQueryPrivate::ResetEvent(IN IMS_UINT32 nEvent)
{
    objMutex4Event.Lock();

    this->nEvent &= (~nEvent);
    objMutex4Event.Unlock();
}

/*

Remarks

*/
PRIVATE
IMS_BOOL AosDnsQueryPrivate::SetEvent(IN IMS_UINT32 nEvent)
{
    objMutex4Event.Lock();

    if ((this->nEvent & nEvent) != 0)
    {
        IMS_TRACE_D("SetEvent :: Event(%d) is already set", nEvent, 0, 0);
        objMutex4Event.Unlock();
        return IMS_FALSE;
    }

    this->nEvent |= nEvent;
    objMutex4Event.Unlock();
    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL AosDnsQueryPrivate::Start()
{
    IMS_TRACE_D("Start", 0, 0, 0);

    if (pThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (pThread->IsRunning())
    {
        IMS_TRACE_D("thread is already running ...", 0, 0, 0);
        return IMS_TRUE;
    }

    return pThread->Activate();
}

/*

Remarks

*/
PUBLIC
void AosDnsQueryPrivate::DoDnsQuery(IN AString &strDomainName_,
        IN INetworkConnection *piConnection_)
{
    IMS_TRACE_D("DoDnsQuery :: domain = %s", strDomainName_.GetStr(), 0, 0);

    if (!SetEvent(DNS_QUERY_EXEC))
    {
        return;
    }

    SetDomainName(strDomainName_);
    SetNetConnection(piConnection_);

    objMutex4Signal.Lock();

    bSignaled = IMS_TRUE;

    pthread_cond_signal(&stSignal);
    objMutex4Signal.Unlock();
}

/*

Remarks

*/
PUBLIC
IMS_BOOL AosDnsQueryPrivate::Terminate()
{
    IMS_TRACE_D("Terminate", 0, 0, 0);

    if (pThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!pThread->IsRunning())
    {
        IMS_TRACE_D("thread is not running ...", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!SetEvent(DNS_QUERY_TERMINATE))
    {
        return IMS_TRUE;
    }

    objMutex4Signal.Lock();
    pthread_cond_signal(&stSignal);
    objMutex4Signal.Unlock();

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
AosDnsQuery::AosDnsQuery()
    : IMSActivityEx()
    , pPrivate(IMS_NULL)
    , piListener(IMS_NULL)
    , piConnection(IMS_NULL)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosDnsQuery = %" PFLS_u "/%" PFLS_x, GetName().GetStr(),
        sizeof(AosDnsQuery), this);

    pPrivate = new AosDnsQueryPrivate(this);

    if (pPrivate != IMS_NULL)
    {
        pPrivate->Start();
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
AosDnsQuery::~AosDnsQuery()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosDnsQuery = %" PFLS_u "/%" PFLS_x, GetName().GetStr(),
        sizeof(AosDnsQuery), this);

    if (pPrivate != IMS_NULL)
    {
        delete pPrivate;
    }
}

/*

Remarks

*/
PUBLIC
void AosDnsQuery::SetListener(IN IAosDnsQueryListener *piListener_)
{
    piListener = piListener_;
}

/*

Remarks

*/
PUBLIC
void AosDnsQuery::Request(IN AString &strDomainName_,  IN INetworkConnection *piConnection_)
{
    strDomainName = strDomainName_;
    piConnection = piConnection_;

    PostMessage(MSG_REQUEST, 0, 0);
}

/*

Remarks

*/
PUBLIC
void AosDnsQuery::Destroy()
{
    PostMessage(MSG_DESTROY, 0, 0);
}

/*

Remarks

*/
PUBLIC
void AosDnsQuery::DnsQueryPrivate_Ready()
{
    PostMessage(MSG_READY, 0, 0);
}

/*

Remarks

*/
PUBLIC
void AosDnsQuery::DnsQueryPrivate_Done(IN IMS_BOOL bResult, IN IMSList<IPAddress> objIPs)
{
    objIPAs = objIPs;
    PostMessage(MSG_DONE, (bResult) ? 1 : 0, 0);
}

/*

Remarks

*/
PUBLIC
void AosDnsQuery::DnsQueryPrivate_Terminated()
{
    PostMessage(MSG_TERMINATED, 0, 0);
}

/*

Remarks

*/
PRIVATE
IMS_BOOL AosDnsQuery::OnMessage(IN IMSMSG &objMSG)
{
    IMS_TRACE_I("OnMessage :: (%d)", objMSG.nMSG, 0, 0);

    if (piListener == IMS_NULL)
    {
        return IMS_FALSE;
    }

    switch (objMSG.nMSG)
    {
        case MSG_READY:
            piListener->DnsQuery_Ready();
            break;

        case MSG_REQUEST:
            pPrivate->DoDnsQuery(strDomainName, piConnection);
            break;

        case MSG_DONE:
            {
                IMS_BOOL bResult = (LONG_TO_INT(objMSG.nWparam) > 0) ? IMS_TRUE : IMS_FALSE;
                piListener->DnsQuery_Done(bResult, objIPAs);
            }
            break;

        case MSG_DESTROY:
            if(!pPrivate->Terminate())
            {
                delete this;
            }
            break;

        case MSG_TERMINATED:
            delete this;
            break;

        default:
            break;
    }

    return IMS_TRUE;
}
