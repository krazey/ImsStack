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

#ifndef _IMS_DIGEST_H_
#define _IMS_DIGEST_H_

#include "AString.h"

#define ImsDigest_CalculateA1       IMSDigestEx_CalculateHA1
#define ImsDigest_CalculateEntity   IMSDigestEx_CalculateHEntity
#define ImsDigest_CalculateResponse IMSDigestEx_CalculateResponse

#define HASHHEX_SIZE                32

typedef IMS_CHAR HASHHEX[HASHHEX_SIZE + 1];

GLOBAL void IMSDigest_CalculateHA1(IN CONST IMS_CHAR* pszAlgorithm, IN CONST IMS_CHAR* pszUsername,
        IN CONST IMS_CHAR* pszRealm, IN CONST IMS_CHAR* pszPassword, IN CONST IMS_CHAR* pszNonce,
        IN CONST IMS_CHAR* pszCNonce, OUT HASHHEX HA1);

GLOBAL void IMSDigest_CalculateHEntity(
        IN CONST IMS_CHAR* pszEntityBody, IN IMS_SINT32 nLength, OUT HASHHEX HEntity);

GLOBAL void IMSDigest_CalculateResponse(IN CONST HASHHEX HA1,  // H(A1)
        IN CONST IMS_CHAR* pszNonce,                           // nonce from the server
        IN CONST IMS_CHAR* pszNonceCount,                      // 8 HEX digits
        IN CONST IMS_CHAR* pszCNonce,                          // Client nonce
        IN CONST IMS_CHAR* pszQoP,                             // qop-value : "", "auth", "auth-int"
        IN CONST IMS_CHAR* pszMethod,                          // Method from the Request-Line
        IN CONST IMS_CHAR* pszDigestURI,                       // Requested URL
        IN CONST HASHHEX HEntity,                              // H(entity-body) if qop = "auth-int"
        OUT HASHHEX HResponse);                                // Request-digest or response-digest

// Extended implementation using AString class
GLOBAL void IMSDigestEx_CalculateHA1(IN CONST AString& strAlgorithm, IN CONST AString& strUsername,
        IN CONST AString& strRealm, IN CONST AString& strPassword, IN CONST AString& strNonce,
        IN CONST AString& strCNonce, OUT HASHHEX HA1);

GLOBAL void IMSDigestEx_CalculateHEntity(IN CONST AString& strEntityBody, OUT HASHHEX HEntity);

GLOBAL void IMSDigestEx_CalculateResponse(IN CONST HASHHEX HA1,  // H(A1)
        IN CONST AString& strNonce,                              // nonce from the server
        IN CONST AString& strNonceCount,                         // 8 HEX digits
        IN CONST AString& strCNonce,                             // Client nonce
        IN CONST AString& strQoP,        // qop-value : "", "auth", "auth-int"
        IN CONST AString& strMethod,     // Method from the Request-Line
        IN CONST AString& strDigestURI,  // Requested URL
        IN CONST HASHHEX HEntity,        // H(entity-body) if qop = "auth-int"
        OUT HASHHEX HResponse);          // Request-digest or response-digest

#endif  // _IMS_DIGEST_H_
