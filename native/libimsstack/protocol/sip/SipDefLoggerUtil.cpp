#include "sip_pf_datatypes.h"
#include "platform/sip_pf_string.h"
#include "SipDefLoggerUtil.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_SIP__;

/******************************************************************************
 * Function name    : SipDefLoggerUtill
 * Description    : Constructor creates and opens the log file.
 *                :
 *
 * Return type    : None

 *
 * Argument      :

 *
 * Side Effects    :
 * NOTE             :
 ******************************************************************************/
SipDefLoggerUtil::SipDefLoggerUtil()
{
}

/******************************************************************************
 * Function name    : ~SipDefLoggerUtill
 * Description    : This function closes the log file.
 *                :
 *
 * Return type    : None

 *
 * Argument      :

 *
 * Side Effects    :
 * NOTE             :
 ******************************************************************************/
SipDefLoggerUtil::~SipDefLoggerUtil()
{
}
/******************************************************************************
 * Function name    : DumpLog
 * Description    : This function logs the errors either to console or filesystem
 *                :
 *
 * Return type    : None
 *
 * Argument      :
 * [IN]            : pszData[IN]       - NULL terminate string to log.

 *
 * Side Effects    :
 * NOTE             :
 ******************************************************************************/
void SipDefLoggerUtil::DumpLog( SIP_UINT32 nCategory, const SIP_CHAR* /*pszFile*/,
        SIP_UINT16 /*nLine*/, const SIP_CHAR* pszFormat, ...)
{
    if (TraceService::GetTraceService()->GetTrace()->IsTraceEnabled(
            nCategory, __IMS_TRACE_MODULE__) )
    {
        va_list args;
        va_start(args, pszFormat);
        const SIP_CHAR *strTag = __IMS_TRACE_NAME__;
        SIP_UINT32 nModule = __IMS_TRACE_MODULE__;

        TraceService::GetTraceService()->GetTrace()->OutV(nCategory, strTag,
                (nModule), pszFormat, args);
        va_end(args);
    }
}
