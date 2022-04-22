/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090831  yhrhee@                   Created
    </table>

    Description

*/

#ifndef _INTERFACE_IMS_MEM_HEAP_H_
#define _INTERFACE_IMS_MEM_HEAP_H_

#include "ImsTypeDef.h"

class IMemHeap
{
public:
    virtual void* Alloc(IN IMS_SIZE_T nSize) = 0;
    virtual void* Realloc(IN void *pMem, IN IMS_SIZE_T nSize) = 0;
    virtual void Free(IN void *pMem) = 0;

    virtual void* AllocDebug(IN IMS_SIZE_T nSize,
            IN IMS_UINT16 nLine, IN CONST IMS_CHAR *pszFile) = 0;
    virtual void* ReallocDebug(IN void *pMem, IN IMS_SIZE_T nSize,
            IN IMS_UINT16 nLine, IN CONST IMS_CHAR *pszFile) = 0;
    virtual void FreeDebug(IN void *pMem,
            IN IMS_UINT16 nLine, IN CONST IMS_CHAR *pszFile) = 0;

    virtual void EnableHeapDebug(IN IMS_BOOL bEnable) = 0;
    virtual void PrintHeapLeakage() = 0;
    virtual void GetHeapUsage(OUT IMS_SIZE_T &nTotalBlock, OUT IMS_SIZE_T &nTotalBytes,
            OUT IMS_SIZE_T &nUsedBytes, OUT IMS_SIZE_T &nMaxUsedBytes,
            OUT IMS_SIZE_T &nMaxRequested) = 0;
};

#endif //_INTERFACE_IMS_MEM_HEAP_H_
