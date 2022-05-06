#ifndef _INTERFACE_SESSION_PARAMETER_H_
#define _INTERFACE_SESSION_PARAMETER_H_

#include "offeranswer/SdpSessionParameter.h"
#include "offeranswer/SdpMediaParameter.h"

/**
 * @brief This class provides an interface to access SDP parameters.
 */
class ISessionParameter
{
public:
    /**
     * @brief This method returns a session-level session description object.
     *
     * @return A SDP session-level parameter.
     */
    virtual const SdpSessionParameter& GetSessionParameter() const = 0;

    /**
     * @brief This method returns the count of media-level session description.
     *
     * @return The count of SdpMediaParameter.
     */
    virtual IMS_SINT32 GetMediaCount() const = 0;

    /**
     * @brief This method returns a media-level session description object with the specified mid.
     *
     * @param nMid Media identifier (zero-based index)
     * @return A SDP media-level parameter.
     */
    virtual SdpMediaParameter* GetMediaParameter(IN IMS_UINT32 nMid) const = 0;
};

#endif  // _INTERFACE_SESSION_PARAMETER_H_
