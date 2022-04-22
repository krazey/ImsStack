/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090819  YR@                       Created
    </table>

    Description

*/

#ifndef _INTERFACE_IMS_MUTEX_H_
#define _INTERFACE_IMS_MUTEX_H_

#include "ImsTypeDef.h"

class IMutex
{
public:
    virtual void Lock() = 0;
    virtual void Unlock() = 0;
};

class LockGuard
{
public:
    inline explicit LockGuard(IN IMutex *piMutex_)
        : piMutex(piMutex_)
    {
        if (piMutex != IMS_NULL)
        {
            piMutex->Lock();
        }
    }
    inline ~LockGuard()
    {
        if (piMutex != IMS_NULL)
        {
            piMutex->Unlock();
        }
    }

private:
    LockGuard(IN const LockGuard &objRHS);
    LockGuard& operator=(IN const LockGuard &objRHS);

private:
    IMutex *piMutex;
};

#endif // _INTERFACE_IMS_MUTEX_H_
