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
package com.android.imsstack.system;

/**
 * This defines the event types and its related parameters for the operations
 * between the Java and native layer.
 */
public class ImsEventDef {
    //1 RVI to IMS native ----- starts{
    public static final int IMS_EVENT_POWER_LOW_BATTERY = 0x00000001;
        // wParam
        public static final int IMS_POWER_LOW_BATTERY = 0;
        public static final int IMS_POWER_LOW_CHANGED = 1;

    public static final int IMS_EVENT_CSCALL_STATE = 0x00000002;
        // wParam
        public static final int IMS_CSCALL_STATE_IDLE = 0;
        public static final int IMS_CSCALL_STATE_INCOMING = 1;
        public static final int IMS_CSCALL_STATE_ACTIVE = 2;

    public static final int IMS_EVENT_ROAMING_STATE = 0x00000004;
        // wParam (PS roaming), lParam (CS roaming)
        public static final int IMS_ROAMING_STATE_OFF = 0;
        public static final int IMS_ROAMING_STATE_ON = 1;

    public static final int IMS_EVENT_ECM_STATE = 0x00000008;
        // wParam
        public static final int IMS_ECM_STATE_OFF = 0;
        public static final int IMS_ECM_STATE_ON = 1;
        public static final int IMS_ECM_STATE_OFF_BY_NEW_ECALL = 2;

    public static final int IMS_EVENT_VOICE_SERVICE_STATE = 0x00000010;
        // wParam
        public static final int IMS_VOICE_SERVICE_IN_SERVICE = 0;
        public static final int IMS_VOICE_SERVICE_OUT_OF_SERVICE = 1;
        public static final int IMS_VOICE_SERVICE_EMERGENCY_ONLY = 2;
        public static final int IMS_VOICE_SERVICE_POWER_OFF = 3;

    public static final int IMS_EVENT_LTE_INFO = 0x00000020;
    // wParam
    public static final int IMS_LTE_INFO_UNKNOWN = 0;
    public static final int IMS_LTE_INFO_EPS_ONLY_ATTACHED = 1;
    public static final int IMS_LTE_INFO_COMBINED_ATTACHED = 2;
    // lParam
    public static final int IMS_LTE_INFO_EXTRA_NONE = 0x0;
    public static final int IMS_LTE_INFO_EXTRA_CSFB_NOT_PREFERRED = 0x1;
    public static final int IMS_LTE_INFO_EXTRA_SMS_ONLY = 0x2;

    public static final int IMS_EVENT_NR_INFO = 0x00000040;
        // wParam
        public static final int IMS_NR_INFO_UNKNOWN = 0;
        public static final int IMS_NR_INFO_REGISTRATION = 1;
        public static final int IMS_NR_INFO_DEREGISTRATION = 2;
        public static final int IMS_NR_INFO_EMERGENCY_REGISTRATION = 3;

    public static final int IMS_EVENT_IMS_VOICE_OVER_PS_STATE = 0x00000080;
        // wParam
        public static final int IMS_VOICE_OVER_PS_NOT_SUPPORTED = 0;
        public static final int IMS_VOICE_OVER_PS_SUPPORTED = 1;

    public static final int IMS_EVENT_LTE_STATE = 0x00000100;
        // wParam
        public static final int IMS_LTE_RACH_REJECT_WITH_WAITTIME = 11;
            // LParam (waittime : ms)

        public static final int IMS_LTE_RACH_IGNORE_DURING_T300_3TIMES = 12;
        public static final int IMS_LTE_SR_REJECT_WITH_EMM9_EMM10 = 13;
        public static final int IMS_LTE_SR_REJECT_WITH_EMM17_3TIMES = 14;
        public static final int IMS_LTE_SR_IGNORE_DURING_5SEC = 15;
        public static final int IMS_LTE_BARRING_MO_DATA = 16;
        public static final int IMS_LTE_SR_REJECT_WITH_EMM = 17;

        public static final int IMS_LTE_BARRING_SSAC = 18;
        public static final int IMS_LTE_BARRING_SSAC_EX = 19;
            // IMS_LTE_BARRING_SSAC lParam
            public static final int IMS_LTE_BARRING_SSAC_VOICE = 0x00000000;
            public static final int IMS_LTE_BARRING_SSAC_VIDEO = 0x01000000;

        public static final int IMS_LTE_QOS_DEDICATED_BEARER_COMPLETED = 31;
            // LParam (QCI)

        // uicc refresh '0' type
        public static final int IMS_LTE_TRIGGER_DEREGISTRATION = 32;
        // no update ims state(0) for hVoLTE
        public static final int IMS_LTE_DEACTIVATE_IMS_PDN = 33;
        // update current ims reg state for hVoLTE when substate is 0 (CSFB)
        public static final int IMS_LTE_CSFB_PREF_SUB_STATE = 34;
        // update current ims reg state for hVoLTE
        public static final int IMS_LTE_UPDATE_CURRENT_REG_STATE = 35;
        // update current ims reg state for SA
        public static final int IMS_LTE_SA_UPDATE_CURRENT_REG_STATE = 36;

        public static final int IMS_LTE_BLOCK_WITH_TIME = 51;
            // LParam (time : ms)

        //AC Barring - SKT
        public static final int IMS_LTE_ACCESS_BARRED_FOR_MO_DATA = 52;

        //Ims Network Info
        public static final int IMS_LTE_VOPS = 101;
        public static final int IMS_WCDMA_VOPS = 102; // (WCDMA, GSM)
        public static final int IMS_LTE_EMC_BS = 103;
        public static final int IMS_LTE_EMERGENCY_SUPPORT = 104;
        public static final int IMS_LTE_AC_BARRING_FOR_EMERGENCY = 105;

    public static final int IMS_EVENT_WFC_SETTING_CHANGED = 0x00000200;
        // wParam
        public static final int IMS_WFC_ON = 1;
        public static final int IMS_WFC_OFF = 0;
        // lParam
        public static final int MODE_WFC_PREFERRED = 2;
        public static final int MODE_WFC_ONLY = 0;
        public static final int MODE_CELLULAR_PREFERRED = 1;
        public static final int MODE_IMS_PREFERRED = 10;

    public static final int IMS_EVENT_VOLTE_SETTING = 0x00000400;
        // wParam
        public static final int IMS_VOLTE_SETTING_OFF = 0x00000000;
        public static final int IMS_VOLTE_SETTING_ON = 0x00000001;

    public static final int IMS_EVENT_AC_BARRING_STATE = 0x00000800;

    public static final int IMS_EVENT_REG_CONTROL = 0x00001000;
        // wParam
        public static final int IMS_REG_CONTROL_RECOVER = 1;
            // IMS_REG_CONTROL_RECOVER lParam
            public static final int IMS_REG_CONTROL_KEEP_DATA_CONNECTION = 11;

        public static final int IMS_REG_CONTROL_UPDATE = 2;

        public static final int IMS_REG_CONTROL_DESTROY = 3;
            // IMS_REG_CONTROL_DESTROY lParam
            public static final int IMS_REG_CONTROL_DESTROY_DCN = 31;

        public static final int IMS_REG_CONTROL_IPCAN = 4;
            // IMS_REG_CONTROL_IPCAN lParam
            public static final int IMS_REG_CONTROL_IPCAN_STOP = 41;
            public static final int IMS_REG_CONTROL_IPCAN_WIFITOLTE = 42;

        public static final int IMS_REG_CONTROL_STOP = 5;

        public static final int IMS_REG_CONTROL_WIFICALL_STOP = 6;

        public static final int IMS_REG_CONTROL_PCSCF = 7;
            // IMS_REG_CONTROL_PCSCF lParam
            public static final int IMS_REG_CONTROL_PCSCF_SAME_CHANGED = 71;

    public static final int IMS_EVENT_RTT_SETTING = 0x00002000;
        // wParam
        public static final int IMS_RTT_MODE_NONE = -1;
        public static final int IMS_RTT_CAPABLE_OFF = 0;
        public static final int IMS_RTT_VISIBLE_DURING_CALLS = 1;
        public static final int IMS_RTT_ALWAYS_VISIBLE = 2;

    //1  MUST NOT use the above(>=) 0x80000000 from java to native

    //1 RVI to IMS native ----- ends}


    //1 IMS native to RVI ----- starts{
    public static final int IMS_EVENT_NATIVE_BOOT_COMPLETED = 0x00000001;

    public static final int IMS_EVENT_WAKE_LOCK = 0x00000002;

    public static final int IMS_EVENT_WIFI_SERVICE = 0x00000004;
        // wParam
        public static final int IMS_WIFI_OFF = 0;
        public static final int IMS_WIFI_ON = 1;

    //1 IMS native to RVI ----- ends}
}
