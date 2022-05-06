/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100323  joonhun.shin@             Created
    </table>

    Description

*/

#ifndef _IMS_APP_THREAD_H_
#define _IMS_APP_THREAD_H_

#include "BaseThread.h"
#include "IMSApp.h"
#include "IMSActivityMngr.h"

class IMSAppThread : public BaseThread
{
private:
    class AppInfo
    {
    public:
        inline AppInfo(IN const AString& strName_, IN IMSApp_Creator pfnCreator_) :
                strName(strName_),
                pfnCreator(pfnCreator_)
        {
        }

    private:
        AppInfo(IN const AppInfo& objRHS);
        AppInfo& operator=(IN const AppInfo& objRHS);

    public:
        AString strName;
        IMSApp_Creator pfnCreator;
    };

public:
    IMSAppThread();
    virtual ~IMSAppThread();

public:
    IMSActivityMngr* GetActivityMngr();

    void AddApp(IN IMSApp_Creator pfnCreator, IN const AString& strName);
    void RemoveApp(IN const AString& strName);
    void RemoveAndDestroyApp(IN const AString& strName);

protected:
    virtual void OnAppControl(IN IMS_SINT32 nParam, IN const AppInfo* pAppInfo);

    IMS_BOOL AttachApp(IN IMSApp* pApp);
    void DetachApp(IN const AString& strName, IN IMS_BOOL bDestroy = IMS_FALSE);
    void ControlAppAsync(IN IMS_SINT32 nParam, IN const AString& strName,
            IN IMSApp_Creator pfnCreator = IMS_NULL);

private:
    void UnloadAllApp();

    // IRunnable class
    virtual IMS_BOOL Runnable_Run(IN IMSMSG& objMSG);

protected:
    // WParam for application control
    enum
    {
        PARAM_APP_CONTROL_ADD = 1,
        PARAM_APP_CONTROL_REMOVE = 2,
        PARAM_APP_CONTROL_REMOVE_N_DESTROY = 3,

        PARAM_APP_CONTROL_BASE_MAX
    };

private:
    IMSList<IMSApp*> objIMSApps;
    IMSActivityMngr objActivityMngr;
};

#endif  // _IMS_APP_THREAD_H_
