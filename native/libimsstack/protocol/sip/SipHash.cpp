/******************************************************************************
 * Project Name   : SIP_RTP
 * Group    : IP-CS [MSG-2]
 * Security   : Confidential
 *****************************************************************************/

/******************************************************************************
 * Filename    : SipHash.cpp
 * Purpose     : Hash Utility Functions
 * Platform    : Windows OR Android
 * Author(s)   : Syed Malgimani
 * E-mail id.    : syed.malgimani@
 * Creation date   : May. 12,2010
 *
 * Edit History     Modification           Description(s)
 *
 * Date      Name    Version    Bug-ID    Description
 * ----------    ----------    -------    ------    -------------
 * Month. Date,10    Name       0.0a    ---    Initial creation
 * July,21,2010    Giridhar    0.0b    ---    Coverted to cpp
 *****************************************************************************/

/*****************************************************************************
  Header Inclusions
 *****************************************************************************/
#include "sip_pf_datatypes.h"
#include "platform/sip_pf_memory.h"
#include "SipHash.h"
#include "sip_error.h"
#include "SipTrace.h"
#include "sip_debug.h"

/* If the thread safety is required */
#ifdef SIP_THREAD_SAFE
#include "platform/SipMutex.h"
#endif


/****************************************************************************
  Function Definition
 *****************************************************************************/
/******************************************************************************
 ** FUNCTION:   SipHash
 **
 ** DESCRIPTION:   This is the function to initialize a new
 **      hash table.
 **
 ** PARAMETERS:
 **  fpCalculateHash   (IN)    : Function to be used to hash the key.
 **  fpCompareHash   (IN)    : Function to compare the hash keys of entries
 **            at time of doing a fetch. If the comparison
 **            function :
 **            returns 0 - the keys that were compared match
 **            returns 1 - the keys don't match
 **  fpElemFreeFunc (IN)    : Function to invoke to free the
 **            element data when the hash entry
 **            has be deleted.
 **  fpKeyFreeHashKey  (IN)    : Function to invoke to free the
 **            element key when the hash entry
 **            has be deleted.
 **  uiNumBuckets  (IN)    : number of chains in the hash table.
 **  uiMaxElements  (IN)    : maximum number of elements to be allowed
 **            in the hash table.
 **   pErr    (IN/OUT)  : Error variable returned in case of failure.
 **
 ******************************************************************************/
SipHash::SipHash(Hash_fnCalculateHash fpCalculateHash, Hash_fnCompareHashKey fpCompareHash,
        Hash_fnFreeHashElement fpElemFreeFunc, Hash_fnFreeHashKey fpKeyFreeHashKey,
        SIP_UINT32 nNumBuckets, SIP_UINT32 nMaxElements, SIP_UINT16* pnError)
{
    SIP_TRACE_NORMAL(ESIPTRACE_MODHASH,
            "SipHash: Numberbuckets %d, MaxElements %d \n",
            nNumBuckets, nMaxElements);

    /* Initialize the max number of buckets.*/
    nSearchBuckets = nNumBuckets;
    nNumOfSearchElements = SIP_ZERO;

    /* Initialize the max number of elements supported in each buckets.*/
    nMaxNumberOfElements = nMaxElements;

    /* Initialize functions */
    this->fpCalculateHash = fpCalculateHash;
    this->fpCompareHash = fpCompareHash;
    this->fpFreeHashElement = fpElemFreeFunc;
    this->fpKeyFreeHashKey = fpKeyFreeHashKey;

    /* Initialize the hash elements */
    ppstHashSearchChains = (Hash_StSearchElement **) new Hash_StSearchElement*[nNumBuckets];

    if (ppstHashSearchChains == SIP_NULL)
    {
        *pnError = EERR_MALLOCFAILED;
        SIP_DEBUG_EXTRLBUG(ESIPTRACE_MODHASH,
                "SipHash: Malloc Failed Seq:%d\n",SIP_ONE, SIP_ZERO);
        bInitialized = SIP_FALSE;
        return;
    }

    for (SIP_UINT32 nIndex = SIP_ZERO; nIndex < nNumBuckets; nIndex++)
    {
        ppstHashSearchChains[nIndex] = SIP_NULL;
    }
    bInitialized = SIP_TRUE;
}


/******************************************************************************
 ** FUNCTION:   ~SipHash
 **
 ** DESCRIPTION:   This is the function to free members from the hash table.
 **        It does not free the hash elements, but frees other  member
 **        variables malloced at the time of Init of the hash table
 **
 ** PARAMETERS:
 **  pErr  (OUT)  : Error variable returned in case of failure.
 **
 ******************************************************************************/
SipHash::~SipHash()
{
    for (SIP_UINT32 nIndex = SIP_ZERO; nIndex < nSearchBuckets ; nIndex++)
    {
        /* Remove the entries from the search list.*/
        if (nNumOfSearchElements > SIP_ZERO)
        {
            Hash_StSearchElement* pstElement = ppstHashSearchChains[nIndex];
            Hash_StSearchElement* pstTempElement = SIP_NULL;

            while (pstElement != SIP_NULL)
            {
                if (pstElement->pvSearchKey != SIP_NULL)
                {
                    pstTempElement = pstElement;
                    pstElement = pstElement->pstNextElement;

                    /* Free the members */
                    if (pstTempElement->pvSearchKey != SIP_NULL)
                    {
                        fpKeyFreeHashKey(pstTempElement->pvSearchKey);
                    }
                    if (pstTempElement->pvElement != SIP_NULL)
                    {
                        fpFreeHashElement(pstTempElement->pvElement);
                    }

                    pstTempElement->pvElement = SIP_NULL;
                    delete pstTempElement;
                }
            }
        }
        else
        {
            if (ppstHashSearchChains[nIndex] != SIP_NULL)
            {
                delete ppstHashSearchChains[nIndex];
                ppstHashSearchChains[nIndex] = SIP_NULL;
            }
        }
    }
    /* Delete search and close nodes.*/
    delete[] ppstHashSearchChains;
}


/******************************************************************************
 ** FUNCTION:   Hash_Add
 **
 ** DESCRIPTION:   This is the function to add an entry into the hash table.
 **
 ** PARAMETERS:
 **  pElement (IN)  : Hash Element that needs to be added
 **
 **  pErr  (OUT)  : Error variable returned in case
 **        of failure.
 **
 ******************************************************************************/
SIP_BOOL SipHash::Hash_Add(SIP_VOID* pvSearchElement, SIP_VOID* pvSearchKey, SIP_UINT16* pnError)
{
    if ((pvSearchKey == SIP_NULL)
            || (fpCalculateHash == SIP_NULL)
            || (fpCompareHash == SIP_NULL)
            || (bInitialized == SIP_FALSE))
    {
        *pnError = EERR_INVALIDPARAM;
        SIP_DEBUG_WARNING(ESIPTRACE_MODHASH, "Hash_Add: Invalid Hash \n", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

#ifdef SIP_THREAD_SAFE
    objMutex.Lock();
#endif

    if (nNumOfSearchElements == (nMaxNumberOfElements))
    {
        *pnError = EERR_HASHELEMENTSEXCEEDED;
        SIP_DEBUG_WARNING(ESIPTRACE_MODHASH,
                "Hash_Add: Exceed Hash Elements \n", SIP_ZERO, SIP_ZERO);
#ifdef SIP_THREAD_SAFE
        objMutex.Unlock();
#endif
        return SIP_FALSE;
    }

    SIP_UINT32 nHashKey = fpCalculateHash(pvSearchKey);
    SIP_UINT32 nBucket = nHashKey % nSearchBuckets;

    /* Ensure that the key is not already present */
    Hash_StSearchElement* pstIterator = ppstHashSearchChains[nBucket];

    while (pstIterator != SIP_NULL)
    {
        if (fpCompareHash(pstIterator->pvSearchKey, pvSearchKey) == SIP_MATCHES)
        {
            /* The key already exists */
            *pnError = EERR_HASHKEYALREADYEXISTS;
            SIP_DEBUG_WARNING(EERR_HASHKEYALREADYEXISTS,
                    "Hash_Add: Key Already Exists \n", SIP_ZERO, SIP_ZERO);
#ifdef SIP_THREAD_SAFE
            objMutex.Unlock();
#endif
            return SIP_FALSE;
        }
        pstIterator = pstIterator->pstNextElement;
    }

    /*  pstNewElement = (Hash_StSearchElement *)
        SipPf_Malloc(sizeof(Hash_StSearchElement));
     */
    Hash_StSearchElement* pstNewElement = (Hash_StSearchElement *)
        new Hash_StSearchElement;

    if (pstNewElement == SIP_NULL)
    {
        /* e_Err_SipPf_MallocFailed */
        *pnError = EERR_MALLOCFAILED;
        SIP_DEBUG_EXTRLBUG(ESIPTRACE_MODHASH,
                "Hash_Add: Malloc Failed Seq:%d\n", SIP_ONE, SIP_ZERO);
#ifdef SIP_THREAD_SAFE
        objMutex.Unlock();
#endif
        return SIP_FALSE;
    }

    pstNewElement->pvElement = pvSearchElement;
    pstNewElement->pvSearchKey = pvSearchKey;
    pstNewElement->pstNextElement = SIP_NULL;
    pstNewElement->bRemove = SIP_FALSE;
    pstNewElement->nRefCount = SIP_ONE;

    pstIterator = ppstHashSearchChains[nBucket];

    if (pstIterator == SIP_NULL)
        pstIterator = pstNewElement;
    else
    {
        while (pstIterator != SIP_NULL)
            pstIterator = pstIterator->pstNextElement;
        pstIterator = pstNewElement;

    }

    /* Push element into the bucket */
    pstNewElement->pstNextElement = ppstHashSearchChains[nBucket];
    ppstHashSearchChains[nBucket] = pstNewElement;
    nNumOfSearchElements = nNumOfSearchElements + SIP_ONE;

    SIP_TRACE_NORMAL(ESIPTRACE_MODHASH,
            "Hash_Add --> Num of entries :%d", nNumOfSearchElements, SIP_ZERO);

#ifdef SIP_THREAD_SAFE
    objMutex.Unlock();
#endif
    return SIP_TRUE;
}

/******************************************************************************
 ** FUNCTION:   Hash_Fetch
 **
 ** DESCRIPTION:   This is the function to fetch an entry from the hash table.
 **        SIP_NULL is returned in case the hash table does not
 **        contain any entries corresponding to the key passed.
 **
 ** PARAMETERS:
 **  pKey  (IN)  : Key corresponding to the element to be
 **        fetched from the hash table.
 **
 ******************************************************************************/
SIP_VOID* SipHash::Hash_Fetch(SIP_VOID* pvSearchKey, SIP_UINT16* pnError)
{
    if ((pvSearchKey == SIP_NULL)
            || (fpCalculateHash == SIP_NULL)
            || (fpCompareHash == SIP_NULL)
            || (bInitialized == SIP_FALSE))
    {
        *pnError = EERR_INVALIDPARAM;
        SIP_DEBUG_WARNING(EERR_INVALIDPARAM,
                "Hash_Fetch: Invalid Hash \n", SIP_ZERO, SIP_ZERO);
        return SIP_NULL;
    }

#ifdef SIP_THREAD_SAFE
    objMutex.Lock();
#endif

    SIP_UINT32 nHashKey = fpCalculateHash(pvSearchKey);
    SIP_UINT32 nBucket = nHashKey % nSearchBuckets;
    Hash_StSearchElement* pstIterator = ppstHashSearchChains[nBucket];

    while (pstIterator != SIP_NULL)
    {
        if (fpCompareHash(pstIterator->pvSearchKey, pvSearchKey) == SIP_MATCHES)
        {
            pstIterator->nRefCount++;
            break;
        }
        pstIterator = pstIterator->pstNextElement;
    }

#ifdef SIP_THREAD_SAFE
    objMutex.Unlock();
#endif

    if (pstIterator == SIP_NULL)
    {
        *pnError = EERR_HASHELEMENTSNOTFOUND;
        SIP_DEBUG_WARNING(EERR_HASHELEMENTSNOTFOUND,
                "Hash_Fetch: Element Not Found", SIP_ZERO, SIP_ZERO);
        return SIP_NULL;
    }
    else
    {
        return pstIterator->pvElement;
    }
}

/******************************************************************************
 ** FUNCTION:   Hash_Fetch
 **
 ** DESCRIPTION:   This is the function to fetch an entry from the hash table.
 **        IMS_NULL is returned in case the hash table does not
 **        contain any entries corresponding to the key passed.
 **
 ** PARAMETERS:
 **   pHash   (IN)  : Hash table from which the entry has to be
 **        extracted.
 **  pKey  (IN)  : Key corresponding to the element to be
 **        fetched from the hash table.
 **  ppKey (OUT) : Stored key
 **
 ******************************************************************************/
SIP_VOID* SipHash::Hash_Fetch(SIP_VOID* pvSearchKey, SIP_VOID** ppKey, SIP_UINT16* pnError)
{
    if ((pvSearchKey == SIP_NULL)
            || (fpCalculateHash == SIP_NULL)
            || (fpCompareHash == SIP_NULL)
            || (bInitialized == SIP_FALSE))
    {
        *pnError = EERR_INVALIDPARAM;
        SIP_DEBUG_WARNING(EERR_INVALIDPARAM, "Hash_Fetch: Invalid Hash \n", SIP_ZERO, SIP_ZERO);
        return SIP_NULL;
    }

#ifdef SIP_THREAD_SAFE
    objMutex.Lock();
#endif

    SIP_UINT32 nHashKey = fpCalculateHash(pvSearchKey);
    SIP_UINT32 nBucket = nHashKey % nSearchBuckets;
    Hash_StSearchElement* pstIterator = ppstHashSearchChains[nBucket];

    while (pstIterator != SIP_NULL)
    {
        if (fpCompareHash(pstIterator->pvSearchKey, pvSearchKey) == SIP_MATCHES)
        {
            pstIterator->nRefCount++;
            break;
        }
        pstIterator = pstIterator->pstNextElement;
    }

#ifdef SIP_THREAD_SAFE
    objMutex.Unlock();
#endif

    if (pstIterator == SIP_NULL)
    {
        *pnError = EERR_HASHELEMENTSNOTFOUND;
        SIP_DEBUG_WARNING(EERR_HASHELEMENTSNOTFOUND,
                "Hash_Fetch: Element Not Found", SIP_ZERO, SIP_ZERO);
        return SIP_NULL;
    }
    else
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODHASH, "Hash_Fetch", SIP_ZERO, SIP_ZERO);
        *ppKey = pstIterator->pvSearchKey;
        return pstIterator->pvElement;
    }
}

/******************************************************************************
 ** FUNCTION:   Hash_Release
 **
 ** DESCRIPTION:  This function should be invoked to
 **        "check in" an element that was obtained
 **        from the hash table. Normally, it would
 **        just decrement reference count for the
 **        element. In case that the element has its
 **        remove flag set, this function frees
 **        the memory too.
 **
 ** PARAMETERS:
 **  pKey  (IN)  : Key corresponding to the element to be
 **        released.
 **
 ******************************************************************************/
SIP_VOID SipHash::Hash_Release(SIP_VOID* pvSearchKey)
{
    if ((pvSearchKey == SIP_NULL)
            || (fpCalculateHash == SIP_NULL)
            || (fpCompareHash == SIP_NULL)
            || (bInitialized == SIP_FALSE))
    {
        /* EERR_INVALIDPARAM */
        SIP_DEBUG_WARNING(EERR_INVALIDPARAM,
                "Hash_Release: Invalid Hash \n", SIP_ZERO, SIP_ZERO);
        return;
    }

#ifdef SIP_THREAD_SAFE
    objMutex.Lock();
#endif

    SIP_UINT32 nHashKey = fpCalculateHash(pvSearchKey);
    SIP_UINT32 nBucket = nHashKey % nSearchBuckets;
    Hash_StSearchElement** ppstIterator = &(ppstHashSearchChains[nBucket]);

    while (*ppstIterator != SIP_NULL)
    {
        if (fpCompareHash((*ppstIterator)->pvSearchKey, pvSearchKey) ==
                SIP_MATCHES)
        {
            (*ppstIterator)->nRefCount--;
            (*ppstIterator)->nRefCount--;
            break;
        }
        ppstIterator = &((*ppstIterator)->pstNextElement);
    }


    if ((*ppstIterator) == SIP_NULL)
    {
#ifdef SIP_THREAD_SAFE
        objMutex.Unlock();
#endif
        /* EERR_HASHELEMENTSNOTFOUND */
        SIP_DEBUG_WARNING(EERR_HASHELEMENTSNOTFOUND,
                "Hash_Fetch: Element Not Found \n", SIP_ZERO, SIP_ZERO);
        return;
    }

    if ((((*ppstIterator)->bRemove) == SIP_TRUE)
            && (SIP_ZERO == (*ppstIterator)->nRefCount))
    {
        Hash_StSearchElement* pstTempElement = *ppstIterator;
        *ppstIterator = (*ppstIterator)->pstNextElement;
        if (pstTempElement->pvSearchKey != SIP_NULL)
        {
            fpKeyFreeHashKey(pstTempElement->pvSearchKey);
        }
        if (pstTempElement->pvElement != SIP_NULL)
        {
            fpFreeHashElement(pstTempElement->pvElement);
        }

        nNumOfSearchElements = nNumOfSearchElements - SIP_ONE;
        pstTempElement->pvElement = SIP_NULL;
        //    SipPf_Free((SIP_VOID**)&pstTempElement);
        delete pstTempElement;
        SIP_TRACE_NORMAL(ESIPTRACE_MODHASH,
                "Hash_Release: - Element Freed: Num of entries :%d\n",
                nNumOfSearchElements, SIP_ZERO);
    }
    else
    {
        (*ppstIterator)->nRefCount = (*ppstIterator)->nRefCount + SIP_ONE;
    }

#ifdef SIP_THREAD_SAFE
    objMutex.Unlock();
#endif

}


/******************************************************************************
 ** FUNCTION: Hash_Remove
 **
 ** DESCRIPTION:   This is the function to remove an entry from the hash
 **        table. If the element is in use at the time of the remove
 **        request, then it is marked for removal and memory actually
 **        gets freed only when the other usage releases the entry.
 **
 ** PARAMETERS:
 **  pKey  (IN)  : Key corresponding to the element to be removed.
 **
 ******************************************************************************/
SIP_BOOL SipHash::Hash_Remove(SIP_VOID* pvSearchKey, SIP_UINT16* pnError)
{
    if ((pvSearchKey == SIP_NULL)
            || (fpCalculateHash == SIP_NULL)
            || (fpCompareHash == SIP_NULL)
            || (bInitialized == SIP_FALSE))
    {
        *pnError = EERR_INVALIDPARAM;
        SIP_DEBUG_WARNING(EERR_INVALIDPARAM,
                "Hash_Remove: Invalid Hash \n", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

#ifdef SIP_THREAD_SAFE
    objMutex.Lock();
#endif

    SIP_UINT32 nHashKey = fpCalculateHash(pvSearchKey);
    SIP_UINT32 nBucket = nHashKey % nSearchBuckets;
    Hash_StSearchElement** ppstIterator = &(ppstHashSearchChains[nBucket]);

    while (*ppstIterator != SIP_NULL)
    {
        if (fpCompareHash((*ppstIterator)->pvSearchKey, pvSearchKey) == SIP_MATCHES)
        {
            break;
        }
        ppstIterator = &((*ppstIterator)->pstNextElement);
    }

    if (*ppstIterator == SIP_NULL)
    {

#ifdef SIP_THREAD_SAFE
        objMutex.Unlock();
#endif
        *pnError = EERR_HASHELEMENTSNOTFOUND;
        SIP_DEBUG_WARNING(EERR_HASHELEMENTSNOTFOUND,
                "Hash_Fetch: Element Not Found \n", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    (*ppstIterator)->nRefCount = (*ppstIterator)->nRefCount - SIP_ONE;

    if ((*ppstIterator)->nRefCount == SIP_ZERO)
    {
        Hash_StSearchElement* pstTempElement = *ppstIterator;
        *ppstIterator = (*ppstIterator)->pstNextElement;
        if (pstTempElement->pvSearchKey != SIP_NULL)
        {
            fpKeyFreeHashKey(pstTempElement->pvSearchKey);
        }
        if (pstTempElement->pvElement != SIP_NULL)
        {
            fpFreeHashElement(pstTempElement->pvElement);
        }

        nNumOfSearchElements = nNumOfSearchElements - SIP_ONE;
        pstTempElement->pvElement = SIP_NULL;
        //    SipPf_Free((SIP_VOID**)&pstTempElement);
        delete pstTempElement;
        SIP_TRACE_NORMAL(ESIPTRACE_MODHASH,
                "*****Hash_Remove: Num of Entries Existing [%d]*****",
                nNumOfSearchElements, SIP_ZERO);
    }
    else
    {
        (*ppstIterator)->bRemove = SIP_TRUE;
        (*ppstIterator)->nRefCount = (*ppstIterator)->nRefCount + SIP_ONE;
        SIP_TRACE_NORMAL(ESIPTRACE_MODHASH,
                "Hash_Remove: - Could not remove as \
                Ref count for this element :%d",(*pstIterator)->nRefCount, SIP_ZERO);
    }

    SIP_DEBUG_WARNING(EERR_NOERR, "Hash_Remove: elementCount=%d", nNumOfSearchElements, SIP_ZERO);

#ifdef SIP_THREAD_SAFE
    objMutex.Unlock();
#endif

    return SIP_TRUE;
}

/******************************************************************************
 ** FUNCTION:   Hash_IterateNext
 **
 ** DESCRIPTION:   This is the function to get the element next to an
 **        iterator in the hash table
 **
 ** PARAMETERS:
 **  pstIterator  (IN/OUT)  : Hash iterator for which the next
 **          element has to be retrieved.
 **
 ******************************************************************************/
void SipHash::Hash_IterateNext(Hash_St_HashIterator* pstIterator, SIP_UINT16* pnError)
{
    if ((fpCalculateHash == SIP_NULL)
            || (fpCompareHash == SIP_NULL)
            || (SIP_NULL == ppstHashSearchChains)
            || (bInitialized == SIP_FALSE))
    {
        *pnError = EERR_INVALIDPARAM;
        SIP_DEBUG_WARNING(ESIPTRACE_MODHASH,
                "Hash_IterateNext: Invalid Hash \n", SIP_ZERO, SIP_ZERO);
        return;
    }

#ifdef SIP_THREAD_SAFE
    objMutex.Lock();
#endif


    /*   If current element in the iterator points to a
         null node - keep it null else make it point to
         next element in the chain. */
    Hash_StSearchElement* pstNextElem = (pstIterator->pCurrentElement == SIP_NULL)\
              ? SIP_NULL : pstIterator->pCurrentElement->pstNextElement;

    /*
     * Check if the current element has to be removed.
     * If so, do it here before fetching the next element
     */
    if (pstIterator->pCurrentElement != SIP_NULL)
    {
        pstIterator->pCurrentElement->nRefCount =
            pstIterator->pCurrentElement->nRefCount - SIP_ONE;
        pstIterator->pCurrentElement->nRefCount =
            pstIterator->pCurrentElement->nRefCount - SIP_ONE;

        if ((pstIterator->pCurrentElement->nRefCount == SIP_ZERO) && \
                (pstIterator->pCurrentElement->bRemove == SIP_TRUE))
        {
            SIP_VOID* pKey = pstIterator->pCurrentElement->pvSearchKey;
            Hash_StSearchElement** ppElement =
                &(ppstHashSearchChains[pstIterator->nCurrentBucket]);

            while (*ppElement != SIP_NULL)
            {
                if (fpCompareHash((*ppElement)->pvSearchKey, pKey) == SIP_ZERO)
                    break;
                ppElement = &((*ppElement)->pstNextElement);
            }
            if (*ppElement != SIP_NULL)
            {
                Hash_StSearchElement* pTempElement = *ppElement;
                *ppElement = (*ppElement)->pstNextElement;
                if (fpFreeHashElement != SIP_NULL)
                    fpFreeHashElement(pTempElement->pvElement);
                if (fpKeyFreeHashKey != SIP_NULL)
                    fpKeyFreeHashKey(pTempElement->pvSearchKey);

                delete pTempElement;
                //      SipPf_Free((SIP_VOID **)&pTempElement);
                nNumOfSearchElements =
                    nNumOfSearchElements - SIP_ONE;
            }
        }
        else
        {
            pstIterator->pCurrentElement->nRefCount =
                pstIterator->pCurrentElement->nRefCount + SIP_ONE;
        }
    }

    if (pstNextElem != SIP_NULL)
    {
        /*   Still in the middle of iteration through a chain.
             We have already taken the next element.
             Return now */
        pstIterator->pCurrentElement = pstNextElem;
        pstIterator->pCurrentElement->nRefCount =
            pstIterator->pCurrentElement->nRefCount + SIP_ONE;
#ifdef SIP_THREAD_SAFE
        objMutex.Unlock();
#endif
        return ;
    }

    /*  Since the next element is Null, we have reached the end
        of the chain. Check if it's the end of the last chain.
        If so, return */
    if ((pstNextElem == SIP_NULL) &&\
            (pstIterator->nCurrentBucket == nSearchBuckets - SIP_ONE))
    {
        pstIterator->pCurrentElement = pstNextElem;
#ifdef SIP_THREAD_SAFE
        objMutex.Unlock();
#endif
        return;
    }

    /*   Find the next non-empty chain and
         make the iterator point to that */
    pstIterator->nCurrentBucket = pstIterator->nCurrentBucket + SIP_ONE;
    while (pstIterator->nCurrentBucket != nSearchBuckets - SIP_ONE)
    {

        if (ppstHashSearchChains[pstIterator->nCurrentBucket]!=SIP_NULL)
            break;
        pstIterator->nCurrentBucket++;
    }

    pstIterator->pCurrentElement = \
                       ppstHashSearchChains[pstIterator->nCurrentBucket];

    /*   Increment reference count of the element being
         returned unless we reached the end of the final
         bucket */
    if (pstIterator->pCurrentElement != SIP_NULL)
        pstIterator->pCurrentElement->nRefCount++;

#ifdef SIP_THREAD_SAFE
    objMutex.Unlock();
#endif
    return;
}

/******************************************************************************
 ** FUNCTION: Hash_InitIterator
 **
 ** DESCRIPTION:  Sets the iterator to the first element of the hashtable
 **
 ** PARAMETERS:
 **  pstIterator  (IN/OUT)  : Hash iterator to be initialized.
 **
 ******************************************************************************/
void SipHash::Hash_InitIterator(Hash_St_HashIterator* pstIterator)
{
    if ((pstIterator == SIP_NULL) || (ppstHashSearchChains == SIP_NULL)
            || (bInitialized == SIP_FALSE))
    {
        return;
    }

    pstIterator->nCurrentBucket = SIP_ZERO;
    pstIterator->pCurrentElement = SIP_NULL;

#ifdef SIP_THREAD_SAFE
    objMutex.Lock();
#endif

    while (pstIterator->nCurrentBucket <= nSearchBuckets - SIP_ONE)
    {
        if (ppstHashSearchChains[pstIterator->nCurrentBucket] == SIP_NULL)
        {
            pstIterator->nCurrentBucket++;
            continue;
        }
        else
        {
            pstIterator->pCurrentElement = ppstHashSearchChains\
                               [pstIterator->nCurrentBucket];
            pstIterator->pCurrentElement->nRefCount++;
            break;
        }
    }

    if (pstIterator->pCurrentElement == SIP_NULL)
    {
        pstIterator->nCurrentBucket = pstIterator->nCurrentBucket - SIP_ONE;
    }


#ifdef SIP_THREAD_SAFE
    objMutex.Unlock();
#endif

    return;

}

/******************************************************************************
 ** FUNCTION: Hash_IsFree
 **
 ** DESCRIPTION: Tells whether there is space to accommodate additional
 **       entries in the hash table or not
 **
 ** PARAMETERS:
 **  pHash   (IN)    : Hash table
 **
 **  pNumFreeEntries(OUT) : Number of free entries
 **
 ******************************************************************************/
SIP_BOOL SipHash::Hash_IsFree(SIP_UINT16* pNumFreeEntries)
{
    if ((pNumFreeEntries == SIP_NULL) || (bInitialized == SIP_FALSE))
    {
        return SIP_FALSE;
    }

#ifdef SIP_THREAD_SAFE
    objMutex.Lock();
#endif
    *pNumFreeEntries = nMaxNumberOfElements\
                 - nNumOfSearchElements;

#ifdef SIP_THREAD_SAFE
    objMutex.Unlock();
#endif
    SIP_DEBUG_WARNING(ESIPTRACE_MODHASH,
            "Hash_IsFree\n", SIP_ZERO, SIP_ZERO);
    if (*pNumFreeEntries != SIP_ZERO)
    {
        return SIP_TRUE;
    }
    return SIP_FALSE;
}
