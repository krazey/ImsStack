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

#include "ServiceMemory.h"
#include "network/OsSelectFdSet.h"

class SelectFds
{
public:
    inline SelectFds() :
            m_nHighestFd(0)
    {
        FD_ZERO(&m_stReadFds);
        FD_ZERO(&m_stWriteFds);
        FD_ZERO(&m_stExceptFds);
    }
    inline SelectFds(IN const SelectFds& other) :
            m_nHighestFd(other.m_nHighestFd)
    {
        IMS_MEM_Memcpy(&m_stReadFds, &(other.m_stReadFds), sizeof(fd_set));
        IMS_MEM_Memcpy(&m_stWriteFds, &(other.m_stWriteFds), sizeof(fd_set));
        IMS_MEM_Memcpy(&m_stExceptFds, &(other.m_stExceptFds), sizeof(fd_set));
    }
    inline ~SelectFds() {}

public:
    inline SelectFds& operator=(IN const SelectFds& other)
    {
        if (this != &other)
        {
            m_nHighestFd = other.m_nHighestFd;
            IMS_MEM_Memcpy(&m_stReadFds, &(other.m_stReadFds), sizeof(fd_set));
            IMS_MEM_Memcpy(&m_stWriteFds, &(other.m_stWriteFds), sizeof(fd_set));
            IMS_MEM_Memcpy(&m_stExceptFds, &(other.m_stExceptFds), sizeof(fd_set));
        }

        return (*this);
    }

    inline void SetHighestFd(IN IMS_SINT32 nFd) { m_nHighestFd = nFd; }

public:
    IMS_SINT32 m_nHighestFd;
    fd_set m_stReadFds;
    fd_set m_stWriteFds;
    fd_set m_stExceptFds;
};

PUBLIC
OsSelectFdSet::OsSelectFdSet() :
        ImsFdSet(),
        m_pFds(new SelectFds())
{
}

PUBLIC
OsSelectFdSet::OsSelectFdSet(IN const OsSelectFdSet& other) :
        ImsFdSet(other),
        m_pFds(IMS_NULL)
{
    if (other.m_pFds != IMS_NULL)
    {
        m_pFds = new SelectFds(*(other.m_pFds));
    }
}

PUBLIC VIRTUAL OsSelectFdSet::~OsSelectFdSet()
{
    if (m_pFds != IMS_NULL)
    {
        delete m_pFds;
    }
}

PUBLIC
OsSelectFdSet& OsSelectFdSet::operator=(IN const OsSelectFdSet& other)
{
    if (this != &other)
    {
        ImsFdSet::operator=(other);

        if (other.m_pFds != IMS_NULL)
        {
            if (m_pFds == IMS_NULL)
            {
                m_pFds = new SelectFds(*(other.m_pFds));
            }
            else
            {
                (*m_pFds) = *(other.m_pFds);
            }
        }
        else
        {
            if (m_pFds != IMS_NULL)
            {
                delete m_pFds;
                m_pFds = IMS_NULL;
            }
        }
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_SINT32 OsSelectFdSet::ClearEvent(IN IMS_SINT32 nFd, IN IMS_SINT32 nEvent)
{
    if (nFd < 0)
    {
        return 0;
    }

    if (m_pFds == IMS_NULL)
    {
        return 0;
    }

    IMS_SINT32 nClearEvent = 0;

    if ((nEvent & EVENT_READ) != 0)
    {
        if (IsEventSet(nFd, EVENT_READ))
        {
            FD_CLR(nFd, &(m_pFds->m_stReadFds));
            nClearEvent |= EVENT_READ;
        }
    }

    if ((nEvent & EVENT_WRITE) != 0)
    {
        if (IsEventSet(nFd, EVENT_WRITE))
        {
            FD_CLR(nFd, &(m_pFds->m_stWriteFds));
            nClearEvent |= EVENT_WRITE;
        }
    }

    if ((nEvent & EVENT_EXCEPT) != 0)
    {
        if (IsEventSet(nFd, EVENT_EXCEPT))
        {
            FD_CLR(nFd, &(m_pFds->m_stExceptFds));
            nClearEvent |= EVENT_EXCEPT;
        }
    }

    return nClearEvent;
}

PUBLIC VIRTUAL void OsSelectFdSet::CopyFrom(IN const ImsFdSet* pFdSet)
{
    const OsSelectFdSet* pSelectFdSet = DYNAMIC_CAST(const OsSelectFdSet*, pFdSet);

    if (pSelectFdSet != IMS_NULL)
    {
        operator=(*pSelectFdSet);
    }
}

PUBLIC VIRTUAL IMS_SINT32 OsSelectFdSet::GetSignaledEvents(
        IN IMS_SINT32 nFd, IN_OUT IMS_SINT32& nSignaledCount)
{
    IMS_SINT32 nSignaledEvents = 0;

    nSignaledCount = 0;

    if (IsEventSet(nFd, EVENT_READ))
    {
        nSignaledEvents |= EVENT_READ;
        ++nSignaledCount;
    }

    if (IsEventSet(nFd, EVENT_WRITE))
    {
        nSignaledEvents |= EVENT_WRITE;
        ++nSignaledCount;
    }

    if (IsEventSet(nFd, EVENT_EXCEPT))
    {
        nSignaledEvents |= EVENT_EXCEPT;
        ++nSignaledCount;
    }

    return nSignaledEvents;
}

PUBLIC VIRTUAL IMS_BOOL OsSelectFdSet::IsEventSet(IN IMS_SINT32 nFd, IN IMS_SINT32 nEvent)
{
    if (nFd < 0)
    {
        return IMS_FALSE;
    }

    if (m_pFds == IMS_NULL)
    {
        return IMS_FALSE;
    }

    nEvent = (nEvent & EVENT_ALL);

    if ((nEvent == EVENT_READ) && FD_ISSET(nFd, &(m_pFds->m_stReadFds)))
    {
        return IMS_TRUE;
    }
    else if ((nEvent == EVENT_WRITE) && FD_ISSET(nFd, &(m_pFds->m_stWriteFds)))
    {
        return IMS_TRUE;
    }
    else if ((nEvent == EVENT_EXCEPT) && FD_ISSET(nFd, &(m_pFds->m_stExceptFds)))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL OsSelectFdSet::IsHighestFdRequired() const
{
    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_SINT32 OsSelectFdSet::SetEvent(IN IMS_SINT32 nFd, IN IMS_SINT32 nEvent)
{
    if (nFd < 0)
    {
        return 0;
    }

    if (m_pFds == IMS_NULL)
    {
        return 0;
    }

    IMS_SINT32 nSetEvent = 0;

    if ((nEvent & EVENT_READ) != 0)
    {
        if (!IsEventSet(nFd, EVENT_READ))
        {
            FD_SET(nFd, &(m_pFds->m_stReadFds));
            nSetEvent |= EVENT_READ;
        }
    }

    if ((nEvent & EVENT_WRITE) != 0)
    {
        if (!IsEventSet(nFd, EVENT_WRITE))
        {
            FD_SET(nFd, &(m_pFds->m_stWriteFds));
            nSetEvent |= EVENT_WRITE;
        }
    }

    if ((nEvent & EVENT_EXCEPT) != 0)
    {
        if (!IsEventSet(nFd, EVENT_EXCEPT))
        {
            FD_SET(nFd, &(m_pFds->m_stExceptFds));
            nSetEvent |= EVENT_EXCEPT;
        }
    }

    return nSetEvent;
}

PUBLIC VIRTUAL void OsSelectFdSet::SetHighestFd(IN IMS_SINT32 nFd)
{
    if (m_pFds != IMS_NULL)
    {
        m_pFds->SetHighestFd(nFd);
    }
}

PUBLIC VIRTUAL IMS_SINT32 OsSelectFdSet::WaitForEvents(IN IMS_SINT32 nMilliseconds /*= NO_TIMEOUT*/)
{
    if (m_pFds == IMS_NULL)
    {
        return 0;
    }

    struct timeval stTv;

    if (nMilliseconds > NO_TIMEOUT)
    {
        stTv.tv_sec = nMilliseconds / 1000;            // seconds
        stTv.tv_usec = (nMilliseconds % 1000) * 1000;  // micro-seconds
    }

    return select(m_pFds->m_nHighestFd + 1, &(m_pFds->m_stReadFds), &(m_pFds->m_stWriteFds),
            &(m_pFds->m_stExceptFds), (nMilliseconds > NO_TIMEOUT) ? &stTv : IMS_NULL);
}
