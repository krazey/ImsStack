/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100323  joonhun.shin@             Created
    </table>

    Description

*/

#ifndef _IMS_PROCESS_H_
#define _IMS_PROCESS_H_

#include "IMSAppThread.h"
#include "IIMSActivityControl.h"

class IMSThreadMap;
class IMutex;

typedef BaseThread* (*Thread_Entry)();
typedef BaseThread* (*Thread_EntryEx)(void*);
typedef IMSAppThread* (*AppThread_Entry)();
typedef IMSAppThread* (*AppThread_EntryEx)(void*);

class IMSProcess
{
private:
    IMSProcess();
    ~IMSProcess();

    IMSProcess(IN const IMSProcess& objRHS);
    IMSProcess& operator=(IN const IMSProcess& objRHS);

public:
    static IMSProcess* GetInstance();
    const AString& GetFrameworkThreadName() const;
    IMS_BOOL Initialize();
    void Uninitialize();

    IMS_BOOL LoadThread(IN const AString& strThreadName, IN Thread_Entry pfnThreadEntry,
            IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IMS_BOOL LoadThreadWithParam(IN const AString& strThreadName,
            IN Thread_EntryEx pfnThreadEntryEx, IN void* pvParam,
            IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void UnloadThread(IN const AString& strThreadName);
    BaseThread* GetThread(IN const AString& strThreadName);

    IMS_BOOL LoadAppThread(IN const AString& strThreadName, IN AppThread_Entry pfnThreadEntry,
            IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IMS_BOOL LoadAppThreadWithParam(IN const AString& strThreadName,
            IN AppThread_EntryEx pfnThreadEntryEx, IN void* pvParam,
            IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void UnloadAppThread(IN const AString& strThreadName);
    IMSAppThread* GetApplicationThread(IN const AString& strThreadName);

    IIMSActivityControl* GetController(IN const AString& strControllerName);

private:
    IMS_BOOL AttachThread(IN const AString& strName, IN BaseThread* pThread);
    void DetachThread(IN const AString& strName);
    AString GetThreadName(IN const AString& strTargetName);

private:
    AString strFrameworkThreadName;
    IMutex* piLock;
    IMSList<IMSThreadMap*> objThreads;
};

#endif  // _IMS_PROCESS_H_
