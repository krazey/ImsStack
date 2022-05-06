/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100128  hwangoo.park@             Created
    </table>

    Description

*/

#include <string.h>

#include "IMSNew.h"
#include "ServiceMemory.h"

#undef new
#undef delete

#ifdef IMS_DEBUG_MEM

GLOBAL
void* operator new(IN IMS_SIZE_T nSize, IN CONST IMS_CHAR* pszFile, IN IMS_UINT16 nLine)
{
    return MemService::GetMemService()->GetMemHeap()->AllocDebug(nSize, nLine, pszFile);
}

GLOBAL
void* operator new[](IN IMS_SIZE_T nSize, IN CONST IMS_CHAR* pszFile, IN IMS_UINT16 nLine)
{
    return operator new(nSize, nLine, pszFile);
}

GLOBAL
void operator delete(
        IN void* pMem, IN CONST IMS_CHAR* pszFile, IN IMS_UINT16 nLine) IMS_NO_EXCEPTION
{
    if (pMem == IMS_NULL)
    {
        return;
    }

    MemService::GetMemService()->GetMemHeap()->FreeDebug(pMem, nLine, pszFile);
}

GLOBAL
void operator delete[](
        IN void* pMem, IN CONST IMS_CHAR* pszFile, IN IMS_UINT16 nLine) IMS_NO_EXCEPTION
{
    operator delete(pMem, nLine, pszFile);
}

GLOBAL
void operator delete(IN void* pMem) IMS_NO_EXCEPTION
{
    if (pMem == IMS_NULL)
    {
        return;
    }

    MemService::GetMemService()->GetMemHeap()->FreeDebug(pMem, __IMS_LINE__, __IMS_FILE__);
}

GLOBAL
void operator delete[](IN void* pMem) IMS_NO_EXCEPTION
{
    operator delete(pMem);
}

#else   // IMS_DEBUG_MEM

GLOBAL
void* operator new(IN IMS_SIZE_T nSize)
{
    return MemService::GetMemService()->GetMemHeap()->Alloc(nSize);
}

GLOBAL
void* operator new[](IN IMS_SIZE_T nSize)
{
    return operator new(nSize);
}

GLOBAL
void operator delete(IN void* pMem) IMS_NO_EXCEPTION
{
    if (pMem == IMS_NULL)
    {
        return;
    }

    MemService::GetMemService()->GetMemHeap()->Free(pMem);
}

GLOBAL
void operator delete[](IN void* pMem) IMS_NO_EXCEPTION
{
    operator delete(pMem);
}
#endif  // IMS_DEBUG_MEM

GLOBAL void* IMS_MEM_Memset(IN_OUT void* pvMem, IN IMS_SINT32 nChar, IN IMS_SIZE_T nCount)
{
    return memset(pvMem, nChar, nCount);
}

GLOBAL void* IMS_MEM_Memcpy(IN_OUT void* pvDest, IN CONST void* pvSrc, IN IMS_SIZE_T nCount)
{
    return memcpy(pvDest, pvSrc, nCount);
}

GLOBAL void* IMS_MEM_Memmove(IN_OUT void* pvDest, IN CONST void* pvSrc, IN IMS_SIZE_T nCount)
{
    return memmove(pvDest, pvSrc, nCount);
}

GLOBAL IMS_SINT32 IMS_MEM_Memcmp(IN CONST void* pvMem1, IN CONST void* pvMem2, IN IMS_SIZE_T nCount)
{
    return memcmp(pvMem1, pvMem2, nCount);
}
