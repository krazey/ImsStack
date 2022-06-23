/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20120217  hwangoo.park@             Created
    </table>

    Description
     This file implements the secure hash algorithm.
*/

#ifndef _IMS_SHA1_H_
#define _IMS_SHA1_H_

#include "IMSTypeDef.h"

#define ImsSha1Context     SHA1Context
#define ImsSha1_Initialize IMSSHA1_Initialize
#define ImsSha1_Update     IMSSHA1_Update
#define ImsSha1_Finalize   IMSSHA1_Finalize

// Structure for SHA1 context
struct SHA1Context
{
    // Length of the original message (64bits)
    IMS_UINT32 nLengthLow;
    IMS_UINT32 nLengthHigh;
    // Initial hex values (H)
    IMS_UINT32 anH[5];
    // 512 bit message block
    IMS_UCHAR aucMessageBlock[64];
};

GLOBAL void IMSSHA1_Initialize(OUT SHA1Context* pstContext);
GLOBAL void IMSSHA1_Update(
        IN CONST IMS_UCHAR* pucSrc, IN IMS_UINT32 nSrcLen, IN_OUT SHA1Context* pstContext);
GLOBAL void IMSSHA1_Finalize(IN_OUT SHA1Context* pstContext, OUT IMS_UCHAR aucHash[20]);

#endif  // _IMS_SHA1_H_
