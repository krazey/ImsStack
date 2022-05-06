/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091024  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ConfigBuffer.h"

PUBLIC
ConfigBuffer::ConfigBuffer(IN const AString& strLocator_, IN const AString& strName_) :
        strLocator(strLocator_),
        strName(strName_)
{
}

PUBLIC VIRTUAL ConfigBuffer::~ConfigBuffer() {}

PROTECTED
const AString& ConfigBuffer::GetLocator() const
{
    return strLocator;
}

PROTECTED
const AString& ConfigBuffer::GetName() const
{
    return strName;
}
