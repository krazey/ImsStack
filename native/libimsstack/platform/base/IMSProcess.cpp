/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100323  joonhun.shin@             Created
    20110525  joonhun.shin@             remove trace
    20130328  joonhun.shin@             add muntex
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceMutex.h"
#include "IMSFramework.h"
#include "IMSProcess.h"

__IMS_TRACE_TAG_FWK__;

#define IMS_FRAMEWORK_THREAD "Framework"

class IMSThreadMap
{
public:
    inline IMSThreadMap(IN const AString& strName_, IN BaseThread* pThread_) :
            strName(strName_),
            pThread(pThread_)
    {
    }

public:
    AString strName;
    BaseThread* pThread;
};

#if 0  // private
#endif

PRIVATE
IMSProcess::IMSProcess() :
        strFrameworkThreadName(IMS_FRAMEWORK_THREAD),
        piLock(IMS_NULL),
        objThreads(IMSList<IMSThreadMap*>())
{
    piLock = MutexService::GetMutexService()->CreateMutex();
}

PRIVATE
IMSProcess::~IMSProcess()
{
    MutexService::GetMutexService()->DestroyMutex(piLock);
}

#if 0  // public
#endif

PUBLIC GLOBAL IMSProcess* IMSProcess::GetInstance()
{
    static IMSProcess* pIMSProcess = IMS_NULL;

    if (pIMSProcess == IMS_NULL)
    {
        pIMSProcess = new IMSProcess();
    }

    return pIMSProcess;
}

PUBLIC
const AString& IMSProcess::GetFrameworkThreadName() const
{
    return strFrameworkThreadName;
}

PUBLIC
IMS_BOOL IMSProcess::Initialize()
{
    const AString& strFwkThreadName = GetFrameworkThreadName();

    if (GetApplicationThread(strFwkThreadName) != IMS_NULL)
    {
        return IMS_TRUE;
    }

    // Start a default thread
    BaseThread* pThread = new IMSFramework();

    if (pThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    (void)AttachThread(strFwkThreadName, pThread);

    pThread->Start(strFwkThreadName);

    return IMS_TRUE;
}

PUBLIC
void IMSProcess::Uninitialize()
{
    IMS_TRACE_I("Uninitialize :: Thread count (%d)", objThreads.GetSize(), 0, 0);

    LockGuard objLock(piLock);

    while (!objThreads.IsEmpty())
    {
        IMSThreadMap* pThreadMap = objThreads.GetAt(0);
        if (pThreadMap->pThread != IMS_NULL)
        {
            pThreadMap->pThread->Terminate();
            delete pThreadMap->pThread;
        }
        delete pThreadMap;
        objThreads.RemoveAt(0);
    }
}

PUBLIC
IMS_BOOL IMSProcess::LoadThread(IN const AString& strThreadName, IN Thread_Entry pfnThreadEntry,
        IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/)
{
    if (GetThread(strThreadName) != IMS_NULL)
    {
        return IMS_FALSE;
    }

    BaseThread* pThread = pfnThreadEntry();

    if (pThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    (void)AttachThread(strThreadName, pThread);

    pThread->Start(strThreadName, nSlotId);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL IMSProcess::LoadThreadWithParam(IN const AString& strThreadName,
        IN Thread_EntryEx pfnThreadEntryEx, IN void* pvParam,
        IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/)
{
    if (GetThread(strThreadName) != IMS_NULL)
    {
        return IMS_FALSE;
    }

    BaseThread* pThread = pfnThreadEntryEx(pvParam);

    if (pThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    (void)AttachThread(strThreadName, pThread);

    pThread->Start(strThreadName, nSlotId);

    return IMS_TRUE;
}

PUBLIC
void IMSProcess::UnloadThread(IN const AString& strThreadName)
{
    BaseThread* pThread = GetThread(strThreadName);

    if (pThread != IMS_NULL)
    {
        pThread->Terminate();

        DetachThread(strThreadName);
        delete pThread;
    }
}

PUBLIC
BaseThread* IMSProcess::GetThread(IN const AString& strThreadName)
{
    LockGuard objLock(piLock);

    for (IMS_UINT32 i = 0; i < objThreads.GetSize(); ++i)
    {
        IMSThreadMap* pThreadMap = objThreads.GetAt(i);

        if (pThreadMap->strName.Equals(strThreadName))
        {
            return pThreadMap->pThread;
        }
    }

    IMS_TRACE_I("GetThread :: Thread(%s) not found", strThreadName.GetStr(), 0, 0);
    return IMS_NULL;
}

PUBLIC
IMS_BOOL IMSProcess::LoadAppThread(IN const AString& strThreadName,
        IN AppThread_Entry pfnThreadEntry, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/)
{
    if (GetThread(strThreadName) != IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMSAppThread* pThread = pfnThreadEntry();

    if (pThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    (void)AttachThread(strThreadName, pThread);

    pThread->Start(strThreadName, nSlotId);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL IMSProcess::LoadAppThreadWithParam(IN const AString& strThreadName,
        IN AppThread_EntryEx pfnThreadEntryEx, IN void* pvParam,
        IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/)
{
    if (GetThread(strThreadName) != IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMSAppThread* pThread = pfnThreadEntryEx(pvParam);

    if (pThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    (void)AttachThread(strThreadName, pThread);

    pThread->Start(strThreadName, nSlotId);

    return IMS_TRUE;
}

PUBLIC
void IMSProcess::UnloadAppThread(IN const AString& strThreadName)
{
    UnloadThread(strThreadName);
}

PUBLIC
IMSAppThread* IMSProcess::GetApplicationThread(IN const AString& strThreadName)
{
    return DYNAMIC_CAST(IMSAppThread*, GetThread(strThreadName));
}

PUBLIC
IIMSActivityControl* IMSProcess::GetController(IN const AString& strControllerName)
{
    AString strThreadName = GetThreadName(strControllerName);
    IMSAppThread* pAppThread = GetApplicationThread(strThreadName);

    IMS_TRACE_D("GetController :: Controller(%s), Thread(%s)", strControllerName.GetStr(),
            strThreadName.GetStr(), 0);

    if (pAppThread != IMS_NULL)
    {
        return pAppThread->GetActivityMngr()->GetController(strControllerName);
    }

    return IMS_NULL;
}

#if 0  // protected
#endif

#if 0  // private
#endif

PRIVATE
IMS_BOOL IMSProcess::AttachThread(IN const AString& strName, IN BaseThread* pThread)
{
    if ((pThread == IMS_NULL) || (strName.GetLength() == 0))
    {
        return IMS_FALSE;
    }

    IMSThreadMap* pThreadMap = new IMSThreadMap(strName, pThread);

    LockGuard objLock(piLock);

    objThreads.Append(pThreadMap);

    IMS_TRACE_I("AttachThread : Thread (%s)", strName.GetStr(), 0, 0);

    return IMS_TRUE;
}

PRIVATE
void IMSProcess::DetachThread(IN const AString& strName)
{
    if (strName.GetLength() == 0)
    {
        return;
    }

    LockGuard objLock(piLock);

    for (IMS_UINT32 i = 0; i < objThreads.GetSize(); ++i)
    {
        IMSThreadMap* pThreadMap = objThreads.GetAt(i);

        if (pThreadMap->strName.Equals(strName))
        {
            IMS_TRACE_I("DetachThread :: Thread (%s)", strName.GetStr(), 0, 0);

            delete pThreadMap;
            objThreads.RemoveAt(i);
            break;
        }
    }
}

PRIVATE
AString IMSProcess::GetThreadName(IN const AString& strTargetName)
{
    IMS_SINT32 nIndex = strTargetName.GetIndexOf('.');

    return strTargetName.GetSubStr(0, nIndex);
}
