/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100713  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _FORKED_DIALOG_METHOD_MANAGER_H_
#define _FORKED_DIALOG_METHOD_MANAGER_H_

#include "AString.h"
#include "IMSMap.h"

class IMutex;
class ISipDialog;
class ISipServerConnection;
class IForkedDialogMethod;

class ForkedDialogMethodManager
{
private:
    ForkedDialogMethodManager();
    ~ForkedDialogMethodManager();

    ForkedDialogMethodManager(IN const ForkedDialogMethodManager& objRHS);
    ForkedDialogMethodManager& operator=(IN const ForkedDialogMethodManager& objRHS);

public:
    IMS_BOOL AddMethod(IN CONST AString& strName, IN IForkedDialogMethod* piMethod);
    void RemoveMethod(IN CONST AString& strName);

    static ForkedDialogMethodManager* GetInstance();

private:
    IMS_BOOL HandleRequestWithinDialog(IN ISipServerConnection* piSSC, IN ISipDialog* piOrigDialog);

private:
    friend class SIPConnectionNotifierManagerPrivate;

    IMutex* piLock;
    // Name (identifier), Pointer of IForkedDialogMethod
    IMSMap<AString, IForkedDialogMethod*> objDialogMethods;
};

#endif  // _FORKED_DIALOG_METHOD_MANAGER_H_
