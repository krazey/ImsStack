/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100324  joonhun.shin@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceThread.h"

#include "IMSProcess.h"
#include "IMSActivity.h"

#if 0  // public
#endif

PUBLIC
IMSActivity::IMSActivity(IN CONST AString& strName_ /*= AString::ConstNull() */)
{
    pIThread = ThreadService::GetThreadService()->GetCurrentThread();
    const AString& strThreadName =
            (pIThread != IMS_NULL) ? pIThread->GetName() : AString::ConstEmpty();
    IMSAppThread* pIMSAppThread = IMSProcess::GetInstance()->GetApplicationThread(strThreadName);

    if (pIMSAppThread != IMS_NULL)
    {
        IMSActivityMngr* pIMSActivityMngr = pIMSAppThread->GetActivityMngr();

        strName = pIMSActivityMngr->GenerateName(strThreadName, strName_);
        pIMSActivityMngr->Attach(this);
    }
}

PUBLIC VIRTUAL IMSActivity::~IMSActivity()
{
    AString strThreadName = GetOwnerThreadName(strName);
    IMSAppThread* pIMSAppThread = IMSProcess::GetInstance()->GetApplicationThread(strThreadName);

    if (pIMSAppThread != IMS_NULL)
    {
        pIMSAppThread->GetActivityMngr()->Detach(this);
    }
}

PUBLIC
IMS_BOOL IMSActivity::PostMessage(IN IMSMSG& objMSG)
{
    if (pIThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    objMSG.SetTarget(strName.GetStr());

    return pIThread->PostMessageI(objMSG);
}

PUBLIC
IMS_BOOL IMSActivity::PostMessage(IN IMS_UINT32 nMSG, IN IMS_UINTP nWparam, IN IMS_UINTP nLparam)
{
    IMSMSG objMSG(nMSG, nWparam, nLparam);

    return PostMessage(objMSG);
}

#if 0  // protected
#endif

#if 0  // private
#endif

PRIVATE
AString IMSActivity::GetOwnerThreadName(CONST AString& strTargetName)
{
    // threadname.activityname
    IMS_SINT32 nIndex = strTargetName.GetIndexOf('.');

    return strTargetName.GetSubStr(0, nIndex);
}
