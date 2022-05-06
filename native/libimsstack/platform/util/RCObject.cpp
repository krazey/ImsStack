/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#include "RCObject.h"

PUBLIC
RCObject::RCObject() :
        nRefCount(0),
        bShareable(IMS_TRUE)
{
}

PUBLIC
RCObject::RCObject(IN CONST RCObject& /* objRHS */) :
        nRefCount(0),
        bShareable(IMS_TRUE)
{
}

PUBLIC VIRTUAL RCObject::~RCObject() {}

PUBLIC
RCObject& RCObject::operator=(IN CONST RCObject& /* objRHS */)
{
    return (*this);
}

PUBLIC
void RCObject::AddReference()
{
    ++nRefCount;
}

PUBLIC
void RCObject::RemoveReference()
{
    if (--nRefCount == 0)
    {
        delete this;
    }
}

PUBLIC
void RCObject::MarkUnshareable()
{
    bShareable = IMS_FALSE;
}

PUBLIC
IMS_BOOL RCObject::IsShareable() const
{
    return bShareable;
}

PUBLIC
IMS_BOOL RCObject::IsShared() const
{
    return nRefCount > 1;
}
