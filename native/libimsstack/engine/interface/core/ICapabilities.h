#ifndef _INTERFACE_CAPABILITIES_H_
#define _INTERFACE_CAPABILITIES_H_

#include "IServiceMethod.h"

class ICapabilitiesListener;

/**
 * @brief This class provides an interface for SIP OPTIONS transaction.
 *
 * The Capabilities is used to query a remote endpoint
 * whether it has matching capabilities matching the local ones.
 *
 * @see ICoreService, ICapabilitiesListener
 */
class ICapabilities : public IServiceMethod
{
public:
    /**
     * @brief Returns an array of strings representing valid user identities
     *        for the remote endpoint.
     *
     * @return All SIP & TEL URIs which are in the Contact header.
     */
    virtual IMSList<AString> GetRemoteUserIdentities() const = 0;

    /**
     * @brief Returns the current state of this ICapabilities.
     *
     * @return State of this Capabilities.\n
     *         #STATE_INACTIVE\n
     *         #STATE_PENDING\n
     *         #STATE_ACTIVE
     */
    virtual IMS_SINT32 GetState() const = 0;

    /**
     * @brief This method returns true if the remote endpoint is believed to be sufficiently
     *        capable of handling requests from a certain core service on the local endpoint.
     *
     * The core service is specified with a "imscore" Connector string.
     *
     * @param strConnection Connector ("imscore") string to be checked
     * @return If this ICapabilities has the given capabilities, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL HasCapabilities(IN CONST AString& strConnection) const = 0;

    /**
     * @brief Sends a capability request to a remote endpoint.
     *
     * @param bSDPInRequest Flag to indicate if SDP needs to be included
     *                      in the capability query request
     * @param bContactInRequest Flag to indicate if Contact header needs to be include
     *                          in the capability query request
     * @param bCheckSupport Flag to indicate if the media property needs to be evaluated
     *                      when forming a capability SDP.\n
     *                      The media properties are the following:
     *                          StreamAudio, StreamVideo, Framed.\n
     *                      Its default value sets to TRUE according to the specification maybe.
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT QueryCapabilities(IN IMS_BOOL bSDPInRequest,
            IN IMS_BOOL bContactInRequest = IMS_TRUE, IN IMS_BOOL bCheckSupport = IMS_TRUE) = 0;

    virtual IMS_RESULT QueryCapabilitiesEx() = 0;

    /**
     * @brief Sets a listener for this ICapabilities, replacing any previous ICapabilitiesListener.
     *
     * A null reference is allowed and has the effect of removing any existing listener.
     *
     * @param piListener Pointer to the listener for receiving the result of capability query
     */
    virtual void SetListener(IN ICapabilitiesListener* piListener) = 0;

    //// IMS extensions

    /**
     * @brief Sends a successful final response to an incoming capability query
     *        from a remote endpoint.
     *
     * @param bFeatureInContact Flag to indicate if the feature-parameter will be included
     *                          in Contact header.\n This field is for VSC service.
     * @param bCheckSupport Flag to indicate if the media property needs to be evaluated
     *                      when forming a capability SDP.\n
     *                      The media properties are the following:
     *                          StreamAudio, StreamVideo, Framed.\n
     *                      Its default value sets to TRUE according to the specification maybe.
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Accept(
            IN IMS_BOOL bFeatureInContact = IMS_TRUE, IN IMS_BOOL bCheckSupport = IMS_TRUE) = 0;

    /**
     * @brief Sends a successful final response to an incoming capability query
     *        from a remote endpoint.
     *
     * The application SHOULD set the Contact header & message body
     * according to the implementation.
     *
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT AcceptEx() = 0;

    /**
     * @brief Sends a failure final response to an incoming capability query
     *        from a remote endpoint.
     *
     * @param nStatusCode SIP status code
     * @param nRetryAfter Value for Retry-After header field (seconds unit)
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Reject(IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nRetryAfter = 0) = 0;

public:
    /// States of ICapabilities
    enum
    {
        STATE_INACTIVE = 1,
        STATE_PENDING = 2,
        STATE_ACTIVE = 3
    };
};

#endif  // _INTERFACE_CAPABILITIES_H_
