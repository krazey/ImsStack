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

import android.annotation.IntDef;
import android.telephony.CarrierConfigManager;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.ssc.SscConstant.AccessNetworkTypes;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
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

    public static final int SERVICE_TYPE_INVALID = -1;
    public static final int SERVICE_TYPE_CW =
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CW; // 0
    public static final int SERVICE_TYPE_CFA =
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CF_ALL; // 1
    public static final int SERVICE_TYPE_CFU =
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CF_CFU; // 2
    public static final int SERVICE_TYPE_CFAC =
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CF_ALL_CONDITONAL_FORWARDING; // 3
    public static final int SERVICE_TYPE_CFB =
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CF_CFB; // 4
    public static final int SERVICE_TYPE_CFNRY =
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CF_CFNRY; // 5
    public static final int SERVICE_TYPE_CFNRC =
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CF_CFNRC; // 6
    public static final int SERVICE_TYPE_CFNL =
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CF_CFNL; // 7
    public static final int SERVICE_TYPE_OIP =
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIP; // 8
    public static final int SERVICE_TYPE_TIP =
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_TIP; // 9
    public static final int SERVICE_TYPE_OIR =
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIR; // 10
    public static final int SERVICE_TYPE_TIR =
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_TIR; // 11
    public static final int SERVICE_TYPE_BAOC =
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BAOC; // 14
    public static final int SERVICE_TYPE_BOIC =
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BOIC; // 15
    public static final int SERVICE_TYPE_BOIC_EXHC =
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BOIC_EXHC; // 16
    public static final int SERVICE_TYPE_BAIC =
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BAIC; // 18
    public static final int SERVICE_TYPE_BIC_ROAM =
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BIC_ROAM; // 19
    public static final int SERVICE_TYPE_ACR =
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_ACR; // 20

    public static final int URI_TYPE_TEL = 0;
    public static final int URI_TYPE_SIP = 1;

    @IntDef(prefix = {"SERVICE_TYPE_"}, value = {
            SERVICE_TYPE_INVALID,
            SERVICE_TYPE_CW,
            SERVICE_TYPE_CFA,
            SERVICE_TYPE_CFU,
            SERVICE_TYPE_CFAC,
            SERVICE_TYPE_CFB,
            SERVICE_TYPE_CFNRY,
            SERVICE_TYPE_CFNRC,
            SERVICE_TYPE_CFNL,
            SERVICE_TYPE_OIP,
            SERVICE_TYPE_TIP,
            SERVICE_TYPE_OIR,
            SERVICE_TYPE_TIR,
            SERVICE_TYPE_BAOC,
            SERVICE_TYPE_BOIC,
            SERVICE_TYPE_BOIC_EXHC,
            SERVICE_TYPE_BAIC,
            SERVICE_TYPE_BIC_ROAM,
            SERVICE_TYPE_ACR
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface CarrierConfigServiceType {}

    private static HashMap<Integer, ConfigInterface> sConfigInterfaces = new HashMap<>();

    public static void init(int slotId) {
        ImsLog.d("init(" + slotId + ")");

        ConfigInterface ci = AgentFactory.getInstance().getAgent(ConfigInterface.class, slotId);
        setConfigInterface(slotId, ci);
    }

    @VisibleForTesting
    static void setConfigInterface(int slotId, ConfigInterface configInterface) {
        if (configInterface == null) {
            return;
        }

        sConfigInterfaces.put(slotId, configInterface);
    }

    public static void clear(int slotId) {
        ImsLog.d("clear (" + slotId + ")");

        sConfigInterfaces.remove(slotId);
    }

    private static String getString(int slotId, String key) {
        CarrierConfig cc = getCarrierConfig(slotId);
        if (cc == null) {
            return null;
        }

        return cc.getString(key);
    }

    private static boolean getBoolean(int slotId, String key) {
        CarrierConfig cc = getCarrierConfig(slotId);
        if (cc == null) {
            return false;
        }

        return cc.getBoolean(key);
    }

    private static int getInt(int slotId, String key) {
        CarrierConfig cc = getCarrierConfig(slotId);
        if (cc == null) {
            return -1;
        }

        return cc.getInt(key);
    }

    private static int[] getIntArray(int slotId, String key) {
        CarrierConfig cc = getCarrierConfig(slotId);
        if (cc == null) {
            return null;
        }

        return cc.getIntArray(key);
    }

    private static CarrierConfig getCarrierConfig(int slotId) {
        ConfigInterface ci = sConfigInterfaces.get(slotId);
        if (ci == null) {
            return null;
        }

        return ci.getCarrierConfig();
    }

    // From CarrierConfigManager
    static boolean isUtSupported(int slotId) {
        return getBoolean(slotId, CarrierConfigManager.KEY_CARRIER_SUPPORTS_SS_OVER_UT_BOOL);
    }

    static String getImsUserAgent(int slotId) {
        return getString(slotId, CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING);
    }

    static int getGbaMode(int slotId) {
        return getInt(slotId, CarrierConfigManager.KEY_GBA_MODE_INT);
    }

    static boolean isCrossSimFeatureEnabled(int slotId) {
        return getBoolean(slotId, CarrierConfigManager.KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL);
    }

    static boolean isImsRegistrationRequired(int slotId) {
        return getBoolean(slotId, CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL);
    }

    static boolean isCsfbSupported(int slotId) {
        return getBoolean(slotId,
                CarrierConfigManager.ImsSs.KEY_USE_CSFB_ON_XCAP_OVER_UT_FAILURE_BOOL);
    }

    static boolean isUtSupportedWhenPsDataOff(int slotId) {
        return getBoolean(slotId,
                CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_PS_DATA_OFF_BOOL);
    }

    static boolean isUtSupportedWhenRoaming(int slotId) {
        return getBoolean(slotId, CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_ROAMING_BOOL);
    }

    static int getUtServerPort(int slotId) {
        return getInt(slotId, CarrierConfigManager.ImsSs.KEY_UT_AS_SERVER_PORT_INT);
    }

    static int getUtTransportType(int slotId) {
        return getInt(slotId, CarrierConfigManager.ImsSs.KEY_UT_TRANSPORT_TYPE_INT);
    }

    static int[] getServerBasedServices(int slotId) {
        return getIntArray(slotId,
                CarrierConfigManager.ImsSs.KEY_UT_SERVER_BASED_SERVICES_INT_ARRAY);
    }

    static int[] getTerminalBasedServices(int slotId) {
        return getIntArray(slotId,
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY);
    }

    static int[] getSupportedRats(int slotId) {
        return getIntArray(slotId,
                CarrierConfigManager.ImsSs.KEY_XCAP_OVER_UT_SUPPORTED_RATS_INT_ARRAY);
    }

    static String getUtServerFqdn(int slotId) {
        return getString(slotId, CarrierConfigManager.ImsSs.KEY_UT_AS_SERVER_FQDN_STRING);
    }

    // CarrierConfig
    static String getAuidPrefix(int slotId) {
        return getString(slotId, CarrierConfig.ImsSs.KEY_XCAP_AUID_PREFIX_STRING);
    }

    static int[] getSmCausePermBlock(int slotId) {
        return getIntArray(slotId, CarrierConfig.ImsSs.KEY_UT_SM_CAUSE_PERMANENT_BLOCK_INT_ARRAY);
    }

    static int[] getHttpPermBlockErrorCodes(int slotId) {
        return getIntArray(slotId, CarrierConfig.ImsSs.KEY_UT_HTTP_PERMANENT_ERROR_CODE_INT_ARRAY);
    }

    static int[] getSmCauseTempBlock(int slotId) {
        return getIntArray(slotId, CarrierConfig.ImsSs.KEY_UT_SM_CAUSE_TEMPORARY_BLOCK_INT_ARRAY);
    }

    static int[] getHttpTempBlockErrorCodes(int slotId) {
        return getIntArray(slotId, CarrierConfig.ImsSs.KEY_UT_HTTP_TEMPORARY_ERROR_CODE_INT_ARRAY);
    }

    static int getMaxRetryCount(int slotId) {
        return getInt(slotId, CarrierConfig.ImsSs.KEY_UT_MAX_RETRY_COUNT_INT);
    }

    static int getTimerForTempBlock(int slotId) {
        return getInt(slotId, CarrierConfig.ImsSs.KEY_UT_TEMPORARY_BLOCK_TIMER_MIN_INT) * 60
                * 1000;
    }

    static boolean isCfActionErasureSupported(int slotId) {
        return getBoolean(slotId, CarrierConfig.ImsSs.KEY_UT_SUPPORT_CF_ACTION_ERASURE_BOOL);
    }

    static boolean isCfnrTimerSupported(int slotId) {
        return getBoolean(slotId, CarrierConfig.ImsSs.KEY_UT_SUPPORT_CFNR_TIMER_BOOL);
    }

    static boolean isCfQueryAllAndCfAllConditionalSupported(int slotId) {
        return getBoolean(slotId,
                CarrierConfig.ImsSs.KEY_UT_QUERY_CF_ALL_AND_CF_ALL_CONDITIONAL_SUPPORT_BOOL);
    }

    static int getOirNetworkDefaultOperation(int slotId) {
        return getInt(slotId, CarrierConfig.ImsSs.KEY_UT_OIR_NETWORK_DEFAULT_OPERATION_INT);
    }

    static boolean isOirTirAlwaysTemporaryMode(int slotId) {
        return getBoolean(slotId,
                CarrierConfig.ImsSs.KEY_UT_OIR_TIR_ALWAYS_TEMPORARY_MODE_BOOL);
    }

    static int getTimerForTempBlockWithAnyReason(int slotId) {
        return getInt(slotId,
                CarrierConfig.ImsSs.KEY_UT_TEMPORARY_BLOCK_TIMER_WITH_ANY_REASON_SEC_INT) * 1000;
    }

    static String getPhoneContextForTargetAddress(int slotId) {
        return getString(slotId, CarrierConfig.ImsSs.KEY_UT_TARGET_ADDRESS_PHONE_CONTEXT_STRING);
    }

    static String getCountryCodeToReplaceCountryCodeWithZero(int slotId) {
        return getString(slotId,
                CarrierConfig.ImsSs.KEY_UT_TARGET_ADDRESS_COUNTRY_CODE_REPLACE_TO_ZERO_STRING);
    }

    static String getCountryCodeToReplaceZeroWithCountryCode(int slotId) {
        return getString(slotId,
                CarrierConfig.ImsSs.KEY_UT_TARGET_ADDRESS_ZERO_REPLACE_TO_COUNTRY_CODE_STRING);
    }

    static boolean isOmitNamespaceOfDocumentElement(int slotId) {
        return getBoolean(slotId,
                CarrierConfig.ImsSs.KEY_UT_OMIT_NAMESPACE_OF_DOCUMENT_ELEMENT_BOOL);
    }

    static boolean isOmitNamespaceSs(int slotId) {
        return getBoolean(slotId, CarrierConfig.ImsSs.KEY_UT_OMIT_NAMESPACE_SS_BOOL);
    }

    static boolean isOmitNamespaceCp(int slotId) {
        return getBoolean(slotId, CarrierConfig.ImsSs.KEY_UT_OMIT_NAMESPACE_CP_BOOL);
    }

    static int getXcapApnInactivityTimer(int slotId) {
        return getInt(slotId, CarrierConfig.ImsSs.KEY_UT_XCAP_APN_INACTIVITY_TIMER_SEC_INT) * 1000;
    }

    static boolean isErrorPhraseDisplayedWith409(int slotId) {
        return getBoolean(slotId,
                CarrierConfig.ImsSs.KEY_UT_DISPLAY_ERROR_PHRASE_WITH_409_ERROR_BOOL);
    }

    static boolean insertNewRule(int slotId) {
        return getBoolean(slotId, CarrierConfig.ImsSs.KEY_UT_INSERT_NEW_RULE_BOOL);
    }

    static int getUriTypeForCfTargetNumber(int slotId) {
        return getInt(slotId, CarrierConfig.ImsSs.KEY_UT_URI_TYPE_FOR_CF_TARGET_NUMBER_INT);
    }

    static String getNafFqdn(int slotId) {
        return getString(slotId, CarrierConfig.ImsSs.KEY_UT_NAF_FQDN_STRING);
    }

    static int getUtTransactionTimer(int slotId) {
        return getInt(slotId, CarrierConfig.ImsSs.KEY_UT_TRANSACTION_TIMER_SEC_INT);
    }

    static boolean isSyncWithCsForTbSs(int slotId) {
        return getBoolean(slotId, CarrierConfig.ImsSs.KEY_UT_SYNC_WITH_CS_FOR_TB_SS_BOOL);
    }

    static boolean isNetworkQueryForTbOirNetworkDefault(int slotId) {
        return getBoolean(slotId,
                CarrierConfig.ImsSs.KEY_UT_NETWORK_QUERY_FOR_TB_OIR_NETWORK_DEFAULT_BOOL);
    }

    // Specific APIs
    static boolean isGbaSupported(int slotId) {
        int gbaType = SscConfig.getGbaMode(slotId);

        return (gbaType == GBA_ME) || (gbaType == GBA_U);
    }

    static boolean isTls(int slotId) {
        return getUtTransportType(slotId) == CarrierConfigManager.Ims.PREFERRED_TRANSPORT_TLS;
    }

    static boolean isServerBasedService(int slotId, @CarrierConfigServiceType int serviceType) {
        int[] serverBasedServices = getServerBasedServices(slotId);
        if (serverBasedServices == null) {
            return false;
        }

        return Arrays.stream(serverBasedServices).anyMatch(value -> value == serviceType);
    }

    static boolean isTerminalBasedService(int slotId, @CarrierConfigServiceType int serviceType) {
        int[] terminalBasedServices = getTerminalBasedServices(slotId);
        if (terminalBasedServices == null) {
            return false;
        }

        return Arrays.stream(terminalBasedServices).anyMatch(value -> value == serviceType);
    }

    static boolean isSupportedNetwork(int slotId, @AccessNetworkTypes int networkType) {
        int[] serverBasedServices = getSupportedRats(slotId);
        if (serverBasedServices == null) {
            return false;
        }

        return Arrays.stream(serverBasedServices).anyMatch(value -> value == networkType);
    }

    static boolean isPermanentBlockSmCause(int slotId, int smCause) {
        int[] permBlockCauses = getSmCausePermBlock(slotId);
        if (permBlockCauses == null) {
            return false;
        }

        return Arrays.stream(permBlockCauses).anyMatch(value -> value == smCause);
    }

    static boolean isTemporaryBlockSmCause(int slotId, int smCause) {
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

    static boolean isPermanentErrorCode(int slotId, int responseCode) {
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

    static boolean isTemporaryErrorCode(int slotId, int responseCode) {
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
}