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
#include <poll.h>
#include <unistd.h>

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "network/OsPollFdSet.h"

__IMS_TRACE_TAG_ADAPT__;

class PollFds
{
public:
    inline PollFds() :
            m_nCount(0)
    {
        Init(IMS_NULL);
    }
    inline PollFds(IN const PollFds& other) :
            m_nCount(other.m_nCount)
    {
        Init(other.m_astFd);
    }
    inline ~PollFds() {}

public:
    inline PollFds& operator=(IN const PollFds& other)
    {
        if (this != &other)
        {
            m_nCount = other.m_nCount;
            Init(other.m_astFd);
        }

        return (*this);
    }

public:
    pollfd* AddFd(IN IMS_SINT32 nFd);
    pollfd* GetFd(IN IMS_SINT32 nFd) const;
    void RemoveFd(IN IMS_SINT32 nFd);
    pollfd* ToArray(IN_OUT IMS_SINT32& nFds);

    inline static void ClearEvent(IN pollfd* pstFd, IN IMS_SINT32 nEvent)
    {
        pstFd->events &= ~nEvent;
    }
    inline static IMS_BOOL IsEventSet(IN pollfd* pstFd, IN IMS_SINT32 nEvent)
    {
        return ((pstFd->events & nEvent) != 0);
    }
    inline static IMS_BOOL IsEventSignaled(IN pollfd* pstFd, IN IMS_SINT32 nEvent)
    {
        return ((pstFd->revents & nEvent) != 0);
    }
    inline static void SetEvent(IN pollfd* pstFd, IN IMS_SINT32 nEvent) { pstFd->events |= nEvent; }

private:
    void Init(IN const pollfd* pstFds, IN IMS_SINT32 nFds = (MAX_POLL_SIZE + 1));

public:
    // Event mapping
    // IN: 0x1, OUT: 0x4, ERR: 0x8, HUP: 0x10, NVAL: 0x20, RDHUP: 0x2000
    enum
    {
        EVENT_READ = POLLIN,
        EVENT_WRITE = POLLOUT,
        // It's meaningless for pollfd.events, but it is used for internal event management
        EVENT_EXCEPT = (POLLERR | POLLHUP | POLLNVAL),
        EVENT_ALL = (EVENT_READ | EVENT_WRITE | EVENT_EXCEPT),
        // POLLRDHUP
        //  : Stream socket peer closed connection or shut down writing half of connection
        EVENT_EXCEPT_TCP_C = POLLRDHUP
    };

    enum
    {
        MAX_POLL_SIZE = 512
    };

    IMS_SINT16 m_nCount;
    pollfd m_astFd[MAX_POLL_SIZE + 1];
};

PUBLIC
pollfd* PollFds::AddFd(IN IMS_SINT32 nFd)
{
    if (m_nCount == MAX_POLL_SIZE)
    {
        IMS_TRACE_E(0, "PollFds :: No empty pollfd", 0, 0, 0);
        return IMS_NULL;
    }

    m_astFd[m_nCount].fd = nFd;
    m_astFd[m_nCount].events = 0;
    m_astFd[m_nCount].revents = 0;
    ++m_nCount;

    IMS_TRACE_D("PollFds :: AddFd() - fd=%d, count=%d", nFd, m_nCount, 0);

    return &(m_astFd[m_nCount - 1]);
}

PUBLIC
pollfd* PollFds::GetFd(IN IMS_SINT32 nFd) const
{
    for (IMS_SINT16 i = 0; i < m_nCount; ++i)
    {
        if (m_astFd[i].fd == nFd)
        {
            return const_cast<pollfd*>(&(m_astFd[i]));
        }
    }

    return IMS_NULL;
}

PUBLIC
void PollFds::RemoveFd(IN IMS_SINT32 nFd)
{
    if (m_nCount == 0)
    {
        // No entry
        return;
    }

    IMS_SINT16 i = 0;

    for (i = 0; i < m_nCount; ++i)
    {
        if (m_astFd[i].fd == nFd)
        {
            break;
        }
    }

    if (i < m_nCount)
    {
        for (IMS_SINT16 j = i; j < m_nCount; ++j)
        {
            m_astFd[j] = m_astFd[j + 1];
        }

        --m_nCount;

        IMS_TRACE_D("PollFds :: RemoveFd() - fd=%d, count=%d", nFd, m_nCount, 0);
    }
}

PUBLIC
pollfd* PollFds::ToArray(IN_OUT IMS_SINT32& nFds)
{
    nFds = m_nCount;

    if (m_nCount == 0)
    {
        return IMS_NULL;
    }

    return m_astFd;
}

PRIVATE
void PollFds::Init(IN const pollfd* pstFds, IN IMS_SINT32 nFds /*= (MAX_POLL_SIZE + 1)*/)
{
    if ((pstFds == IMS_NULL) || (nFds == 0))
    {
        for (IMS_SINT32 i = 0; i < (MAX_POLL_SIZE + 1); ++i)
        {
            m_astFd[i].fd = (-1);
            m_astFd[i].events = 0;
            m_astFd[i].revents = 0;
        }
    }
    else
    {
        if (nFds > (MAX_POLL_SIZE + 1))
        {
            nFds = (MAX_POLL_SIZE + 1);
        }

        IMS_MEM_Memcpy(m_astFd, pstFds, sizeof(pollfd) * nFds);
    }
}

PUBLIC
OsPollFdSet::OsPollFdSet() :
        ImsFdSet(),
        m_pFds(new PollFds())
{
}

PUBLIC
OsPollFdSet::OsPollFdSet(IN const OsPollFdSet& other) :
        ImsFdSet(other),
        m_pFds(IMS_NULL)
{
    if (other.m_pFds != IMS_NULL)
    {
        m_pFds = new PollFds(*(other.m_pFds));
    }
}

PUBLIC VIRTUAL OsPollFdSet::~OsPollFdSet()
{
    if (m_pFds != IMS_NULL)
    {
        delete m_pFds;
    }
}

PUBLIC
OsPollFdSet& OsPollFdSet::operator=(IN const OsPollFdSet& other)
{
    if (this != &other)
    {
        ImsFdSet::operator=(other);

        if (other.m_pFds != IMS_NULL)
        {
            if (m_pFds == IMS_NULL)
            {
                m_pFds = new PollFds(*(other.m_pFds));
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

PUBLIC VIRTUAL IMS_SINT32 OsPollFdSet::ClearEvent(IN IMS_SINT32 nFd, IN IMS_SINT32 nEvent)
{
    pollfd* pstFd = (m_pFds != IMS_NULL) ? m_pFds->GetFd(nFd) : IMS_NULL;

    if (pstFd == IMS_NULL)
    {
        return 0;
    }

    IMS_SINT32 nClearEvent = 0;

    if ((nEvent & EVENT_READ) != 0)
    {
        if (PollFds::IsEventSet(pstFd, PollFds::EVENT_READ))
        {
            PollFds::ClearEvent(pstFd, PollFds::EVENT_READ);
            nClearEvent |= EVENT_READ;
        }
    }

    if ((nEvent & EVENT_WRITE) != 0)
    {
        if (PollFds::IsEventSet(pstFd, PollFds::EVENT_WRITE))
        {
            PollFds::ClearEvent(pstFd, PollFds::EVENT_WRITE);
            nClearEvent |= EVENT_WRITE;
        }
    }

    if ((nEvent & EVENT_EXCEPT) != 0)
    {
        if (PollFds::IsEventSet(pstFd, PollFds::EVENT_EXCEPT))
        {
            PollFds::ClearEvent(pstFd, PollFds::EVENT_EXCEPT);
            nClearEvent |= EVENT_EXCEPT;
        }

        // When CLOSE event is cleared, it also needs to be cleared
        if ((nEvent & EVENT_TCP_C) != 0)
        {
            if (PollFds::IsEventSet(pstFd, PollFds::EVENT_EXCEPT_TCP_C))
            {
                PollFds::ClearEvent(pstFd, PollFds::EVENT_EXCEPT_TCP_C);
                nClearEvent |= EVENT_EXCEPT;
            }
        }
    }

    if (!PollFds::IsEventSet(pstFd, PollFds::EVENT_ALL))
    {
        if (m_pFds != IMS_NULL)
        {
            m_pFds->RemoveFd(nFd);
        }
    }

    return nClearEvent;
}

PUBLIC VIRTUAL void OsPollFdSet::CopyFrom(IN const ImsFdSet* pFdSet)
{
    const OsPollFdSet* pPollFdSet = DYNAMIC_CAST(const OsPollFdSet*, pFdSet);

    if (pPollFdSet != IMS_NULL)
    {
        operator=(*pPollFdSet);
    }
}

PUBLIC VIRTUAL IMS_SINT32 OsPollFdSet::GetSignaledEvents(
        IN IMS_SINT32 nFd, IN_OUT IMS_SINT32& nSignaledCount)
{
    IMS_SINT32 nSignaledEvents = 0;

    nSignaledCount = 0;

    pollfd* pstFd = (m_pFds != IMS_NULL) ? m_pFds->GetFd(nFd) : IMS_NULL;

    if (pstFd != IMS_NULL)
    {
        if (PollFds::IsEventSignaled(pstFd, PollFds::EVENT_READ))
        {
            nSignaledEvents |= EVENT_READ;
        }

        if (PollFds::IsEventSignaled(pstFd, PollFds::EVENT_WRITE))
        {
            nSignaledEvents |= EVENT_WRITE;
        }

        if (PollFds::IsEventSignaled(pstFd, PollFds::EVENT_EXCEPT))
        {
            nSignaledEvents |= EVENT_EXCEPT;
        }

        if (PollFds::IsEventSignaled(pstFd, PollFds::EVENT_EXCEPT_TCP_C))
        {
            nSignaledEvents |= EVENT_EXCEPT;
        }
    }

    if (nSignaledEvents != 0)
    {
        nSignaledCount = 1;
    }

    return nSignaledEvents;
}

PUBLIC VIRTUAL IMS_BOOL OsPollFdSet::IsEventSet(IN IMS_SINT32 nFd, IN IMS_SINT32 nEvent)
{
    pollfd* pstFd = (m_pFds != IMS_NULL) ? m_pFds->GetFd(nFd) : IMS_NULL;

    if (pstFd == IMS_NULL)
    {
        return IMS_FALSE;
    }

    nEvent = (nEvent & EVENT_ALL);

    if (nEvent == EVENT_READ)
    {
        return PollFds::IsEventSet(pstFd, PollFds::EVENT_READ);
    }
    else if (nEvent == EVENT_WRITE)
    {
        return PollFds::IsEventSet(pstFd, PollFds::EVENT_WRITE);
    }
    else if (nEvent == EVENT_EXCEPT)
    {
        return PollFds::IsEventSet(pstFd, PollFds::EVENT_EXCEPT);
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL IMS_SINT32 OsPollFdSet::SetEvent(IN IMS_SINT32 nFd, IN IMS_SINT32 nEvent)
{
    if (nFd < 0)
    {
        IMS_TRACE_E(0, "PollFdSet :: Invalid fd=%d", nFd, 0, 0);
        return 0;
    }

    pollfd* pstFd = (m_pFds != IMS_NULL) ? m_pFds->GetFd(nFd) : IMS_NULL;

    if (pstFd == IMS_NULL)
    {
        // Ignores the initial CLOSE event for TCP sockets
        if (((nEvent & EVENT_TCP) != 0) && ((nEvent & EVENT_ALL) == EVENT_EXCEPT))
        {
            IMS_TRACE_D("PollFdSet :: TCP CLOSE event will be set later; fd=%d", nFd, 0, 0);
            return 0;
        }

        if (m_pFds == IMS_NULL)
        {
            IMS_TRACE_E(0, "pFds is null", 0, 0, 0);
            return 0;
        }

        pstFd = m_pFds->AddFd(nFd);

        if (pstFd == IMS_NULL)
        {
            IMS_TRACE_E(0, "PollFdSet :: AddFd() failed; fd=%d", nFd, 0, 0);
            return 0;
        }
    }

    IMS_SINT32 nSetEvent = 0;

    if ((nEvent & EVENT_READ) != 0)
    {
        if (!PollFds::IsEventSet(pstFd, PollFds::EVENT_READ))
        {
            PollFds::SetEvent(pstFd, PollFds::EVENT_READ);
            nSetEvent |= EVENT_READ;
        }
    }

    if ((nEvent & EVENT_WRITE) != 0)
    {
        if (!PollFds::IsEventSet(pstFd, PollFds::EVENT_WRITE))
        {
            PollFds::SetEvent(pstFd, PollFds::EVENT_WRITE);
            nSetEvent |= EVENT_WRITE;
        }
    }

    if ((nEvent & EVENT_EXCEPT) != 0)
    {
        if (!PollFds::IsEventSet(pstFd, PollFds::EVENT_EXCEPT))
        {
            PollFds::SetEvent(pstFd, PollFds::EVENT_EXCEPT);
            nSetEvent |= EVENT_EXCEPT;
        }

        if ((nEvent & EVENT_TCP_C) != 0)
        {
            if (!PollFds::IsEventSet(pstFd, PollFds::EVENT_EXCEPT_TCP_C))
            {
                PollFds::SetEvent(pstFd, PollFds::EVENT_EXCEPT_TCP_C);
                nSetEvent |= EVENT_EXCEPT;
            }
        }
    }

    return nSetEvent;
}

PUBLIC VIRTUAL IMS_SINT32 OsPollFdSet::WaitForEvents(IN IMS_SINT32 nMilliseconds /*= NO_TIMEOUT*/)
{
    if (m_pFds == IMS_NULL)
    {
        return 0;
    }

    IMS_SINT32 nFds = 0;
    pollfd* pstFds = m_pFds->ToArray(nFds);

    return poll(pstFds, nFds, nMilliseconds);
}
