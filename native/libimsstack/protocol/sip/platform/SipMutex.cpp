
/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename          : SipMutex.cpp
 * Purpose           :
 * Platform          : Windows
 * Author(s)         :
 * E-mail id.        :
 * Creation date        : 03 May 11
 *
 * Edit History             Modification                         Description(s)
 *
 * Date                Name            Version        Bug-ID        Description
 * ----------        ----------        -------        ------        -------------
 * 03 May 11        birender          --                    Initial version
 *****************************************************************************/

#include "sip_pf_datatypes.h"
#include "platform/sip_pf_string.h"
#include "platform/sip_pf_memory.h"
#include "platform/SipMutex.h"
#include "pthread.h"

SipMutex::SipMutex()
    : pMutex(SIP_NULL)
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
    pthread_mutex_t* pThreadMutex = (pthread_mutex_t *)pMutex;

    if (pThreadMutex != SIP_NULL)
    {
        pthread_mutex_destroy(pThreadMutex);
        delete pThreadMutex;
    }
}

void SipMutex::Lock()
{
    pthread_mutex_t* pThreadMutex = (pthread_mutex_t *)pMutex;

    if (pThreadMutex == SIP_NULL)
    {
        return;
    }

    pthread_mutex_lock(pThreadMutex);
}

void SipMutex::TryLock()
{
    pthread_mutex_t* pThreadMutex = (pthread_mutex_t *)pMutex;

    if (pThreadMutex == SIP_NULL)
    {
        return;
    }

    pthread_mutex_trylock(pThreadMutex);
}

void SipMutex::Unlock()
{
    pthread_mutex_t* pThreadMutex = (pthread_mutex_t *)pMutex;

    if (pThreadMutex == SIP_NULL)
    {
        return;
    }

    pthread_mutex_unlock(pThreadMutex);
}
