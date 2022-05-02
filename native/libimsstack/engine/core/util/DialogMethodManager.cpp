/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100518  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceMutex.h"
#include "ISipDialog.h"
#include "ISipServerConnection.h"
#include "util/IDialogMethod.h"
#include "util/DialogMethodManager.h"

__IMS_TRACE_TAG_IMS_CORE__;



PRIVATE
DialogMethodManager::DialogMethodManager()
    : piLock(IMS_NULL)
    , objDialogMethods(IMSMap<AString, IDialogMethod*>())
{
    piLock = MutexService::GetMutexService()->CreateMutex();
}

PRIVATE
DialogMethodManager::~DialogMethodManager()
{
    {
        LockGuard objLock(piLock);
        objDialogMethods.Clear();
    }

    MutexService::GetMutexService()->DestroyMutex(piLock);
}

PUBLIC
IMS_BOOL DialogMethodManager::AddMethod(IN CONST AString &strName, IN IDialogMethod *piMethod)
{
    LockGuard objLock(piLock);

    //---------------------------------------------------------------------------------------------

    return objDialogMethods.SetValue(strName, piMethod);
}

PUBLIC
void DialogMethodManager::RemoveMethod(IN CONST AString &strName)
{
    LockGuard objLock(piLock);

    //---------------------------------------------------------------------------------------------

    objDialogMethods.Remove(strName);
}

PUBLIC
IMS_BOOL DialogMethodManager::IsEmpty() const
{
    LockGuard objLock(piLock);

    //---------------------------------------------------------------------------------------------

    return objDialogMethods.IsEmpty();
}

PUBLIC GLOBAL
DialogMethodManager* DialogMethodManager::GetInstance()
{
    static DialogMethodManager *pDialogMethodMngr = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (pDialogMethodMngr == IMS_NULL)
    {
        pDialogMethodMngr = new DialogMethodManager();
    }

    return pDialogMethodMngr;
}

PRIVATE
IMS_BOOL DialogMethodManager::HandleRequestWithinDialog(IN ISIPServerConnection *piSSC)
{
    LockGuard objLock(piLock);

    //---------------------------------------------------------------------------------------------

    if (objDialogMethods.IsEmpty())
    {
        IMS_TRACE_D("There is no method to handle SIP request within a dialog (2nd)", 0, 0, 0);
        return IMS_FALSE;
    }

    IDialogMethod *piMethod = IMS_NULL;

    for (IMS_UINT32 i = 0; i < objDialogMethods.GetSize(); ++i)
    {
        piMethod = objDialogMethods.GetValueAt(i);

        if (piMethod->Dialog_Compare(piSSC))
            break;

        piMethod = IMS_NULL;
    }

    if (piMethod == IMS_NULL)
    {
        IMS_TRACE_E(0, "No matched dialog method", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!piMethod->Dialog_NotifyRequest(piSSC))
    {
        IMS_TRACE_E(0, "Handling an incoming SIP request within a dialog failed", 0, 0, 0);
    }

    return IMS_TRUE;
}
