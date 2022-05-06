/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100518  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _METHOD_MANAGER_H_
#define _METHOD_MANAGER_H_

#include "IMSList.h"
#include "RCObject.h"

class Method;

class MethodManager : public RCObject
{
public:
    MethodManager();
    MethodManager(IN CONST MethodManager& objRHS);
    virtual ~MethodManager();

private:
    MethodManager& operator=(IN CONST MethodManager& objRHS);

public:
    IMS_BOOL AddMethod(IN Method* pMethod);
    const IMSList<Method*>& GetMethods() const;
    void RemoveMethod(IN Method* pMethod);

private:
    IMSList<Method*> objMethods;
};

#endif  // _METHOD_MANAGER_H_
