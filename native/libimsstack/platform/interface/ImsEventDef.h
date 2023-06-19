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
#ifndef IMS_EVENT_DEF_H_
#define IMS_EVENT_DEF_H_

// Java to Native {
#define IMS_EVENT_POWER_LOW_BATTERY                    (0x00000001)
// WParam
#define IMS_POWER_LOW_BATTERY                          (0)
#define IMS_POWER_LOW_CHANGED                          (1)

#define IMS_EVENT_CSCALL_STATE                         (0x00000002)
// WParam
#define IMS_CSCALL_STATE_IDLE                          (0)
#define IMS_CSCALL_STATE_INCOMING                      (1)
#define IMS_CSCALL_STATE_ACTIVE                        (2)
#define IMS_CSCALL_STATE_ACTIVE_E911                   (3)

#define IMS_EVENT_ROAMING_STATE                        (0x00000004)
// WParam (PS roaming), LParam (CS roaming)
#define IMS_ROAMING_STATE_OFF                          (0)
#define IMS_ROAMING_STATE_ON                           (1)

#define IMS_EVENT_ECM_STATE                            (0x00000008)
// WParam
#define IMS_ECM_STATE_OFF                              (0)
#define IMS_ECM_STATE_ON                               (1)
#define IMS_ECM_STATE_OFF_BY_NEW_ECALL                 (2)

#define IMS_EVENT_VOICE_SERVICE_STATE                  (0x00000010)
// WParam
#define IMS_VOICE_SERVICE_IN_SERVICE                   (0)
#define IMS_VOICE_SERVICE_OUT_OF_SERVICE               (1)
#define IMS_VOICE_SERVICE_EMERGENCY_ONLY               (2)
#define IMS_VOICE_SERVICE_POWER_OFF                    (3)

#define IMS_EVENT_LTE_INFO                             (0x00000020)
// WParam
#define IMS_LTE_INFO_UNKNOWN                           (0)
#define IMS_LTE_INFO_EPS_ONLY_ATTACHED                 (1)
#define IMS_LTE_INFO_COMBINED_ATTACHED                 (2)

// LParam
#define IMS_LTE_INFO_EXTRA_NONE                        (0x0)
#define IMS_LTE_INFO_EXTRA_CSFB_NOT_PREFERRED          (0x1)
#define IMS_LTE_INFO_EXTRA_SMS_ONLY                    (0x2)

#define IMS_EVENT_NR_INFO                              (0x00000040)
// WParam
#define IMS_NR_INFO_UNKNOWN                            (0)
#define IMS_NR_INFO_REGISTRATION                       (1)
#define IMS_NR_INFO_DEREGISTRATION                     (2)
#define IMS_NR_INFO_EMERGENCY_REGISTRATION             (3)

#define IMS_EVENT_IMS_VOICE_OVER_PS_STATE              (0x00000080)
// WParam
#define IMS_VOICE_OVER_PS_NOT_SUPPORTED                (0)
#define IMS_VOICE_OVER_PS_SUPPORTED                    (1)

// LTE STATE
#define IMS_EVENT_LTE_STATE                            (0x00000100)
// WParam
#define IMS_LTE_RACH_REJECT_WITH_WAITTIME              (11)  // LParam (waittime : ms)
#define IMS_LTE_RACH_IGNORE_DURING_T300_3TIMES         (12)
#define IMS_LTE_SR_REJECT_WITH_EMM9_EMM10              (13)
#define IMS_LTE_SR_REJECT_WITH_EMM17_3TIMES            (14)
#define IMS_LTE_SR_IGNORE_DURING_5SEC                  (15)
#define IMS_LTE_BARRING_MO_DATA                        (16)
#define IMS_LTE_SR_REJECT_WITH_EMM                     (17)  // LParam (waittime : ms)

#define IMS_EVENT_WFC_SETTING_CHANGED                  (0x00000200)
// WParam
#define IMS_WFC_ON                                     (1)
#define IMS_WFC_OFF                                    (0)
// LParam
#define MODE_WFC_ONLY                                  (0)
#define MODE_CELLULAR_PREFERRED                        (1)
#define MODE_WFC_PREFERRED                             (2)

// Event for modem state
#define IMS_EVENT_VOLTE_SETTING                        (0x00000400)
// WParam
#define IMS_VOLTE_SETTING_OFF                          (0)
#define IMS_VOLTE_SETTING_ON                           (1)
#define IMS_VOLTE_SETTING_UNKNOWN                      (2)

// AC Barring
#define IMS_EVENT_AC_BARRING_STATE                     (0x00000800)

#define IMS_EVENT_REG_CONTROL                          (0x00001000)
// WParam
#define IMS_REG_CONTROL_RECOVER                        (1)
#define IMS_REG_CONTROL_UPDATE                         (2)
#define IMS_REG_CONTROL_DESTROY                        (3)
#define IMS_REG_CONTROL_IPCAN                          (4)
#define IMS_REG_CONTROL_STOP                           (5)
#define IMS_REG_CONTROL_WIFICALL_STOP                  (6)
#define IMS_REG_CONTROL_PCSCF                          (7)
// LParam
// IMS_REG_CONTROL_RECOVER
#define IMS_REG_CONTROL_KEEP_DATA_CONNECTION           (11)
// IMS_REG_CONTROL_DESTROY
#define IMS_REG_CONTROL_DESTROY_DCN                    (31)
// IMS_REG_CONTROL_IPCAN
#define IMS_REG_CONTROL_IPCAN_STOP                     (41)
#define IMS_REG_CONTROL_IPCAN_WIFITOLTE                (42)
// IMS_REG_CONTROL_PCSCF
#define IMS_REG_CONTROL_PCSCF_SAME_CHANGED             (71)

#define IMS_EVENT_RTT_SETTING                          (0x00002000)
// WParam
#define IMS_RTT_MODE_NONE                              (-1)
#define IMS_RTT_CAPABLE_OFF                            (0)
#define IMS_RTT_VISIBLE_DURING_CALLS                   (1)
#define IMS_RTT_ALWAYS_VISIBLE                         (2)

// 1  MUST NOT use the above(>=) 0x80000000 from java to native

// Java to Native }

// Native to Java {

// Notify native booting completed
#define IMS_EVENT_NATIVE_BOOT_COMPLETED                (0x00000001)

#define IMS_EVENT_WAKE_LOCK                            (0x00000002)

// IMS to IMS Phone Manager
#define IMS_EVENT_WIFI_SERVICE                         (0x00000004)
// WParam
#define IMS_WIFI_OFF                                   (0)
#define IMS_WIFI_ON                                    (1)

// Native to Java }

// Macro definition for event parameters
#define IMS_MAKEPARAM(HIWORD, LOWORD)                  (IMS_UINT32)(((HIWORD) << 16) | (LOWORD))

#define IMS_HIWORD(LPARAM)                             (IMS_UINT16)(((LPARAM) >> 16) & (0xFFFF))

#define IMS_LOWORD(LPARAM)                             (IMS_UINT16)((LPARAM)&0xFFFF)

#endif
