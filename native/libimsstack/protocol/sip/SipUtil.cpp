/******************************************************************************
 * Project Name    : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************
 * Filename        : SipUtil.cpp
 * Purpose        :  utility Functions
 * Platform        : Windows OR Android
 * Author(s)        : Seema
 * E-mail id.        : seema.lijo@
 * Creation date     : may 4,2010
 *
 * Edit History             Modification                     Description(s)
 *
 * Date                Name            Version        Bug-ID        Description
 * ----------        ----------        -------        ------        -------------
 * may 4,2010         Seema          0.0a        ---            Initial creation
 * July,21,2010        Giridhar        0.0b        ---            Coverted to cpp

 ****************************************************************************/
/*****************************************************************************
  Header Inclusions
 *****************************************************************************/
#include "sip_pf_datatypes.h"
#include "platform/sip_pf_string.h"

#include "SipUtil.h"
#include "sip_error.h"

#include "SipDefTimerUtil.h"
#include "SipDefLoggerUtil.h"
#include "SipDefNetworkUtil.h"

/****************************************************************************
  Macro Definitions
 *****************************************************************************/

/****************************************************************************
  Global Extern Variables
 *****************************************************************************/

/****************************************************************************
  Global Variables
 *****************************************************************************/
static SipUtil *gpUtil = SIP_NULL;

/****************************************************************************
  Local Function Declaration [Static Functions]
 *****************************************************************************/

/****************************************************************************
  Local Function Implementation [STARTS]
 *****************************************************************************/


/****************************************************************************
  Local Function Implementation [ENDS]
 *****************************************************************************/

/****************************************************************************
  Function Implementation [STARTS]
 *****************************************************************************/
SipUtil::SipUtil()
    : m_pTxnListener(SIP_NULL)
{
    /* Create Default In-Build Services */
    m_pLoggerUtil = new SipDefLoggerUtil();
    m_pTimerUtil = new SipDefTimerUtil();
    m_pNetworkUtil = new SipDefNetworkUtil();
}

SipUtil::~SipUtil()
{
    if (m_pTxnListener != SIP_NULL)
    {
        delete m_pTxnListener;
    }

    if (m_pNetworkUtil != SIP_NULL)
    {
        delete m_pNetworkUtil;
    }

    if (m_pTimerUtil != SIP_NULL)
    {
        delete m_pTimerUtil;
    }

    if (m_pLoggerUtil != SIP_NULL)
    {
        delete m_pLoggerUtil;
    }

}

SIP_VOID SipUtil::RegisterTimer(ISipTimerUtil* pTimerUtil)
{
    if (m_pTimerUtil != SIP_NULL)
    {
        delete m_pTimerUtil;
    }

    m_pTimerUtil = pTimerUtil;
}

SIP_VOID SipUtil::RegisterLogger(ISipLoggerUtil* pLoggerUtil)
{
    if (m_pLoggerUtil != SIP_NULL)
    {
        delete m_pLoggerUtil;
    }

    m_pLoggerUtil = pLoggerUtil;
}
SIP_VOID SipUtil::RegisterNetwork(ISipNetworkUtil* pNwUtil)
{
    if (m_pNetworkUtil != SIP_NULL)
    {
        delete m_pNetworkUtil;
    }

    m_pNetworkUtil = pNwUtil;
}

SIP_VOID SipUtil::RegisterTxnListener(ISipTxnListener* pTxnListener)
{
    if (m_pTxnListener != SIP_NULL)
    {
        delete m_pTxnListener;
    }

    m_pTxnListener = pTxnListener;
}

ISipTimerUtil* SipUtil::GetTimer()
{
    return m_pTimerUtil;
}

ISipLoggerUtil* SipUtil::GetLogger()
{
    return m_pLoggerUtil;
}
ISipNetworkUtil* SipUtil::GetNetwork()
{
    return m_pNetworkUtil;
}

ISipTxnListener* SipUtil::GetTxnListener()
{
    return m_pTxnListener;
}

/****************************************************************************
  Function Implementation [ENDS]
 ****************************************************************************/
/****************************************************************************
  Global Function Implementation [START]
 *****************************************************************************/
SIP_VOID SipUtil_Construct()
{
    SipUtil* pUtil = gpUtil;

    if (pUtil)
    {
        return;
    }

    pUtil = new SipUtil();
    gpUtil = pUtil;
}

SIP_VOID SipUtil_Destruct()
{
    SipUtil* pUtil = gpUtil;

    if (pUtil == SIP_NULL)
    {
        return;
    }

    delete pUtil;
    gpUtil = SIP_NULL;
}

SipUtil* SipUtil_GetInstance()
{
    SipUtil* pUtil = gpUtil;
    return pUtil;
}

/****************************************************************************
  global Function Implementation [ENDS]
 *****************************************************************************/
