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
#include "ServiceTrace.h"
#include "ImsMessageDef.h"
#include "IMSAppThread.h"

__IMS_TRACE_TAG_BASE__;

#if 0  // public
#endif

PUBLIC
IMSAppThread::IMSAppThread() :
        BaseThread(),
        objIMSApps(IMSList<IMSApp*>())
{
}

PUBLIC VIRTUAL IMSAppThread::~IMSAppThread() {}

PUBLIC
IMSActivityMngr* IMSAppThread::GetActivityMngr()
{
    return &objActivityMngr;
}

PUBLIC
void IMSAppThread::AddApp(IN IMSApp_Creator pfnCreator, IN const AString& strName)
{
    ControlAppAsync(PARAM_APP_CONTROL_ADD, strName, pfnCreator);
}

PUBLIC
void IMSAppThread::RemoveApp(IN const AString& strName)
{
    ControlAppAsync(PARAM_APP_CONTROL_REMOVE, strName);
}

PUBLIC
void IMSAppThread::RemoveAndDestroyApp(IN const AString& strName)
{
    ControlAppAsync(PARAM_APP_CONTROL_REMOVE_N_DESTROY, strName);
}

#if 0  // protected
#endif

PROTECTED VIRTUAL void IMSAppThread::OnAppControl(IN IMS_SINT32 nParam, IN const AppInfo* pAppInfo)
{
    if (pAppInfo == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("OnAppControl :: param=%d, name=%s", nParam, pAppInfo->strName.GetStr(), 0);

    switch (nParam)
    {
        case PARAM_APP_CONTROL_ADD:
        {
            IMSApp* pApp = pAppInfo->pfnCreator(pAppInfo->strName);
            AttachApp(pApp);
            break;
        }

        case PARAM_APP_CONTROL_REMOVE:
            DetachApp(pAppInfo->strName);
            break;

        case PARAM_APP_CONTROL_REMOVE_N_DESTROY:
            DetachApp(pAppInfo->strName, IMS_TRUE);
            break;

        default:
            // no-op
            break;
    }

    delete pAppInfo;
}

PROTECTED
IMS_BOOL IMSAppThread::AttachApp(IN IMSApp* pApp)
{
    if (pApp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    objIMSApps.Append(pApp);

    return IMS_TRUE;
}

PROTECTED
void IMSAppThread::DetachApp(IN const AString& strName, IN IMS_BOOL bDestroy /*= IMS_FALSE*/)
{
    for (IMS_UINT32 i = 0; i < objIMSApps.GetSize(); ++i)
    {
        IMSApp* pApp = objIMSApps.GetAt(i);

        if (strName.Equals(pApp->GetName()))
        {
            objIMSApps.RemoveAt(i);

            if (bDestroy)
            {
                delete pApp;
            }
            break;
        }
    }
}

PROTECTED
void IMSAppThread::ControlAppAsync(IN IMS_SINT32 nParam, IN const AString& strName,
        IN IMSApp_Creator pfnCreator /*= IMS_NULL*/)
{
    IThread* piThread = GetThread();

    if (piThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "No target thread - App(%s)", strName.GetStr(), 0, 0);
        return;
    }

    AppInfo* pAppInfo = new AppInfo(strName, pfnCreator);

    piThread->PostMessageI(IMS_MSG_APP_CONTROL, nParam, reinterpret_cast<IMS_UINTP>(pAppInfo));
}

#if 0  // private
#endif

PRIVATE
void IMSAppThread::UnloadAllApp()
{
    IMS_TRACE_D("UnloadAllApp", 0, 0, 0);

    while (!objIMSApps.IsEmpty())
    {
        /* FIXME: needs to be checked for all enablers
        IMSApp *pApp = objIMSApps.GetAt(0);

        if (pApp != IMS_NULL)
        {
            delete pApp;
        }
        */

        objIMSApps.RemoveAt(0);
    }
}

PRIVATE VIRTUAL IMS_BOOL IMSAppThread::Runnable_Run(IN IMSMSG& objMSG)
{
    switch (objMSG.GetName())
    {
        case IMS_MSG_START:
            if (!Initialize())
            {
                return IMS_FALSE;
            }

            return OnStart(objMSG);

        case IMS_MSG_TERMINATE:
            OnTerminate(objMSG);

            UnloadAllApp();
            Uninitialize();

            return IMS_TRUE;

        case IMS_MSG_APP_CONTROL:
            OnAppControl(LONG_TO_INT(objMSG.nWparam), reinterpret_cast<AppInfo*>(objMSG.nLparam));
            return IMS_TRUE;

        default:
            break;
    }

    if (!IsThreadMessage(objMSG))
    {
        return objActivityMngr.HandleMessage(objMSG);
    }

    return OnMessage(objMSG);
}
