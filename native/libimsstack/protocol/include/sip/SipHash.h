#ifndef __SIP_HASH_H__
#define __SIP_HASH_H__

#define SIP_THREAD_SAFE 1

#include "sip_pf_datatypes.h"
#include "platform/SipMutex.h"

/*
 * This structure stores the hash table element and the search key to
 * retrieve the hash element.
 */
typedef struct _Hash_StSearchElement
{
    SIP_VOID* pvElement;   /* Hash element*/
    SIP_VOID* pvSearchKey; /* Hash key*/

    struct _Hash_StSearchElement* pstNextElement;

    /*
     * Flag that will be set to true if the API Hash_HashTableRemove is
     * unable to free the memory for the hash element at time of
     * invocation. As soon as the structures refCount reduces to zero, the
     * memory will be freed if this flag is set.
     */
    SIP_BOOL bRemove;

    /*
     * Keep track of how many check-outs of this data have happened. If the
     * refCount is greater than 1 and Hash_HashTableRemove is invoked,
     * memory will not be freed and the bRemove flag will be set.
     */
    SIP_UINT32 nRefCount;

} Hash_StSearchElement;

/*
 * This structure stores the hash table element and the search key to
 * retrieve the hash element. This is used to retrieve the hash element
 * during session shutdown operation.
 */
typedef struct _Hash_StShutDownElement
{
    SIP_VOID* pvElement;     /* Hash element*/
    SIP_VOID* pvShutDownKey; /* Hash key */
    struct _Hash_StShutDownElement* pstNextElement;
} Hash_StShutDownElement;

/*
 * The function to be used to calculate the hash value for the key passed.
 */
typedef SIP_UINT32 (*Hash_fnCalculateHash)(SIP_VOID* pData);

/*
 * The function to be used to compare keys of 2 hash elements.
 */
typedef SIP_CHAR (*Hash_fnCompareHashKey)(SIP_VOID* pKey1, SIP_VOID* pKey2);

/*
 * The function to free the data stored in the hash element.
 */
typedef SIP_VOID (*Hash_fnFreeHashElement)(SIP_VOID* pElement);

/*
 * The function to free the key stored in the hash element.
 */
typedef SIP_VOID (*Hash_fnFreeHashKey)(SIP_VOID* pKey);

/*
 * Iterator needed to operate on all the hash elements. User will need to
 * do Hash_HashTableIterateNext()
 * to get the next element from the hash table.
 */
typedef struct
{
    Hash_StSearchElement* pCurrentElement;
    SIP_UINT32 nCurrentBucket;
} Hash_St_HashIterator;

/*
 * This structure stores the all the hash table element details.
 */
// typedef struct  _Hash_St_HashTable
class SipHash
{
private:
    /* function to calculate hash value */
    Hash_fnCalculateHash fpCalculateHash;
    /* function to compare keys of 2 elements */
    Hash_fnCompareHashKey fpCompareHash;
    /* function to free the data being stored */
    Hash_fnFreeHashElement fpFreeHashElement;
    /* function to free the key given for an element */
    Hash_fnFreeHashKey fpKeyFreeHashKey;

    SIP_UINT32 nSearchBuckets; /* Max number of buckets*/
    /* Current number of hash element in each bucket */
    SIP_UINT32 nNumOfSearchElements;
    /* Max number of hash elements in each bucket*/
    SIP_UINT32 nMaxNumberOfElements;
    /* Hash elements structure*/
    Hash_StSearchElement** ppstHashSearchChains;

#ifdef SIP_THREAD_SAFE
    /* One for each function - it should be ok for the current architechure where
       not more than one thread is making call for hash otherwise per buket a mutex
       can be assigned on the cost of memory */
    SipMutex objMutex;
#endif

    SIP_BOOL bInitialized;

    /****************************************************************************
      Private Member Functions
     *****************************************************************************/
    SipHash& operator=(IN const SipHash& objRHS);
    SipHash(IN const SipHash& objRHS);

public:
    /****************************************************************************
      Functions Declaration
     *****************************************************************************/

    /******************************************************************************
     ** FUNCTION:   SipHash
     **
     ** DESCRIPTION:   This is the constructor to initialize a new
     **      hash table with the function pointers
     **
     ** PARAMETERS:
     **  fpHashFunc   (IN)    : Function to be used to hash the key.
     **  fpCompareFunc   (IN)    : Function to compare the hash keys of entries
     **            at time of doing a fetch. If the comparison
     **            function :
     **            returns 0 - the keys that were compared match
     **            returns 1 - the keys don't match
     **  fpElemFreeFunc (IN)    : Function to invoke to free the
     **            element data when the hash entry
     **            has be deleted.
     **  fpKeyFreeFunc  (IN)    : Function to invoke to free the
     **            element key when the hash entry
     **            has be deleted.
     **  nNumBuckets  (IN)    : number of chains in the hash table.
     **  nMaxElements  (IN)    : maximum number of elements to be allowed
     **            in the hash table.
     **   pErr    (IN/OUT)  : Error variable returned in case of failure.
     **
     ******************************************************************************/
    SipHash(Hash_fnCalculateHash fpCalculateHash, Hash_fnCompareHashKey fpCompareHash,
            Hash_fnFreeHashElement fpElemFreeFunc, Hash_fnFreeHashKey fpKeyFreeHashKey,
            SIP_UINT32 nNumBuckets, SIP_UINT32 nMaxElements, SIP_UINT16* pnError);

    /******************************************************************************
     ** FUNCTION:   Hash_DeInit
     **
     ** DESCRIPTION:   This is the function to free members from the hash table.
     **        It does not free the hash elements, but frees other  member
     **        variables malloced at the time of Init of the hash table
     **
     ** PARAMETERS:
     **   pHash   (IN)  : Hash table to be freed.
     **  pErr  (OUT)  : Error variable returned in case of failure.
     **
     ******************************************************************************/
    ~SipHash();

    /******************************************************************************
     ** FUNCTION:   Hash_Add
     **
     ** DESCRIPTION:   This is the function to add an entry into the hash table.
     **
     ** PARAMETERS:
     **   pHash   (IN)  : Hash table to which the entry has
     **        to be added.
     **  pElement (IN)  : Hash Element that needs to be added
     **
     **  pErr  (OUT)  : Error variable returned in case
     **        of failure.
     **
     ******************************************************************************/
    SIP_BOOL Hash_Add(SIP_VOID* pvSearchElement, SIP_VOID* pvSearchKey, SIP_UINT16* pnError);

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
     **
     ******************************************************************************/
    SIP_VOID* Hash_Fetch(SIP_VOID* pvSearchKey, SIP_UINT16* pnError);

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
    SIP_VOID* Hash_Fetch(SIP_VOID* pvSearchKey, SIP_VOID** ppKey, SIP_UINT16* pnError);

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
     **   pHash   (IN)  : Hash table from which the entry has to
     **        be released.
     **  pKey  (IN)  : Key corresponding to the element to be
     **        released.
     **
     ******************************************************************************/
    SIP_VOID Hash_Release(SIP_VOID* pvKey);

    /******************************************************************************
     ** FUNCTION: Hash_Remove
     **
     ** DESCRIPTION:   This is the function to remove an entry from the hash
     **        table. If the element is in use at the time of the remove
     **        request, then it is marked for removal and memory actually
     **        gets freed only when the other usage releases the entry.
     **
     ** PARAMETERS:
     **   pHash   (IN)  : Hash table from which the entry has to be released.
     **  pKey  (IN)  : Key corresponding to the element to be removed.
     **
     ******************************************************************************/
    SIP_BOOL Hash_Remove(SIP_VOID* pvSearchKey, SIP_UINT16* pnError);

    /******************************************************************************
     ** FUNCTION:   Hash_IterateNext
     **
     ** DESCRIPTION:   This is the function to get the element next to an
     **        iterator in the hash table
     **
     ** PARAMETERS:
     **   pHash   (IN)    : Hash table from which the next entry
     **          has to be retrieved.
     **  pIterator  (IN/OUT)  : Hash iterator for which the next
     **          element has to be retrieved.
     **
     ******************************************************************************/
    void Hash_IterateNext(Hash_St_HashIterator* pstIterator, SIP_UINT16* pnError);

    /******************************************************************************
     ** FUNCTION: Hash_HashTableInitIterator
     **
     ** DESCRIPTION:  Sets the iterator to the first element of the hashtable
     **
     ** PARAMETERS:
     **  pIterator  (IN/OUT)  : Hash iterator to be inited.
     **
     ******************************************************************************/
    void Hash_InitIterator(Hash_St_HashIterator* pIterator);

    /******************************************************************************
     ** FUNCTION: Hash_IsFree
     **
     ** DESCRIPTION: Tells whether there is space to accommodate additional
     **       entries in the hash table or not
     **
     ** PARAMETERS:
     **  pHash   (IN)    : Hash table
     **
     **  puiNumFreeEntries(OUT) : Number of free entries
     **
     ******************************************************************************/
    SIP_BOOL Hash_IsFree(SIP_UINT16* pNumFreeEntries);
};
#endif  //__SIP_HASH_H__
