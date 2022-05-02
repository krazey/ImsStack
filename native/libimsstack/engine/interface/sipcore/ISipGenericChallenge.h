#ifndef _INTERFACE_SIP_GENERIC_CHALLENGE_H_
#define _INTERFACE_SIP_GENERIC_CHALLENGE_H_

#include "AString.h"

/**
 * @brief This class provides an interface to handle SIP authentication challenge.
 */
class ISIPGenericChallenge
{
public:
    /**
     * @brief Clones a SIP generic challenge.
     *
     * @return Pointer to ISIPGenericChallenge.
     */
    virtual ISIPGenericChallenge* Clone() const = 0;

    /**
     * @brief Destroys the authentication challenge.
     */
    virtual void Destroy() = 0;

    /**
     * @brief Returns the authentication algorithm (MD5 / AKAv1-MD5 / AKAv2-MD5).
     *
     * @return Authentication algorithm as string.
     */
    virtual const AString& GetAlgorithm() const = 0;

    /**
     * @brief Returns the nonce value of the authentication challenge.
     *
     * @return Authentication nonce.
     */
    virtual const AString& GetNonce() const = 0;

    /**
     * @brief Returns the nonce count value of the authentication challenge.
     *
     * @return Authentication nonce count.
     */
    virtual IMS_UINT32 GetNonceCount() const = 0;

    /**
     * @brief Returns the qop of the authentication challenge.
     *
     * @return Qop (Quality of Protection) value.
     */
    virtual const AString& GetQop() const = 0;

    /**
     * @brief Returns the realm of the authentication challenge.
     *
     * @return Authentication realm.
     */
    virtual const AString& GetRealm() const = 0;

    /**
     * @brief Returns the authentication scheme (Basic / Digest).
     *
     * @return Authentication scheme.
     */
    virtual const AString& GetScheme() const = 0;

    /**
     * @brief Returns the type of the authentication challenge.
     *
     * It is same to the SIP header type (WWW-Authenticate / Proxy-Authenticate).
     *
     * @return Authentication related header type in ISIPHeader.\n
     *         ISIPHeader#WWW_AUTHENTICATE\n
     *         ISIPHeader#PROXY_AUTHENTICATE.
     */
    virtual IMS_SINT32 GetType() const = 0;

    /**
     * @brief Increases the nonce count when refresh request is sent.
     */
    virtual void IncreaseNonceCount() = 0;

    /**
     * @brief Sets the nonce.
     *
     * @param strNonce Nonce value of this challenge
     */
    virtual void SetNonce(IN CONST AString &strNonce) = 0;

    /**
     * @brief Sets the nonce count.
     *
     * @param nNonceCount Nonce count of this challenge
     */
    virtual void SetNonceCount(IN IMS_UINT32 nNonceCount) = 0;
};

#endif // _INTERFACE_SIP_GENERIC_CHALLENGE_H_
