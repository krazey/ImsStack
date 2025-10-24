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
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <cutils/log.h>

#include "OsMutex.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

static void osMutex_TraceError(IN const IMS_CHAR* pszTag, IN IMS_SINT32 nError)
{
    switch (nError)
    {
        case EINVAL:
            (void)ALOG(
                    LOG_ERROR, IMS_LOG_TAG, "[IPL] mutex_%s: invalid parameter", _TRACE_S_(pszTag));
            break;
        case EDEADLK:
            (void)ALOG(LOG_ERROR, IMS_LOG_TAG, "[IPL] mutex_%s: deadlock", _TRACE_S_(pszTag));
            break;
        case EPERM:
            (void)ALOG(LOG_ERROR, IMS_LOG_TAG, "[IPL] mutex_%s: invalid permission",
                    _TRACE_S_(pszTag));
            break;
        case EBUSY:
            (void)ALOG(LOG_DEBUG, IMS_LOG_TAG, "[IPL] mutex_%s: busy", _TRACE_S_(pszTag));
            break;
        default:
            (void)ALOG(
                    LOG_ERROR, IMS_LOG_TAG, "[IPL] mutex_%s: error=%d", _TRACE_S_(pszTag), nError);
            break;
    }
}

class OsMutexPrivate
{
public:
    inline OsMutexPrivate()
    {
        m_pstMutex = new pthread_mutex_t;

        memset(&m_stMutexAttr, 0, sizeof(pthread_mutexattr_t));

        pthread_mutexattr_settype(&m_stMutexAttr, PTHREAD_MUTEX_RECURSIVE);
    }
    inline ~OsMutexPrivate()
    {
        if (m_pstMutex != IMS_NULL)
        {
            delete m_pstMutex;
            m_pstMutex = IMS_NULL;
        }
    }

    OsMutexPrivate(IN const OsMutexPrivate&) = delete;
    OsMutexPrivate& operator=(IN const OsMutexPrivate&) = delete;

public:
    pthread_mutexattr_t m_stMutexAttr;
    pthread_mutex_t* m_pstMutex;
};

PUBLIC
OsMutex::OsMutex(IN IMS_SINT32 nType /*= ATTRIBUTE_RECURSIVE*/) :
        m_pMutexP(new OsMutexPrivate())
{
    SetMutexType(nType);

    IMS_SINT32 nRc = pthread_mutex_init((m_pMutexP->m_pstMutex), &(m_pMutexP->m_stMutexAttr));

    if (nRc == 0)
    {
        // no-op
    }
    else
    {
        osMutex_TraceError("init", nRc);
    }
}

PUBLIC VIRTUAL OsMutex::~OsMutex()
{
    if (m_pMutexP != IMS_NULL)
    {
        IMS_SINT32 nRc = pthread_mutex_destroy(m_pMutexP->m_pstMutex);

        if (nRc == 0)
        {
            // no-op
        }
        else
        {
            osMutex_TraceError("destroy", nRc);
        }

        delete m_pMutexP;
        m_pMutexP = IMS_NULL;
    }
}

PUBLIC VIRTUAL void OsMutex::Lock()
{
    IMS_SINT32 nRc = pthread_mutex_lock(m_pMutexP->m_pstMutex);

    if (nRc == 0)
    {
        // no-op
    }
    else
    {
        osMutex_TraceError("lock", nRc);
    }
}

PUBLIC VIRTUAL void OsMutex::Unlock()
{
    IMS_SINT32 nRc = pthread_mutex_unlock(m_pMutexP->m_pstMutex);

    if (nRc == 0)
    {
        // no-op
    }
    else
    {
        osMutex_TraceError("unlock", nRc);
    }
}

PUBLIC
IMS_SINT32 OsMutex::TryLock()
{
    IMS_SINT32 nRc = pthread_mutex_trylock(m_pMutexP->m_pstMutex);

    if (nRc == 0)
    {
        // no-op
    }
    else
    {
        osMutex_TraceError("trylock", nRc);
    }

    return nRc;
}

PUBLIC
IMS_PVOID OsMutex::GetMutexObj()
{
    return reinterpret_cast<IMS_PVOID>(m_pMutexP->m_pstMutex);
}

PRIVATE
IMS_SINT32 OsMutex::GetMutexType()
{
    IMS_SINT32 nType = (-1);
    IMS_SINT32 nRc = pthread_mutexattr_gettype(&(m_pMutexP->m_stMutexAttr), &nType);

    if (nRc == 0)
    {
        // no-op
    }
    else
    {
        osMutex_TraceError("attr_gettype", nRc);
    }

    return nType;
}

PRIVATE
IMS_SINT32 OsMutex::SetMutexType(IN IMS_SINT32 nType)
{
    IMS_SINT32 nMutexType = (-1);

    switch (nType)
    {
        case ATTRIBUTE_NORMAL:
            nMutexType = PTHREAD_MUTEX_NORMAL;
            break;
        case ATTRIBUTE_RECURSIVE:
            nMutexType = PTHREAD_MUTEX_RECURSIVE;
            break;
        case ATTRIBUTE_ERRORCHECK:
            nMutexType = PTHREAD_MUTEX_ERRORCHECK;
            break;
        default:
            break;
    }

    if (nMutexType < 0)
    {
        return (-1);
    }

    IMS_SINT32 nRc = pthread_mutexattr_settype(&(m_pMutexP->m_stMutexAttr), nMutexType);

    if (nRc == 0)
    {
        // no-op
    }
    else
    {
        osMutex_TraceError("attr_settype", nRc);
    }

    return nRc;
}
