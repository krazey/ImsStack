#ifndef _SIP_URN_HELPER_H_
#define _SIP_URN_HELPER_H_

#include "AStringBuffer.h"

/**
 * @brief This class provides an interface to create SIP URN string.
 */
class SIPURNHelper
{
public:
    /// Supported type of URN
    enum
    {
        /// Default URN for IMS services
        GSMA_IMEI = 0,
        /// Hashed value of IMEI - UUID (MD5)
        UUID_IMEI_MD5,
        /// Hashed value of IMEI - UUID (SHA1)
        UUID_IMEI_SHA1,
        /// UUID version 3
        UUID_IMEI_NAMED_V3,
        /// UUID version 5
        UUID_IMEI_NAMED_V5,
        /// UUID version 4
        UUID_IMEI_V4
    };

private:
    SIPURNHelper();

public:
    /**
     * @brief Creates IMEI based URN string.
     *
     * @param nSlotId Slot id
     * @param nType URN type\n
     *              #GSMA_IMEI\n
     *              #UUID_IMEI_MD5\n
     *              #UUID_IMEI_SHA1\n
     *              #UUID_IMEI_NAMED_V3\n
     *              #UUID_IMEI_NAMED_V5\n
     *              #UUID_IMEI_V4
     * @param bSV Flag to indicate whether software version is included or not
     * @return An IMEI URN string.
     */
    static AString GetURN(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nType, IN IMS_BOOL bSV = IMS_TRUE);

    /**
     * @brief Creates UUID based URN string.
     *
     * @param nVersion UUID version\n
     *                 IMSUUID::VERSION_1\n
     *                 IMSUUID::VERSION_2\n
     *                 IMSUUID::VERSION_3\n
     *                 IMSUUID::VERSION_4\n
     *                 IMSUUID::VERSION_5
     * @param strName Any string to be hashed
     * @return An UUID URN string.
     */
    static AString GetURN(IN IMS_SINT32 nVersion, IN CONST AString& strName);

private:
    static const IMS_CHAR IMEI[];
    static const IMS_CHAR IMEI_SV[];
};

#endif  // _SIP_URN_HELPER_H_
