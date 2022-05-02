#ifndef _SIP_HEADER_UTIL_H_
#define _SIP_HEADER_UTIL_H_

#include "IMSTypeDef.h"

/**
 * @brief This class defines the warning error codes and to generate the retry-after field value.
 */
class SIPHeaderUtil
{
private:
    SIPHeaderUtil();

public:
    /**
     * @brief Generates the retry-after header field as second.
     *
     * @param nExtent The value for modular operation; maximum interval value
     * @return A second value for Retry-After header field.
     */
    static IMS_SINT32 GenerateRetryAfterSeconds(IN IMS_SINT32 nExtent = 0);

public:
    /// Warning codes that provides information supplemental to the status code
    /// in SIP response messages when the failure of the transaction results from a SDP problem.
    enum
    {
        WC_INVALID = 0,

        /// Incompatible Network Protocol
        WC_300 = 300,
        /// Incompatible Network Address Formats
        WC_301,
        /// Incompatible Transport Protocol
        WC_302,
        /// Incompatible Bandwidth Units
        WC_303,
        /// Media Type Not Available
        WC_304,
        /// Incompatible Media Format
        WC_305,
        /// Attribute Not Understand
        WC_306,
        /// Session Description Parameter Not Understood
        WC_307,
        /// Multicast Not Available
        WC_330 = 330,
        /// Unicast Not Available
        WC_331,
        /// Insufficient Bandwidth
        WC_370 = 370,
        /// Miscellaneous Warning
        WC_399 = 399,

        WC_MAX
    };
};

#endif // _SIP_HEADER_UTIL_H_
