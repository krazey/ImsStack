/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090819  YR@                       Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceThread.h"
#include "IMSActivity.h"
#include "ServiceMessage.h"

__IMS_TRACE_TAG_ADAPT__;

PRIVATE GLOBAL
AString MSGService::GetThreadName(IN const AString& strTargetName)
{
    // threadname.activitiname
    IMS_SINT32 nIndex = strTargetName.GetIndexOf('.');

    return strTargetName.GetSubStr(0, nIndex);
}

PUBLIC GLOBAL
IMS_BOOL MSGService::PostMessage(IN const AString &strTarget, IN IMSMSG &objMSG)
{
    AString strThreadName = GetThreadName(strTarget);
    ThreadService *pThreadService = ThreadService::GetThreadService();

    pThreadService->LockThreadPool();

    IThread *piTargetThread = pThreadService->GetThread(strThreadName);
    IMS_BOOL bResult = IMS_FALSE;

    if (piTargetThread != IMS_NULL)
    {
        objMSG.SetTarget(strTarget.GetStr());

        bResult = piTargetThread->PostMessageI(objMSG);

        if (!bResult)
        {
            IMS_TRACE_D("PostMessage(failed) :: target=%s, msg=%d",
                    strTarget.GetStr(), objMSG.GetName(), 0);
        }
    }

    pThreadService->UnlockThreadPool();

    return bResult;
}

PUBLIC GLOBAL
IMS_BOOL MSGService::PostMessageThread(IN IThread *piTargetThread, IN IMSMSG &objMSG)
{
    if (piTargetThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ThreadService *pThreadService = ThreadService::GetThreadService();
    IMS_BOOL bResult = IMS_FALSE;

    pThreadService->LockThreadPool();

    if (pThreadService->Contains(piTargetThread))
    {
        bResult = piTargetThread->PostMessageI(objMSG);

        if (!bResult)
        {
            IMS_TRACE_D("PostMessage(failed) :: msg=%d", objMSG.GetName(), 0, 0);
        }
    }

    pThreadService->UnlockThreadPool();

    return bResult;
}

PUBLIC GLOBAL
IMS_BOOL MSGService::PostMessageActivity(IN IMSActivity *pTargetActivity, IN IMSMSG &objMSG)
{
    if (pTargetActivity == IMS_NULL)
    {
        return IMS_FALSE;
    }

    objMSG.SetTarget(pTargetActivity->GetName().GetStr());

    return pTargetActivity->PostMessage(objMSG);
}
