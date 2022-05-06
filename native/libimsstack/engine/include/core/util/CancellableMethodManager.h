/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100518  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _CANCELLABLE_METHOD_MANAGER_H_
#define _CANCELLABLE_METHOD_MANAGER_H_

#include "AString.h"
#include "IMSMap.h"

class IMutex;
class ISipServerConnection;
class ICancellableMethod;

class CancellableMethodManager
{
private:
    CancellableMethodManager();
    ~CancellableMethodManager();

private:
    CancellableMethodManager(IN const CancellableMethodManager& objRHS);
    CancellableMethodManager& operator=(IN const CancellableMethodManager& objRHS);

public:
    IMS_BOOL AddMethod(IN CONST AString& strName, IN ICancellableMethod* piMethod);
    void RemoveMethod(IN CONST AString& strName);

    static CancellableMethodManager* GetInstance();

private:
    IMS_BOOL HandleCancelRequest(IN ISipServerConnection* piSSC);

private:
    friend class SIPConnectionNotifierManagerPrivate;

    IMutex* piLock;
    // Name (identifier), Pointer of ICancellableMethod
    IMSMap<AString, ICancellableMethod*> objCancellableMethods;
};

#endif  // _CANCELLABLE_METHOD_MANAGER_H_
