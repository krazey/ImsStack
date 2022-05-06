/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100518  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "base/Method.h"
#include "util/MethodManager.h"

PUBLIC
MethodManager::MethodManager() :
        RCObject(),
        objMethods(IMSList<Method*>())
{
}

PUBLIC
MethodManager::MethodManager(IN CONST MethodManager& objRHS) :
        RCObject(objRHS),
        objMethods(objRHS.objMethods)
{
}

PUBLIC
MethodManager::~MethodManager() {}

PUBLIC
IMS_BOOL MethodManager::AddMethod(IN Method* pMethod)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objMethods.GetSize(); ++i)
    {
        const Method* pTempMethod = objMethods.GetAt(i);

        if (pTempMethod->Equals(pMethod))
        {
            return IMS_TRUE;
        }
    }

    // If not found, adds a new method ...

    return objMethods.Append(pMethod);
}

PUBLIC
const IMSList<Method*>& MethodManager::GetMethods() const
{
    //---------------------------------------------------------------------------------------------

    return objMethods;
}

PUBLIC
void MethodManager::RemoveMethod(IN Method* pMethod)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objMethods.GetSize(); ++i)
    {
        const Method* pTempMethod = objMethods.GetAt(i);

        if (pTempMethod->Equals(pMethod))
        {
            objMethods.RemoveAt(i);
            return;
        }
    }
}
