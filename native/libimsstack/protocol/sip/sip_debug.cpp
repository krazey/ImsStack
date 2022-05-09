#include "sip_pf_datatypes.h"
#include "platform/sip_pf_memory.h"
#include "platform/sip_pf_string.h"

#include "sip_debug.h"
#include "SipUtil.h"

/******************************************************************************
 * Function name    : SIP_DEBUG_LOG
 * Description    : This function logs the errors
 *                :
 *
 * Return type    : None
 *
 * Argument      :
 *    [IN]        : pszFilename[IN] - Filename
 *    [IN]        : nLine[IN]       - Line number in file.
 eError[IN]    - Error
 pcFormat[IN]     - Format String
 *
 * Side Effects    :
 * NOTE             :
 ******************************************************************************/
void SIP_DEBUG_LOG(SIP_UINT32 nCategory, SIP_CHAR* pszFilename, SIP_INT32 nLine,
    SIP_CHAR* pszFormat,...)
{
    SipUtil* pUtil = SipUtil_GetInstance();

    if (pUtil == SIP_NULL)
    {
        return;
    }

    SIP_CHAR szTemp[DEBUG_MSG_MAX_SIZE + 1] = {SIP_ZERO};

    va_list args;
    va_start(args, pszFormat);
    vsnprintf(szTemp, DEBUG_MSG_MAX_SIZE, pszFormat, args);
    va_end(args);

    SIP_CHAR* pszTempFilename = SipPf_Strdup(pszFilename);

    const SIP_CHAR* pTemp = (pszTempFilename != SIP_NULL) ?\
            SipPf_StripFileName(pszTempFilename) : "xxx";

    SIP_CHAR szFrmtString[DEBUG_MSG_MAX_SIZE + 1] = {SIP_ZERO};
    SipPf_Snprintf(szFrmtString, DEBUG_MSG_MAX_SIZE, "[%s:%d] %s", pTemp, nLine, szTemp);

    pUtil->GetLogger()->DumpLog(nCategory, SIP_NULL, nLine, szFrmtString);

    if (pszTempFilename != SIP_NULL)
    {
        delete[] pszTempFilename;
    }
}
