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
#ifndef IMS_PRIVATE_PROPERTIES_H_
#define IMS_PRIVATE_PROPERTIES_H_

#include "ImsTypeDef.h"

/**
 * Constant values for IMS private properties.
 */
class ImsPrivateProperties
{
public:
    class Ephemeral
    {
    public:
        static const IMS_CHAR KEY_KR_TTA_VERSION[];
        static const IMS_CHAR KEY_REG_0_BT[];
        static const IMS_CHAR KEY_REG_1_MT[];
        static const IMS_CHAR KEY_REG_2_CF[];
        static const IMS_CHAR KEY_REG_3_UWT[];
        static const IMS_CHAR KEY_REG_4_AWT[];
        static const IMS_CHAR KEY_SMS_NETWORK_REG_BIND[];
        static const IMS_CHAR KEY_THIRD_PARTY_DIALER_FOR_VIDEO_CALL[];
    };

    class Persistent
    {
    public:
        static const IMS_CHAR KEY_SHOW_CODEC_INFO[];
        static const IMS_CHAR KEY_WIFI_TEST[];
        static const IMS_CHAR KEY_CARRIER_SIGNAL_PCO_TEST[];
        static const IMS_CHAR KEY_SIP_DEVICE_ID[];
        // SIM operator / country / operator-sub
        static const IMS_CHAR KEY_SIM_OPERATOR[];
        static const IMS_CHAR KEY_SIM_COUNTRY[];
        static const IMS_CHAR KEY_SIM_OPERATOR_SUB[];

        /**
         * Tracks the configuration items that need to be shared with the native layer.
         *  - ISIM/USIM enabled (that is currently used for IMS registration)
         *  - Primary public user identity (that will be used for IMS registration)
         */
        static const IMS_CHAR KEY_ISIM_ENABLED[];
        static const IMS_CHAR KEY_USIM_ENABLED[];
        static const IMS_CHAR KEY_PRIMARY_IMPU[];

        /**
         * Configuration items which can be provisioned for a test purpose.
         *  - List of P-CSCF address (comma-separated string)
         *  - IMPI (Private user identity)
         *  - List of IMPU (Public user identities) (comma-separated string)
         *  - Home domain name
         */
        static const IMS_CHAR KEY_CONFIG_PCSCF_ADDRESS_LIST[];
        static const IMS_CHAR KEY_CONFIG_IMPI[];
        static const IMS_CHAR KEY_CONFIG_IMPU_LIST[];
        static const IMS_CHAR KEY_CONFIG_HOME_DOMAIN_NAME[];

        /**
         * Test configurations.
         */
        static const IMS_CHAR KEY_TEST_IMS_DEREGISTER[];
        static const IMS_CHAR KEY_TEST_LOG_OPTIONS[];
    };
};

#endif
