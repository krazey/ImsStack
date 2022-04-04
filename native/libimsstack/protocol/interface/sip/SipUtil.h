/******************************************************************************
 * Project Name    : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************
 * Filename        : SipUtil.h
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
 * may 4,2010          Seema              0.0a            ---   Initial creation
 * July,21,2010        Giridhar        0.0b        ---            Coverted to cpp

 *****************************************************************************/
#ifndef __SIP_UTIL_H__
#define __SIP_UTIL_H__

#include "ISipLoggerUtil.h"
#include "ISipNetworkUtil.h"
#include "ISipTimerUtil.h"
#include "ISipTxnListener.h"

/*****************************************************************************
  Header Inclusions
 *****************************************************************************/

/****************************************************************************
  Macro Definitions
 *****************************************************************************/

/****************************************************************************
  Enum Declaration
 *****************************************************************************/

/****************************************************************************
  Structure/Class Declaration
 *****************************************************************************/
class SipUtil
{
    public:
        SipUtil();
        virtual ~SipUtil();

        SIP_VOID RegisterTimer(ISipTimerUtil* pTimerUtil);
        SIP_VOID RegisterLogger(ISipLoggerUtil* pLoggerUtil);
        SIP_VOID RegisterNetwork(ISipNetworkUtil* pNwUtil);
        SIP_VOID RegisterTxnListener(ISipTxnListener* pTxnListener);
        ISipTimerUtil* GetTimer();
        ISipLoggerUtil* GetLogger();
        ISipNetworkUtil* GetNetwork();
        ISipTxnListener* GetTxnListener();

    private:
        ISipTimerUtil* m_pTimerUtil;
        ISipLoggerUtil* m_pLoggerUtil;
        ISipNetworkUtil* m_pNetworkUtil;
        ISipTxnListener* m_pTxnListener;

        /*******************************************************************
          Private Member Functions
         ********************************************************************/
        SipUtil& operator=(IN const SipUtil& objRHS);
        SipUtil(IN const SipUtil& objRHS);
};


/****************************************************************************
  Declaration of Functions
 *****************************************************************************/
void SipUtil_Construct();
void SipUtil_Destruct();
SipUtil* SipUtil_GetInstance();

#endif //__SIP_UTIL_H__
