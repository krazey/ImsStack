/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20170801  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _BASE_THREAD_H_
#define _BASE_THREAD_H_

#include "IThread.h"

class BaseThread : public IRunnable
{
public:
    BaseThread();
    virtual ~BaseThread();

public:
    inline const AString& GetName() const { return strName; }
    inline IThread* GetThread() const { return piThread; }

    IMS_BOOL Start(IN const AString& strName, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void Terminate();

protected:
    // IRunnable class
    virtual IMS_BOOL Runnable_Run(IN IMSMSG& objMSG);

    virtual IMS_BOOL Initialize();
    virtual void Uninitialize();

    virtual IMS_BOOL OnStart(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnTerminate(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMSG);

    IMS_BOOL IsThreadMessage(IN IMSMSG& objMSG) const;

private:
    AString strName;
    IThread* piThread;
};

#endif  // _BASE_THREAD_H_
