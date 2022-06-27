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
#include "sip_pf_datatypes.h"
#include "platform/sip_pf_string.h"
#include "platform/sip_pf_memory.h"
#include "platform/SipMutex.h"
#include "pthread.h"

SipMutex::SipMutex() :
        pMutex(SIP_NULL)
{
    pthread_mutex_t* pThreadMutex = new pthread_mutex_t;
    if (pThreadMutex != SIP_NULL)
    {
        SIP_INT32 nRet = pthread_mutex_init(pThreadMutex, SIP_NULL);

        if (nRet == SIP_ZERO)
        {
            pMutex = pThreadMutex;
        }
        else
        {
            delete pThreadMutex;
        }
    }
}

SipMutex::~SipMutex()
{
    pthread_mutex_t* pThreadMutex = (pthread_mutex_t*)pMutex;

    if (pThreadMutex != SIP_NULL)
    {
        pthread_mutex_destroy(pThreadMutex);
        delete pThreadMutex;
    }
}

void SipMutex::Lock()
{
    pthread_mutex_t* pThreadMutex = (pthread_mutex_t*)pMutex;

    if (pThreadMutex == SIP_NULL)
    {
        return;
    }

    pthread_mutex_lock(pThreadMutex);
}

void SipMutex::TryLock()
{
    pthread_mutex_t* pThreadMutex = (pthread_mutex_t*)pMutex;

    if (pThreadMutex == SIP_NULL)
    {
        return;
    }

    pthread_mutex_trylock(pThreadMutex);
}

void SipMutex::Unlock()
{
    pthread_mutex_t* pThreadMutex = (pthread_mutex_t*)pMutex;

    if (pThreadMutex == SIP_NULL)
    {
        return;
    }

    pthread_mutex_unlock(pThreadMutex);
}
