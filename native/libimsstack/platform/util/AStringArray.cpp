/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090531  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "AStringArray.h"

PUBLIC
AStringArray::AStringArray() :
        objElements(IMSList<AString>())
{
}

PUBLIC
AStringArray::AStringArray(IN CONST IMSList<AString>& objElements_) :
        objElements(objElements_)
{
}

PUBLIC
AStringArray::AStringArray(IN CONST AStringArray& objRHS) :
        objElements(objRHS.objElements)
{
}

PUBLIC
AStringArray::~AStringArray()
{
    RemoveAllElements();
}

PUBLIC
AStringArray& AStringArray::operator=(IN CONST AStringArray& objRHS)
{
    if (this != &objRHS)
    {
        objElements = objRHS.objElements;
    }

    return (*this);
}

PUBLIC
AStringArray& AStringArray::operator=(IN CONST IMSList<AString>& objElements)
{
    this->objElements = objElements;

    return (*this);
}

PUBLIC
void AStringArray::AddElement(IN CONST AString& strElem)
{
    objElements.Append(strElem);
}

PUBLIC
IMS_BOOL AStringArray::Contains(
        IN CONST AString& strElem, IN IMS_BOOL bCaseSensitive /* = IMS_TRUE */) const
{
    if (bCaseSensitive)
    {
        for (IMS_UINT32 i = 0; i < objElements.GetSize(); ++i)
        {
            const AString& strExValue = objElements.GetAt(i);

            if (strExValue.Equals(strElem))
            {
                return IMS_TRUE;
            }
        }
    }
    else
    {
        for (IMS_UINT32 i = 0; i < objElements.GetSize(); ++i)
        {
            const AString& strExValue = objElements.GetAt(i);

            if (strExValue.EqualsIgnoreCase(strElem))
            {
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_SINT32 AStringArray::GetCount() const
{
    return static_cast<IMS_SINT32>(objElements.GetSize());
}

PUBLIC
const AString& AStringArray::GetElementAt(IN IMS_SINT32 nIndex) const
{
    return objElements.GetAt(nIndex);
}

PUBLIC
const IMSList<AString>& AStringArray::GetElements() const
{
    return objElements;
}

PUBLIC
const AString& AStringArray::GetFirstElement() const
{
    if (objElements.GetSize() == 0)
    {
        return AString::ConstNull();
    }

    return objElements.GetAt(0);
}

PUBLIC
IMS_SINT32 AStringArray::GetIndexOf(IN CONST AString& strElem, IN IMS_SINT32 nOffset /* = 0 */,
        IN IMS_BOOL bCaseSensitive /* = IMS_TRUE */) const
{
    if (nOffset < 0)
    {
        nOffset = 0;
    }

    if (bCaseSensitive)
    {
        for (IMS_UINT32 i = nOffset; i < objElements.GetSize(); ++i)
        {
            const AString& strExValue = objElements.GetAt(i);

            if (strExValue.Equals(strElem))
            {
                return i;
            }
        }
    }
    else
    {
        for (IMS_UINT32 i = nOffset; i < objElements.GetSize(); ++i)
        {
            const AString& strExValue = objElements.GetAt(i);

            if (strExValue.EqualsIgnoreCase(strElem))
            {
                return i;
            }
        }
    }

    return AString::NPOS;
}

PUBLIC
const AString& AStringArray::GetLastElement() const
{
    if (objElements.GetSize() == 0)
    {
        return AString::ConstNull();
    }

    return objElements.GetAt(objElements.GetSize() - 1);
}

PUBLIC
IMS_SINT32 AStringArray::GetLastIndexOf(IN CONST AString& strElem, IN IMS_SINT32 nOffset /* = 0 */,
        IN IMS_BOOL bCaseSensitive /* = IMS_TRUE */) const
{
    if (nOffset < 0)
    {
        nOffset = 0;
    }

    if (bCaseSensitive)
    {
        for (IMS_SINT32 i = objElements.GetSize(); i >= nOffset; --i)
        {
            const AString& strExValue = objElements.GetAt(i);

            if (strExValue.Equals(strElem))
            {
                return i;
            }
        }
    }
    else
    {
        for (IMS_SINT32 i = objElements.GetSize(); i >= nOffset; --i)
        {
            const AString& strExValue = objElements.GetAt(i);

            if (strExValue.EqualsIgnoreCase(strElem))
            {
                return i;
            }
        }
    }

    return AString::NPOS;
}

PUBLIC
void AStringArray::InsertElementAt(IN CONST AString& strElem, IN IMS_SINT32 nIndex)
{
    objElements.InsertAt(strElem, nIndex);
}

PUBLIC
IMS_BOOL AStringArray::IsEmpty() const
{
    return (objElements.GetSize() == 0);
}

PUBLIC
void AStringArray::RemoveAllElements()
{
    objElements.Clear();
}

PUBLIC
IMS_BOOL AStringArray::RemoveElement(
        IN CONST AString& strElem, IN IMS_BOOL bCaseSensitive /* = IMS_TRUE */)
{
    if (bCaseSensitive)
    {
        for (IMS_UINT32 i = 0; i < objElements.GetSize(); ++i)
        {
            const AString& strExValue = objElements.GetAt(i);

            if (strExValue.Equals(strElem))
            {
                objElements.RemoveAt(i);
                return IMS_TRUE;
            }
        }
    }
    else
    {
        for (IMS_UINT32 i = 0; i < objElements.GetSize(); ++i)
        {
            const AString& strExValue = objElements.GetAt(i);

            if (strExValue.EqualsIgnoreCase(strElem))
            {
                objElements.RemoveAt(i);
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PUBLIC
void AStringArray::RemoveElementAt(IN IMS_SINT32 nIndex)
{
    objElements.RemoveAt(nIndex);
}

PUBLIC
void AStringArray::SetElementAt(IN CONST AString& strElem, IN IMS_SINT32 nIndex)
{
    objElements.SetAt(strElem, nIndex);
}

PUBLIC GLOBAL const AStringArray& AStringArray::ConstNull()
{
    static const AStringArray CONST_NULL;

    return CONST_NULL;
}
