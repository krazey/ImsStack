#ifndef _INTERFACE_SIP_TOKEN_GENERATOR_H_
#define _INTERFACE_SIP_TOKEN_GENERATOR_H_

#include "AString.h"

/**
 * @brief This class provides an interface to generate SIP related tokens.
 */
class ISIPTokenGenerator
{
public:
    /**
     * @brief Gets the boundary token for the multipart MIME body parts.
     *
     * @param strBoundary Boundary token
     */
    virtual void GenerateBoundary(OUT AString &strBoundary) = 0;

    /**
     * @brief Gets the call-id token for the Call-ID header.
     *
     * @param strHost Host information (i.e. IP address of the device)
     * @param strCallId Call-id token
     */
    virtual void GenerateCallId(IN CONST AString &strHost, OUT AString &strCallId) = 0;

    /**
     * @brief Gets the tag token for the From/To header.
     *
     * @param strMagicCookie Magic cookie (prefix of the tag)
     * @param strTag Tag token
     */
    virtual void GenerateTag(IN CONST AString &strMagicCookie, OUT AString &strTag) = 0;

    /**
     * @brief Gets the via branch token for the Via header.
     *
     * @param strViaBranch Via branch token
     * @param strExtensionToken Extension token
     */
    virtual void GenerateViaBranch(OUT AString &strViaBranch,
            IN CONST AString &strExtensionToken = AString::ConstNull()) = 0;
};

#endif // _INTERFACE_SIP_TOKEN_GENERATOR_H_
