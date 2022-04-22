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
package com.android.imsstack.enabler.aos;

import java.util.Set;
import android.net.Uri;

public interface IAosRegistrationListener {

    /**
     * Notify the application that the device is connected to the IMS network.
     *
     * @param networkType The radio access technology.
     * @param featureTagBits Type of bits an integer.
     * @param featureTags Type of Set<String>.
     * @see {@link NetworkType}
     * @see {@link FeatureTagMask}
     */
    public void notifyRegistered(int networkType, int featureTagBits,
            Set<String> featureTags);
    /**
     * Notify the application that the device is connected to the IMS network.
     *
     * @param networkType The radio access technology.
     * @param featureTagBits Type of bits an integer.
     * @param featureTags Type of Set<String>.
     * @see {@link NetworkType}
     * @see {@link FeatureTagMask}
     */
    public void notifyRegistering(int networkType, int featureTagBits,
            Set<String> featureTags);
    /**
     * Notify the application that the device is disconnected from the IMS network.
     *
     * @param reason associated with why registration was disconnected.
     * @see {@link ReasonCode}
     */
    public void notifyDeregistered(int reason);

    /**
     * Notify the framework that the handover from the current radio technology to the other
     * technology has failed.
     *
     * @param networkType The technology that has failed to be changed to.
     * @param causeCode The handover failure cause.
     * @see {@link NetworkType}
     * @see {@link android.telephony.DataFailCause}
     */
    public void notifyTechnologyChangeFailed(int networkType, int causeCode);

    /**
     * This device's subscriber associated {@link Uri}s have changed, which are used to filter out
     * this device's {@link Uri}s during conference calling.
     *
     * @param uris the network provisioned public user identities.
     */
    public void notifyAssociatedUriChanged(Uri[] uris);

    /**
     * This method is called when capability update fails after
     * {@link IAosRegistration#changeCapabilities} is called.
     *
     * @param capabilities capabilities that failed to update.
     * @param networkType Type of {@link NetworkType}.
     * @param reason Reason for update failure.
     * @see {@link Capability}
     * @see {@link NetworkType}
     * @see {@link CapabilityReason}
     */
    public void notifyCapabilitiesUpdateFailed(int capabilities, int networkType, int reason);

    /**
     * NETWORK_TYPE
     */
    class NetworkType {
        public static final int NONE = -1;
        public static final int LTE = 0;
        public static final int IWLAN = 1;
        public static final int CROSS_SIM = 2;
        public static final int NR = 3;
        public static final int UTRAN = 4;
    }

    /**
     * Capability
     */
    class Capability {

        public static final int NONE = 0;
        public static final int VOICE = 1 << 0;
        public static final int VIDEO = 1 << 1;
        public static final int UT = 1 << 2;
        public static final int SMS = 1 << 3;
        public static final int CALL_COMPOSER = 1 << 4;
        public static final int OPTIONS_UCE = 1 << 5;
        public static final int PRESENCE_UCE = 1 << 6;
    }

    /**
     * FeatureTag Mask
     */
    class FeatureTagMask {

        public static final int NONE = 0;
        /// MTC
        public static final int MMTEL = 0x00000001;
        public static final int VIDEO = 0x00000002;
        public static final int TEXT = 0x00000004;
        public static final int USSI = 0x00000008;
        public static final int VERSTAT = 0x00000010;
        /// MTS
        public static final int SMSIP = 0x00000020;
        /// SIP Controller
        public static final int STANDALONE_MSG = 0x00000040;
        public static final int CHAT_IM = 0x00000080;
        public static final int CHAT_SESSION = 0x00000100;
        public static final int FILE_TRANSFER = 0x00000200;
        public static final int FILE_TRANSFER_VIA_SMS = 0x00000400;
        public static final int CALL_COMPOSER_ENRICHED_CALLING = 0x00000800;
        public static final int CALL_COMPOSER_VIA_TELEPHONY = 0x00001000;
        public static final int POST_CALL = 0x00002000;
        public static final int SHARED_MAP = 0x00004000;
        public static final int SHARED_SKETCH = 0x00008000;
        public static final int GEO_PUSH = 0x00010000;
        public static final int GEO_PUSH_VIA_SMS = 0x00020000;
        public static final int CHATBOT_COMMUNICATION_USING_SESSION = 0x00040000;
        public static final int CHATBOT_COMMUNICATION_USING_STANDALONE_MSG = 0x00080000;
        public static final int CHATBOT_VERSION_SUPPORTED = 0x00100000;
        public static final int CHATBOT_VERSION_V2_SUPPORTED = 0x00200000;
        public static final int CHATBOT_ROLE = 0x00400000;
        public static final int PRESENCE = 0x00800000;
    }

    // TODO : need to check this class.

    /**
     * ReasonCode
     */
    class ReasonCode {

        /**
         * The Reason is unspecified.
         */
        public static final int CODE_UNSPECIFIED = 0;
        /**
         * IMS Registration error code
         */
        public static final int CODE_REGISTRATION_ERROR = 1000;
        /**
         * Indicates the registration attempt on IWLAN failed due to IKEv2 authetication failure
         * during tunnel establishment.
         */
        public static final int CODE_IKEV2_AUTH_FAILURE = 1408;
        /**
         * Call/IMS registration failed/dropped because of a RLF
         */
        public static final int CODE_RADIO_LINK_FAILURE = 1506;
        /**
         * Call/IMS registration failed/dropped because of radio link lost
         */
        public static final int CODE_RADIO_LINK_LOST = 1507;
        /**
         * The call Call/IMS registration failed because of a radio uplink issue
         */
        public static final int CODE_RADIO_UPLINK_FAILURE = 1508;
    }

    /**
     * CapabilityReason
     */
    class CapabilityReason {

        /**
         * The capability was unable to be changed.
         */
        public static final int ERROR_GENERIC = -1;
        /**
         * The capability was able to be changed.
         */
        public static final int SUCCESS = 0;
    }
}
