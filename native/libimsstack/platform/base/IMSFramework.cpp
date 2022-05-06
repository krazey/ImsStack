/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100323  joonhun.shin@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceMutex.h"
#include "SystemConfigManager.h"
#include "IFrameworkThreadListener.h"
#include "IMSFramework.h"

#if 0  // public
#endif

PUBLIC
IMSFramework::IMSFramework() :
        IMSAppThread(),
        piThisMutex(IMS_NULL),
        objListeners(IMSList<IFrameworkThreadListener*>())
{
    piThisMutex = MutexService::GetMutexService()->CreateMutex();
}

PUBLIC VIRTUAL IMSFramework::~IMSFramework()
{
    MutexService::GetMutexService()->DestroyMutex(piThisMutex);
    SystemConfigManager::GetInstance()->SetProxyThread(IMS_NULL);
}

PUBLIC
void IMSFramework::AddListener(IN IFrameworkThreadListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    LockGuard objLock(piThisMutex);

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IFrameworkThreadListener* piThreadListener = objListeners.GetAt(i);

        if (piThreadListener == piListener)
        {
            // Listener is already registered.
            return;
        }
    }

    objListeners.Append(piListener);
}

PUBLIC
void IMSFramework::RemoveListener(IN IFrameworkThreadListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    LockGuard objLock(piThisMutex);

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IFrameworkThreadListener* piThreadListener = objListeners.GetAt(i);

        if (piThreadListener == piListener)
        {
            objListeners.RemoveAt(i);
            break;
        }
    }
}

#if 0  // pretected
#endif

PROTECTED VIRTUAL IMS_BOOL IMSFramework::Initialize()
{
    SystemConfigManager::CacheSystemFeatures();
    SystemConfigManager::GetInstance()->SetProxyThread(GetThread());

    return IMS_TRUE;
}

PROTECTED VIRTUAL void IMSFramework::Uninitialize()
{
    SystemConfigManager::GetInstance()->SetProxyThread(IMS_NULL);
}

PROTECTED VIRTUAL IMS_BOOL IMSFramework::OnStart(IN IMSMSG& objMSG)
{
    IMSAppThread::OnStart(objMSG);

    NotifyThreadStarted();

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL IMSFramework::OnTerminate(IN IMSMSG& objMSG)
{
    IMSAppThread::OnTerminate(objMSG);

    NotifyThreadTerminated();

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL IMSFramework::OnMessage(IN IMSMSG& objMSG)
{
    IMSAppThread::OnMessage(objMSG);

    return IMS_TRUE;
}

PRIVATE
void IMSFramework::NotifyThreadStarted()
{
    IMSList<IFrameworkThreadListener*> objThreadListeners;

    {
        LockGuard objLock(piThisMutex);
        objThreadListeners = objListeners;
    }

    for (IMS_UINT32 i = 0; i < objThreadListeners.GetSize(); ++i)
    {
        IFrameworkThreadListener* piThreadListener = objThreadListeners.GetAt(i);

        if (piThreadListener != IMS_NULL)
        {
            piThreadListener->FrameworkThread_OnStarted();
        }
    }
}

PRIVATE
void IMSFramework::NotifyThreadTerminated()
{
    IMSList<IFrameworkThreadListener*> objThreadListeners;

    {
        LockGuard objLock(piThisMutex);
        objThreadListeners = objListeners;
    }

    for (IMS_UINT32 i = 0; i < objThreadListeners.GetSize(); ++i)
    {
        IFrameworkThreadListener* piThreadListener = objThreadListeners.GetAt(i);

        if (piThreadListener != IMS_NULL)
        {
            piThreadListener->FrameworkThread_OnTerminated();
        }
    }
}
