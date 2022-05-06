/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100727  hwangoo.park@             Created (RFC 2104)
    </table>

    Description
     This file implements HMAC, a mechanism for message authentication using cryptographic hash
    functions. HMAC can be used with any iterative cryptographic hash function, e.g., MD5, SHA-1,
    in combination with a secret shared key.
    The cryptographic strength of HMAC depends on the properties of the underlying hash function.
    HMAC can be used in combination with any iterated cryptographic hash function.
    MD5 and SHA-1 are examples of such hash functions. HMAC also uses a secret key for calculation
    and verification of the message authentication values.
*/

#ifndef _IMS_HMAC_H_
#define _IMS_HMAC_H_

#include "IMSTypeDef.h"

GLOBAL void IMSHMAC_MD5(IN CONST IMS_UCHAR* pucText, IN IMS_SINT32 nTextLen,
        IN CONST IMS_UCHAR* pucKey, IN IMS_SINT32 nKeyLen, OUT IMS_UCHAR aucDigest[16]);

GLOBAL void IMSHMAC_SHA1(IN CONST IMS_UCHAR* pucText, IN IMS_SINT32 nTextLen,
        IN CONST IMS_UCHAR* pucKey, IN IMS_SINT32 nKeyLen, OUT IMS_UCHAR aucHash[20]);

#endif  // _IMS_HMAC_H_
