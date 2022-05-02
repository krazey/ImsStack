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
#include "util/ICancellableMethod.h"
#include "util/CancellableMethodManager.h"

__IMS_TRACE_TAG_IMS_CORE__;



PRIVATE
CancellableMethodManager::CancellableMethodManager()
    : piLock(IMS_NULL)
    , objCancellableMethods(IMSMap<AString, ICancellableMethod*>())
{
    piLock = MutexService::GetMutexService()->CreateMutex();
}

PRIVATE
CancellableMethodManager::~CancellableMethodManager()
{
    {
        LockGuard objLock(piLock);
        objCancellableMethods.Clear();
    }

    MutexService::GetMutexService()->DestroyMutex(piLock);
}

PUBLIC
IMS_BOOL CancellableMethodManager::AddMethod(IN CONST AString &strName,
        IN ICancellableMethod *piMethod)
{
    LockGuard objLock(piLock);

    //---------------------------------------------------------------------------------------------

    return objCancellableMethods.SetValue(strName, piMethod);
}

PUBLIC
void CancellableMethodManager::RemoveMethod(IN CONST AString &strName)
{
    LockGuard objLock(piLock);

    //---------------------------------------------------------------------------------------------

    objCancellableMethods.Remove(strName);
}

PUBLIC GLOBAL
CancellableMethodManager* CancellableMethodManager::GetInstance()
{
    static CancellableMethodManager *pCancellableMethodMngr = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (pCancellableMethodMngr == IMS_NULL)
    {
        pCancellableMethodMngr = new CancellableMethodManager();
    }

    return pCancellableMethodMngr;
}

PRIVATE
IMS_BOOL CancellableMethodManager::HandleCancelRequest(IN ISIPServerConnection *piSSC)
{
    LockGuard objLock(piLock);

    //---------------------------------------------------------------------------------------------

    if (objCancellableMethods.IsEmpty())
    {
        IMS_TRACE_D("There is no method to handle SIP CANCEL request", 0, 0, 0);
        return IMS_FALSE;
    }

    ICancellableMethod *piMethod = IMS_NULL;

    for (IMS_UINT32 i = 0; i < objCancellableMethods.GetSize(); ++i)
    {
        piMethod = objCancellableMethods.GetValueAt(i);

        if (piMethod->Cancellable_Compare(piSSC))
            break;

        piMethod = IMS_NULL;
    }

    if (piMethod == IMS_NULL)
    {
        IMS_TRACE_D("No matched cancellable method", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!piMethod->Cancellable_NotifyRequest(piSSC))
    {
        IMS_TRACE_E(0, "Handling an incoming CANCEL request failed", 0, 0, 0);
    }

    return IMS_TRUE;
}
