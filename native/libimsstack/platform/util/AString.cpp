/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090302  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "IMSLib.h"
#include "IMSStrLib.h"
#include "AString.h"

// Format specification flag types for Vsprintf(...)
enum FLAG_ENTYPE
{
    FLAG_NONE = 0,
    FLAG_ALTERNATE = 0x01,
    FLAG_ZERO_PADDED = 0x02,
    FLAG_LEFT_ALIGNED = 0x04,
    FLAG_BLANK_BEFORE_POSITIVE = 0x08,
    FLAG_ALWAYS_SHOW_SIGN = 0x10,
    FLAG_CAPITAL_E_X = 0x20
};

PRIVATE GLOBAL AString::Data AString::SHARED_NULL =
{
    0, 0, AString::SHARED_NULL.acValue,
    {
        0
    }
};

PRIVATE GLOBAL AString::Data AString::SHARED_EMPTY =
{
    0, 0, AString::SHARED_EMPTY.acValue,
    {
        0
    }
};

LOCAL
IMS_SINT32 astring_CharToWideChar(IN CONST IMS_CHAR* pszSrc, IN IMS_SINT32 nSrcLen,
        OUT IMS_WCHAR* pwszDest, IN IMS_SINT32 nDestLen)
{
    if (nDestLen == 0)
    {
        if (nSrcLen == (-1))
        {
            return IMS_StrLen(pszSrc);
        }

        return nSrcLen;
    }

    if (nSrcLen < 0)
    {
        nSrcLen = IMS_StrLen(pszSrc);
    }

    IMS_SINT32 nCountOfWChar = 0;

    for (IMS_SINT32 i = 0; i < nSrcLen; ++i)
    {
        if (i < nDestLen)
        {
            pwszDest[i] = (IMS_WCHAR)((IMS_UCHAR)pszSrc[i]);
            nCountOfWChar++;
        }
    }

    pwszDest[nDestLen - 1] = (IMS_WCHAR)0;

    return nCountOfWChar;
}

LOCAL
IMS_SINT32 astring_WideCharToChar(IN CONST IMS_WCHAR* pwszSrc, IN IMS_SINT32 nSrcLen,
        OUT IMS_CHAR* pszDest, IN IMS_SINT32 nDestLen)
{
    if (nDestLen == 0)
    {
        if (nSrcLen == (-1))
        {
            return IMS_UcStrLen(pwszSrc);
        }

        return nSrcLen;
    }

    if (nSrcLen < 0)
    {
        nSrcLen = IMS_UcStrLen(pwszSrc);
    }

    IMS_SINT32 nCountOfChar = 0;

    for (IMS_SINT32 i = 0; i < nSrcLen; ++i)
    {
        if (i < nDestLen)
        {
            // Truncate
            // pszDest[i] = (IMS_CHAR)(pwszSrc[i]);
            pszDest[i] = (pwszSrc[i] > 0xFF) ? '?' : (IMS_CHAR)pwszSrc[i];

            nCountOfChar++;
        }
    }

    pszDest[nDestLen - 1] = '\0';

    return nCountOfChar;
}

LOCAL
IMS_SINT64 astring_AToLL(IN CONST IMS_CHAR* pszLL, IN IMS_SINT32 nBase, OUT IMS_BOOL* pbOK,
        OUT CONST IMS_CHAR** ppEndPtr)
{
    const IMS_CHAR* p;
    IMS_UCHAR ch;
    IMS_ULONG nAcc;
    IMS_ULONG nBase2, nCutOff;
    IMS_SINT32 nNegative, nAny, nCutLim;

    if (pbOK)
    {
        (*pbOK) = IMS_TRUE;
    }

    // Skip white space and pick up leading +/- sign if present.
    // If base is 0, allow 0x for hex and 0 for octal.
    // Else, assume a decimal; if base is already 16, allow 0x.
    p = pszLL;

    do
    {
        ch = *p++;
    } while (IMS_ISSPACE(ch));

    if (ch == '-')
    {
        nNegative = 1;
        ch = *p++;
    }
    else
    {
        nNegative = 0;

        if (ch == '+')
        {
            ch = *p++;
        }
    }

    if (((nBase == 0) || (nBase == 16)) && (ch == '0') && ((*p == 'x') || (*p == 'X')))
    {
        ch = p[1];
        p += 2;
        nBase = 16;
    }

    if (nBase == 0)
    {
        nBase = (ch == '0') ? 8 : 10;
    }

    nBase2 = unsigned(nBase);
    nCutOff =
            nNegative ? IMS_ULONG(0 - (IMS_LONG_MIN + IMS_LONG_MAX)) + IMS_LONG_MAX : IMS_LONG_MAX;
    nCutLim = nCutOff % nBase2;
    nCutOff /= nBase2;

    for (nAcc = 0, nAny = 0;; ch = *p++)
    {
        if (!IMS_ISASCII(ch))
        {
            break;
        }

        if (IMS_ISDIGIT(ch))
        {
            ch -= '0';
        }
        else if (IMS_ISALPHA(ch))
        {
            ch -= IMS_ISUPPER(ch) ? 'A' - 10 : 'a' - 10;
        }
        else
        {
            break;
        }

        if (ch >= nBase)
        {
            break;
        }

        if (nAny < 0 || nAcc > nCutOff || (nAcc == nCutOff && ch > nCutLim))
        {
            nAny = -1;
        }
        else
        {
            nAny = 1;
            nAcc *= nBase2;
            nAcc += ch;
        }
    }

    if (nAny < 0)
    {
        nAcc = nNegative ? IMS_LONG_MIN : IMS_LONG_MAX;

        if (pbOK != IMS_NULL)
        {
            (*pbOK) = IMS_FALSE;
        }
    }
    else if (nNegative)
    {
        nAcc = (~nAcc) + 1;
    }

    if (ppEndPtr)
    {
        *ppEndPtr = (nAny ? p - 1 : pszLL);
    }

    return nAcc;
}

LOCAL
IMS_UINT64 astring_AToULL(IN CONST IMS_CHAR* pszULL, IN IMS_SINT32 nBase, OUT IMS_BOOL* pbOK,
        OUT CONST IMS_CHAR** ppEndPtr)
{
    const IMS_CHAR* p;
    IMS_UCHAR ch;
    IMS_ULONG nAcc;
    IMS_ULONG nBase2, nCutOff;
    IMS_SINT32 nNegative, nAny, nCutLim;

    if (pbOK)
    {
        (*pbOK) = IMS_TRUE;
    }

    // Skip white space and pick up leading +/- sign if present.
    // If base is 0, allow 0x for hex and 0 for octal.
    // Else, assume a decimal; if base is already 16, allow 0x.
    p = pszULL;

    do
    {
        ch = *p++;
    } while (IMS_ISSPACE(ch));

    if (ch == '-')
    {
        if (pbOK != IMS_NULL)
        {
            (*pbOK) = IMS_FALSE;
        }

        if (ppEndPtr != IMS_NULL)
        {
            (*ppEndPtr) = p - 1;
        }

        return 0;
    }
    else
    {
        nNegative = 0;

        if (ch == '+')
        {
            ch = *p++;
        }
    }

    if (((nBase == 0) || (nBase == 16)) && (ch == '0') && ((*p == 'x') || (*p == 'X')))
    {
        ch = p[1];
        p += 2;
        nBase = 16;
    }

    if (nBase == 0)
    {
        nBase = (ch == '0') ? 8 : 10;
    }

    nBase2 = unsigned(nBase);
    nCutOff = IMS_ULONG(IMS_ULONG_MAX) / nBase2;
    nCutLim = IMS_ULONG(IMS_ULONG_MAX) % nBase2;

    for (nAcc = 0, nAny = 0;; ch = *p++)
    {
        if (!IMS_ISASCII(ch))
        {
            break;
        }

        if (IMS_ISDIGIT(ch))
        {
            ch -= '0';
        }
        else if (IMS_ISALPHA(ch))
        {
            ch -= IMS_ISUPPER(ch) ? 'A' - 10 : 'a' - 10;
        }
        else
        {
            break;
        }

        if (ch >= nBase)
        {
            break;
        }

        if (nAny < 0 || nAcc > nCutOff || (nAcc == nCutOff && ch > nCutLim))
        {
            nAny = -1;
        }
        else
        {
            nAny = 1;
            nAcc *= nBase2;
            nAcc += ch;
        }
    }

    if (nAny < 0)
    {
        nAcc = IMS_ULONG_MAX;

        if (pbOK != IMS_NULL)
        {
            (*pbOK) = IMS_FALSE;
        }
    }
    else if (nNegative)
    {
        nAcc = (~nAcc) + 1;
    }

    if (ppEndPtr)
    {
        *ppEndPtr = (nAny ? p - 1 : pszULL);
    }

    return nAcc;
}

LOCAL
AString astring_ULLToA(IN IMS_UINT64 nULL, IN IMS_SINT32 nBase)
{
    // Length of MAX_UINT64 in base 2
    IMS_CHAR acBuffer[65] = {
            0,
    };
    IMS_CHAR* pULL = acBuffer + 65;
    IMS_SINT32 n1Cipher;

    if (nBase != 10)
    {
        if (nULL == 0)
        {
            return AString("0", 1);
        }

        while (nULL != 0)
        {
            n1Cipher = nULL % nBase;

            --pULL;

            if (n1Cipher < 10)
            {
                *pULL = '0' + n1Cipher;
            }
            else
            {
                *pULL = n1Cipher - 10 + 'a';
            }

            nULL /= nBase;
        }
    }
    else
    {
        if (nULL == 0)
        {
            return AString("0", 1);
        }

        while (nULL != 0)
        {
            n1Cipher = nULL % nBase;

            *(--pULL) = '0' + n1Cipher;

            nULL /= nBase;
        }
    }

    return AString(pULL, 65 - (pULL - acBuffer));
}

LOCAL
AString astring_LLToA(IN IMS_SINT64 nLL, IN IMS_SINT32 nBase)
{
    return astring_ULLToA(nLL < 0 ? -nLL : nLL, nBase);
}

LOCAL
IMS_SINT32 astring_AllocateMore(IN IMS_SIZE_T nAlloc, IN IMS_SIZE_T nExtra)
{
    const IMS_SIZE_T nPage = 1 << 12;  // 4096 bytes
    IMS_SIZE_T nNumAlloc;

    nAlloc += nExtra;

    if (nAlloc < (1 << 5))  // 32 bytes
    {
        nNumAlloc = (1 << 3) + ((nAlloc >> 3) << 3);
    }
    else
    {
        nNumAlloc = (nAlloc < nPage) ? (1 << 3) : nPage;

        while (nNumAlloc < nAlloc)
        {
            nNumAlloc *= 2;
        }
    }

    return LONG_TO_INT(nNumAlloc - nExtra);
}

LOCAL
AString& astring_Insert(
        IN AString* pAS, IN IMS_SINT32 nOffset, IN CONST IMS_CHAR* pszValue, IN IMS_SINT32 nCount)
{
    if ((nOffset < 0) || (nCount <= 0) || (pszValue == IMS_NULL))
    {
        return (*pAS);
    }

    const IMS_SINT32 nOldLen = pAS->GetLength();

    pAS->Resize(IMS_MAX(nOffset, nOldLen) + nCount);

    IMS_CHAR* pDest = pAS->GetStr();

    if (nOffset > nOldLen)
    {
        IMS_MEM_Memset(pDest + nOldLen, 0x20, nOffset - nOldLen);
    }
    else
    {
        IMS_MEM_Memmove(pDest + nOffset + nCount, pDest + nOffset, nOldLen - nOffset);
    }

    IMS_MEM_Memcpy(pDest + nOffset, pszValue, nCount);

    return (*pAS);
}

LOCAL
AString astring_LLToString(IN IMS_SINT64 nLL, IN IMS_SINT32 nPrecision = -1,
        IN IMS_SINT32 nBase = 10, IN IMS_SINT32 nWidth = -1, IN IMS_UINT32 nFlags = FLAG_NONE)
{
    IMS_BOOL bPrecisionNotSpecified = IMS_FALSE;

    if (nPrecision == (-1))
    {
        bPrecisionNotSpecified = IMS_TRUE;
        nPrecision = 1;
    }

    IMS_BOOL bNegative = (nLL < 0);

    if (nBase != 10)
    {
        // These are not supported by sprintf for octal and hex
        nFlags &= ~FLAG_ALWAYS_SHOW_SIGN;
        nFlags &= ~FLAG_BLANK_BEFORE_POSITIVE;
        bNegative = IMS_FALSE;  // Neither are negative numbers
    }

    AString strNumber;

    if (nBase == 10)
    {
        strNumber = astring_LLToA(nLL, nBase);
    }
    else
    {
        strNumber = astring_ULLToA(nLL, nBase);
    }

    /*
    IMS_UINT32 nThousandSepCount = 0;

    if ((nFlags & FLAG_THOUSANDS_GROUP) && (nBase == 10))
    {
        for (IMS_SINT32 i = strNumber.GetLength() - 3; i > 0; i -= 3)
        {
            strNumber.Insert(i, ',');
            ++nThousandSepCount;
        }
    }
    */

    for (IMS_SINT32 i = strNumber.GetLength(); i < nPrecision; ++i)
    {
        strNumber.Prepend('0');
    }

    if ((nFlags & FLAG_ALTERNATE) && (nBase == 8) && (strNumber.IsEmpty()))
    {
        strNumber.Prepend('0');
    }

    // FLAG_LEFT_ALIGNED overrides the flag, FLAG_ZERO_PADDED.
    // sprintf(...) only padds when precision is not specified in the format string.
    IMS_BOOL bZeroPadded =
            (nFlags & FLAG_ZERO_PADDED) && !(nFlags & FLAG_LEFT_ALIGNED) && bPrecisionNotSpecified;

    if (bZeroPadded)
    {
        IMS_SINT32 nPaddingCount = nWidth - strNumber.GetLength();

        // Leave space for the sign
        if (bNegative || (nFlags & FLAG_ALWAYS_SHOW_SIGN) || (nFlags & FLAG_BLANK_BEFORE_POSITIVE))
        {
            --nPaddingCount;
        }

        // Leave space for optional '0x' in hex form
        if ((nBase == 16) && (nFlags & FLAG_ALTERNATE) && (nLL != 0))
        {
            nPaddingCount -= 2;
        }

        for (IMS_SINT32 i = 0; i < nPaddingCount; ++i)
        {
            strNumber.Prepend('0');
        }
    }

    if ((nBase == 16) && (nFlags & FLAG_ALTERNATE) && (nLL != 0))
    {
        strNumber.Prepend("0x");
    }

    // Add sign
    if (bNegative)
    {
        strNumber.Prepend('-');
    }
    else if (nFlags & FLAG_ALWAYS_SHOW_SIGN)
    {
        strNumber.Prepend('+');
    }
    else if (nFlags & FLAG_BLANK_BEFORE_POSITIVE)
    {
        strNumber.Prepend(' ');
    }

    if (nFlags & FLAG_CAPITAL_E_X)
    {
        strNumber = strNumber.MakeUpper();
    }

    return strNumber;
}

LOCAL
AString astring_ULLToString(IN IMS_UINT64 nULL, IN IMS_SINT32 nPrecision = -1,
        IN IMS_SINT32 nBase = 10, IN IMS_SINT32 nWidth = -1, IN IMS_UINT32 nFlags = FLAG_NONE)
{
    IMS_BOOL bPrecisionNotSpecified = IMS_FALSE;

    if (nPrecision == (-1))
    {
        bPrecisionNotSpecified = IMS_TRUE;
        nPrecision = 1;
    }

    AString strNumber = astring_ULLToA(nULL, nBase);

    /*
    IMS_UINT32 nThousandSepCount = 0;

    if ((nFlags & ThousandsGroup) && (nBase == 10))
    {
        for (IMS_SINT32 i = strNumber.GetLength() - 3; i > 0; i -= 3)
        {
            strNumber.Insert(i, ',');
            ++nThousandSepCount;
        }
    }
    */

    for (IMS_SINT32 i = strNumber.GetLength(); i < nPrecision; ++i)
    {
        strNumber.Prepend('0');
    }

    if ((nFlags & FLAG_ALTERNATE) && (nBase == 8) && (strNumber.IsEmpty()))
    {
        strNumber.Prepend('0');
    }

    // FLAG_LEFT_ALIGNED overrides the flag, FLAG_ZERO_PADDED.
    // sprintf only padds when precision is not specified in the format string.
    IMS_BOOL bZeroPadded =
            (nFlags & FLAG_ZERO_PADDED) && !(nFlags & FLAG_LEFT_ALIGNED) && bPrecisionNotSpecified;

    if (bZeroPadded)
    {
        IMS_SINT32 nPaddingCount = nWidth - strNumber.GetLength();

        // Leave space for optional '0x' in hex form
        if ((nBase == 16) && (nFlags & FLAG_ALTERNATE) && (nULL != 0))
        {
            nPaddingCount -= 2;
        }

        for (IMS_SINT32 i = 0; i < nPaddingCount; ++i)
        {
            strNumber.Prepend('0');
        }
    }

    if ((nBase == 16) && (nFlags & FLAG_ALTERNATE) && (nULL != 0))
    {
        strNumber.Prepend("0x");
    }

    if (nFlags & FLAG_CAPITAL_E_X)
    {
        strNumber = strNumber.MakeUpper();
    }

    return strNumber;
}

PUBLIC
AString::CharRef::CharRef(IN AString& objStr_, IN IMS_SINT32 nIndex_) :
        objStr(objStr_),
        nIndex(nIndex_)
{
}

PUBLIC
AString::CharRef& AString::CharRef::operator=(IN CONST CharRef& objRHS)
{
    if (this != &objRHS)
    {
        objStr.pData->pValue[nIndex] = objRHS.objStr.pData->pValue[objRHS.nIndex];
    }

    return (*this);
}

PUBLIC
AString::CharRef& AString::CharRef::operator=(IN IMS_CHAR ch)
{
    objStr.pData->pValue[nIndex] = ch;
    return (*this);
}

PUBLIC
AString::AString() :
        pData(&SHARED_NULL)
{
}

PUBLIC
AString::AString(IN CONST IMS_CHAR* pszValue)
{
    if (pszValue == IMS_NULL)
    {
        pData = &SHARED_NULL;
    }
    else if ((*pszValue) == '\0')
    {
        pData = &SHARED_EMPTY;
    }
    else
    {
        IMS_SINT32 nLen = IMS_StrLen(pszValue);

        pData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data) + nLen));

        if (pData == IMS_NULL)
        {
            pData = &SHARED_NULL;
        }
        else
        {
            pData->nAlloc = pData->nSize = nLen;
            pData->pValue = pData->acValue;
            // copy with null-terminated character
            IMS_MEM_Memcpy(pData->acValue, pszValue, nLen + 1);
        }
    }
}

PUBLIC
AString::AString(IN CONST IMS_CHAR* pszValue, IN IMS_SINT32 nSize)
{
    if (pszValue == IMS_NULL)
    {
        pData = &SHARED_NULL;
    }
    else if (nSize <= 0)
    {
        pData = &SHARED_EMPTY;
    }
    else
    {
        pData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data) + nSize));

        if (pData == IMS_NULL)
        {
            pData = &SHARED_NULL;
        }
        else
        {
            pData->nAlloc = pData->nSize = nSize;
            pData->pValue = pData->acValue;
            IMS_MEM_Memcpy(pData->acValue, pszValue, nSize);
            pData->acValue[nSize] = '\0';
        }
    }
}

PUBLIC
AString::AString(IN CONST IMS_CHAR* pszValue, IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount)
{
    if (pszValue == IMS_NULL)
    {
        pData = &SHARED_NULL;
    }
    else if ((nCount <= 0) || (nOffset <= 0))
    {
        pData = &SHARED_EMPTY;
    }
    else
    {
        pData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data) + nCount));

        if (pData == IMS_NULL)
        {
            pData = &SHARED_NULL;
        }
        else
        {
            pData->nAlloc = pData->nSize = nCount;
            pData->pValue = pData->acValue;
            IMS_MEM_Memcpy(pData->acValue, &(pszValue[nOffset]), nCount);
            pData->acValue[nCount] = '\0';
        }
    }
}

PUBLIC
AString::AString(IN IMS_SINT32 nSize, IN CONST IMS_CHAR ch)
{
    if (nSize <= 0)
    {
        pData = &SHARED_EMPTY;
    }
    else
    {
        pData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data) + nSize));

        if (pData == IMS_NULL)
        {
            pData = &SHARED_NULL;
        }
        else
        {
            pData->nAlloc = pData->nSize = nSize;
            pData->pValue = pData->acValue;
            IMS_MEM_Memset(pData->acValue, ch, nSize);
            pData->acValue[nSize] = '\0';
        }
    }
}

PUBLIC
AString::AString(IN CONST AString& objRHS)
{
    if (objRHS.pData == &SHARED_EMPTY)
    {
        pData = &SHARED_EMPTY;
    }
    else if (objRHS.pData == &SHARED_NULL)
    {
        pData = &SHARED_NULL;
    }
    else
    {
        pData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data) + objRHS.pData->nAlloc));

        if (pData == IMS_NULL)
        {
            pData = &SHARED_NULL;
        }
        else
        {
            pData->nAlloc = objRHS.pData->nAlloc;
            pData->nSize = objRHS.pData->nSize;
            pData->pValue = pData->acValue;
            IMS_MEM_Memcpy(pData->acValue, objRHS.pData->pValue, pData->nSize + 1);
        }
    }
}

PUBLIC
AString::~AString()
{
    Clear();
}

PRIVATE
AString::AString(IN IMS_SINT32 nSize)
{
    if (nSize <= 0)
    {
        pData = &SHARED_EMPTY;
    }
    else
    {
        pData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data) + nSize));

        if (pData == IMS_NULL)
        {
            pData = &SHARED_NULL;
        }
        else
        {
            pData->nAlloc = nSize;
            pData->nSize = 0;
            pData->pValue = pData->acValue;
            IMS_MEM_Memset(pData->acValue, 0x00, nSize);
            pData->acValue[nSize] = '\0';
        }
    }
}

PRIVATE
AString::AString(IN Data* pData_) :
        pData(pData_)
{
}

PUBLIC
AString& AString::operator=(IN CONST AString& objRHS)
{
    if (this != &objRHS)
    {
        if (objRHS.IsEmpty())
        {
            Clear();
            pData = &SHARED_EMPTY;
        }
        else if (objRHS.IsNULL())
        {
            Clear();
            pData = &SHARED_NULL;
        }
        else
        {
            IMS_SINT32 nLen = objRHS.pData->nSize;

            if ((nLen > pData->nAlloc) || ((nLen < pData->nSize) && (nLen < (pData->nAlloc >> 1))))
            {
                Realloc(nLen);
            }

            // include NULL character
            IMS_MEM_Memcpy(pData->pValue, objRHS.pData->pValue, nLen + 1);
            pData->nSize = nLen;
        }
    }

    return (*this);
}

PUBLIC
AString& AString::operator=(IN CONST IMS_CHAR ch)
{
    const IMS_CHAR acValue[2] = {ch, '\0'};

    return operator=(acValue);
}

PUBLIC
AString& AString::operator=(IN CONST IMS_CHAR* pszValue)
{
    if (pszValue == IMS_NULL)
    {
        Clear();
        pData = &SHARED_NULL;
    }
    else if ((*pszValue) == '\0')
    {
        Clear();
        pData = &SHARED_EMPTY;
    }
    else
    {
        IMS_SINT32 nLen = IMS_StrLen(pszValue);

        if ((nLen > pData->nAlloc) || ((nLen < pData->nSize) && (nLen < (pData->nAlloc >> 1))))
        {
            Realloc(nLen);
        }

        // include NULL character
        IMS_MEM_Memcpy(pData->pValue, pszValue, nLen + 1);
        pData->nSize = nLen;
    }

    return (*this);
}

PUBLIC
AString& AString::operator=(IN CONST IMS_WCHAR* pwszValue)
{
    if (pwszValue == IMS_NULL)
    {
        Clear();
        pData = &SHARED_NULL;
    }

    IMS_SINT32 nLen = astring_WideCharToChar(pwszValue, -1, IMS_NULL, 0);

    if (nLen == 0)
    {
        Clear();
        pData = &SHARED_EMPTY;
    }
    else
    {
        if ((nLen > pData->nAlloc) || ((nLen < pData->nSize) && (nLen < (pData->nAlloc >> 1))))
        {
            Realloc(nLen);
        }

        astring_WideCharToChar(pwszValue, nLen, pData->pValue, nLen + 1);
        pData->nSize = nLen;
    }

    return (*this);
}

PUBLIC
IMS_CHAR AString::operator[](IN IMS_SINT32 nIndex) const
{
    IMS_ASSERT(nIndex >= 0 && nIndex < GetLength());

    return pData->pValue[nIndex];
}

PUBLIC
AString::CharRef AString::operator[](IN IMS_SINT32 nIndex)
{
    IMS_ASSERT(nIndex >= 0 && nIndex < GetLength());

    return CharRef(*this, nIndex);
}

PUBLIC
IMS_CHAR AString::operator[](IN IMS_UINT32 nIndex) const
{
    IMS_ASSERT(nIndex < IMS_UINT32(GetLength()));

    return pData->pValue[nIndex];
}

PUBLIC
AString::CharRef AString::operator[](IN IMS_UINT32 nIndex)
{
    IMS_ASSERT(nIndex < IMS_UINT32(GetLength()));

    return CharRef(*this, nIndex);
}

PUBLIC
IMS_BOOL AString::operator<(IN CONST AString& objRHS)
{
    if (pData == objRHS.pData)
    {
        return IMS_FALSE;
    }

    if (!IsNULL() && objRHS.IsNULL())
    {
        return IMS_FALSE;
    }

    if (IsNULL() && !objRHS.IsNULL())
    {
        return IMS_TRUE;
    }

    return (Compare(pData->pValue, objRHS.pData->pValue) < 0);
}

PUBLIC
AString AString::GetSubStr(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount /* = -1 */) const
{
    if ((pData == &SHARED_NULL) || (pData == &SHARED_EMPTY) || (nOffset >= pData->nSize))
    {
        return AString(&SHARED_NULL);
    }

    if (nCount < 0)
    {
        nCount = pData->nSize - nOffset;
    }

    if (nOffset < 0)
    {
        nCount += nOffset;
        nOffset = 0;
    }

    if ((nCount + nOffset) > pData->nSize)
    {
        nCount = pData->nSize - nOffset;
    }

    if ((nOffset == 0) && (nCount == pData->nSize))
    {
        return (*this);
    }

    return AString(pData->pValue + nOffset, nCount);
}

PUBLIC
void AString::Chop(IN IMS_SINT32 nCount)
{
    if (nCount > 0)
    {
        Resize(pData->nSize - nCount);
    }
}

PUBLIC
void AString::Truncate(IN IMS_SINT32 nOffset)
{
    if (nOffset < pData->nSize)
    {
        Resize(nOffset);
    }
}

PUBLIC
void AString::Attach(IN CONST IMS_CHAR* pValue, IN IMS_SINT32 nSize)
{
    Data* pNewData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data)));

    if (pValue != IMS_NULL)
    {
        pNewData->pValue = const_cast<IMS_CHAR*>(pValue);
    }
    else
    {
        pNewData->pValue = pNewData->acValue;
        nSize = 0;
    }

    pNewData->nAlloc = pNewData->nSize = nSize;
    pNewData->acValue[0] = '\0';

    Clear();

    pData = pNewData;
}

PUBLIC
void AString::Detach()
{
    if (pData->pValue != pData->acValue)
    {
        Realloc(pData->nSize);
    }
}

PUBLIC
void AString::Resize(IN IMS_SINT32 nSize)
{
    if (nSize <= 0)
    {
        if (pData == &SHARED_NULL)
        {
            return;
        }

        Clear();
        pData = &SHARED_EMPTY;
    }
    else
    {
        if ((nSize > pData->nAlloc) || ((nSize < pData->nSize) && (nSize < (pData->nAlloc >> 1))))
        {
            Realloc(astring_AllocateMore(nSize, sizeof(Data)));
        }

        if (pData->nAlloc >= nSize)
        {
            pData->nSize = nSize;
            pData->pValue = pData->acValue;
            pData->acValue[nSize] = '\0';
        }
    }
}

PUBLIC
IMS_SINT32 AString::GetIndexOf(IN CONST IMS_CHAR ch, IN IMS_SINT32 nOffset /* = 0 */) const
{
    if (nOffset < 0)
    {
        nOffset = IMS_MAX(nOffset + pData->nSize, 0);
    }

    if (nOffset < pData->nSize)
    {
        const IMS_CHAR* pStart = pData->pValue + nOffset - 1;
        const IMS_CHAR* pEnd = pData->pValue + pData->nSize;

        while (++pStart != pEnd)
        {
            if (*pStart == ch)
            {
                return (pStart - pData->pValue);
            }
        }
    }

    return NPOS;
}

PUBLIC
IMS_SINT32 AString::GetIndexOf(IN CONST IMS_CHAR* pszValue, IN IMS_SINT32 nOffset /* = 0 */) const
{
    return GetIndexOf(FromRawData(pszValue, IMS_StrLen(pszValue)), nOffset);
}

PUBLIC
IMS_SINT32 AString::GetIndexOf(IN CONST AString& objStr, IN IMS_SINT32 nOffset /* = 0 */) const
{
    const IMS_SINT32 nLen = pData->nSize;
    const IMS_SINT32 nOtherLen = objStr.pData->nSize;

    if ((nOffset > pData->nSize) || (nOtherLen + nOffset > nLen))
    {
        return NPOS;
    }

    if (nOtherLen == 0)
    {
        return NPOS;
    }

    if (nOtherLen == 1)
    {
        return GetIndexOf(*(objStr.pData->pValue), nOffset);
    }

    const IMS_CHAR* pNeedle = objStr.pData->pValue;
    const IMS_CHAR* pHayStack = pData->pValue + nOffset;
    const IMS_CHAR* pEnd = pData->pValue + (nLen - nOtherLen);
    // FIXME: __IMS_LP64__
    const IMS_UINT32 OL_MINUS_1 = nOtherLen - 1;
    IMS_UINT32 nHashNeedle = 0, nHashHayStack = 0;
    IMS_SINT32 nIndex;

    for (nIndex = 0; nIndex < nOtherLen; ++nIndex)
    {
        nHashNeedle = ((nHashNeedle << 1) + pNeedle[nIndex]);
        nHashHayStack = ((nHashHayStack << 1) + pHayStack[nIndex]);
    }

    nHashHayStack -= *(pHayStack + OL_MINUS_1);

    while (pHayStack <= pEnd)
    {
        nHashHayStack += *(pHayStack + OL_MINUS_1);

        if ((nHashHayStack == nHashNeedle) && (*pNeedle == *pHayStack) &&
                (IMS_StrNCmp(pNeedle, pHayStack, nOtherLen) == 0))
        {
            return LONG_TO_INT(pHayStack - pData->pValue);
        }

        if (OL_MINUS_1 < sizeof(IMS_UINT32) * 8 /* number of char bits */)
        {
            nHashHayStack -= (*pHayStack) << OL_MINUS_1;
        }

        nHashHayStack <<= 1;

        ++pHayStack;
    }

    return NPOS;
}

PUBLIC
IMS_SINT32 AString::GetLastIndexOf(IN CONST IMS_CHAR ch, IN IMS_SINT32 nOffset /* = -1 */) const
{
    if (nOffset < 0)
    {
        nOffset += pData->nSize;
    }
    else if (nOffset > pData->nSize)
    {
        nOffset = pData->nSize - 1;
    }

    if (nOffset >= 0)
    {
        const IMS_CHAR* pStart = pData->pValue;
        const IMS_CHAR* pEnd = pData->pValue + nOffset + 1;

        while (pEnd-- != pStart)
        {
            if (*pEnd == ch)
            {
                return (pEnd - pStart);
            }
        }
    }

    return NPOS;
}

PUBLIC
IMS_SINT32 AString::GetLastIndexOf(
        IN CONST IMS_CHAR* pszValue, IN IMS_SINT32 nOffset /* = -1 */) const
{
    return GetLastIndexOf(FromRawData(pszValue, IMS_StrLen(pszValue)), nOffset);
}

PUBLIC
IMS_SINT32 AString::GetLastIndexOf(IN CONST AString& objStr, IN IMS_SINT32 nOffset /* = -1 */) const
{
    const IMS_SINT32 nLen = pData->nSize;
    const IMS_SINT32 nOtherLen = objStr.pData->nSize;
    IMS_SINT32 nDelta = nLen - nOtherLen;

    if (nOffset < 0)
    {
        nOffset = nDelta;
    }

    if ((nOffset < 0) || (nOffset > nLen))
    {
        return NPOS;
    }

    if (nOffset > nDelta)
    {
        nOffset = nDelta;
    }

    if (nOtherLen == 1)
    {
        return GetLastIndexOf(*(objStr.pData->pValue), nOffset);
    }

    const IMS_CHAR* pNeedle = objStr.pData->pValue;
    const IMS_CHAR* pHayStack = pData->pValue + nOffset;
    const IMS_CHAR* pEnd = pData->pValue;
    // FIXME: __IMS_LP64__
    const IMS_UINT32 OL_MINUS_1 = nOtherLen - 1;
    const IMS_CHAR* pEndOfNeedle = pNeedle + OL_MINUS_1;
    const IMS_CHAR* pEndOfHayStack = pHayStack + OL_MINUS_1;

    IMS_UINT32 nHashNeedle = 0, nHashHayStack = 0;
    IMS_SINT32 nIndex;

    for (nIndex = 0; nIndex < nOtherLen; ++nIndex)
    {
        nHashNeedle = ((nHashNeedle << 1) + *(pEndOfNeedle - nIndex));
        nHashHayStack = ((nHashHayStack << 1) + *(pEndOfHayStack - nIndex));
    }

    nHashHayStack -= *pHayStack;

    while (pHayStack >= pEnd)
    {
        nHashHayStack += *pHayStack;

        if ((nHashHayStack == nHashNeedle) && (IMS_StrNCmp(pNeedle, pHayStack, nOtherLen) == 0))
        {
            return (pHayStack - pData->pValue);
        }

        --pHayStack;

        if (OL_MINUS_1 < sizeof(IMS_UINT32) * 8 /* number of char bits */)
        {
            nHashHayStack -= *(pHayStack + nOtherLen) << OL_MINUS_1;
        }

        nHashHayStack <<= 1;
    }

    return NPOS;
}

PUBLIC
IMS_BOOL AString::Contains(IN CONST IMS_CHAR ch) const
{
    return (GetIndexOf(ch) != NPOS);
}

PUBLIC
IMS_BOOL AString::Contains(IN CONST IMS_CHAR* pszValue) const
{
    return (GetIndexOf(pszValue) != NPOS);
}

PUBLIC
IMS_BOOL AString::Contains(IN CONST AString& objStr) const
{
    return (GetIndexOf(objStr) != NPOS);
}

PUBLIC
IMS_BOOL AString::EndsWith(IN CONST IMS_CHAR ch) const
{
    if (pData->nSize == 0)
    {
        return IMS_FALSE;
    }

    return (pData->pValue[pData->nSize - 1] == ch);
}

PUBLIC
IMS_BOOL AString::EndsWith(IN CONST IMS_CHAR* pszValue) const
{
    if (pszValue == IMS_NULL)
    {
        return IMS_TRUE;
    }

    if ((*pszValue) == '\0')
    {
        return IMS_TRUE;
    }

    IMS_SINT32 nLen = IMS_StrLen(pszValue);

    if (pData->nSize < nLen)
    {
        return IMS_FALSE;
    }

    return (IMS_StrNCmp(pData->pValue + pData->nSize - nLen, pszValue, nLen) == 0);
}

PUBLIC
IMS_BOOL AString::EndsWith(IN CONST AString& objStr) const
{
    if ((pData == objStr.pData) || (objStr.pData->nSize == 0))
    {
        return IMS_TRUE;
    }

    if (pData->nSize < objStr.pData->nSize)
    {
        return IMS_FALSE;
    }

    return (IMS_MEM_Memcmp(pData->pValue + pData->nSize - objStr.pData->nSize, objStr.pData->pValue,
                    objStr.pData->nSize) == 0);
}

PUBLIC
IMS_BOOL AString::StartsWith(IN CONST IMS_CHAR ch) const
{
    if (pData->nSize == 0)
    {
        return IMS_FALSE;
    }

    return (pData->pValue[0] == ch);
}

PUBLIC
IMS_BOOL AString::StartsWith(IN CONST IMS_CHAR* pszValue) const
{
    if (pszValue == IMS_NULL)
    {
        return IMS_TRUE;
    }

    if ((*pszValue) == '\0')
    {
        return IMS_TRUE;
    }

    IMS_SINT32 nLen = IMS_StrLen(pszValue);

    if (pData->nSize < nLen)
    {
        return IMS_FALSE;
    }

    return (IMS_StrNCmp(pData->pValue, pszValue, nLen) == 0);
}

PUBLIC
IMS_BOOL AString::StartsWith(IN CONST AString& objStr) const
{
    if ((pData == objStr.pData) || (objStr.pData->nSize == 0))
    {
        return IMS_TRUE;
    }

    if (pData->nSize < objStr.pData->nSize)
    {
        return IMS_FALSE;
    }

    return (IMS_MEM_Memcmp(pData->pValue, objStr.pData->pValue, objStr.pData->nSize) == 0);
}

PUBLIC
IMS_BOOL AString::Equals(IN CONST IMS_CHAR ch) const
{
    if (pData->nSize != 1)
    {
        return IMS_FALSE;
    }

    return (pData->pValue[0] == ch);
}

PUBLIC
IMS_BOOL AString::Equals(IN CONST IMS_CHAR* pszValue) const
{
    return (Compare(pData->pValue, pszValue) == 0);
}

PUBLIC
IMS_BOOL AString::Equals(IN CONST AString& objStr) const
{
    if (pData == objStr.pData)
    {
        return IMS_TRUE;
    }

    if (pData->nSize != objStr.pData->nSize)
    {
        return IMS_FALSE;
    }

    if ((IsNULL() && !objStr.IsNULL()) || (!IsNULL() && objStr.IsNULL()))
    {
        return IMS_FALSE;
    }

    return (Compare(pData->pValue, objStr.pData->pValue) == 0);
}

PUBLIC
IMS_BOOL AString::EqualsIgnoreCase(IN CONST IMS_CHAR ch) const
{
    if (pData->nSize != 1)
    {
        return IMS_FALSE;
    }

    IMS_CHAR ch1 = IMS_TOLOWER(pData->pValue[0]);
    IMS_CHAR ch2 = IMS_TOLOWER(ch);

    return (ch1 == ch2);
}

PUBLIC
IMS_BOOL AString::EqualsIgnoreCase(IN CONST IMS_CHAR* pszValue) const
{
    return (CompareIgnoreCase(pData->pValue, pszValue) == 0);
}

PUBLIC
IMS_BOOL AString::EqualsIgnoreCase(IN CONST AString& objStr) const
{
    if (pData->nSize != objStr.pData->nSize)
    {
        return IMS_FALSE;
    }

    if ((IsNULL() && !objStr.IsNULL()) || (!IsNULL() && objStr.IsNULL()))
    {
        return IMS_FALSE;
    }

    return (CompareIgnoreCase(pData->pValue, objStr.pData->pValue) == 0);
}

PUBLIC
AString& AString::Append(IN CONST IMS_CHAR ch)
{
    if (pData->nSize + 1 > pData->nAlloc)
    {
        Realloc(astring_AllocateMore(pData->nSize + 1, sizeof(Data)));
    }

    pData->pValue[pData->nSize++] = ch;
    pData->pValue[pData->nSize] = '\0';

    return (*this);
}

PUBLIC
AString& AString::Append(IN CONST IMS_CHAR* pszValue)
{
    if (pszValue != IMS_NULL)
    {
        IMS_SINT32 nLen = IMS_StrLen(pszValue);

        if (pData->nSize + nLen > pData->nAlloc)
        {
            Realloc(astring_AllocateMore(pData->nSize + nLen, sizeof(Data)));
        }

        // include NULL character
        IMS_MEM_Memcpy(pData->pValue + pData->nSize, pszValue, nLen + 1);
        pData->nSize += nLen;
    }

    return (*this);
}

PUBLIC
AString& AString::Append(IN CONST AString& objStr)
{
    if ((pData == &SHARED_NULL) || (pData == &SHARED_EMPTY))
    {
        (*this) = objStr;
    }
    else if (objStr.pData != &SHARED_NULL)
    {
        if (pData->nSize + objStr.pData->nSize > pData->nAlloc)
        {
            Realloc(astring_AllocateMore(pData->nSize + objStr.pData->nSize, sizeof(Data)));
        }

        IMS_MEM_Memcpy(pData->pValue + pData->nSize, objStr.pData->pValue, objStr.pData->nSize);
        pData->nSize += objStr.pData->nSize;
        pData->pValue[pData->nSize] = '\0';
    }

    return (*this);
}

PUBLIC
AString& AString::Erase(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount)
{
    if ((nCount <= 0) || (nOffset >= pData->nSize) || (nOffset < 0))
    {
        return (*this);
    }

    if ((nOffset + nCount) >= pData->nSize)
    {
        Resize(nOffset);
    }
    else
    {
        IMS_MEM_Memmove(pData->pValue + nOffset, pData->pValue + nOffset + nCount,
                pData->nSize - nOffset - nCount);
        Resize(pData->nSize - nCount);
    }

    return (*this);
}

PUBLIC
AString& AString::Fill(IN IMS_CHAR ch, IN IMS_SINT32 nSize /* = -1 */)
{
    Resize((nSize < 0) ? pData->nSize : nSize);

    if (pData->nSize != 0)
    {
        IMS_MEM_Memset(pData->pValue, ch, pData->nSize);
    }

    return (*this);
}

PUBLIC
AString& AString::Insert(IN IMS_SINT32 i, IN CONST IMS_CHAR ch)
{
    return astring_Insert(this, i, &ch, 1);
}

PUBLIC
AString& AString::Insert(IN IMS_SINT32 i, IN CONST IMS_CHAR* pszValue)
{
    return astring_Insert(this, i, pszValue, IMS_StrLen(pszValue));
}

PUBLIC
AString& AString::Insert(IN IMS_SINT32 i, IN CONST AString& objStr)
{
    return astring_Insert(this, i, objStr.pData->pValue, objStr.pData->nSize);
}

PUBLIC
AString& AString::Prepend(IN CONST IMS_CHAR ch)
{
    if (pData->nSize + 1 > pData->nAlloc)
    {
        Realloc(astring_AllocateMore(pData->nSize + 1, sizeof(Data)));
    }

    IMS_MEM_Memmove(pData->pValue + 1, pData->pValue, pData->nSize);
    pData->pValue[0] = ch;
    ++(pData->nSize);
    pData->pValue[pData->nSize] = '\0';

    return (*this);
}

PUBLIC
AString& AString::Prepend(IN CONST IMS_CHAR* pszValue)
{
    if (pszValue != IMS_NULL)
    {
        IMS_SINT32 nLen = IMS_StrLen(pszValue);

        if (pData->nSize + nLen > pData->nAlloc)
        {
            Realloc(astring_AllocateMore(pData->nSize + nLen, sizeof(Data)));
        }

        IMS_MEM_Memmove(pData->pValue + nLen, pData->pValue, pData->nSize);
        IMS_MEM_Memcpy(pData->pValue, pszValue, nLen);
        pData->nSize += nLen;
        pData->pValue[pData->nSize] = '\0';
    }

    return (*this);
}

PUBLIC
AString& AString::Prepend(IN CONST AString& objStr)
{
    if ((pData == &SHARED_NULL) || (pData == &SHARED_EMPTY))
    {
        (*this) = objStr;
    }
    else if (objStr.pData != &SHARED_NULL)
    {
        AString strTmp = (*this);

        (*this) = objStr;
        Append(strTmp);
    }

    return (*this);
}

PUBLIC
AString& AString::Replace(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount, IN CONST IMS_CHAR* pszNew)
{
    Erase(nOffset, nCount);
    return Insert(nOffset, pszNew);
}

PUBLIC
AString& AString::Replace(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount, IN CONST AString& strNew)
{
    Erase(nOffset, nCount);
    return Insert(nOffset, strNew);
}

PUBLIC
AString& AString::Replace(IN CONST IMS_CHAR cOld, IN CONST IMS_CHAR cNew)
{
    if (pData->nSize != 0)
    {
        IMS_CHAR* p = GetStr();
        IMS_CHAR* pEnd = p + pData->nSize;

        while (p != pEnd)
        {
            if ((*p) == cOld)
            {
                (*p) = cNew;
            }

            ++p;
        }
    }

    return (*this);
}

PUBLIC
AString& AString::Replace(IN CONST IMS_CHAR cOld, IN CONST IMS_CHAR* pszNew)
{
    IMS_CHAR acOld[2] = {cOld, '\0'};

    return Replace(FromRawData(acOld, 1), FromRawData(pszNew, IMS_StrLen(pszNew)));
}

PUBLIC
AString& AString::Replace(IN CONST IMS_CHAR cOld, IN CONST AString& strNew)
{
    IMS_CHAR acOld[2] = {cOld, '\0'};

    return Replace(FromRawData(acOld, 1), strNew);
}

PUBLIC
AString& AString::Replace(IN CONST IMS_CHAR* pszOld, IN CONST IMS_CHAR* pszNew)
{
    return Replace(
            FromRawData(pszOld, IMS_StrLen(pszOld)), FromRawData(pszNew, IMS_StrLen(pszNew)));
}

PUBLIC
AString& AString::Replace(IN CONST IMS_CHAR* pszOld, IN CONST AString& strNew)
{
    return Replace(FromRawData(pszOld, IMS_StrLen(pszOld)), strNew);
}

PUBLIC
AString& AString::Replace(IN CONST AString& strOld, IN CONST IMS_CHAR* pszNew)
{
    return Replace(strOld, FromRawData(pszNew, IMS_StrLen(pszNew)));
}

PUBLIC
AString& AString::Replace(IN CONST AString& strOld, IN CONST AString& strNew)
{
    if (IsNULL() || (strOld == strNew))
    {
        return (*this);
    }

    IMS_SINT32 nIndex = 0;
    const IMS_SINT32 nOldSize = strOld.pData->nSize;
    const IMS_SINT32 nNewSize = strNew.pData->nSize;

    IMS_SINT32 nDataSize = pData->nSize;
    IMS_CHAR* p = GetStr();

    if (nOldSize == nNewSize)
    {
        if (nOldSize > 0)
        {
            while ((nIndex = GetIndexOf(strOld, nIndex)) != NPOS)
            {
                IMS_MEM_Memcpy(p + nIndex, strNew.GetStr(), nNewSize);
                nIndex += nNewSize;
            }
        }
    }
    else if (nOldSize > nNewSize)
    {
        IMS_UINT32 nTail = 0;
        IMS_UINT32 nMoveFrom = 0;
        IMS_UINT32 nNumOfReplace = 0;

        while ((nIndex = GetIndexOf(strOld, nIndex)) != NPOS)
        {
            if (nNumOfReplace != 0)
            {
                IMS_SINT32 nSkipSize = nIndex - nMoveFrom;

                if (nSkipSize > 0)
                {
                    IMS_MEM_Memmove(p + nTail, p + nMoveFrom, nSkipSize);
                    nTail += nSkipSize;
                }
            }
            else
            {
                nTail = nIndex;
            }

            if (nNewSize != 0)
            {
                IMS_MEM_Memcpy(p + nTail, strNew.GetStr(), nNewSize);
                nTail += nNewSize;
            }

            nIndex += nOldSize;
            nMoveFrom = nIndex;
            ++nNumOfReplace;
        }

        if (nNumOfReplace != 0)
        {
            IMS_SINT32 nSkipSize = nDataSize - nMoveFrom;

            if (nSkipSize > 0)
            {
                IMS_MEM_Memmove(p + nTail, p + nMoveFrom, nSkipSize);
            }

            Resize(nDataSize - nNumOfReplace * (nOldSize - nNewSize));
        }
    }
    else
    {
        // The size of matched string is less than the size of replace string.
        while (nIndex != NPOS)
        {
            IMS_UINT32 nIndices[1024];
            IMS_UINT32 nNumOfMatch = 0;

            while (nNumOfMatch < 1023)
            {
                nIndex = GetIndexOf(strOld, nIndex);

                if (nIndex == NPOS)
                {
                    break;
                }

                nIndices[nNumOfMatch++] = nIndex;
                nIndex += nOldSize;

                // To avoid infinite loop
                if (nOldSize == 0)
                {
                    nIndex++;
                }
            }

            if (nNumOfMatch == 0)
            {
                break;
            }

            IMS_SINT32 nAdjustSize = nNumOfMatch * (nNewSize - nOldSize);

            if (nIndex != NPOS)
            {
                nIndex += nAdjustSize;
            }

            // Calculate a new size of data
            IMS_SINT32 nNewDataSize = nDataSize + nAdjustSize;
            // Store the end of the original data
            IMS_SINT32 nMoveEnd = nDataSize;

            // Replaced string is greater than the old string;
            // So, allocate more space.
            if (nNewDataSize > nDataSize)
            {
                Resize(nNewDataSize);
                nDataSize = nNewDataSize;
            }

            p = pData->pValue;

            while (nNumOfMatch > 0)
            {
                --nNumOfMatch;

                IMS_SINT32 nMoveStart = nIndices[nNumOfMatch] + nOldSize;
                IMS_SINT32 nInsertStart =
                        nIndices[nNumOfMatch] + nNumOfMatch * (nNewSize - nOldSize);
                IMS_SINT32 nMoveTo = nInsertStart + nNewSize;

                IMS_MEM_Memmove(p + nMoveTo, p + nMoveStart, (nMoveEnd - nMoveStart));

                if (nNewSize != 0)
                {
                    IMS_MEM_Memcpy(p + nInsertStart, strNew.GetStr(), nNewSize);
                }

                nMoveEnd = nMoveStart - nOldSize;
            }
        }
    }

    return (*this);
}

PUBLIC
AString AString::MakeLower() const
{
    AString str(*this);
    IMS_CHAR* p = str.GetStr();

    if (p != IMS_NULL)
    {
        while (*p)
        {
            (*p) = IMS_TOLOWER(*p);
            ++p;
        }
    }

    return str;
}

PUBLIC
AString AString::MakeUpper() const
{
    AString str(*this);
    IMS_CHAR* p = str.GetStr();

    if (p != IMS_NULL)
    {
        while (*p)
        {
            (*p) = IMS_TOUPPER(*p);
            ++p;
        }
    }

    return str;
}

PUBLIC
AString AString::SimplifyWSP() const
{
    if (pData->nSize == 0)
    {
        return (*this);
    }

    AString strResult;

    strResult.Resize(pData->nSize);

    const IMS_CHAR* pStart = pData->pValue;
    const IMS_CHAR* pEnd = pStart + pData->nSize;
    IMS_SINT32 nCount = 0;
    IMS_CHAR* pResult = strResult.pData->pValue;

    for (;;)
    {
        while (pStart != pEnd && IMS_ISSPACE(*pStart))
        {
            pStart++;
        }

        while (pStart != pEnd && !IMS_ISSPACE(*pStart))
        {
            pResult[nCount++] = *pStart++;
        }

        if (pStart != pEnd)
        {
            pResult[nCount++] = ' ';
        }
        else
        {
            break;
        }
    }

    if ((nCount > 0) && (pResult[nCount - 1] == ' '))
    {
        nCount--;
    }

    strResult.Resize(nCount);
    return strResult;
}

PUBLIC
AString AString::Trim() const
{
    if (pData->nSize == 0)
    {
        return (*this);
    }

    const IMS_CHAR* pStart = pData->pValue;

    if (!IMS_ISSPACE(*pStart) && !IMS_ISSPACE(pStart[pData->nSize - 1]))
    {
        return (*this);
    }

    IMS_SINT32 nStart = 0;
    IMS_SINT32 nEnd = pData->nSize - 1;

    while ((nStart <= nEnd) && IMS_ISSPACE(pStart[nStart]))
    {
        nStart++;
    }

    if (nStart <= nEnd)
    {
        while ((nEnd > 0) && IMS_ISSPACE(pStart[nEnd]))
        {
            nEnd--;
        }
    }

    IMS_SINT32 nLen = nEnd - nStart + 1;

    if (nLen <= 0)
    {
        return AString(&SHARED_EMPTY);
    }

    return AString(pStart + nStart, nLen);
}

PUBLIC
AString AString::TrimLeft() const
{
    if (pData->nSize == 0)
    {
        return (*this);
    }

    const IMS_CHAR* pStart = pData->pValue;

    if (!IMS_ISSPACE(*pStart))
    {
        return (*this);
    }

    IMS_SINT32 nStart = 0;
    IMS_SINT32 nEnd = pData->nSize - 1;

    while ((nStart <= nEnd) && IMS_ISSPACE(pStart[nStart]))
    {
        nStart++;
    }

    IMS_SINT32 nLen = nEnd - nStart + 1;

    if (nLen <= 0)
    {
        return AString(&SHARED_EMPTY);
    }

    return AString(pStart + nStart, nLen);
}

PUBLIC
AString AString::TrimRight() const
{
    if (pData->nSize == 0)
    {
        return (*this);
    }

    const IMS_CHAR* pStart = pData->pValue;

    if (!IMS_ISSPACE(pStart[pData->nSize - 1]))
    {
        return (*this);
    }

    IMS_SINT32 nEnd = pData->nSize - 1;

    while ((nEnd >= 0) && IMS_ISSPACE(pStart[nEnd]))
    {
        nEnd--;
    }

    return AString(pStart, nEnd + 1);
}

PUBLIC
AString AString::Left(IN IMS_SINT32 nCount) const
{
    if (nCount >= pData->nSize)
    {
        return (*this);
    }

    if (nCount < 0)
    {
        nCount = 0;
    }

    return AString(pData->pValue, nCount);
}

PUBLIC
AString AString::Mid(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount /* = -1 */) const
{
    if ((pData == &SHARED_NULL) || (pData == &SHARED_EMPTY) || (nOffset >= pData->nSize))
    {
        return AString();
    }

    if (nCount < 0)
    {
        nCount = pData->nSize - nCount;
    }

    if (nOffset < 0)
    {
        nCount += nOffset;
        nOffset = 0;
    }

    if (nCount + nOffset > pData->nSize)
    {
        nCount = pData->nSize - nOffset;
    }

    if ((nOffset == 0) && (nCount == pData->nSize))
    {
        return (*this);
    }

    return AString(pData->pValue + nOffset, nCount);
}

PUBLIC
AString AString::Right(IN IMS_SINT32 nCount) const
{
    if (nCount >= pData->nSize)
    {
        return (*this);
    }

    if (nCount < 0)
    {
        nCount = 0;
    }

    return AString(pData->pValue + pData->nSize - nCount, nCount);
}

PUBLIC
AString AString::AlignLeft(IN IMS_SINT32 nWidth, IN IMS_CHAR cFill /* = ' ' */,
        IN IMS_BOOL bTruncate /* = IMS_FALSE */) const
{
    AString strResult;
    IMS_SINT32 nLen = IMS_StrLen(pData->pValue);
    IMS_SINT32 nPadLen = nWidth - nLen;

    if (nPadLen > 0)
    {
        strResult.Resize(nLen + nPadLen);

        if (nLen != 0)
        {
            IMS_MEM_Memcpy(strResult.pData->pValue, pData->pValue, nLen);
        }

        IMS_MEM_Memset(strResult.pData->pValue + nLen, cFill, nPadLen);
    }
    else
    {
        if (bTruncate)
        {
            strResult = Left(nWidth);
        }
        else
        {
            strResult = (*this);
        }
    }

    return strResult;
}

PUBLIC
AString AString::AlignRight(IN IMS_SINT32 nWidth, IN IMS_CHAR cFill /* = ' ' */,
        IN IMS_BOOL bTruncate /* = IMS_FALSE */) const
{
    AString strResult;
    IMS_SINT32 nLen = IMS_StrLen(pData->pValue);
    IMS_SINT32 nPadLen = nWidth - nLen;

    if (nPadLen > 0)
    {
        strResult.Resize(nLen + nPadLen);

        if (nLen != 0)
        {
            IMS_MEM_Memcpy(strResult.pData->pValue + nPadLen, pData->pValue, nLen);
        }

        IMS_MEM_Memset(strResult.pData->pValue, cFill, nPadLen);
    }
    else
    {
        if (bTruncate)
        {
            strResult = Left(nWidth);
        }
        else
        {
            strResult = (*this);
        }
    }

    return strResult;
}

PUBLIC
AString& AString::SetNumber(IN IMS_SINT16 n, IN IMS_SINT32 nBase /* = 10 */)
{
    return SetNumber(IMS_SINT32(n), nBase);
}

PUBLIC
AString& AString::SetNumber(IN IMS_UINT16 n, IN IMS_SINT32 nBase /* = 10 */)
{
    return SetNumber(IMS_UINT32(n), nBase);
}

PUBLIC
AString& AString::SetNumber(IN IMS_SINT32 n, IN IMS_SINT32 nBase /* = 10 */)
{
    return SetNumber(IMS_SINT64(n), nBase);
}

PUBLIC
AString& AString::SetNumber(IN IMS_UINT32 n, IN IMS_SINT32 nBase /* = 10 */)
{
    return SetNumber(IMS_UINT64(n), nBase);
}

PUBLIC
AString& AString::SetNumber(IN IMS_SINT64 n, IN IMS_SINT32 nBase /* = 10 */)
{
    if ((nBase < 2) || (nBase > 16))
    {
        nBase = 10;
    }

    if (nBase == 10)
    {
        (*this) = astring_LLToString(n, -1, nBase);
    }
    else
    {
        (*this) = astring_ULLToString(n, -1, nBase);
    }

    return (*this);
}

PUBLIC
AString& AString::SetNumber(IN IMS_UINT64 n, IN IMS_SINT32 nBase /* = 10 */)
{
    if ((nBase < 2) || (nBase > 16))
    {
        nBase = 10;
    }

    (*this) = astring_ULLToString(n, -1, nBase);

    return (*this);
}

PUBLIC
IMS_SINT16 AString::ToInt16(
        OUT IMS_BOOL* pbOK /* = IMS_NULL */, IN IMS_SINT32 nBase /* = 10 */) const
{
    IMS_BOOL bOK = IMS_FALSE;
    const IMS_CHAR* pEndPtr;

    if ((nBase < 2) || (nBase > 16))
    {
        nBase = 10;
    }

    IMS_SLONG nL = astring_AToLL(GetStr(), nBase, &bOK, &pEndPtr);

    if (!bOK || ((*pEndPtr) != '\0'))
    {
        if (pbOK)
        {
            *pbOK = IMS_FALSE;
        }
        return 0;
    }

    if (pbOK)
    {
        *pbOK = IMS_TRUE;
    }

    if ((nL < IMS_SHORT_MIN) || (nL > IMS_SHORT_MAX))
    {
        if (pbOK)
        {
            *pbOK = IMS_FALSE;
        }

        nL = 0;
    }

    return IMS_SINT16(nL);
}

PUBLIC
IMS_UINT16 AString::ToUInt16(
        OUT IMS_BOOL* pbOK /* = IMS_NULL */, IN IMS_SINT32 nBase /* = 10 */) const
{
    IMS_BOOL bOK = IMS_FALSE;
    const IMS_CHAR* pEndPtr;

    if ((nBase < 2) || (nBase > 16))
    {
        nBase = 10;
    }

    IMS_SLONG nUL = astring_AToULL(GetStr(), nBase, &bOK, &pEndPtr);

    if (!bOK || ((*pEndPtr) != '\0'))
    {
        if (pbOK)
        {
            *pbOK = IMS_FALSE;
        }
        return 0;
    }

    if (pbOK)
    {
        *pbOK = IMS_TRUE;
    }

    if (nUL > IMS_USHORT_MAX)
    {
        if (pbOK)
        {
            *pbOK = IMS_FALSE;
        }

        nUL = 0;
    }

    return IMS_UINT16(nUL);
}

PUBLIC
IMS_SINT32 AString::ToInt32(
        OUT IMS_BOOL* pbOK /* = IMS_NULL */, IN IMS_SINT32 nBase /* = 10 */) const
{
    IMS_BOOL bOK = IMS_FALSE;
    const IMS_CHAR* pEndPtr;

    if ((nBase < 2) || (nBase > 16))
    {
        nBase = 10;
    }

    IMS_SLONG nL = astring_AToLL(GetStr(), nBase, &bOK, &pEndPtr);

    if (!bOK || ((*pEndPtr) != '\0'))
    {
        if (pbOK)
        {
            *pbOK = IMS_FALSE;
        }
        return 0;
    }

    if (pbOK)
    {
        *pbOK = IMS_TRUE;
    }

    if ((nL < IMS_INT_MIN) || (nL > IMS_INT_MAX))
    {
        if (pbOK)
        {
            *pbOK = IMS_FALSE;
        }

        nL = 0;
    }

    return IMS_SINT32(nL);
}

PUBLIC
IMS_UINT32 AString::ToUInt32(
        OUT IMS_BOOL* pbOK /* = IMS_NULL */, IN IMS_SINT32 nBase /* = 10 */) const
{
    IMS_BOOL bOK = IMS_FALSE;
    const IMS_CHAR* pEndPtr;

    if ((nBase < 2) || (nBase > 16))
    {
        nBase = 10;
    }

    IMS_ULONG nUL = astring_AToULL(GetStr(), nBase, &bOK, &pEndPtr);

    if (!bOK || ((*pEndPtr) != '\0'))
    {
        if (pbOK)
        {
            *pbOK = IMS_FALSE;
        }
        return 0;
    }

    if (pbOK)
    {
        *pbOK = IMS_TRUE;
    }

    if (nUL > IMS_UINT_MAX)
    {
        if (pbOK)
        {
            *pbOK = IMS_FALSE;
        }

        nUL = 0;
    }

    return IMS_UINT32(nUL);
}

PUBLIC
IMS_SINT64 AString::ToInt64(
        OUT IMS_BOOL* pbOK /* = IMS_NULL */, IN IMS_SINT32 nBase /* = 10 */) const
{
    IMS_BOOL bOK = IMS_FALSE;
    const IMS_CHAR* pEndPtr;

    if ((nBase < 2) || (nBase > 16))
    {
        nBase = 10;
    }

    IMS_SLONG nL = astring_AToLL(GetStr(), nBase, &bOK, &pEndPtr);

    if (!bOK || ((*pEndPtr) != '\0'))
    {
        if (pbOK)
        {
            *pbOK = IMS_FALSE;
        }
        return 0;
    }

    if (pbOK)
    {
        *pbOK = IMS_TRUE;
    }

    if ((nL < IMS_LONG_MIN) || (nL > IMS_LONG_MAX))
    {
        if (pbOK)
        {
            *pbOK = IMS_FALSE;
        }

        nL = 0;
    }

    return IMS_SINT64(nL);
}

PUBLIC
IMS_UINT64 AString::ToUInt64(
        OUT IMS_BOOL* pbOK /* = IMS_NULL */, IN IMS_SINT32 nBase /* = 10 */) const
{
    IMS_BOOL bOK = IMS_FALSE;
    const IMS_CHAR* pEndPtr;

    if ((nBase < 2) || (nBase > 16))
    {
        nBase = 10;
    }

    IMS_ULONG nUL = astring_AToULL(GetStr(), nBase, &bOK, &pEndPtr);

    if (!bOK || ((*pEndPtr) != '\0'))
    {
        if (pbOK)
        {
            *pbOK = IMS_FALSE;
        }
        return 0;
    }

    if (pbOK)
    {
        *pbOK = IMS_TRUE;
    }

    if (nUL > IMS_ULONG_MAX)
    {
        if (pbOK)
        {
            *pbOK = IMS_FALSE;
        }

        nUL = 0;
    }

    return IMS_UINT64(nUL);
}

PUBLIC
AString AString::ToBase64() const
{
    const IMS_CHAR acAlphabet[] = "ABCDEFGH"
                                  "IJKLMNOP"
                                  "QRSTUVWX"
                                  "YZabcdef"
                                  "ghijklmn"
                                  "opqrstuv"
                                  "wxyz0123"
                                  "456789+/";
    const IMS_CHAR cPad = '=';
    IMS_SINT32 nPadLen = 0;
    AString strTmp;

    strTmp.Resize(((pData->nSize * 4) / 3) + 3);

    IMS_SINT32 i = 0;
    IMS_CHAR* pOut = strTmp.GetStr();

    while (i < pData->nSize)
    {
        IMS_SINT32 nChunk = 0;

        nChunk |= IMS_SINT32(IMS_UCHAR(pData->pValue[i++])) << 16;

        if (i == pData->nSize)
        {
            nPadLen = 2;
        }
        else
        {
            nChunk |= IMS_SINT32(IMS_UCHAR(pData->pValue[i++])) << 8;

            if (i == pData->nSize)
            {
                nPadLen = 1;
            }
            else
            {
                nChunk |= IMS_SINT32(IMS_UCHAR(pData->pValue[i++]));
            }
        }

        IMS_SINT32 j = (nChunk & 0x00FC0000) >> 18;
        IMS_SINT32 k = (nChunk & 0x0003F000) >> 12;
        IMS_SINT32 l = (nChunk & 0x00000FC0) >> 6;
        IMS_SINT32 m = (nChunk & 0x0000003F);

        *pOut++ = acAlphabet[j];
        *pOut++ = acAlphabet[k];

        if (nPadLen > 1)
        {
            *pOut++ = cPad;
        }
        else
        {
            *pOut++ = acAlphabet[l];
        }

        if (nPadLen > 0)
        {
            *pOut++ = cPad;
        }
        else
        {
            *pOut++ = acAlphabet[m];
        }
    }

    strTmp.Truncate(pOut - strTmp.GetStr());

    return strTmp;
}

PUBLIC
AString AString::ToHexString() const
{
    if (pData == &SHARED_NULL)
    {
        return AString::ConstNull();
    }
    else if (pData == &SHARED_EMPTY)
    {
        return AString::ConstEmpty();
    }
    else
    {
        AString strTemp;
        AString strHexString;

        for (IMS_SINT32 i = 0; i < pData->nSize; ++i)
        {
            strTemp.Sprintf("%02x", pData->pValue[i]);
            strHexString.Append(strTemp);
        }

        return strHexString;
    }
}

PUBLIC
AString AString::ToUTF8() const
{
    if (pData == &SHARED_NULL)
    {
        return AString::ConstNull();
    }
    else if (pData == &SHARED_EMPTY)
    {
        return AString::ConstEmpty();
    }
    else
    {
        IMS_CHAR cOut[3];
        AString strUTF8(pData->nSize * 2);

        for (IMS_SINT32 i = 0; i < pData->nSize; ++i)
        {
            IMS_UCHAR c = static_cast<IMS_UCHAR>(pData->pValue[i]);

            if (c < 0x80)
            {
                cOut[0] = c;
                cOut[1] = '\0';
                cOut[2] = '\0';
            }
            else
            {
                // 110000xx 10xxxxxx
                cOut[0] = (c >> 6) | 0xC0;
                cOut[1] = (c & 0x3F) | 0x80;
                cOut[2] = '\0';
            }

            strUTF8.Append(cOut);
        }

        return strUTF8;
    }
}

PUBLIC
AString& AString::Sprintf(IN CONST IMS_CHAR* pszFormat, ...)
{
    va_list ap;

    va_start(ap, pszFormat);
    AString& str = Vsprintf(pszFormat, ap);
    va_end(ap);

    return str;
}

PUBLIC
AString& AString::Vsprintf(IN CONST IMS_CHAR* pszFormat, va_list ap)
{
    if (pszFormat == IMS_NULL)
    {
        (*this) = (const IMS_CHAR*)IMS_NULL;
        return (*this);
    }

    if (*pszFormat == '\0')
    {
        (*this) = "";
        return (*this);
    }

    // Parse the format
    AString strResult;
    const IMS_CHAR* pc = pszFormat;

    for (;;)
    {
        // Copy non-escape chars to result
        while ((*pc != '\0') && (*pc != '%'))
        {
            strResult.Append(*pc++);
        }

        if (*pc == '\0')
        {
            break;
        }

        // Found '%'
        const IMS_CHAR* pStart_Escape = pc;

        ++pc;
        if (*pc == '\0')
        {
            strResult.Append('%');  // A '%' at the end of the string - treat as non-escape text
            break;
        }

        if (*pc == '%')
        {
            strResult.Append('%');  // %%
            ++pc;
            continue;
        }

        // Parse flag characters
        IMS_UINT32 nFlags = FLAG_NONE;
        IMS_BOOL bNoMoreFlags = IMS_FALSE;

        do
        {
            switch (*pc)
            {
                case '#':
                    nFlags |= FLAG_ALTERNATE;
                    break;
                case '0':
                    nFlags |= FLAG_ZERO_PADDED;
                    break;
                case '-':
                    nFlags |= FLAG_LEFT_ALIGNED;
                    break;
                case ' ':
                    nFlags |= FLAG_BLANK_BEFORE_POSITIVE;
                    break;
                case '+':
                    nFlags |= FLAG_ALWAYS_SHOW_SIGN;
                    break;
                    /*
                    case '\'':
                        nFlags |= F_THOUSANDSGROUP;
                        break;
                    */
                default:
                    bNoMoreFlags = IMS_TRUE;
                    break;
            }

            if (!bNoMoreFlags)
            {
                ++pc;
            }

        } while (!bNoMoreFlags);

        if (*pc == '\0')
        {
            strResult.Append(pStart_Escape);  // Incomplete escape, treat as non-escape text
            break;
        }

        // Parse field width
        IMS_SINT32 nWidth = -1;  // -1 means unspecified

        if (IMS_ISDIGIT(*pc))
        {
            AString strWidth;

            while ((*pc != '\0') && IMS_ISDIGIT(*pc))
            {
                strWidth.Append(*pc++);
            }

            // Can't be negative - started with a digit
            // It contains at least one digit
            nWidth = strWidth.ToInt32();
        }
        else if (*pc == '*')
        {
            nWidth = va_arg(ap, IMS_SINT32);

            if (nWidth < 0)
            {
                nWidth = -1;  // Treat all negative numbers as unspecified
            }

            ++pc;
        }

        if (*pc == '\0')
        {
            strResult.Append(pStart_Escape);  // Incomplete escape, treat as non-escape text
            break;
        }

        // Parse precision
        IMS_SINT32 nPrecision = -1;  // -1 means unspecified

        if (*pc == '.')
        {
            ++pc;
            if (IMS_ISDIGIT(*pc))
            {
                AString strPrecision;

                while ((*pc != '\0') && IMS_ISDIGIT(*pc))
                {
                    strPrecision.Append(*pc++);
                }

                // Can't be negative - started with a digit
                // It contains at least one digit
                nPrecision = strPrecision.ToInt32();
            }
            else if (*pc == '*')
            {
                nPrecision = va_arg(ap, IMS_SINT32);

                if (nPrecision < 0)
                {
                    nPrecision = -1;  // Treat all negative numbers as unspecified
                }

                ++pc;
            }
        }

        if (*pc == '\0')
        {
            strResult.Append(pStart_Escape);  // Incomplete escape, treat as non-escape text
            break;
        }

        // Parse the length modifier
        enum LENGTH_MOD
        {
            LM_NONE,
            LM_hh,
            LM_h,
            LM_l,
            LM_ll,
            LM_L,
            LM_j,
            LM_z,
            LM_t
        };
        LENGTH_MOD enLengthMod = LM_NONE;

        switch (*pc)
        {
            case 'h':
                ++pc;
                if (*pc == 'h')
                {
                    enLengthMod = LM_hh;
                    ++pc;
                }
                else
                {
                    enLengthMod = LM_h;
                }
                break;

            case 'l':
                ++pc;
                if (*pc == 'l')
                {
                    enLengthMod = LM_ll;
                    ++pc;
                }
                else
                {
                    enLengthMod = LM_l;
                }
                break;

            case 'L':
                ++pc;
                enLengthMod = LM_L;
                break;

            case 'j':
                ++pc;
                enLengthMod = LM_j;
                break;

            case 'z':
            case 'Z':
                ++pc;
                enLengthMod = LM_z;
                break;

            case 't':
                ++pc;
                enLengthMod = LM_t;
                break;

            default:
                break;
        }

        if (*pc == '\0')
        {
            strResult.Append(pStart_Escape);  // incomplete escape, treat as non-escape text
            break;
        }

        // Parse the conversion specifier and do the conversion
        AString strSubStr;

        switch (*pc)
        {
            case 'd':
            case 'i':
            {
                IMS_SINT64 i;

                switch (enLengthMod)
                {
                    case LM_NONE:
                    case LM_hh:
                    case LM_h:
                    case LM_t:
                        i = va_arg(ap, IMS_SINT32);
                        break;
                    case LM_l:
                    case LM_j:
                    case LM_z:  // size_t type
                        i = va_arg(ap, IMS_SLONG);
                        break;
                    case LM_ll:
                        i = va_arg(ap, IMS_SINT64);
                        break;
                    default:
                        i = 0;
                        break;
                }

                strSubStr = astring_LLToString(i, nPrecision, 10, nWidth, nFlags);
                ++pc;
                break;
            }
            case 'o':
            case 'u':
            case 'x':
            case 'X':
            {
                IMS_UINT64 u;

                switch (enLengthMod)
                {
                    case LM_NONE:
                    case LM_hh:
                    case LM_h:
                        u = va_arg(ap, IMS_UINT32);
                        break;
                    case LM_l:
                        u = va_arg(ap, IMS_SLONG);
                        break;
                    case LM_ll:
                        u = va_arg(ap, IMS_UINT64);
                        break;
                    default:
                        u = 0;
                        break;
                }

                if (IMS_ISUPPER(*pc))
                {
                    nFlags |= FLAG_CAPITAL_E_X;
                }

                IMS_SINT32 nBase = 10;
                switch (IMS_TOLOWER(*pc))
                {
                    case 'o':
                        nBase = 8;
                        break;
                    case 'u':
                        nBase = 10;
                        break;
                    case 'x':
                        nBase = 16;
                        break;
                }

                strSubStr = astring_ULLToString(u, nPrecision, nBase, nWidth, nFlags);
                ++pc;
                break;
            }
            case 'E':
            case 'e':
            case 'F':
            case 'f':
            case 'G':
            case 'g':
            case 'A':
            case 'a':
            {
                // CURRENTLY, NOT SUPPORTED
                break;
            }
            case 'c':
            {
                strSubStr.Append((IMS_CHAR)va_arg(ap, IMS_SINT32));
                ++pc;
                break;
            }
            case 's':
            {
                strSubStr.Append((IMS_CHAR*)va_arg(ap, const IMS_CHAR*));

                if (nPrecision != -1)
                {
                    strSubStr.Truncate(nPrecision);
                }

                ++pc;
                break;
            }
            case 'p':
            {
                void* pvArg = va_arg(ap, void*);
                IMS_UINT64 i = reinterpret_cast<IMS_UINT64>(pvArg);

                nFlags |= FLAG_ALTERNATE;
                strSubStr = astring_ULLToString(i, nPrecision, 16, nWidth, nFlags);
                ++pc;
                break;
            }
            case 'n':
            {
                switch (enLengthMod)
                {
                    case LM_hh:
                    {
                        IMS_CHAR* n = va_arg(ap, IMS_CHAR*);
                        *n = strResult.GetLength();
                        break;
                    }
                    case LM_h:
                    {
                        IMS_SINT16* n = va_arg(ap, IMS_SINT16*);
                        *n = strResult.GetLength();
                        break;
                    }
                    case LM_l:
                    {
                        IMS_SLONG* n = va_arg(ap, IMS_SLONG*);
                        *n = strResult.GetLength();
                        break;
                    }
                    case LM_ll:
                    {
                        IMS_SINT64* n = va_arg(ap, IMS_SINT64*);
                        volatile IMS_UINT32 nTmp = strResult.GetLength();
                        *n = nTmp;
                        break;
                    }
                    default:
                    {
                        IMS_SINT32* n = va_arg(ap, IMS_SINT32*);
                        *n = strResult.GetLength();
                        break;
                    }
                }

                ++pc;
                break;
            }
            default:  // bad escape, treat as non-escape text
            {
                for (const IMS_CHAR* pcc = pStart_Escape; pcc != pc; ++pcc)
                {
                    strResult.Append(*pcc);
                }
            }
                continue;
        }

        if (nFlags & FLAG_LEFT_ALIGNED)
        {
            strResult.Append(strSubStr.AlignLeft(nWidth));
        }
        else
        {
            strResult.Append(strSubStr.AlignRight(nWidth));
        }
    }

    (*this) = strResult;

    return (*this);
}

PUBLIC
IMS_CHAR* AString::Duplicate() const
{
    if (pData == &SHARED_NULL)
    {
        return IMS_NULL;
    }

    IMS_CHAR* pTmpVal = static_cast<IMS_CHAR*>(IMS_MEM_Malloc(pData->nSize + 1));

    if (pTmpVal == IMS_NULL)
    {
        return IMS_NULL;
    }

    // include NULL character
    IMS_MEM_Memcpy(pTmpVal, pData->pValue, pData->nSize + 1);

    return pTmpVal;
}

PUBLIC
IMSList<AString> AString::Split(IN IMS_CHAR cSep, IN IMS_BOOL bTrim /* = IMS_TRUE */) const
{
    if (pData == &SHARED_NULL)
    {
        return IMSList<AString>();
    }

    if (pData == &SHARED_EMPTY)
    {
        IMSList<AString> objTokens;
        objTokens.Append(ConstEmpty());

        return objTokens;
    }

    IMSList<AString> objTokens;
    IMS_SINT32 nStart = 0;
    IMS_SINT32 nEnd;

    if (bTrim)
    {
        while ((nEnd = GetIndexOf(cSep, nStart)) != NPOS)
        {
            objTokens.Append(Mid(nStart, nEnd - nStart).Trim());
            nStart = nEnd + 1;
        }

        objTokens.Append(Mid(nStart).Trim());
    }
    else
    {
        while ((nEnd = GetIndexOf(cSep, nStart)) != NPOS)
        {
            objTokens.Append(Mid(nStart, nEnd - nStart));
            nStart = nEnd + 1;
        }

        objTokens.Append(Mid(nStart));
    }

    return objTokens;
}

PUBLIC
IMS_SINT32 AString::SplitF(IN IMS_CHAR cSep, OUT AString& strLHS, OUT AString& strRHS,
        IN IMS_BOOL bTrim /* = IMS_TRUE */) const
{
    if (pData == &SHARED_NULL)
    {
        return 0;
    }

    if (pData == &SHARED_EMPTY)
    {
        return 0;
    }

    IMS_SINT32 nIndex = GetIndexOf(cSep);

    if (nIndex == AString::NPOS)
    {
        if (bTrim)
        {
            strLHS = Trim();
        }
        else
        {
            strLHS = (*this);
        }

        return 1;
    }
    else
    {
        if (bTrim)
        {
            strLHS = Mid(0, nIndex).Trim();
            strRHS = Mid(nIndex + 1).Trim();
        }
        else
        {
            strLHS = Mid(0, nIndex);
            strRHS = Mid(nIndex + 1);
        }

        return 2;
    }
}

PUBLIC
IMS_SINT32 AString::SplitB(IN IMS_CHAR cSep, OUT AString& strLHS, OUT AString& strRHS,
        IN IMS_BOOL bTrim /* = IMS_TRUE */) const
{
    if (pData == &SHARED_NULL)
    {
        return 0;
    }

    if (pData == &SHARED_EMPTY)
    {
        return 0;
    }

    IMS_SINT32 nIndex = GetLastIndexOf(cSep);

    if (nIndex == AString::NPOS)
    {
        if (bTrim)
        {
            strLHS = Trim();
        }
        else
        {
            strLHS = (*this);
        }

        return 1;
    }
    else
    {
        if (bTrim)
        {
            strLHS = Mid(0, nIndex).Trim();
            strRHS = Mid(nIndex + 1).Trim();
        }
        else
        {
            strLHS = Mid(0, nIndex);
            strRHS = Mid(nIndex + 1);
        }

        return 2;
    }
}

PUBLIC
IMS_UINT32 AString::GetHashCode() const
{
    if ((pData == &SHARED_NULL) || (pData == &SHARED_EMPTY))
    {
        return 0;
    }

    return GetHashCode(pData->pValue);
}

PUBLIC
RCPtr<WCharPtr> AString::GetWCharPtr() const
{
    return (new WCharPtr(*this));
}

PUBLIC GLOBAL const AString& AString::ConstEmpty()
{
    static const AString CONST_EMPTY("");

    return CONST_EMPTY;
}

PUBLIC GLOBAL const AString& AString::ConstNull()
{
    static const AString CONST_NULL;

    return CONST_NULL;
}

PUBLIC GLOBAL IMS_SINT32 AString::Compare(IN CONST IMS_CHAR* pszStr1, IN CONST IMS_CHAR* pszStr2)
{
    return (pszStr1 && pszStr2) ? IMS_StrCmp(pszStr1, pszStr2) : (pszStr1 ? 1 : (pszStr2 ? -1 : 0));
}

PUBLIC GLOBAL IMS_SINT32 AString::CompareIgnoreCase(
        IN CONST IMS_CHAR* pszStr1, IN CONST IMS_CHAR* pszStr2)
{
    return (pszStr1 && pszStr2) ? IMS_StrICmp(pszStr1, pszStr2)
                                : (pszStr1 ? 1 : (pszStr2 ? -1 : 0));
}

PUBLIC GLOBAL AString AString::FromBase64(IN CONST AString& strBase64)
{
    IMS_UINT32 nBuf = 0;
    IMS_SINT32 nBits = 0;
    AString strTmp;

    strTmp.Resize((strBase64.GetLength() * 3) / 4);

    IMS_SINT32 nOffset = 0;

    for (IMS_SINT32 i = 0; i < strBase64.GetLength(); ++i)
    {
        IMS_SINT32 ch = strBase64[i];
        IMS_SINT32 nCode;

        if ((ch >= 'A') && (ch <= 'Z'))
        {
            nCode = ch - 'A';
        }
        else if ((ch >= 'a') && (ch <= 'z'))
        {
            nCode = ch - 'a' + 26;
        }
        else if ((ch >= '0') && (ch <= '9'))
        {
            nCode = ch - '0' + 52;
        }
        else if (ch == '+')
        {
            nCode = 62;
        }
        else if (ch == '/')
        {
            nCode = 63;
        }
        else
        {
            nCode = -1;
        }

        if (nCode != -1)
        {
            nBuf = (nBuf << 6) | nCode;
            nBits += 6;

            if (nBits >= 8)
            {
                nBits -= 8;
                strTmp[nOffset++] = nBuf >> nBits;
                nBuf &= (1 << nBits) - 1;
            }
        }
    }

    strTmp.Truncate(nOffset);

    return strTmp;
}

PUBLIC GLOBAL AString AString::FromRawData(IN CONST IMS_CHAR* pValue, IN IMS_SINT32 nSize)
{
    Data* pNewData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data)));

    if (pValue != IMS_NULL)
    {
        pNewData->pValue = const_cast<IMS_CHAR*>(pValue);
    }
    else
    {
        pNewData->pValue = pNewData->acValue;
        nSize = 0;
    }

    pNewData->nAlloc = pNewData->nSize = nSize;
    pNewData->acValue[0] = '\0';

    return AString(pNewData);
}

PUBLIC GLOBAL IMS_UINT32 AString::GetHashCode(IN CONST IMS_CHAR* pszValue)
{
    if (pszValue == IMS_NULL)
    {
        return 0;
    }

    IMS_UINT32 nHValue = 0;
    IMS_UINT32 nHTmp = 0;

    while (*pszValue)
    {
        nHValue = (nHValue << 4) + *pszValue++;

        if ((nHTmp = nHValue & 0xF0000000L) != 0)
        {
            nHValue ^= (nHTmp >> 24);
        }

        nHValue &= ~nHTmp;
    }

    return nHValue;
}

PRIVATE
void AString::Clear()
{
    if (pData == &SHARED_NULL)
    {
        return;
    }

    if (pData == &SHARED_EMPTY)
    {
        return;
    }

    IMS_MEM_Free(pData);
    pData = &SHARED_NULL;
}

PRIVATE
void AString::Realloc(IN IMS_SINT32 nAlloc)
{
    Data* pNewData;

    if ((pData == &SHARED_NULL) || (pData == &SHARED_EMPTY) || (pData->pValue != pData->acValue))
    {
        pNewData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data) + nAlloc));

        if (pNewData == IMS_NULL)
        {
            return;
        }

        pNewData->nSize = IMS_MIN(nAlloc, pData->nSize);
        IMS_MEM_Memcpy(pNewData->acValue, pData->pValue, pNewData->nSize);
        pNewData->acValue[pNewData->nSize] = '\0';
        pNewData->nAlloc = nAlloc;
        pNewData->pValue = pNewData->acValue;

        // Release the previous resource
        Clear();
    }
    else
    {
        pNewData = static_cast<Data*>(IMS_MEM_Realloc(pData, sizeof(Data) + nAlloc));

        if (pNewData == IMS_NULL)
        {
            return;
        }

        pNewData->nAlloc = nAlloc;
        pNewData->pValue = pNewData->acValue;
    }

    pData = pNewData;
}

PUBLIC
WCharPtr::WCharPtr(IN CONST AString& str) :
        RCObject(),
        nSize(0),
        pwszData(IMS_NULL)
{
    nSize = astring_CharToWideChar(str.GetStr(), str.GetLength(), pwszData, 0);

    pwszData = static_cast<IMS_WCHAR*>(IMS_MEM_Malloc((nSize + 1) * sizeof(IMS_WCHAR)));

    astring_CharToWideChar(str.GetStr(), str.GetLength(), pwszData, nSize + 1);
}

PUBLIC
WCharPtr::WCharPtr(IN CONST WCharPtr& objRHS) :
        RCObject(objRHS),
        nSize(objRHS.nSize),
        pwszData(IMS_NULL)
{
    pwszData = static_cast<IMS_WCHAR*>(IMS_MEM_Malloc((nSize + 1) * sizeof(IMS_WCHAR)));
    IMS_UcStrCpy(pwszData, nSize, objRHS.pwszData);
}

PUBLIC VIRTUAL WCharPtr::~WCharPtr()
{
    IMS_MEM_Free(pwszData);
}

PUBLIC
WCharPtr& WCharPtr::operator=(IN CONST WCharPtr& objRHS)
{
    if (this != &objRHS)
    {
        RCObject::operator=(objRHS);
        nSize = objRHS.nSize;

        if (pwszData != IMS_NULL)
        {
            IMS_MEM_Free(pwszData);
        }

        pwszData = static_cast<IMS_WCHAR*>(IMS_MEM_Malloc((nSize + 1) * sizeof(IMS_WCHAR)));
        IMS_UcStrCpy(pwszData, nSize, objRHS.pwszData);
    }

    return (*this);
}
