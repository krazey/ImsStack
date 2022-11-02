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

package com.android.imsstack.enabler.ssc;

import android.telephony.CarrierConfigManager;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigAgent;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Arrays;
import java.util.HashMap;

public final class SscConfig {
    // CarrierConfigManager.OIR_NOT_PROVISIONED;
    public static final int OIR_NOT_PROVISIONED = 0;
    // CarrierConfigManager.OIR_TEMP_MODE_WITHOUT_DEFAULT_BEHAVIOUR;
    public static final int OIR_TEMP_MODE_WITHOUT_DEFAULT_BEHAVIOUR = 1;
    // CarrierConfigManager.OIR_TEMP_MODE_RESTRICTED;
    public static final int OIR_TEMP_MODE_RESTRICTED = 2;
    // CarrierConfigManager.OIR_TEMP_MODE_ALLOWED;
    public static final int OIR_TEMP_MODE_ALLOWED = 3;

    public static final int GBA_ME = CarrierConfigManager.GBA_ME; // 1
    public static final int GBA_U = CarrierConfigManager.GBA_U; // 2
    public static final int GBA_DIGEST = CarrierConfigManager.GBA_DIGEST; // 3

    private static HashMap<Integer, ConfigAgent> mConfigAgent = new HashMap<>();

    public static void init(int slotId) {
        ImsLog.d("init(" + slotId + ")");

        ConfigAgent ca = (ConfigAgent) AgentFactory.getInstance().getAgent(ConfigInterface.class,
                slotId);
        setConfigAgent(slotId, ca);
    }

    @VisibleForTesting
    protected static void setConfigAgent(int slotId, ConfigAgent configAgent) {
        if (configAgent == null) {
            return;
        }

        mConfigAgent.put(slotId, configAgent);
    }

    public static void clear(int slotId) {
        ImsLog.d("clear (" + slotId + ")");

        mConfigAgent.remove(slotId);
    }

    private static String getString(int slotId, String key) {
        ConfigAgent ca = mConfigAgent.get(slotId);
        if (ca == null) {
            return null;
        }

        return ca.getCarrierConfig().getString(key);
    }

    private static boolean getBoolean(int slotId, String key) {
        ConfigAgent ca = mConfigAgent.get(slotId);
        if (ca == null) {
            return false;
        }

        return ca.getCarrierConfig().getBoolean(key);
    }

    private static int getInt(int slotId, String key) {
        ConfigAgent ca = mConfigAgent.get(slotId);
        if (ca == null) {
            return -1;
        }

        return ca.getCarrierConfig().getInt(key);
    }

    private static int[] getIntArray(int slotId, String key) {
        ConfigAgent ca = mConfigAgent.get(slotId);
        if (ca == null) {
            return null;
        }

        return ca.getCarrierConfig().getIntArray(key);
    }

    // From CarrierConfigManager
    public static boolean isUtSupported(int slotId) {
        return getBoolean(slotId, CarrierConfigManager.KEY_CARRIER_SUPPORTS_SS_OVER_UT_BOOL);
    }

    public static String getImsUserAgent(int slotId) {
        return getString(slotId, CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING);
    }

    public static int getGbaMode(int slotId) {
        return getInt(slotId, CarrierConfigManager.KEY_GBA_MODE_INT);
    }

    public static int[] getServerBasedServices(int slotId) {
        return getIntArray(slotId,
                CarrierConfigManager.ImsSs.KEY_UT_SERVER_BASED_SERVICES_INT_ARRAY);
    }

    public static boolean isCsfbSupported(int slotId) {
        return getBoolean(slotId,
                CarrierConfigManager.ImsSs.KEY_USE_CSFB_ON_XCAP_OVER_UT_FAILURE_BOOL);
    }

    public static String getUtServerFqdn(int slotId) {
        return getString(slotId, CarrierConfigManager.ImsSs.KEY_UT_AS_SERVER_FQDN_STRING);
    }

    public static int getUtServerPort(int slotId) {
        return getInt(slotId, CarrierConfigManager.ImsSs.KEY_UT_AS_SERVER_PORT_INT);
    }

    public static int getUtTransportType(int slotId) {
        return getInt(slotId, CarrierConfigManager.ImsSs.KEY_UT_TRANSPORT_TYPE_INT);
    }

    // CarrierConfig
    public static String getAuidPrefix(int slotId) {
        return getString(slotId, CarrierConfig.ImsSs.KEY_XCAP_AUID_PREFIX_STRING);
    }

    public static int[] getSmCausePermBlock(int slotId) {
        return getIntArray(slotId, CarrierConfig.ImsSs.KEY_UT_SM_CAUSE_PERMANENT_BLOCK_INT_ARRAY);
    }

    public static int[] getHttpPermBlockErrorCodes(int slotId) {
        return getIntArray(slotId, CarrierConfig.ImsSs.KEY_UT_HTTP_PERMANENT_ERROR_CODE_INT_ARRAY);
    }

    // Asset
    protected static boolean isSrvRecordsRequired(int slotId) {
        return getBoolean(slotId, CarrierConfig.Assets.KEY_XCAP_ROOT_URI_REQUIRES_SRV_QUERY_BOOL);
    }

    protected static int[] getSmCauseTempBlock(int slotId) {
        return getIntArray(slotId, CarrierConfig.Assets.KEY_UT_SM_CAUSE_TEMPORARY_BLOCK_INT_ARRAY);
    }

    protected static int[] getHttpTempBlockErrorCodes(int slotId) {
        return getIntArray(slotId, CarrierConfig.Assets.KEY_UT_HTTP_TEMPORARY_ERROR_CODE_INT_ARRAY);
    }

    protected static int getMaxRetryCount(int slotId) {
        return getInt(slotId, CarrierConfig.Assets.KEY_UT_MAX_RETRY_COUNT_INT);
    }

    protected static int getTimerForTempBlock(int slotId) {
        return getInt(slotId, CarrierConfig.Assets.KEY_UT_TEMPORARY_BLOCK_TIMER_MIN_INT) * 60
                * 1000;
    }

    protected static boolean isCfActionErasureSupported(int slotId) {
        return getBoolean(slotId, CarrierConfig.Assets.KEY_UT_SUPPORT_CF_ACTION_ERASURE_BOOL);
    }

    protected static boolean isCfQueryAllAndCfAllConditionalSupported(int slotId) {
        return getBoolean(slotId,
                CarrierConfig.Assets.KEY_UT_QUERY_CF_ALL_AND_CF_ALL_CONDITIONAL_SUPPORT_BOOL);
    }

    protected static int getOirNetworkDefaultOperation(int slotId) {
        return getInt(slotId, CarrierConfig.Assets.KEY_UT_OIR_NETWORK_DEFAULT_OPERATION_INT);
    }

    protected static boolean isOirTirAlwaysTemporaryMode(int slotId) {
        return getBoolean(slotId,
                CarrierConfig.Assets.KEY_UT_OIR_TIR_ALWAYS_TEMPORARY_MODE_BOOL);
    }

    protected static int getTimerForTempBlockWithAnyReason(int slotId) {
        return getInt(slotId,
                CarrierConfig.Assets.KEY_UT_TEMPORARY_BLOCK_TIMER_WITH_ANY_REASON_SEC_INT) * 1000;
    }

    protected static String getPhoneContextForTargetAddress(int slotId) {
        return getString(slotId, CarrierConfig.Assets.KEY_UT_TARGET_ADDRESS_PHONE_CONTEXT_STRING);
    }

    protected static String getCountryCodeToReplaceCountryCodeWithZero(int slotId) {
        return getString(slotId,
                CarrierConfig.Assets.KEY_UT_TARGET_ADDRESS_COUNTRY_CODE_REPLACE_TO_ZERO_STRING);
    }

    protected static String getCountryCodeToReplaceZeroWithCountryCode(int slotId) {
        return getString(slotId,
                CarrierConfig.Assets.KEY_UT_TARGET_ADDRESS_ZERO_REPLACE_TO_COUNTRY_CODE_STRING);
    }

    protected static boolean isOmitNamespaceOfDocumentElement(int slotId) {
        return getBoolean(slotId,
                CarrierConfig.Assets.KEY_UT_OMIT_NAMESPACE_OF_DOCUMENT_ELEMENT_BOOL);
    }

    protected static boolean isOmitNamespaceSs(int slotId) {
        return getBoolean(slotId, CarrierConfig.Assets.KEY_UT_OMIT_NAMESPACE_SS_BOOL);
    }

    protected static boolean isOmitNamespaceCp(int slotId) {
        return getBoolean(slotId, CarrierConfig.Assets.KEY_UT_OMIT_NAMESPACE_CP_BOOL);
    }

    /* TODO: This isn't used now.
    public static boolean isHostHeaderRequiresPortNumber(int slotId) {
        return getBoolean(slotId,
                CarrierConfig.Assets.KEY_UT_HOST_HEADER_REQUIRES_PORT_NUMBER_BOOL);
    }
    */

    protected static int getXcapApnInactivityTimer(int slotId) {
        return getInt(slotId, CarrierConfig.Assets.KEY_UT_XCAP_APN_INACTIVITY_TIMER_SEC_INT) * 1000;
    }

    protected static boolean isErrorPhraseDisplayedWith409(int slotId) {
        return getBoolean(slotId,
                CarrierConfig.Assets.KEY_UT_DISPLAY_ERROR_PHRASE_WITH_409_ERROR_BOOL);
    }

    protected static boolean insertNewRule(int slotId) {
        return getBoolean(slotId, CarrierConfig.Assets.KEY_UT_INSERT_NEW_RULE_BOOL);
    }

    // Specific APIs
    protected static boolean isCfnlOverUtSupported(int slotId) {
        int[] serverBasedServices = getServerBasedServices(slotId);
        if (serverBasedServices == null) {
            return false;
        }

        return Arrays.stream(serverBasedServices).anyMatch(value -> {
            return value == CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CF_CFNL;
        });
    }

    protected static boolean isTls(int slotId) {
        return getUtTransportType(slotId) == CarrierConfigManager.Ims.PREFERRED_TRANSPORT_TLS;
    }

    protected static boolean isPermanentBlockSmCause(int slotId, int smCause) {
        int[] permBlockCauses = getSmCausePermBlock(slotId);
        if (permBlockCauses == null) {
            return false;
        }

        return Arrays.stream(permBlockCauses).anyMatch(value -> {
            return value == smCause;
        });
    }

    protected static boolean isTemporaryBlockSmCause(int slotId, int smCause) {
        int[] tempBlockCauses = getSmCauseTempBlock(slotId);
        if (tempBlockCauses == null) {
            return false;
        }

        return Arrays.stream(tempBlockCauses).anyMatch(value -> {
            if (value == 0) {
                return true;
            }
            return value == smCause;
        });
    }

    protected static boolean isPermanentErrorCode(int slotId, int responseCode) {
        int[] permBlockCodes = getHttpPermBlockErrorCodes(slotId);
        if (permBlockCodes == null) {
            return false;
        }

        return Arrays.stream(permBlockCodes).anyMatch(value -> {
            if (value % 100 == 99) { // N99 case
                if ((value / 100) == (responseCode / 100)) {
                    return true;
                }
            }
            return value == responseCode;
        });
    }

    protected static boolean isTemporaryErrorCode(int slotId, int responseCode) {
        int[] tempBlockCodes = getHttpTempBlockErrorCodes(slotId);
        if (tempBlockCodes == null) {
            return false;
        }

        return Arrays.stream(tempBlockCodes).anyMatch(value -> {
            if (value % 100 == 99) { // N99 case
                if ((value / 100) == (responseCode / 100)) {
                    return true;
                }
            }
            return value == responseCode;
        });
    }

    protected static boolean isGbaSupported(int slotId) {
        int gbaType = SscConfig.getGbaMode(slotId);
        if (gbaType == GBA_ME || gbaType == GBA_U) {
            return true;
        }

        return false;
    }

    // Temporary APIs
    public static boolean isTrustAllHosts(int slotId) {
        // TODO: Is this functation really needed?
        return false;
    }

    public static String getTargetAddrScheme(int slotId) {
        // TODO: Is this functation really needed?
        return "tel";
    }

    public static String getUtPdnType(int slotId) {
        // TODO: could be removed?
        return "mobile_xcap";
    }

    /* TODO: If there is an error, this method will be used.
    public static boolean isSetPortInHostHeader(int slotId) {
        return ImsGlobal.isOperator(slotId, "TLS") ? false : true;
    }
     */

    /*
    public static boolean isIgnoreActiveInOir(int slotId) {
        if (ImsGlobal.isOperatorCountry(slotId, "TMO", "US") || ImsGlobal.isOperator(slotId, "MPCS")) {
            return true;
        }

        if (ImsGlobal.isOperator(slotId, "MBK")) {
            return true;
        }
        return false;
    }

    public static String getDefaultOIR(int slotId) {
        if (ImsGlobal.isOperator(slotId, "MBK")) {
            return SscXmlFormat.PRESENTATION_NOT_RESTRICTED;
        }

        return null;
    }
     */

    public static boolean isPdnConnCheckedByDataState(int slotId) {
        // TODO:
        //return ImsGlobal.isOperatorCountry(slotId, "RJIL", "IN")  ? true : false;
        return false;
    }

    /*
    // Will be added to Carrier Config
    // getHttpTempBlockErrorCodes
    public static boolean isTempUtBlockCode(int slotId, int responseCode) {
        if (ImsGlobal.isOperator(slotId, "ORG") || ImsGlobal.isOperator(slotId, "NJU")
                || ImsGlobal.isOperator(slotId, "TLS")) {
            if (responseCode == 403 || (responseCode >= 500 && responseCode < 600)) {
                return true;
            }
            return false;
        }

        return false;
    }
     */

    /*
    // Will be added to Carrier Config
    // getHttpPermBlockErrorCodes
    public static boolean isPermUtBlockCode(int slotId, int responseCode) {
        if (ImsGlobal.isOperator(slotId, "ATT") || ImsGlobal.isOperator(slotId, "XPM")
                || ImsGlobal.isOperator(slotId, "KDDI")
                || ImsGlobal.isOperatorCountry(slotId, "SAA", "AS")) {
            return false; // do not block the UT
        } else if (ImsGlobal.isOperator(slotId, "VDF") || ImsGlobal.isOperator(slotId, "TEL")) {
            if (responseCode == 403 || (responseCode >= 500 && responseCode < 600)) {
                return true;
            }

            if (ImsGlobal.isOperatorCountry(slotId, "VDF", "CH")) {
                if (responseCode == 404) {
                    return true;
                }
            }

            return false;
        } else if (ImsGlobal.isOperator(slotId, "EEO")) {
            if (responseCode == 404 || responseCode == 408) {
                return true;
            }

            return false;
        }

        return (responseCode == 403) ? true : false; // 3GPP 24.623 5.3.1.2.2
    }
     */

    /*
    // Will be added to Carrier Config
    // getTimerForTempBlock
    public static int getUtBlockTimerByResponseCode(int slotId) {
        final int minute = 60 * 1000;
        if (ImsGlobal.isOperator(slotId, "ORG") || ImsGlobal.isOperator(slotId, "NJU")) {
            return 4 * 60 * minute; // 4 hours
        } else if (ImsGlobal.isOperator(slotId, "TLS")) {
            return 5 * minute; // 5 minutes
        }

        return minute; // 1 minute
    }
     */

    /*
    // Asset - KEY_UT_XCAP_APN_INACTIVITY_TIMER_SECOND_INT
    // getXcapApnInactivityTimer
    public static long getConnectionInactivityTimer(int slotId) {
        if (ImsGlobal.isOperator(slotId, "KDDI")) {
            return 300 * 1000;
        } else if (ImsGlobal.isOperator(slotId, "VDF")) {
            return 10 * 1000;
        } else if (ImsGlobal.isOperator(slotId, "EEO")) {
            return 240 * 1000;
        }

        // following IR92.v13 4.3.1
        return 120 * 1000;
    }
     */
}