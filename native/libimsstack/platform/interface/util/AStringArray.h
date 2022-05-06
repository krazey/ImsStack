/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090603  toastops@                 Created
    </table>

    Description

*/

#ifndef _ANSI_STRING_ARRAY_H_
#define _ANSI_STRING_ARRAY_H_

#include "AString.h"

class AStringArray
{
public:
    AStringArray();
    AStringArray(IN CONST IMSList<AString>& objElements_);
    AStringArray(IN CONST AStringArray& objRHS);
    ~AStringArray();

public:
    AStringArray& operator=(IN CONST AStringArray& objRHS);
    AStringArray& operator=(IN CONST IMSList<AString>& objElements);

public:
    void AddElement(IN CONST AString& strElem);
    IMS_BOOL Contains(IN CONST AString& strElem, IN IMS_BOOL bCaseSensitive = IMS_TRUE) const;
    IMS_SINT32 GetCount() const;
    const AString& GetElementAt(IN IMS_SINT32 nIndex) const;
    const IMSList<AString>& GetElements() const;
    const AString& GetFirstElement() const;
    IMS_SINT32 GetIndexOf(IN CONST AString& strElem, IN IMS_SINT32 nOffset = 0,
            IN IMS_BOOL bCaseSensitive = IMS_TRUE) const;
    const AString& GetLastElement() const;
    IMS_SINT32 GetLastIndexOf(IN CONST AString& strElem, IN IMS_SINT32 nOffset = 0,
            IN IMS_BOOL bCaseSensitive = IMS_TRUE) const;
    void InsertElementAt(IN CONST AString& strElem, IN IMS_SINT32 nIndex);
    IMS_BOOL IsEmpty() const;
    void RemoveAllElements();
    IMS_BOOL RemoveElement(IN CONST AString& strElem, IN IMS_BOOL bCaseSensitive = IMS_TRUE);
    void RemoveElementAt(IN IMS_SINT32 nIndex);
    void SetElementAt(IN CONST AString& strElem, IN IMS_SINT32 nIndex);

    static const AStringArray& ConstNull();

private:
    IMSList<AString> objElements;
};

#endif  // _ANSI_STRING_ARRAY_H_
