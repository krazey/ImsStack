/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100323  joonhun.shin@             Created
    </table>

    Description

*/

#ifndef _IMS_FRAMEWORK_H_
#define _IMS_FRAMEWORK_H_

#include "IMSAppThread.h"

class IMutex;
class IFrameworkThreadListener;

class IMSFramework : public IMSAppThread
{
public:
    IMSFramework();
    virtual ~IMSFramework();

private:
    IMSFramework(IN const IMSFramework& objRHS);
    IMSFramework& operator=(IN const IMSFramework& objRHS);

public:
    void AddListener(IN IFrameworkThreadListener* piListener);
    void RemoveListener(IN IFrameworkThreadListener* piListener);

protected:
    virtual IMS_BOOL Initialize();
    virtual void Uninitialize();

    virtual IMS_BOOL OnStart(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnTerminate(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMSG);

private:
    void NotifyThreadStarted();
    void NotifyThreadTerminated();

private:
    IMutex* piThisMutex;
    IMSList<IFrameworkThreadListener*> objListeners;
};

#endif  // _IMS_FRAMEWORK_H_
