/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090303  toastops@                 Porting (From Aricent SIP stack)
    </table>

    Description
     This file implements the digest authentication scheme.
*/

#ifndef _IMS_MD5_H_
#define _IMS_MD5_H_

#include "IMSTypeDef.h"

// Structure for MD5 context
struct MD5Context
{
    IMS_UINT32 anState[4];    // State (ABCD)
    IMS_UINT32 anCount[2];    // Number of bits, modulo 2^64 (LSB first)
    IMS_UCHAR aucBuffer[64];  // Input buffer
};

GLOBAL void IMSMD5_Initialize(OUT MD5Context* pstContext);
GLOBAL void IMSMD5_Update(
        IN CONST IMS_UCHAR* pucSrc, IN IMS_UINT32 nSrcLen, IN_OUT MD5Context* pstContext);
GLOBAL void IMSMD5_Finalize(IN_OUT MD5Context* pstContext, OUT IMS_UCHAR aucDigest[16]);

#endif  // _IMS_MD5_H_
