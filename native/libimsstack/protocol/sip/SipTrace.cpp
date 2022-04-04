/******************************************************************************
 * Project Name    : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************
 * Filename        : SipTrace.cpp
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

 ******************************************************************************/
/*****************************************************************************
  Header Inclusions
 *****************************************************************************/
#include "sip_pf_datatypes.h"
#include "platform/sip_pf_memory.h"
#include "platform/sip_pf_string.h"

#include "SipTrace.h"
#include "SipUtil.h"
#include "sip_error.h"

/****************************************************************************
  Global Variables
 *****************************************************************************/
SipTrace    *gpTrace = SIP_NULL;

/****************************************************************************
  Function Implementation [STARTS]
 *****************************************************************************/

/******************************************************************************
 * Function name    : SipTrace
 * Description    :
 *                :
 *
 * Return type    : None
 *
 * Argument      :
 *                :

 *
 * Side Effects    :
 * NOTE             :
 ******************************************************************************/
SipTrace::SipTrace()
{
    for (SIP_UINT16 nIndex = SIP_ZERO; nIndex < ESIPTRACE_MODEND; nIndex++)
    {
        m_ausModTrace[nIndex] =
            (ESIPTRACE_TYPEMESSAGE | ESIPTRACE_TYPENORMAL | ESIPTRACE_TYPEALL);
    }

    return;
}
/******************************************************************************
 * Function name    : ~SipTrace
 * Description    :
 *                :
 *
 * Return type    : None
 *
 * Argument      :
 *                :

 *
 * Side Effects    :
 * NOTE             :
 ******************************************************************************/
SipTrace::~SipTrace()
{
}

/******************************************************************************
 * Function name    : EnableTrace
 * Description    : This function will enable the given trace type t for the module.
 *                :
 *
 * Return type    : SIP_BOOL
 SIP_TRUE if setting trace type is
 successful
 *
 * Argument      :
 *    [IN]        : eModule[IN]   - Module to trace
 *    [IN]        : ulTraceType[IN] - Type of trace to be enabled.
 pusError[OUT]    - Error if any
 *
 * Side Effects    :
 * NOTE             :
 ******************************************************************************/
SIP_BOOL SipTrace::EnableTrace(SipEn_TraceModules eModule, SIP_UINT32 nTraceType)
{
    if (eModule >= ESIPTRACE_MODEND)
    {
        return SIP_FALSE;
    }

    m_ausModTrace[eModule] = nTraceType;
    return SIP_TRUE;
}

/******************************************************************************
 * Function name    : EnableTrace
 * Description    : This function will enable the given trace type for all the module.
 *                :
 *
 * Return type    : SIP_BOOL
 SIP_TRUE if setting trace type is
 successful
 *
 * Argument      :
 *    [IN]        : nTraceType[IN] - Type of trace to be enabled.
 pusError[OUT]    - Error if any
 *
 * Side Effects    :
 * NOTE             :
 ******************************************************************************/
SIP_BOOL SipTrace::EnableTrace(SIP_UINT32 nTraceType)
{
    for (SIP_UINT16 nIndex = SIP_ZERO; nIndex < ESIPTRACE_MODEND; nIndex++)
    {
        m_ausModTrace[nIndex] = nTraceType;
    }
    return SIP_TRUE;
}

/******************************************************************************
 * Function name    : DisableTrace
 * Description    : This function will disable the given trace type for the module.
 *                :
 *
 * Return type    : SIP_BOOL
 SIP_TRUE if setting trace type is
 successful
 *
 * Argument      :
 *    [IN]        : eModule[IN]   - Module
 *    [IN]        : nTraceType[IN] - Type of trace to be disabled.
 pusError[OUT]    - Error if any
 *
 * Side Effects    :
 * NOTE             :
 ******************************************************************************/
SIP_BOOL SipTrace::DisableTrace(SipEn_TraceModules eModule, SIP_UINT32 nTraceType)
{
    if (eModule >= ESIPTRACE_MODEND)
    {
        return SIP_FALSE;
    }

    m_ausModTrace[eModule] = m_ausModTrace[eModule] & ~nTraceType;
    return SIP_TRUE;
}

/******************************************************************************
 * Function name    : DisableTrace
 * Description    : This function will disable the given trace type for all the module.
 *                :
 *
 * Return type    : SIP_BOOL
 SIP_TRUE if setting trace type is
 successful
 *
 * Argument      :
 *    [IN]        : nTraceType[IN] - Type of trace to be enabled.
 pusError[OUT]    - Error if any
 *
 * Side Effects    :
 * NOTE             :
 ******************************************************************************/
SIP_BOOL SipTrace::DisableTrace(SIP_UINT32 nTraceType)
{
    for (SIP_UINT16 nIndex = SIP_ZERO; nIndex < ESIPTRACE_MODEND; nIndex++)
    {
        m_ausModTrace[nIndex] = m_ausModTrace[nIndex] & ~nTraceType;
    }
    return SIP_TRUE;
}

/******************************************************************************
 * Function name    : IsTraceEnable
 * Description    : This function check if for the given module particulr trace type is enable or not
 *                :
 *
 * Return type    : SIP_BOOL
 SIP_TRUE if trace type is enable
 *
 * Argument      :
 *    [IN]        : eModule[IN]   - Module
 *    [IN]        : nTraceType[IN] - Type of trace to.
 *
 * Side Effects    :
 * NOTE             :
 ******************************************************************************/
SIP_BOOL SipTrace::IsTraceEnable(SipEn_TraceModules eModule, SipEn_TraceTypes eTraceType)
{
    if (eModule >= ESIPTRACE_MODEND)
    {
        return SIP_FALSE;
    }

    if (m_ausModTrace[eModule] & eTraceType)
    {
        return SIP_TRUE;
    }

    return SIP_FALSE;
}


/******************************************************************************
 * Function name    : SIP_TRACE_LOG
 * Description    : This function logs the trace in case module is set for tracing.
 *                :
 *
 * Return type    : SIP_BOOL

 SIP_TRUE if logging of trace is successful
 *
 * Argument      :
 *    [IN]        : eModule[IN] : Module traced
 eTraceType[IN] - Trace type
 *    [IN]        : pszFilename[IN] - File traced.
 iLine[IN]    - Line number in file traced
 pcFormat[IN] - Format String
 *
 * Side Effects    :
 * NOTE             :
 ******************************************************************************/
void SIP_TRACE_LOG(SIP_UINT32 nCategory, SIP_CHAR* pszFilename, SIP_INT32 nLine,
        SIP_CHAR* pszFormat,...)
{
    SipUtil* pUtil = SipUtil_GetInstance();
    SipTrace* pTrace = SipTrace_GetInstance();

    if ((pTrace == SIP_NULL) || (pUtil == SIP_NULL))
    {
        return;
    }

    va_list args;
    va_start(args, pszFormat);
    SIP_CHAR szTemp[SIP_TRACE_MAX_SIZE + 1]= {SIP_ZERO};
    vsnprintf(szTemp, SIP_TRACE_MAX_SIZE, pszFormat, args);
    va_end(args);

    SIP_CHAR* pszTempFilename = SipPf_Strdup(pszFilename);
    const SIP_CHAR* pTemp = (pszTempFilename != SIP_NULL) ?\
            SipPf_StripFileName(pszTempFilename) : "xxx";

    SIP_CHAR szFrmtString[SIP_TRACE_MAX_SIZE + 1] = {SIP_ZERO};
    SipPf_Snprintf(szFrmtString, SIP_TRACE_MAX_SIZE, "[%s:%d] %s", pTemp, nLine, szTemp);

    pUtil->GetLogger()->DumpLog(nCategory, SIP_NULL, nLine, szFrmtString);

    if (pszTempFilename != SIP_NULL)
    {
        delete[] pszTempFilename;
    }
}

/****************************************************************************
  Local Function Implementation [ENDS]
 *****************************************************************************/
/****************************************************************************
  Global Function Implementation [START]
 *****************************************************************************/
void SipTrace_Construct()
{
    SipTrace* pTrace = gpTrace;

    if (pTrace)
    {
        return;
    }

    pTrace = new SipTrace();
    gpTrace = pTrace;
}

void SipTrace_Destruct()
{
    SipTrace* pTrace = gpTrace;

    if (pTrace == SIP_NULL)
    {
        return;
    }

    delete pTrace;
    gpTrace = SIP_NULL;
}

SipTrace* SipTrace_GetInstance()
{
    SipTrace* pTrace = gpTrace;
    return pTrace;
}

/****************************************************************************
  global Function Implementation [ENDS]
 *****************************************************************************/
