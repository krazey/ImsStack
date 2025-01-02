/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef SIP_URN_HELPER_H_
#define SIP_URN_HELPER_H_

#include "AStringBuffer.h"

#include "ISipConfig.h"

/**
 * @brief This class provides an interface to create SIP URN string.
 */
class SipUrnHelper
{
public:
    /// Supported type of URN
    enum
    {
        /// IMEI URN format (Default URN for IMS services)
        GSMA_IMEI = ISipConfig::DEVICE_ID_GSMA_IMEI,
        /// IMEI URN format with "svn" parameter
        GSMA_IMEISV,
        /// Hashed value of IMEI - UUID (MD5)
        UUID_IMEI_MD5,
        /// Hashed value of IMEI - UUID (SHA1)
        UUID_IMEI_SHA1,
        /// UUID version 3
        UUID_IMEI_NAMED_V3
    };

private:
    SipUrnHelper();

public:
    /**
     * @brief Creates IMEI based URN string.
     *
     * @param nSlotId Slot id
     * @param nType URN type\n
     *              #GSMA_IMEI\n
     *              #GSMA_IMEISV\n
     *              #UUID_IMEI_MD5\n
     *              #UUID_IMEI_SHA1\n
     *              #UUID_IMEI_NAMED_V3
     * @return An IMEI URN string.
     */
    static AString GetUrn(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nType);

    /**
     * @brief Creates UUID based URN string.
     *
     * @param nVersion UUID version\n
     *                 ImsUuid::VERSION_1\n
     *                 ImsUuid::VERSION_3\n
     *                 ImsUuid::VERSION_4
     * @param strName Any string to be hashed
     * @return An UUID URN string.
     */
    static AString GetUuidUrn(
            IN IMS_SINT32 nVersion, IN const AString& strName = AString::ConstNull());

private:
    static const IMS_CHAR IMEI[];
    static const IMS_CHAR IMEI_SV[];
};

#endif
