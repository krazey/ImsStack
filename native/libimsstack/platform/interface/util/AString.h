/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _ANSI_STRING_H_
#define _ANSI_STRING_H_

#include <stdarg.h>
#include "IMSTypeDef.h"
#include "ImsList.h"
// Temp
#include "RcObject.h"

class WCharPtr;

class AString
{
public:
    class CharRef
    {
    public:
        CharRef(IN AString& objStr_, IN IMS_SINT32 nIndex_);

    public:
        // lvalue operation
        CharRef& operator=(IN CONST CharRef& objRHS);
        CharRef& operator=(IN IMS_CHAR ch);
        // rvalue operation
        inline operator IMS_CHAR() const
        {
            return (nIndex < objStr.pData->nSize) ? objStr.pData->pValue[nIndex] : 0;
        }
        inline IMS_BOOL operator==(IN IMS_CHAR ch) const
        {
            return (objStr.pData->pValue[nIndex] == ch);
        }
        inline IMS_BOOL operator!=(IN IMS_CHAR ch) const
        {
            return (objStr.pData->pValue[nIndex] != ch);
        }
        inline IMS_BOOL operator>(IN IMS_CHAR ch) const
        {
            return (objStr.pData->pValue[nIndex] > ch);
        }
        inline IMS_BOOL operator>=(IN IMS_CHAR ch) const
        {
            return (objStr.pData->pValue[nIndex] >= ch);
        }
        inline IMS_BOOL operator<(IN IMS_CHAR ch) const
        {
            return (objStr.pData->pValue[nIndex] < ch);
        }
        inline IMS_BOOL operator<=(IN IMS_CHAR ch) const
        {
            return (objStr.pData->pValue[nIndex] <= ch);
        }

    private:
        AString& objStr;
        IMS_SINT32 nIndex;
    };

public:
    AString();
    AString(IN CONST IMS_CHAR* pszValue);
    AString(IN CONST IMS_CHAR* pszValue, IN IMS_SINT32 nSize);
    AString(IN CONST IMS_CHAR* pszValue, IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount);
    AString(IN IMS_SINT32 nSize, IN CONST IMS_CHAR ch);
    AString(IN CONST AString& objRHS);
    ~AString();

private:
    explicit AString(IN IMS_SINT32 nSize);

public:
    AString& operator=(IN CONST AString& objRHS);
    AString& operator=(IN CONST IMS_CHAR ch);
    AString& operator=(IN CONST IMS_CHAR* pszValue);
    AString& operator=(IN CONST IMS_WCHAR* pwszValue);
    inline AString& operator+=(IN CONST AString& objStr) { return Append(objStr); }
    inline AString& operator+=(IN CONST IMS_CHAR ch) { return Append(ch); }
    inline AString& operator+=(IN CONST IMS_CHAR* pszValue) { return Append(pszValue); }
    IMS_CHAR operator[](IN IMS_SINT32 nIndex) const;
    CharRef operator[](IN IMS_SINT32 nIndex);
    IMS_CHAR operator[](IN IMS_UINT32 nIndex) const;
    CharRef operator[](IN IMS_UINT32 nIndex);

    // For IMSMap class
    IMS_BOOL operator<(IN CONST AString& objRHS);

public:
    inline IMS_SINT32 GetCapacity() const { return pData->nAlloc; }
    inline IMS_SINT32 GetLength() const { return pData->nSize; }

    inline IMS_BOOL IsEmpty() const
    {
        return ((pData == &SHARED_EMPTY) || (!IsNULL() && (pData->nSize == 0)));
    }
    inline IMS_BOOL IsNULL() const { return (pData == &SHARED_NULL); }

    inline IMS_CHAR* GetStr()
    {
        Detach();
        return pData->pValue;
    }
    inline const IMS_CHAR* GetStr() const { return pData->pValue; }
    AString GetSubStr(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount = -1) const;

    void Chop(IN IMS_SINT32 nCount);
    void Truncate(IN IMS_SINT32 nOffset);
    void Attach(IN CONST IMS_CHAR* pValue, IN IMS_SINT32 nSize);
    void Detach();
    void Resize(IN IMS_SINT32 nSize);

    IMS_SINT32 GetIndexOf(IN CONST IMS_CHAR ch, IN IMS_SINT32 nOffset = 0) const;
    IMS_SINT32 GetIndexOf(IN CONST IMS_CHAR* pszValue, IN IMS_SINT32 nOffset = 0) const;
    IMS_SINT32 GetIndexOf(IN CONST AString& objStr, IN IMS_SINT32 nOffset = 0) const;

    IMS_SINT32 GetLastIndexOf(IN CONST IMS_CHAR ch, IN IMS_SINT32 nOffset = -1) const;
    IMS_SINT32 GetLastIndexOf(IN CONST IMS_CHAR* pszValue, IN IMS_SINT32 nOffset = -1) const;
    IMS_SINT32 GetLastIndexOf(IN CONST AString& objStr, IN IMS_SINT32 nOffset = -1) const;

    IMS_BOOL Contains(IN CONST IMS_CHAR ch) const;
    IMS_BOOL Contains(IN CONST IMS_CHAR* pszValue) const;
    IMS_BOOL Contains(IN CONST AString& objStr) const;
    IMS_BOOL EndsWith(IN CONST IMS_CHAR ch) const;
    IMS_BOOL EndsWith(IN CONST IMS_CHAR* pszValue) const;
    IMS_BOOL EndsWith(IN CONST AString& objStr) const;
    IMS_BOOL StartsWith(IN CONST IMS_CHAR ch) const;
    IMS_BOOL StartsWith(IN CONST IMS_CHAR* pszValue) const;
    IMS_BOOL StartsWith(IN CONST AString& objStr) const;

    IMS_BOOL Equals(IN CONST IMS_CHAR ch) const;
    IMS_BOOL Equals(IN CONST IMS_CHAR* pszValue) const;
    IMS_BOOL Equals(IN CONST AString& objStr) const;
    IMS_BOOL EqualsIgnoreCase(IN CONST IMS_CHAR ch) const;
    IMS_BOOL EqualsIgnoreCase(IN CONST IMS_CHAR* pszValue) const;
    IMS_BOOL EqualsIgnoreCase(IN CONST AString& objStr) const;

    AString& Append(IN CONST IMS_CHAR ch);
    AString& Append(IN CONST IMS_CHAR* pszValue);
    AString& Append(IN CONST AString& objStr);
    AString& Erase(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount);
    AString& Fill(IN IMS_CHAR ch, IN IMS_SINT32 nSize = -1);
    AString& Insert(IN IMS_SINT32 i, IN CONST IMS_CHAR ch);
    AString& Insert(IN IMS_SINT32 i, IN CONST IMS_CHAR* pszValue);
    AString& Insert(IN IMS_SINT32 i, IN CONST AString& objStr);
    AString& Prepend(IN CONST IMS_CHAR ch);
    AString& Prepend(IN CONST IMS_CHAR* pszValue);
    AString& Prepend(IN CONST AString& objStr);
    AString& Replace(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount, IN CONST IMS_CHAR* pszNew);
    AString& Replace(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount, IN CONST AString& strNew);
    AString& Replace(IN CONST IMS_CHAR cOld, IN CONST IMS_CHAR cNew);
    AString& Replace(IN CONST IMS_CHAR cOld, IN CONST IMS_CHAR* pszNew);
    AString& Replace(IN CONST IMS_CHAR cOld, IN CONST AString& strNew);
    AString& Replace(IN CONST IMS_CHAR* pszOld, IN CONST IMS_CHAR* pszNew);
    AString& Replace(IN CONST IMS_CHAR* pszOld, IN CONST AString& strNew);
    AString& Replace(IN CONST AString& strOld, IN CONST IMS_CHAR* pszNew);
    AString& Replace(IN CONST AString& strOld, IN CONST AString& strNew);

    AString MakeLower() const;
    AString MakeUpper() const;
    AString SimplifyWSP() const;
    AString Trim() const;
    AString TrimLeft() const;
    AString TrimRight() const;
    AString Left(IN IMS_SINT32 nCount) const;
    AString Mid(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount = -1) const;
    AString Right(IN IMS_SINT32 nCount) const;
    AString AlignLeft(
            IN IMS_SINT32 nWidth, IN IMS_CHAR cFill = ' ', IN IMS_BOOL bTruncate = IMS_FALSE) const;
    AString AlignRight(
            IN IMS_SINT32 nWidth, IN IMS_CHAR cFill = ' ', IN IMS_BOOL bTruncate = IMS_FALSE) const;

    AString& SetNumber(IN IMS_SINT16 n, IN IMS_SINT32 nBase = 10);
    AString& SetNumber(IN IMS_UINT16 n, IN IMS_SINT32 nBase = 10);
    AString& SetNumber(IN IMS_SINT32 n, IN IMS_SINT32 nBase = 10);
    AString& SetNumber(IN IMS_UINT32 n, IN IMS_SINT32 nBase = 10);
    AString& SetNumber(IN IMS_SINT64 n, IN IMS_SINT32 nBase = 10);
    AString& SetNumber(IN IMS_UINT64 n, IN IMS_SINT32 nBase = 10);
    IMS_SINT16 ToInt16(OUT IMS_BOOL* pbOK = IMS_NULL, IN IMS_SINT32 nBase = 10) const;
    IMS_UINT16 ToUInt16(OUT IMS_BOOL* pbOK = IMS_NULL, IN IMS_SINT32 nBase = 10) const;
    IMS_SINT32 ToInt32(OUT IMS_BOOL* pbOK = IMS_NULL, IN IMS_SINT32 nBase = 10) const;
    IMS_UINT32 ToUInt32(OUT IMS_BOOL* pbOK = IMS_NULL, IN IMS_SINT32 nBase = 10) const;
    IMS_SINT64 ToInt64(OUT IMS_BOOL* pbOK = IMS_NULL, IN IMS_SINT32 nBase = 10) const;
    IMS_UINT64 ToUInt64(OUT IMS_BOOL* pbOK = IMS_NULL, IN IMS_SINT32 nBase = 10) const;
    AString ToBase64() const;
    AString ToHexString() const;
    AString ToUTF8() const;

    AString& Sprintf(IN CONST IMS_CHAR* pszFormat, ...);
    AString& Vsprintf(IN CONST IMS_CHAR* pszFormat, va_list ap);

    IMS_CHAR* Duplicate() const;
    IMSList<AString> Split(IN IMS_CHAR cSep, IN IMS_BOOL bTrim = IMS_TRUE) const;
    IMS_SINT32 SplitF(IN IMS_CHAR cSep, OUT AString& strLHS, OUT AString& strRHS,
            IN IMS_BOOL bTrim = IMS_TRUE) const;
    IMS_SINT32 SplitB(IN IMS_CHAR cSep, OUT AString& strLHS, OUT AString& strRHS,
            IN IMS_BOOL bTrim = IMS_TRUE) const;

    IMS_UINT32 GetHashCode() const;

    // FIXME: it's for a temporary usage
    RCPtr<WCharPtr> GetWCharPtr() const;

    static const AString& ConstEmpty();
    static const AString& ConstNull();

    static IMS_SINT32 Compare(IN CONST IMS_CHAR* pszStr1, IN CONST IMS_CHAR* pszStr2);
    static IMS_SINT32 CompareIgnoreCase(IN CONST IMS_CHAR* pszStr1, IN CONST IMS_CHAR* pszStr2);
    static AString FromBase64(IN CONST AString& strBase64);
    static AString FromRawData(IN CONST IMS_CHAR* pszValue, IN IMS_SINT32 nSize);
    static IMS_UINT32 GetHashCode(IN CONST IMS_CHAR* pszValue);

private:
    void Clear();
    void Realloc(IN IMS_SINT32 nAlloc);

public:
    enum
    {
        NPOS = (-1)
    };

private:
    friend class CharRef;
    friend class AStringBuffer;

    struct Data
    {
        IMS_SINT32 nAlloc;    // Size of allocated buffer
        IMS_SINT32 nSize;     // Size of null-terminated string (excludes NULL character)
        IMS_CHAR* pValue;     // Pointer to the null-terminated string (Points to acValue)
        IMS_CHAR acValue[1];  // Pointer to the null-terminated string
    };

    explicit AString(IN Data* pData_);

    static Data SHARED_NULL;
    static Data SHARED_EMPTY;

    Data* pData;
};

inline IMS_BOOL operator==(IN CONST AString& objA1, IN CONST AString& objA2)
{
    return objA1.Equals(objA2);
}
inline IMS_BOOL operator==(IN CONST AString& objA1, IN CONST IMS_CHAR* pszA2)
{
    return objA1.Equals(pszA2);
}
inline IMS_BOOL operator==(IN CONST IMS_CHAR* pszA1, IN CONST AString& objA2)
{
    return objA2.Equals(pszA1);
}

inline IMS_BOOL operator!=(IN CONST AString& objA1, IN CONST AString& objA2)
{
    return !objA1.Equals(objA2);
}
inline IMS_BOOL operator!=(IN CONST AString& objA1, IN CONST IMS_CHAR* pszA2)
{
    return !objA1.Equals(pszA2);
}
inline IMS_BOOL operator!=(IN CONST IMS_CHAR* pszA1, IN CONST AString& objA2)
{
    return !objA2.Equals(pszA1);
}

inline const AString operator+(IN CONST AString& objA1, IN CONST AString& objA2)
{
    return AString(objA1) += objA2;
}
inline const AString operator+(IN CONST AString& objA1, IN CONST IMS_CHAR* pszA2)
{
    return AString(objA1) += pszA2;
}
inline const AString operator+(IN CONST AString& objA1, IN CONST IMS_CHAR cA2)
{
    return AString(objA1) += cA2;
}
inline const AString operator+(IN CONST IMS_CHAR* pszA1, IN CONST AString& objA2)
{
    return AString(pszA1) += objA2;
}
inline const AString operator+(IN CONST IMS_CHAR cA1, IN CONST AString& objA1)
{
    return AString(&cA1, 1) += objA1;
}

// For IMSMap class
inline IMS_BOOL operator<(IN CONST AString& objA1, IN CONST AString& objA2)
{
    return (AString::Compare(objA1.GetStr(), objA2.GetStr()) < 0);
}

// TODO:: temporary -- starts

class WCharPtr : public RCObject
{
public:
    WCharPtr(IN CONST AString& str);
    WCharPtr(IN CONST WCharPtr& objRHS);
    virtual ~WCharPtr();

public:
    WCharPtr& operator=(IN CONST WCharPtr& objRHS);

public:
    inline IMS_WCHAR* GetStr() const { return pwszData; }
    inline IMS_SINT32 GetLength() const { return nSize; }

private:
    IMS_SINT32 nSize;
    IMS_WCHAR* pwszData;
};
// TODO:: temporary -- ends

#endif  // _ANSI_STRING_H_
