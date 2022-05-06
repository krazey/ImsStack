/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100426  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _ANSI_STRING_BUFFER_H_
#define _ANSI_STRING_BUFFER_H_

#include "AString.h"

class AStringBuffer
{
public:
    AStringBuffer();
    explicit AStringBuffer(IN IMS_SINT32 nSize);
    AStringBuffer(IN CONST AString& strValue_);
    AStringBuffer(IN CONST AStringBuffer& objRHS);
    ~AStringBuffer();

public:
    AStringBuffer& operator=(IN CONST AStringBuffer& objRHS);
    AStringBuffer& operator=(IN CONST IMS_CHAR ch);
    AStringBuffer& operator=(IN CONST IMS_CHAR* pszValue);
    AStringBuffer& operator=(IN CONST AString& strValue);
    AStringBuffer& operator+=(IN CONST AStringBuffer& objRHS);
    inline AStringBuffer& operator+=(IN CONST IMS_CHAR ch) { return Append(ch); }
    inline AStringBuffer& operator+=(IN CONST IMS_CHAR* pszValue) { return Append(pszValue); }
    inline AStringBuffer& operator+=(IN CONST AString& strValue) { return Append(strValue); }
    inline AStringBuffer& operator+=(IN CONST IMS_SINT16 nValue) { return Append(nValue); }
    inline AStringBuffer& operator+=(IN CONST IMS_UINT16 nValue) { return Append(nValue); }
    inline AStringBuffer& operator+=(IN CONST IMS_SINT32 nValue) { return Append(nValue); }
    inline AStringBuffer& operator+=(IN CONST IMS_UINT32 nValue) { return Append(nValue); }
    inline AStringBuffer& operator+=(IN CONST IMS_SLONG nValue) { return Append(nValue); }
    inline AStringBuffer& operator+=(IN CONST IMS_ULONG nValue) { return Append(nValue); }
    inline IMS_CHAR operator[](IN IMS_SINT32 nIndex) const { return strValue[nIndex]; }
    inline AString::CharRef operator[](IN IMS_SINT32 nIndex) { return strValue[nIndex]; }
    inline IMS_CHAR operator[](IN IMS_UINT32 nIndex) const { return strValue[nIndex]; }
    inline AString::CharRef operator[](IN IMS_UINT32 nIndex) { return strValue[nIndex]; }

public:
    inline IMS_SINT32 GetCapacity() const { return strValue.GetCapacity(); }
    void SetCapacity(IN IMS_SINT32 nSize);
    inline IMS_SINT32 GetLength() const { return strValue.GetLength(); }

    inline IMS_CHAR* GetCharString() { return strValue.GetStr(); }
    inline const IMS_CHAR* GetCharString() const
    {
        return static_cast<const AString&>(strValue).GetStr();
    }
    inline AString GetString() { return strValue; }
    inline const AString& GetString() const { return strValue; }

    inline IMS_BOOL IsEmpty() const { return strValue.IsEmpty(); }
    inline IMS_BOOL IsNULL() const { return strValue.IsNULL(); }

    inline void Chop(IN IMS_SINT32 nCount) { strValue.Chop(nCount); }
    inline void Truncate(IN IMS_SINT32 nOffset) { strValue.Truncate(nOffset); }

    inline AStringBuffer& Append(IN CONST IMS_CHAR ch)
    {
        strValue.Append(ch);
        return (*this);
    }
    inline AStringBuffer& Append(IN CONST IMS_CHAR* pszValue)
    {
        strValue.Append(pszValue);
        return (*this);
    }
    inline AStringBuffer& Append(IN CONST AString& strValue)
    {
        this->strValue.Append(strValue);
        return (*this);
    }
    AStringBuffer& Append(IN CONST IMS_SINT16 nValue);
    AStringBuffer& Append(IN CONST IMS_UINT16 nValue);
    AStringBuffer& Append(IN CONST IMS_SINT32 nValue);
    AStringBuffer& Append(IN CONST IMS_UINT32 nValue);
    AStringBuffer& Append(IN CONST IMS_SLONG nValue);
    AStringBuffer& Append(IN CONST IMS_ULONG nValue);
    inline AStringBuffer& Erase(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount)
    {
        strValue.Erase(nOffset, nCount);
        return (*this);
    }
    inline AStringBuffer& Fill(IN IMS_CHAR ch, IN IMS_SINT32 nSize = -1)
    {
        strValue.Fill(ch, nSize);
        return (*this);
    }
    inline AStringBuffer& Insert(IN IMS_SINT32 i, IN CONST IMS_CHAR ch)
    {
        strValue.Insert(i, ch);
        return (*this);
    }
    inline AStringBuffer& Insert(IN IMS_SINT32 i, IN CONST IMS_CHAR* pszValue)
    {
        strValue.Insert(i, pszValue);
        return (*this);
    }
    inline AStringBuffer& Insert(IN IMS_SINT32 i, IN CONST AString& strValue)
    {
        this->strValue.Insert(i, strValue);
        return (*this);
    }
    AStringBuffer& Insert(IN IMS_SINT32 i, IN CONST IMS_SINT16 nValue);
    AStringBuffer& Insert(IN IMS_SINT32 i, IN CONST IMS_UINT16 nValue);
    AStringBuffer& Insert(IN IMS_SINT32 i, IN CONST IMS_SINT32 nValue);
    AStringBuffer& Insert(IN IMS_SINT32 i, IN CONST IMS_UINT32 nValue);
    AStringBuffer& Insert(IN IMS_SINT32 i, IN CONST IMS_SLONG nValue);
    AStringBuffer& Insert(IN IMS_SINT32 i, IN CONST IMS_ULONG nValue);
    AStringBuffer& Prepend(IN CONST IMS_CHAR ch)
    {
        strValue.Prepend(ch);
        return (*this);
    }
    inline AStringBuffer& Prepend(IN CONST IMS_CHAR* pszValue)
    {
        strValue.Prepend(pszValue);
        return (*this);
    }
    inline AStringBuffer& Prepend(IN CONST AString& strValue)
    {
        this->strValue.Prepend(strValue);
        return (*this);
    }
    inline AStringBuffer& Replace(
            IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount, IN CONST IMS_CHAR* pszNew)
    {
        strValue.Replace(nOffset, nCount, pszNew);
        return (*this);
    }
    inline AStringBuffer& Replace(
            IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount, IN CONST AString& strNew)
    {
        strValue.Replace(nOffset, nCount, strNew);
        return (*this);
    }
    inline AStringBuffer& Replace(IN CONST IMS_CHAR cOld, IN CONST IMS_CHAR cNew)
    {
        strValue.Replace(cOld, cNew);
        return (*this);
    }
    inline AStringBuffer& Replace(IN CONST IMS_CHAR cOld, IN CONST IMS_CHAR* pszNew)
    {
        strValue.Replace(cOld, pszNew);
        return (*this);
    }
    inline AStringBuffer& Replace(IN CONST IMS_CHAR cOld, IN CONST AString& strNew)
    {
        strValue.Replace(cOld, strNew);
        return (*this);
    }
    inline AStringBuffer& Replace(IN CONST IMS_CHAR* pszOld, IN CONST IMS_CHAR* pszNew)
    {
        strValue.Replace(pszOld, pszNew);
        return (*this);
    }
    inline AStringBuffer& Replace(IN CONST IMS_CHAR* pszOld, IN CONST AString& strNew)
    {
        strValue.Replace(pszOld, strNew);
        return (*this);
    }
    inline AStringBuffer& Replace(IN CONST AString& strOld, IN CONST IMS_CHAR* pszNew)
    {
        strValue.Replace(strOld, pszNew);
        return (*this);
    }
    inline AStringBuffer& Replace(IN CONST AString& strOld, IN CONST AString& strNew)
    {
        strValue.Replace(strOld, strNew);
        return (*this);
    }

    AStringBuffer& Sprintf(IN CONST IMS_CHAR* pszFormat, ...);

private:
    AString strValue;
};

inline const AStringBuffer operator+(IN CONST AStringBuffer& objA1, IN CONST AStringBuffer& objA2)
{
    return AStringBuffer(objA1) += objA2;
}

inline const AStringBuffer operator+(IN CONST AStringBuffer& objA1, IN CONST IMS_CHAR* pszA2)
{
    return AStringBuffer(objA1) += pszA2;
}

inline const AStringBuffer operator+(IN CONST AStringBuffer& objA1, IN CONST IMS_CHAR cA2)
{
    return AStringBuffer(objA1) += cA2;
}

inline const AStringBuffer operator+(IN CONST IMS_CHAR* pszA1, IN CONST AStringBuffer& objA2)
{
    return AStringBuffer(AString(pszA1)) += objA2;
}

inline const AStringBuffer operator+(IN CONST IMS_CHAR cA1, IN CONST AStringBuffer& objA1)
{
    return AStringBuffer(AString(&cA1, 1)) += objA1;
}

#endif  // _ANSI_STRING_BUFFER_H_
