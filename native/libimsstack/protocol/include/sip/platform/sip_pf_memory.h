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
#ifndef __SIP_PF_MEMORY_H__
#define __SIP_PF_MEMORY_H__

#include "sip_pf_datatypes.h"

/******************************************************************************
 * Function name   : SipPf_Memset
 * Description     : This function is use to set memory with new
 * Return type     : SIP_VOID
 * Arguments       : [IN] pvMem_block:Pointer to the memory block to be set
 *                    [IN] cChars:Characters to be set with
 *                    [IN] sztSize:Size of the character
 * Side Effect     : None
 * NOTE            : None
 *****************************************************************************/
SIP_VOID SipPf_Memset(SIP_VOID* pvMem_block, SIP_UCHAR cChars, SIP_SIZE_T nSize);

/******************************************************************************
 * Function name   : SipPf_Memcpy
 * Description     : This function is used to copy block of memory from one
 *                    to another
 * Return type     : SIP_VOID
 * Arguments       : [IN] pvMem_Destination:Pointer to destination memory block
 *                    [IN] pvMem_Source:Pointer to the source memory block
 *                    [IN] sztSize:Size of the memory block to be copied
 * Side Effect     : None
 * NOTE            : None
 *****************************************************************************/
SIP_VOID SipPf_Memcpy(SIP_VOID* pvMem_Dest, const SIP_VOID* pvMem_Source, SIP_SIZE_T nSize);

/******************************************************************************
 * Function name   : SipPf_Memcmp
 * Description     : This function is used to compare block of memory.
 * Return type     : SIP_VOID
 * Arguments       : [IN] pvMem_Destination:Pointer to destination memory block
 *                    [IN] pvMem_Source:Pointer to the source memory block
 *                    [IN] sztSize:Size of the memory block to be copied
 * Side Effect     : None
 * NOTE            : None
 *****************************************************************************/
SIP_UINT32 SipPf_Memcmp(const SIP_VOID* pvMem1, const SIP_VOID* pvMem2, SIP_SIZE_T nSize);

/******************************************************************************
 * Function name   : SipPf_Free
 * Description     : This function is used to free memory block
 * Return type     : SIP_VOID
 * Arguments       : [IN] pvMem_ptr:Pointer to the memory block to be freed
 * Side Effect     : None
 * NOTE            : None
 *****************************************************************************/
SIP_VOID SipPf_Free(SIP_VOID** ppvMem);

/******************************************************************************
 * Function name   : SipPf_Realloc
 * Description     : This function is used to reallocate memory block
 * Return type     : SIP_VOID
 * Arguments       : [IN] pvMem_ptr:Pointer to memory block to be reallocated
 *                    [IN] usNew_Size :New size of memory block
 * Side Effect     : None
 * NOTE            : None
 *****************************************************************************/
SIP_VOID* SipPf_Realloc(SIP_VOID* pvMem_ptr, SIP_SIZE_T nNew_Size);

/****************************************************************************
 * Function name    : SipPf_CleanMalloc
 * Description        :
 * Return type        : <Name>
 *                      <Description>
 * Argument           : <Name>: <In/Out/InOut>
 *                      <Description>
 * Argument           : <Name>: <In/Out/InOut>
 *                      <Description>
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ****************************************************************************/
SIP_VOID* SipPf_CleanMalloc(SIP_SIZE_T nMemBlockSize);

#endif /*__SIP_PF_MEMORY_H__*/
