/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090819  YR@                       Created
    </table>

    Description

*/

#ifndef _SERVICE_IMS_THREAD_H_
#define _SERVICE_IMS_THREAD_H_

#include "IThread.h"

class IMutex;

class ThreadService
{
private:
    ThreadService();
    ~ThreadService();

    ThreadService(IN const ThreadService& objRHS);
    ThreadService& operator=(IN const ThreadService& objRHS);

public:
    IThread* Create(IN const AString &strName, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    // Creates a thread which needs to communicate with the external module (IN & OUT)
    IThread* CreateEx(IN const AString &strName, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void Destroy(IN IThread *&piThread);

    IMS_BOOL Contains(IN const IThread *piThread) const;
    IMS_BOOL ContainsLocked(IN const IThread *piThread) const;
    IThread* GetCurrentThread() const;
    IThread* GetThread(IN const AString &strName) const;
    IThread* GetThreadLocked(IN const AString &strName) const;

    static ThreadService* GetThreadService();

    /**
     * It returns the slot-id based on the current thread.
     * If thread is not found, then it returns the input argument.
     * If thread is a Framework thread, then it always returns slot-0.
     */
    static IMS_SINT32 GetCurrentSlotId(IN IMS_SINT32 nDefaultSlotId = IMS_SLOT_ANY);

private:
    void LockThreadPool() const;
    void UnlockThreadPool() const;

private:
    friend class MessageService;

    IMutex *piMutex;
    // List of (IThread*)
    IMSList<IThread*> objThreads;
};

#endif // _SERVICE_IMS_THREAD_H_
