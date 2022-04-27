/***********************************************************
 * Project Name : MSG2_SIP_RTP
 * Group        : MSG2
 * Security     : Confidential
 ***********************************************************/

/**********************************************************
 * Filename          : sip_IDefTimerUtil.cpp
 * Purpose           : Default implementation of timer service
 * Platform          : (E.g. Windows 2000)
 * Author(s)         :
 * E-mail id.        :
 * Creation date      : Jul 31 , 2010
 *
 * Edit History             Modification                     Description(s)
 *
 * Date                Name            Version        Bug-ID        Description
 * ----------        ----------        -------        ------        -------------

 *****************************************************************************
 *****************************************************************************
 Header Inclusions
 *****************************************************************************/
#include "sip_pf_datatypes.h"
#include "platform/sip_pf_string.h"
//#include "ITimer.h"
#include "SipDefTimerUtil.h"
#include "ServiceTimer.h"
/****************************************************************************
  Macro Definitions
 *****************************************************************************/

/****************************************************************************
  Global Extern Variables
 *****************************************************************************/
extern SIP_BOOL sip_cbk_startTimer(IN SIP_UINT32 nDuration,
        IN SipTimerCallback pfnTimerCallback, IN SIP_VOID* pvData, IN SIP_VOID** ppvHandle);
extern SIP_BOOL sip_cbk_stopTimer(IN SIP_VOID* pvHandle, IN SIP_VOID** ppvData);

/****************************************************************************
  Global Variables
 *****************************************************************************/

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




SipDefTimerUtil::SipDefTimerUtil()
{

}
SipDefTimerUtil::~SipDefTimerUtil()
{

}

SIP_BOOL SipDefTimerUtil::StartTimer(SIP_VOID** ppvTimerId, SIP_UINT32 nDuration,
        SIP_UINT16 nResetFlag, SipTimerCallback pfnTimerCallback, SIP_VOID* pvData)
{
    if (ppvTimerId == SIP_NULL)
    {
        return SIP_FALSE;
    }

    (void)nResetFlag;

    return sip_cbk_startTimer(nDuration, pfnTimerCallback, pvData, ppvTimerId);

}
SIP_VOID* SipDefTimerUtil::StopTimer(SIP_VOID* pvTimerId)
{
    if (pvTimerId == SIP_NULL)
    {
        return SIP_NULL;
    }

    SIP_VOID* pvData = SIP_NULL;
    sip_cbk_stopTimer(pvTimerId, &pvData);

    return pvData;
}

SIP_VOID* SipDefTimerUtil::StopTimerEx(SIP_VOID* pvTimerId)
{
    return StopTimer(pvTimerId);
}

SIP_BOOL SipDefTimerUtil::ResetTimer(SIP_VOID* pvTimerId, SIP_UINT32 nNewDuration)
{
    (void)pvTimerId;
    (void)nNewDuration;
    return SIP_FALSE;
}

/****************************************************************************
  Function Implementation [ENDS]
 *****************************************************************************/
