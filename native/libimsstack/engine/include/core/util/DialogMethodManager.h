/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100518  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _DIALOG_METHOD_MANAGER_H_
#define _DIALOG_METHOD_MANAGER_H_

#include "AString.h"
#include "IMSMap.h"

class IMutex;
class ISipServerConnection;
class IDialogMethod;

class DialogMethodManager
{
private:
    DialogMethodManager();
    ~DialogMethodManager();

    DialogMethodManager(IN const DialogMethodManager& objRHS);
    DialogMethodManager& operator=(IN const DialogMethodManager& objRHS);

public:
    IMS_BOOL AddMethod(IN CONST AString& strName, IN IDialogMethod* piMethod);
    void RemoveMethod(IN CONST AString& strName);
    IMS_BOOL IsEmpty() const;

    static DialogMethodManager* GetInstance();

private:
    IMS_BOOL HandleRequestWithinDialog(IN ISipServerConnection* piSSC);
    // In case of receiving a forked request ...
    IMS_BOOL HandleRequestWithinDialog(IN ISipServerConnection* piSSC, IN ISipDialog* piOrigDialog);

private:
    friend class SIPConnectionNotifierManagerPrivate;

    IMutex* piLock;
    // Name (identifier), Pointer of IDialogMethod
    IMSMap<AString, IDialogMethod*> objDialogMethods;
};

#endif  // _DIALOG_METHOD_MANAGER_H_
