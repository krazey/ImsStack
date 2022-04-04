/*
   Author
   <table>
   date      author                 description
   --------  --------------         ----------
   20100000  syed.malgimani@        Created
   20170110  vijay.nair@            Modified
   </table>

   Description

 */

#ifndef __SIP_TXN_DB_H__
#define __SIP_TXN_DB_H__

#include "sip_pf_datatypes.h"
#include "msg/sip_comdef.h"
#include "SipHash.h"

class SipTxnDb
{
    public:
        SipTxnDb();
        virtual ~SipTxnDb();

        SIP_BOOL AddElement(SIP_VOID* pvElement, SIP_VOID* pvKey, SIP_UINT16* pnError);

        SIP_BOOL FetchElement(SIP_VOID* pvKey, SIP_VOID** ppElement, SIP_UINT16* pnError);

        SIP_BOOL FetchElement(SIP_VOID* pvKey, SIP_VOID** ppElement, SIP_VOID** ppKey,
                SIP_UINT16* pnError);

        SIP_BOOL RemoveElement(SIP_VOID* pvKey, SIP_UINT16* pnError);

    private:
        SipHash* m_pTxnHash;

        /**************************************************
          Private Member Functions
         ***************************************************/
        SipTxnDb& operator=(IN const SipTxnDb& objRHS);
        SipTxnDb(IN const SipTxnDb& objRHS);
};

void SipTxnDb_Construct();
void SipTxnDb_Destruct();

SipTxnDb* SipTxnDb_GetInstance();
SIP_CHAR sipTxnCompareHashKey(SIP_VOID* pvStoredKey, SIP_VOID* pvUserKey);

#endif //__SIP_TXN_DB_H__
