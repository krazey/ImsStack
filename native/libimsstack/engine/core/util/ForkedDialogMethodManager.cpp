/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100713  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceMutex.h"
#include "ISipDialog.h"
#include "ISipServerConnection.h"
#include "util/IForkedDialogMethod.h"
#include "util/ForkedDialogMethodManager.h"

__IMS_TRACE_TAG_IMS_CORE__;

PRIVATE
ForkedDialogMethodManager::ForkedDialogMethodManager() :
        piLock(IMS_NULL),
        objDialogMethods(IMSMap<AString, IForkedDialogMethod*>())
{
    piLock = MutexService::GetMutexService()->CreateMutex();
}

PRIVATE
ForkedDialogMethodManager::~ForkedDialogMethodManager()
{
    {
        LockGuard objLock(piLock);
        objDialogMethods.Clear();
    }

    MutexService::GetMutexService()->DestroyMutex(piLock);
}

PUBLIC
IMS_BOOL ForkedDialogMethodManager::AddMethod(
        IN CONST AString& strName, IN IForkedDialogMethod* piMethod)
{
    LockGuard objLock(piLock);

    //---------------------------------------------------------------------------------------------

    return objDialogMethods.SetValue(strName, piMethod);
}

PUBLIC
void ForkedDialogMethodManager::RemoveMethod(IN CONST AString& strName)
{
    LockGuard objLock(piLock);

    //---------------------------------------------------------------------------------------------

    objDialogMethods.Remove(strName);
}

PUBLIC GLOBAL ForkedDialogMethodManager* ForkedDialogMethodManager::GetInstance()
{
    static ForkedDialogMethodManager* pForkedDialogMethodMngr = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (pForkedDialogMethodMngr == IMS_NULL)
    {
        pForkedDialogMethodMngr = new ForkedDialogMethodManager();
    }

    return pForkedDialogMethodMngr;
}

PRIVATE
IMS_BOOL ForkedDialogMethodManager::HandleRequestWithinDialog(
        IN ISipServerConnection* piSSC, IN ISipDialog* piOrigDialog)
{
    LockGuard objLock(piLock);

    //---------------------------------------------------------------------------------------------

    if (objDialogMethods.IsEmpty())
    {
        IMS_TRACE_D("There is no method to handle SIP request within a dialog (2nd)", 0, 0, 0);
        return IMS_FALSE;
    }

    IForkedDialogMethod* piMethod = IMS_NULL;

    for (IMS_UINT32 i = 0; i < objDialogMethods.GetSize(); ++i)
    {
        piMethod = objDialogMethods.GetValueAt(i);

        if (piMethod->ForkedDialog_Compare(piOrigDialog))
            break;

        piMethod = IMS_NULL;
    }

    if (piMethod == IMS_NULL)
    {
        IMS_TRACE_E(0, "No matched dialog method", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!piMethod->ForkedDialog_NotifyRequest(piSSC))
    {
        IMS_TRACE_E(0, "Handling an incoming SIP forked request within a dialog failed", 0, 0, 0);
    }

    return IMS_TRUE;
}
