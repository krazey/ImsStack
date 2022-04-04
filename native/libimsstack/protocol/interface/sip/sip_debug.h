/******************************************************************************
 * Project Name    : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************
 * Filename        : fwk_debug.h
 * Purpose        :  debug Functions
 * Platform        : Windows OR Android
 * Author(s)        : Seema
 * E-mail id.        : seema.lijo@
 * Creation date     : april 6,2010
 *
 * Edit History             Modification                     Description(s)
 *
 * Date                Name            Version        Bug-ID        Description
 * ----------        ----------        -------        ------        -------------
 * april 6,10          Seema              0.0a            ---   Initial creation
 * July,21,2010        Giridhar        0.0b        ---            Coverted to cpp

 *****************************************************************************/
//SIP_DEBUG_ENABLE to be set in settings for enabling debugging
#ifndef __SIP_DEBUG_H__
#define __SIP_DEBUG_H__

/*****************************************************************************
  Header Inclusions
 *****************************************************************************/
#include "sip_pf_datatypes.h"
#include "sip_error.h"

#define SIP_DEBUG_ENABLE
#define DEBUG_MSG_MAX_SIZE    500

#ifdef SIP_DEBUG_ENABLE
#define SIP_DEBUG_WARNING(a,b,c,d)           SIP_DEBUG_LOG(CAT_D,(SIP_CHAR*)__FILE__, \
        __LINE__,(SIP_CHAR*) b,\
        c,d)
#define SIP_DEBUG_EXTRLBUG(a,b,c,d)          SIP_DEBUG_LOG(CAT_D,(SIP_CHAR*)__FILE__, \
        __LINE__,(SIP_CHAR*) b, \
        c,d)
#define SIP_DEBUG_STACKBUG(a,b,c,d)          SIP_DEBUG_LOG(CAT_D,(SIP_CHAR*)__FILE__, \
        __LINE__,(SIP_CHAR*) b, \
        c,d)
//Added New INFO log capturing macro function
#define SIP_TRACE_I(a,b,c)                   SIP_DEBUG_LOG(CAT_I,(SIP_CHAR*)__FILE__, \
        __LINE__,a,\
        b,c)
//Added New ERROR log capturing macro function
#define SIP_TRACE_E(a,b,c)                   SIP_DEBUG_LOG(CAT_E,(SIP_CHAR*)__FILE__, \
        __LINE__,a,\
        b,c)
//Added New DEBUG og capturing macro function
#define SIP_TRACE_D(a,b,c)                   SIP_DEBUG_LOG(CAT_D,(SIP_CHAR*)__FILE__, \
        __LINE__,a,\
        b,c)
#else
#define SIP_DEBUG_WARNING(a,b,c,d)
#define SIP_DEBUG_EXTRLBUG(a,b,c,d)
#define SIP_DEBUG_STACKBUG(a,b,c,d)
#define SIP_TRACE_I(a,b,c)
#define SIP_TRACE_E(a,b,c)
#define SIP_TRACE_D(a,b,c)
#endif

/****************************************************************************
  Enum Declaration
 *****************************************************************************/
typedef enum   _SipEn_DebugTypes
{
    ESIPDEBUG_WARNING = SIP_ZERO,        /*when set logs expected error
                         or stack limitations
                         eg:invalid header input*/
    ESIPDEBUG_EXTERNALRSRC,    /*logs memory,transport,network,timer,
                      os related     errors*/
    ESIPDEBUG_STACKBUG,        /*logs bugs in stack    */
    ESIPDEBUG_END,
    ESIPDEBUG_INVALID    = SIP_INVALID
}SipEn_DebugTypes;

/****************************************************************************
  Declaration of Functions
 *****************************************************************************/
/******************************************************************************
 * Function name    : SIP_DEBUG_LOG
 * Description    : This function logs the errors
 *                :
 *
 * Return type    : none
 *
 * Argument      :
 *    [IN]        : pszFilename[IN]  - Filename
 *    [IN]        : iLine[IN]           - Line number in file.
 eError[IN]        - Error
 pszFormat[IN]     - Format String
 *
 * Side Effects    :
 * NOTE             :
 ******************************************************************************/
void SIP_DEBUG_LOG(SIP_UINT32 nCategory, SIP_CHAR* pszFilename, SIP_INT32 nLine,
        SIP_CHAR* pszFormat,...);

#endif //__SIP_DEBUG_H__
