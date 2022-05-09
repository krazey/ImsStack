#include "sip_pf_datatypes.h"
#include "platform/sip_pf_memory.h"
#include <malloc.h>
#include <memory.h>
#include <string.h>

/******************************************************************************
 * Function name   : SipPf_Memset
 * Description     : This function is use to set memory with new
 * Return type     : SIP_VOID
 * Arguments       : [IN] pvMem_block:Pointer to the memory block to be set
 *                    [IN] uChars:Characters to be set with
 *                    [IN] sztSize:Size of the character
 * Side Effect     : None
 * NOTE            : None
 *****************************************************************************/
SIP_VOID SipPf_Memset(SIP_VOID* pvMem_block, SIP_UCHAR uChars, SIP_SIZE_T nSize)
{
    memset(pvMem_block, uChars, nSize);
}

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
SIP_VOID SipPf_Memcpy(SIP_VOID* pvMem_Dest, const SIP_VOID* pvMem_Source, SIP_SIZE_T nSize)
{
    memcpy(pvMem_Dest, pvMem_Source, nSize);
}

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
SIP_UINT32 SipPf_Memcmp(const SIP_VOID* pvMem1, const SIP_VOID* pvMem2, SIP_SIZE_T nSize)
{
    return (memcmp(pvMem1, pvMem2, nSize));
}

/******************************************************************************
 * Function name   : SipPf_Free
 * Description     : This function is used to free memory block
 * Return type     : SIP_VOID
 * Arguments       : [IN] pvMem_ptr:Pointer to the memory block to be freed
 * Side Effect     : None
 * NOTE            : None
 *****************************************************************************/
SIP_VOID SipPf_Free(SIP_VOID** ppvMem)
{
    // free(*ppvMem);
    if (*ppvMem == SIP_NULL)
    {
        return;
    }
    delete ((char*)(*ppvMem));
    *ppvMem = SIP_NULL;
}

/******************************************************************************
 * Function name   : SipPf_Realloc
 * Description     : This function is used to reallocate memory block
 * Return type     : SIP_VOID
 * Arguments       : [IN] pvMem_ptr:Pointer to memory block to be reallocated
 *                    [IN] usNew_Size :New size of memory block
 * Side Effect     : None
 * NOTE            : None
 *****************************************************************************/
SIP_VOID* SipPf_Realloc(SIP_VOID* pvMem_ptr, SIP_SIZE_T usNew_Size)
{
    return realloc(pvMem_ptr, usNew_Size);
}
