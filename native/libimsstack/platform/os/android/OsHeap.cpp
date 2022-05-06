/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef __IMS_DEBUG_MEM__
#define LOG_TAG "ImsStackN"
#define LOG_NDDEBUG 0
#define MALLOC_DEBUG 1
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef __IMS_DEBUG_MEM__

#include <cutils/log.h>
#include <private/android_filesystem_config.h>

extern "C" void get_malloc_leak_info(uint8_t** info, size_t* overallSize, size_t* infoSize,
        size_t* totalMemory, size_t* backtraceSize);
extern "C" void free_malloc_leak_info(uint8_t* info);

typedef struct
{
    size_t nSize;
    size_t nCount;
    intptr_t* pBacktrace;
} AllocEntry;

size_t gnBacktraceSize = 0;

extern "C" int CBHeapInfoCmp(const void* a, const void* b)
{
    AllocEntry* pEntry1 = (AllocEntry*)a;
    AllocEntry* pEntry2 = (AllocEntry*)b;

    if (pEntry1->nSize < pEntry2->nSize)
    {
        return 1;
    }
    else if (pEntry1->nSize > pEntry2->nSize)
    {
        return -1;
    }
    else  // if (pEntry1->nSize == pEntry2->nSize)
    {
        for (size_t j = 0; j < gnBacktraceSize; ++j)
        {
            if (pEntry1->pBacktrace[j] == pEntry2->pBacktrace[j])
            {
                continue;
            }

            if (pEntry1->pBacktrace[j] < pEntry2->pBacktrace[j])
            {
                return 1;
            }
        }
    }

    return 0;
}
#endif  // __IMS_DEBUG_MEM__

#include "OsHeap.h"

PRIVATE
OsHeap::OsHeap() :
        m_bDebugEnabled(IMS_FALSE)
{
}

PRIVATE VIRTUAL OsHeap::~OsHeap() {}

PUBLIC VIRTUAL void* OsHeap::Alloc(IN IMS_SIZE_T nSize)
{
    return malloc(nSize);
}

PUBLIC VIRTUAL void* OsHeap::Realloc(IN void* pMem, IN IMS_SIZE_T nSize)
{
    return realloc(pMem, nSize);
}

PUBLIC VIRTUAL void OsHeap::Free(IN void* pMem)
{
    if (pMem == IMS_NULL)
    {
        return;
    }

    free(pMem);
    pMem = IMS_NULL;
}

PUBLIC VIRTUAL void* OsHeap::AllocDebug(
        IN IMS_SIZE_T nSize, IN IMS_UINT16 nLine, IN const IMS_CHAR* pszFile)
{
    (void)nLine;
    (void)pszFile;

    return Alloc(nSize);
}

PUBLIC VIRTUAL void* OsHeap::ReallocDebug(
        IN void* pMem, IN IMS_SIZE_T nSize, IN IMS_UINT16 nLine, IN const IMS_CHAR* pszFile)
{
    (void)nLine;
    (void)pszFile;

    return Realloc(pMem, nSize);
}

PUBLIC VIRTUAL void OsHeap::FreeDebug(
        IN void* pMem, IN IMS_UINT16 nLine, IN const IMS_CHAR* pszFile)
{
    (void)nLine;
    (void)pszFile;

    Free(pMem);
}

PUBLIC VIRTUAL void OsHeap::EnableHeapDebug(IN IMS_BOOL bEnable)
{
    m_bDebugEnabled = bEnable;
}

PUBLIC VIRTUAL void OsHeap::PrintHeapLeakage()
{
#ifdef __IMS_DEBUG_MEM__
#define MAX_BUFF_SIZE (1024)
#define TOTAL_MAX_BUFF_SIZE (512 * 1024)

    IMS_CHAR acBuffer[MAX_BUFF_SIZE];
    IMS_CHAR acTotalBuffer[TOTAL_MAX_BUFF_SIZE] = {
            0,
    };

    uint8_t* pInfo = NULL;
    size_t nOverallSize = 0;
    size_t nInfoSize = 0;
    size_t nTotalMemory = 0;

    gnBacktraceSize = 0;

    get_malloc_leak_info(&pInfo, &nOverallSize, &nInfoSize, &nTotalMemory, &gnBacktraceSize);

    size_t nAllocCount = nOverallSize / nInfoSize;

    LOGD("Allocation Count (%u), Total Memory (%u)", nAllocCount, nTotalMemory);

    if (!bDebugEnabled)
    {
        return;
    }

    if (pInfo != NULL)
    {
        uint8_t* pTmpInfo = pInfo;

        LOGD(">>>>> MEM STAT -- STARTS <<<<<");

        // snprintf(buffer, SIZE, " Allocation count %i\n", count);
        // strcat(result, buffer);
        // snprintf(buffer, SIZE, " Total memory %i\n", totalMemory);
        // strcat(result, buffer);

        AllocEntry* pEntries = new AllocEntry[nAllocCount];

        for (size_t i = 0; i < nAllocCount; ++i)
        {
            // Each entry should be size_t, size_t, intptr_t[backtraceSize]
            AllocEntry* pEntry = &pEntries[i];

            pEntry->nSize = *reinterpret_cast<size_t*>(pTmpInfo);
            pTmpInfo += sizeof(size_t);

            pEntry->nCount = *reinterpret_cast<size_t*>(pTmpInfo);
            pTmpInfo += sizeof(size_t);

            pEntry->pBacktrace = reinterpret_cast<intptr_t*>(pTmpInfo);
            pTmpInfo += sizeof(intptr_t) * gnBacktraceSize;
        }

        // Now we need to sort the entries.  They come sorted by size but
        // not by stack trace which causes problems using diff.
#if 1
        bool bMoved;
        do
        {
            bMoved = false;
            for (size_t i = 0; i < (nAllocCount - 1); ++i)
            {
                AllocEntry* pEntry1 = &pEntries[i];
                AllocEntry* pEntry2 = &pEntries[i + 1];

                bool bSwap = pEntry1->nSize < pEntry2->nSize;

                if (pEntry1->nSize == pEntry2->nSize)
                {
                    for (size_t j = 0; j < gnBacktraceSize; ++j)
                    {
                        if (pEntry1->pBacktrace[j] == pEntry2->pBacktrace[j])
                        {
                            continue;
                        }
                        bSwap = pEntry1->pBacktrace[j] < pEntry2->pBacktrace[j];
                        break;
                    }
                }

                if (bSwap)
                {
                    AllocEntry objTmp = pEntries[i];
                    pEntries[i] = pEntries[i + 1];
                    pEntries[i + 1] = objTmp;
                    bMoved = true;
                }
            }
        } while (bMoved);
#else
        qsort(pEntries, nAllocCount, sizeof(AllocEntry), CBHeapInfoCmp);
#endif

        char acFile[MAX_BUFF_SIZE] = {
                0,
        };
        snprintf(acFile, MAX_BUFF_SIZE, IMS_SOLUTION_STORAGE_ROOT_DIR "/memleak_%d_%d", nAllocCount,
                nTotalMemory);
        FILE* pFile = fopen(acFile, "w+");

        if (pFile == IMS_NULL)
        {
            LOGE("MEM dump file open error %s", acFile);
            return;
        }

        for (size_t i = 0; i < nAllocCount; ++i)
        {
            // acTotalBuffer[0] = '\0';
            AllocEntry* pEntry = &pEntries[i];

            snprintf(acBuffer, MAX_BUFF_SIZE, "size %8lld, allocations %4i, ", pEntry->nSize,
                    pEntry->nCount);
            strcpy(acTotalBuffer, acBuffer);
            for (size_t j = 0; (j < gnBacktraceSize) && pEntry->pBacktrace[j]; ++j)
            {
                if (j != 0)
                {
                    strcat(acTotalBuffer, ", ");
                }

                snprintf(acBuffer, MAX_BUFF_SIZE, "0x%08x", pEntry->pBacktrace[j]);
                strcat(acTotalBuffer, acBuffer);
            }

            // LOGD("%s", acTotalBuffer);
            strcat(acTotalBuffer, "\n");
            fwrite(acTotalBuffer, 1, strlen(acTotalBuffer), pFile);
        }

        fclose(pFile);

        delete[] pEntries;
        free_malloc_leak_info(pInfo);

        LOGD(">>>>> MEM STAT -- ENDS <<<<<");
    }
#endif  // __IMS_DEBUG_MEM__
}

PUBLIC VIRTUAL void OsHeap::GetHeapUsage(OUT IMS_SIZE_T& nTotalBlock, OUT IMS_SIZE_T& nTotalBytes,
        OUT IMS_SIZE_T& nUsedBytes, OUT IMS_SIZE_T& nMaxUsedBytes, OUT IMS_SIZE_T& nMaxRequested)
{
    nTotalBlock = 0;
    nTotalBytes = 0;
    nUsedBytes = 0;
    nMaxUsedBytes = 0;
    nMaxRequested = 0;
}

PUBLIC GLOBAL OsHeap* OsHeap::GetHeap()
{
    static OsHeap s_objHeap;

    return &s_objHeap;
}

PUBLIC GLOBAL void OsHeap::Initialize() {}
