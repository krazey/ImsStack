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
import android.telephony.AccessNetworkConstants.AccessNetworkType;
import android.telephony.TelephonyManager;
import android.telephony.ims.ImsCallForwardInfo;
import android.telephony.ims.ImsSsInfo;
import android.telephony.ims.stub.ImsUtImplBase;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

public class SscConstant {
    public static final int EVENT_SSC_BASE = 13200;

    // Event param for callback
    public static final int EVENT_SSC_QUERY_DOCUMENT = EVENT_SSC_BASE + 1;
    public static final int EVENT_SSC_QUERY_CB = EVENT_SSC_BASE + 2;
    public static final int EVENT_SSC_QUERY_CF = EVENT_SSC_BASE + 3;
    public static final int EVENT_SSC_QUERY_CW = EVENT_SSC_BASE + 4;
    public static final int EVENT_SSC_QUERY_OIR = EVENT_SSC_BASE + 5;
    public static final int EVENT_SSC_QUERY_OIP = EVENT_SSC_BASE + 6;
    public static final int EVENT_SSC_QUERY_TIR = EVENT_SSC_BASE + 7;
    public static final int EVENT_SSC_QUERY_TIP = EVENT_SSC_BASE + 8;

    public static final int EVENT_SSC_UPDATE_CB = EVENT_SSC_BASE + 11;
    public static final int EVENT_SSC_UPDATE_CF = EVENT_SSC_BASE + 12;
    public static final int EVENT_SSC_UPDATE_CW = EVENT_SSC_BASE + 13;
    public static final int EVENT_SSC_UPDATE_OIR = EVENT_SSC_BASE + 14;
    public static final int EVENT_SSC_UPDATE_OIP = EVENT_SSC_BASE + 15;
    public static final int EVENT_SSC_UPDATE_TIR = EVENT_SSC_BASE + 16;
    public static final int EVENT_SSC_UPDATE_TIP = EVENT_SSC_BASE + 17;

    public static final int EVENT_SSC_INSERT_CB = EVENT_SSC_BASE + 21;
    public static final int EVENT_SSC_INSERT_CF = EVENT_SSC_BASE + 22;

    public static final int EVENT_SSC_QUERY_OIR_TB_NETWORK_DEFAULT = EVENT_SSC_BASE + 23;

    // NoReplyTimer value range according to 3GPP 24.604 4.9.2
    public static final int CFNR_TIMER_MIN = 5;
    public static final int CFNR_TIMER_MAX = 180;

    public static final int CONDITION_INVALID = -1;

    // CF Condition Param
    public static final int CONDITION_CFU = ImsCallForwardInfo.CDIV_CF_REASON_UNCONDITIONAL; // 0
    public static final int CONDITION_CFB = ImsCallForwardInfo.CDIV_CF_REASON_BUSY; // 1
    public static final int CONDITION_CFNR = ImsCallForwardInfo.CDIV_CF_REASON_NO_REPLY; // 2
    public static final int CONDITION_CFNRC = ImsCallForwardInfo.CDIV_CF_REASON_NOT_REACHABLE; // 3
    public static final int CONDITION_CFA = ImsCallForwardInfo.CDIV_CF_REASON_ALL; // 4
    public static final int CONDITION_CFAC = ImsCallForwardInfo.CDIV_CF_REASON_ALL_CONDITIONAL; // 5
    public static final int CONDITION_CFNL = ImsCallForwardInfo.CDIV_CF_REASON_NOT_LOGGED_IN; // 6

    // CF Condition internal Param
    public static final int CONDITION_CFNR_TIMER = 10;

    // CB Condition Param
    public static final int CONDITION_BAIC = ImsUtImplBase.CALL_BARRING_ALL_INCOMING; // 1
    public static final int CONDITION_BAOC = ImsUtImplBase.CALL_BARRING_ALL_OUTGOING; // 2
    public static final int CONDITION_BOIC = ImsUtImplBase.CALL_BARRING_OUTGOING_INTL; // 3
    public static final int CONDITION_BOIC_EXHC
            = ImsUtImplBase.CALL_BARRING_OUTGOING_INTL_EXCL_HOME; // 4
    public static final int CONDITION_BIC_WR
            = ImsUtImplBase.CALL_BLOCKING_INCOMING_WHEN_ROAMING; // 5
    public static final int CONDITION_ACR = ImsUtImplBase.CALL_BARRING_ANONYMOUS_INCOMING; // 6

    // 'm' parameter shows the subscriber CLIR/OIR service status in the network from TS 27.007 7.7
    public static final int OIR_NOT_PROVISIONED = ImsSsInfo.CLIR_STATUS_NOT_PROVISIONED; // 0
    //public static final int OIR_PROVISIONED_PERMANENT = ImsSsInfo.CLIR_STATUS_PROVISIONED_PERMANENT;
    //public static final int OIR_UNKNOWN = ImsSsInfo.CLIR_STATUS_UNKNOWN;
    public static final int OIR_TEMPORARY_MODE_PRESENTATION_RESTRICTED
            = ImsSsInfo.CLIR_STATUS_TEMPORARILY_RESTRICTED; // 3
    public static final int OIR_TEMPORARY_MODE_PRESENTATION_ALLOWED
            = ImsSsInfo.CLIR_STATUS_TEMPORARILY_ALLOWED; // 4

    // The 'n' parameter sets the adjustment for outgoing calls from TS 27.007 7.7
    public static final int OIR_DEFAULT = ImsSsInfo.CLIR_OUTGOING_DEFAULT; // 0
    public static final int OIR_INVOCATION = ImsSsInfo.CLIR_OUTGOING_INVOCATION; // 1
    public static final int OIR_SUPPRESSION = ImsSsInfo.CLIR_OUTGOING_SUPPRESSION; // 2

    // TIR provisioned status
    public static final int TIR_UNKNOWN = ImsSsInfo.SERVICE_PROVISIONING_UNKNOWN; // -1
    public static final int TIR_NOT_PROVISIONED = ImsSsInfo.SERVICE_NOT_PROVISIONED ; // 0
    public static final int TIR_PROVISIONED = ImsSsInfo.SERVICE_PROVISIONED; // 1

    // CF, CB Action Param
    public static final int ACTION_DEACTIVATION = 0; // ImsUtInterface.ACTION_DEACTIVATION
    public static final int ACTION_ACTIVATION = 1; // ImsUtInterface.ACTION_ACTIVATION
    public static final int ACTION_REGISTRATION = 3; // ImsUtInterface.ACTION_REGISTRATION
    public static final int ACTION_ERASURE = 4; // ImsUtInterface.ACTION_ERASURE
    public static final int ACTION_INTERROGATION = 5; // ImsUtInterface.ACTION_INTERROGATION

    // CF, CB Status Param
    public static final int STATUS_NOT_REGISTERED = ImsSsInfo.NOT_REGISTERED; // -1
    public static final int STATUS_DISABLE = ImsSsInfo.DISABLED; // 0, Same as ACTION_DEACTIVATION
    public static final int STATUS_ENABLE = ImsSsInfo.ENABLED; // 1, Same as ACTION_ACTIVATION

    // RequestResult Param
    public static final int REQUEST_FAILURE = 0;
    public static final int REQUEST_SUCCESS = 1;

    // HTTP Response Code
    public static final int HTTP_OK = 200;
    public static final int HTTP_UNAUTHORIZED = 401;
    public static final int HTTP_FORBIDDEN = 403;
    public static final int HTTP_CONFLICT = 409;
    public static final int HTTP_PRECONDITION_FAILURE = 412;

    // Temp. Ssc Block List
    public static final int BLOCK_REASON_GBA_FAILURE = 0x00000001;
    public static final int BLOCK_REASON_DNS_QUERY_FAILURE = 0x00000002;
    public static final int BLOCK_REASON_SOCKET_CONNECTION_TIMEOUT = 0x00000004;
    public static final int BLOCK_REASON_PDN_CONNECTION_TIMEOUT = 0x00000008;
    public static final int BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP = 0x00000010;
    public static final int BLOCK_REASON_BY_RESPONSE_CODE_TEMP = 0x00000020;

    // Perm. Ssc Block List
    public static final int BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM = 0x00010000;
    public static final int BLOCK_REASON_BY_RESPONSE_CODE_PERM = 0x00020000;

    public static final int BLOCK_REASON_NONE = 0;

    // Telephony SIM Application type
    public static final int APPTYPE_USIM = TelephonyManager.APPTYPE_USIM; // 2
    public static final int APPTYPE_ISIM = TelephonyManager.APPTYPE_ISIM; // 5

    // AccessNetworkTypes match the radio technologies listed in
    // {@link CarrierConfigManager.ImsSs#KEY_XCAP_OVER_UT_SUPPORTED_RATS_INT_ARRAY}.
    public static final int NETWORK_TYPE_UNKNOWN = AccessNetworkType.UNKNOWN; // 0
    public static final int NETWORK_TYPE_GERAN = AccessNetworkType.GERAN; // 1
    public static final int NETWORK_TYPE_UTRAN = AccessNetworkType.UTRAN; // 2
    public static final int NETWORK_TYPE_EUTRAN = AccessNetworkType.EUTRAN; // 3
    public static final int NETWORK_TYPE_IWLAN = AccessNetworkType.IWLAN; // 5
    public static final int NETWORK_TYPE_NGRAN = AccessNetworkType.NGRAN; // 6

    @IntDef(prefix = {"NETWORK_TYPE_"}, value = {
            NETWORK_TYPE_UNKNOWN,
            NETWORK_TYPE_GERAN,
            NETWORK_TYPE_UTRAN,
            NETWORK_TYPE_EUTRAN,
            NETWORK_TYPE_IWLAN,
            NETWORK_TYPE_NGRAN
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface AccessNetworkTypes {}
}