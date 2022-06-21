/*
    Author
    <table>
    date              author                    description
    --------      --------------            ----------
    20111206      hyunho.shin@           Created
    </table>

    Description

*/

#ifndef _IMS_UCE_SERVICE_THREAD_H_
#define _IMS_UCE_SERVICE_THREAD_H_

#include "BaseService.h"
#include "ImsAppThread.h"

class JniUceServiceThread : public ImsAppThread
{
private:
    JniUceServiceThread();

public:
    static ImsAppThread* GetInstance();
    virtual ~JniUceServiceThread();

    int SetCallback(IN IMS_UINTP nNativeObj, IN Jni_SendDataToJava pfnSendDataToJava);

protected:
    virtual IMS_BOOL Initialize();
    virtual void Uninitialize();

    virtual IMS_BOOL OnStart(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnTerminate(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMSG);

private:
    IMS_UINTP m_nNativeObj;
    Jni_SendDataToJava m_pfnSendDataToJava;
};

#endif
