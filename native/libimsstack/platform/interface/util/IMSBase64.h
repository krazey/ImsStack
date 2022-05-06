/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20050920  LEE.D.C                   Created
    20060417  LEE.D.C                   Moved to upgraded Fwk
    20090303  toastops@                 Porting
    </table>

    Description
    This file implements the base64 encoding. It is designed to represent arbitrary sequences of
    octets in a form that allows the use of both upper- and lowercase letters but that need not be
    human readable.
    A 65-character subset of US-ASCII is used, enabling 6 bits to be represented per printable
    character. (The extra 65th character, "=" is used to signify a special processing function.)
*/

#ifndef _IMS_BASE64_H_
#define _IMS_BASE64_H_

#include "IMSTypeDef.h"

#ifdef __cplusplus
extern "C"
{
    GLOBAL IMS_SINT32 IMSBase64_Encode(IN IMS_BYTE* pSrcData, IN IMS_SIZE_T nSrcLen,
            IN_OUT IMS_CHAR* pszDest, IN IMS_SIZE_T nDestLen, IN IMS_BOOL bAddCRLF = IMS_TRUE);

    GLOBAL IMS_SINT32 IMSBase64_Decode(IN IMS_CHAR* pszSrcData, IN IMS_SIZE_T nSrcLen,
            IN_OUT IMS_BYTE* pDest, IN IMS_SIZE_T nDestLen);
}
#endif
#endif  // _IMS_BASE64_H_
