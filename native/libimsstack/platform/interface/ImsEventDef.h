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
#define IMS_EVENT_POWER_LOW_BATTERY                    (0x00000008)
// WParam
#define IMS_POWER_LOW_BATTERY                          (0)
#define IMS_POWER_LOW_CHANGED                          (1)

#define IMS_EVENT_PHONE_NUMBER_AVAILABLE               (0x00000010)
// WParam
#define IMS_PHONE_NUMBER_INITIAL                       (0)
#define IMS_PHONE_NUMBER_REFRESH                       (1)
// LParam
#define IMS_PHONE_NUMBER_SIM_LOADED                    (0)
#define IMS_PHONE_NUMBER_RETRY_SUCCESS                 (1)
#define IMS_PHONE_NUMBER_RETRY_FAILURE                 (2)

#define IMS_EVENT_CSCALL_STATE                         (0x00000020)
// WParam
#define IMS_CSCALL_STATE_IDLE                          (0)
#define IMS_CSCALL_STATE_INCOMING                      (1)
#define IMS_CSCALL_STATE_ACTIVE                        (2)
#define IMS_CSCALL_STATE_ACTIVE_E911                   (3)

#define IMS_EVENT_CONFIG_UPDATE                        (0x00000040)
// WParam (HIWORD : CAT, LOWORD : Item ID)
// This is a category for operator specific
#define IMS_CONFIG_CAT_0                               (0)
#define IMS_CONFIG_CAT_1                               (1)
// This is a category for service common (e.g. RCS provisioning, APCS, ...)
#define IMS_CONFIG_CAT_100                             (100)
// TTA DM
#define IMS_CONFIG_CAT_103                             (103)
// This is a category for engine (do not use the value greater than 10000)
#define IMS_CONFIG_CAT_10002                           (10002)

// VZW
#define IMS_EVENT_DATA_MODE                            (0x00000080)
// WParam
#define IMS_DATA_MODE_ALLOWED                          (0)
#define IMS_DATA_MODE_BLOCKED                          (1)

#define IMS_EVENT_RADIO_OFF                            (0x00000100)

#define IMS_EVENT_ROAMING_STATE                        (0x00000200)
// WParam (PS roaming), LParam (CS roaming)
#define IMS_ROAMING_STATE_OFF                          (0)
#define IMS_ROAMING_STATE_ON                           (1)

#define IMS_EVENT_ROAMING_TYPE                         (0x00000300)
// WParam
#define IMS_ROAMING_TYPE_VOICE                         (0)
#define IMS_ROAMING_TYPE_DATA                          (1)
// LParam
#define ROAMING_TYPE_NOT_ROAMING                       (0)
#define ROAMING_TYPE_UNKNOWN                           (1)
#define ROAMING_TYPE_DOMESTIC                          (2)
#define ROAMING_TYPE_INTERNATIONAL                     (3)

#define IMS_EVENT_ECM_STATE                            (0x00000400)
// WParam
#define IMS_ECM_STATE_OFF                              (0)
#define IMS_ECM_STATE_ON                               (1)
#define IMS_ECM_STATE_OFF_BY_NEW_ECALL                 (2)

#define IMS_EVENT_VOICE_SERVICE_STATE                  (0x00000500)
// WParam
#define IMS_VOICE_SERVICE_IN_SERVICE                   (0)
#define IMS_VOICE_SERVICE_OUT_OF_SERVICE               (1)
#define IMS_VOICE_SERVICE_EMERGENCY_ONLY               (2)
#define IMS_VOICE_SERVICE_POWER_OFF                    (3)

#define IMS_EVENT_LTE_INFO                             (0x00000600)
// WParam
#define IMS_LTE_INFO_UNKNOWN                           (0)
#define IMS_LTE_INFO_DETACHED                          (1)
#define IMS_LTE_INFO_EPS_ONLY_ATTACHED                 (2)
#define IMS_LTE_INFO_NORMAL_ATTACHED                   (3)
#define IMS_LTE_INFO_EMERGENCY_ATTACHED                (4)
#define IMS_LTE_INFO_REATTACH_REQUIRED                 (5)

// LParam
#define IMS_LTE_INFO_UPDATE_RESULT_NO_ADD_INFO         (0)
#define IMS_LTE_INFO_UPDATE_RESULT_CSFB_NOT_PREFERRED  (1)
#define IMS_LTE_INFO_UPDATE_RESULT_SMS_ONLY            (2)
#define IMS_LTE_INFO_UPDATE_RESULT_RESERVED            (3)

#define IMS_EVENT_NR_INFO                              (0x00000700)
// WParam
#define IMS_NR_INFO_UNKNOWN                            (0)
#define IMS_NR_INFO_REGISTRATION                       (1)
#define IMS_NR_INFO_DEREGISTRATION                     (2)
#define IMS_NR_INFO_EMERGENCY_REGISTRATION             (3)

#define IMS_EVENT_IMS_VOICE_OVER_PS_STATE              (0x00000800)
// WParam
#define IMS_VOICE_OVER_PS_NOT_SUPPORTED                (0)
#define IMS_VOICE_OVER_PS_SUPPORTED                    (1)

// LTE STATE
#define IMS_EVENT_LTE_STATE                            (0x00001000)
// WParam
#define IMS_LTE_RACH_REJECT_WITH_WAITTIME              (11)  // LParam (waittime : ms)
#define IMS_LTE_RACH_IGNORE_DURING_T300_3TIMES         (12)
#define IMS_LTE_SR_REJECT_WITH_EMM9_EMM10              (13)
#define IMS_LTE_SR_REJECT_WITH_EMM17_3TIMES            (14)
#define IMS_LTE_SR_IGNORE_DURING_5SEC                  (15)
#define IMS_LTE_BARRING_MO_DATA                        (16)
#define IMS_LTE_SR_REJECT_WITH_EMM                     (17)  // LParam (waittime : ms)
#define IMS_LTE_BARRING_SSAC                           (18)  // LParam (?)
#define IMS_LTE_BARRING_SSAC_EX                        (19)  // LParam (?)
#define IMS_LTE_QOS_DEDICATED_BEARER_COMPLETED         (31)  // LParam (QCI)
#define IMS_LTE_TRIGGER_DEREGISTRATION                 (32)  // uicc refresh '0' type
#define IMS_LTE_DEACTIVATE_IMS_PDN                     (33)  // no update ims state(0) for hVoLTE
// update current ims reg state for hVoLTE (state 0(CSFB))
#define IMS_LTE_CSFB_PREF_SUB_STATE                    (34)
// update current ims reg state for hVoLTE
#define IMS_LTE_UPDATE_CURRENT_REG_STATE               (35)
#define IMS_LTE_SA_UPDATE_CURRENT_REG_STATE            (36)  // update current ims reg state for SA
#define IMS_LTE_BLOCK_WITH_TIME                        (51)  // LParam (time : ms)

// add rove state event
#define IMS_EVENT_ROVE_STATE_CHANGED                   (0x00002000)
#define IMS_ROVE_IN                                    (1)
#define IMS_ROVE_OUT                                   (0)

// Add LTE with VOPS-1 cellular rove state
#define IMS_EVENT_LTE_ROVE_STATE_CHANGED               (0x00002200)
#define IMS_LTE_ROVE_IN                                (1)
#define IMS_LTE_ROVE_OUT                               (0)

// go2071,red.kim To notify IMS REG to native
#define IMS_EVENT_REG_STATE_INTERNAL                   (0x00004000)
// WParam
#define IMS_REG_INTERNAL_ON                            (0)
#define IMS_REG_INTERNAL_OFF                           (1)

// IMS service settings - service on/off
#define IMS_EVENT_SERVICE_SETTING                      (0x00008000)
// WParam
#define IMS_SERVICE_OFF                                (0)
#define IMS_SERVICE_ON                                 (1)
// Service availability (on/off) will be determined by the lParam
#define IMS_SERVICE_PRESENTITY                         (2)
// LParam - IUIMS.h (bitmask)

// VZW
#define IMS_EVENT_SYNC_TO_NATIVE                       (0x00010002)
// WParam
#define IMS_SYNC_TO_NATIVE_INCOMPLETED                 (0)
#define IMS_SYNC_TO_NATIVE_COMPLETED                   (1)

#define IMS_EVENT_REG_PREF_STATE                       (0x00020000)
// WParam (System Mode)
#define IMS_REG_PREF_SYS_NONE                          (0)
#define IMS_REG_PREF_SYS_CDMA                          (1)  // RADIO_IF_CDMA_1XEVDO
#define IMS_REG_PREF_SYS_LTE                           (2)  // RADIO_IF_LTE
// LParam (REG State)
#define IMS_REG_PREF_REG_NONE                          (0)
#define IMS_REG_PREF_REG_VOIP                          (1)  // VOIP+SMS
#define IMS_REG_PREF_REG_SMS                           (2)  // SMS

#define IMS_EVENT_WFC_SETTING_CHANGED                  (0x00040000)
// WParam
#define IMS_WFC_ON                                     (1)
#define IMS_WFC_OFF                                    (0)
// LParam
#define MODE_WFC_PREFERRED                             (2)
#define MODE_WFC_ONLY                                  (0)
#define MODE_CELLULAR_PREFERRED                        (1)
#define MODE_IMS_PREFERRED                             (10)

// SPR
#define IMS_EVENT_WFC_AP_SETTING_CHANGED               (0x00040002)
// WParam
#define IMS_WFC_AP_ON                                  (1)
#define IMS_WFC_AP_OFF                                 (0)

#define IMS_EVENT_CALLINGPLUS_SETTING_CHANGED          (0x00040004)
// WParam
#define IMS_CALLINGPLUS_ON                             (1)
#define IMS_CALLINGPLUS_OFF                            (0)

// VZW/SPR : VoIP / VT enabled
#define IMS_EVENT_OMADM_UPDATED                        (0x00080000)
// WParam
#define IMS_OMADM_VLT                                  (0)
#define IMS_OMADM_LVC                                  (1)
#define IMS_OMADM_EAB                                  (2)
#define IMS_OMADM_ALLOW_VT_REGARDLESS_OF_EAB           (4)
#define IMS_OMADM_MULTIDEVICE                          (5)
#define IMS_OMADM_VOWIFI                               (6)
#define IMS_OMADM_SERVICE_OVER_LTE                     (7)  // SPR
#define IMS_OMADM_ACTIVE_CALL_HO                       (8)  // SPR
// LParam
#define IMS_OMADM_DISABLED                             (0)
#define IMS_OMADM_ENABLED                              (1)

// Event for modem state
#define IMS_EVENT_MODEM_STATE                          (0x00100000)
// WParam
#define IMS_MODEM_STATE_READY                          (1)
#define IMS_MODEM_STATE_IN_SERVICE                     (2)
#define IMS_MODEM_STATE_OUT_OF_SERVICE                 (3)
#define IMS_MODEM_STATE_MODEM_RESET                    (4)

// VZW
#define IMS_EVENT_VOIP_SETTING                         (0x00200000)
// WParam
#define IMS_VOIP_SETTING_OFF                           (0)
#define IMS_VOIP_SETTING_ON                            (1)

// Event for modem state
#define IMS_EVENT_VOLTE_SETTING                        (0x00400000)
// WParam
#define IMS_VOLTE_SETTING_OFF                          (0)
#define IMS_VOLTE_SETTING_ON                           (1)
#define IMS_VOLTE_SETTING_UNKNOWN                      (2)

// Event for modem state
#define IMS_EVENT_NETWORK_MODE_SETTING                 (0x00400001)
// WParam
#define IMS_NETWORK_MODE_SETTING_OFF                   (0)
#define IMS_NETWORK_MODE_SETTING_ON                    (1)

// PS Barring
#define IMS_EVENT_PS_BARRING_STATE                     (0x00800000)

// AC Barring for U+
#define IMS_EVENT_AC_BARRING_STATE                     (0x00800001)

// VZW
#define IMS_EVENT_VOIP_NETWORK_CAPAVILITY              (0x01000000)
// WParam
#define IMS_VOIP_CAPAVILITY_OFF                        (0)
#define IMS_VOIP_CAPAVILITY_ON                         (1)

// changik.jeong: 2012.10.23
// UE initiated IMS PDN disconnection event by usually changing IMS APN setting.
#define IMS_EVENT_UEINITIATED_IMSPDN_DISCONNECTION     (0x02000000)

#define IMS_EVENT_EPDG_KEEPING_SERVICE                 (0x04000000)
// wPARAM
#define EPDG_KEEPING_ON                                (1)
#define EPDG_KEEPING_OFF                               (2)

// VZW
#define IMS_EVENT_WPS_CALL_STATE                       (0x08000000)
// WPARAM
#define IMS_WPS_CALL_STATE_END                         (0)
#define IMS_WPS_CALL_STATE_START                       (1)

// VZW
#define IMS_EVENT_CALL_BLOCK                           = (0x09000000)
// WParam
#define IMS_CALL_BLOCK_STOP                            (0)
#define IMS_CALL_BLOCK_START                           (1)

// Memory Leakage Debug -- last Item
#define IMS_EVENT_MEM_DEBUG                            (0x10000000)
// WParam
#define IMS_MEM_DEBUG_ENABLE_FILE_WRITE                (0)
// LParam
#define IMS_MEM_DEBUG_ENABLE_FILE_WRITE_OFF            (0)
#define IMS_MEM_DEBUG_ENABLE_FILE_WRITE_ON             (1)
// WParam
#define IMS_MEM_DEBUG_PRINT_HEAP_LEAKAGE               (1)

// VZW
#define IMS_EVENT_MOBILE_DATA_SETTING                  (0x20000000)
// WParam
#define IMS_MOBILE_DATA_SETTING_OFF                    (0)
#define IMS_MOBILE_DATA_SETTING_ON                     (1)

#define IMS_EVENT_MOBILE_DATA_LIMIT_CHANGED            (0x20000001)
// WParam
#define IMS_MOBILE_DATA_LIMITED                        (0)
#define IMS_MOBILE_DATA_NOT_LIMITED                    (1)

// Preferred Voice Call Network in Roaming
#define IMS_EVENT_ROAMING_PREFERRED_VOICE_CALL_NETWORK (0x20000004)
// WParam
#define IMS_ROAMING_PREFERRED_CELLULAR_NETWORK         (0)
#define IMS_ROAMING_PREFERRED_WIFI_NETWORK             (1)

#define IMS_EVENT_VIDEO_SETTING                        (0x20000008)
// WParam
#define IMS_VIDEO_SETTING_OFF                          (0)
#define IMS_VIDEO_SETTING_ON                           (1)

#define IMS_EVENT_DATA_ROAMING_SETTING                 (0x20000010)
// WParam
#define IMS_DATA_ROAMING_DENYED                        (0)
#define IMS_DATA_ROAMING_ALLOWED                       (1)

#define IMS_EVENT_NETWORK_MODE_SUPPORTS_CDMA           (0x20000020)
// WParam
#define IMS_CDMA_NOT_SUPPORTED                         (0)
#define IMS_CDMA_SUPPORTED                             (1)

#define IMS_EVENT_REG_CONTROL                          (0x30000000)
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

#define IMS_EVENT_IPCAN_HO_NOTIFICATION                (0x40000001)
// WParam
#define IMS_IPCAN_HANDOVER_UNKNOWN                     (0x00)
#define IMS_IPCAN_HANDOVER_STARTED                     (0x01)
#define IMS_IPCAN_HANDOVER_SUCCESS                     (0x02)
#define IMS_IPCAN_HANDOVER_FAILURE                     (0x03)

#define IMS_EVENT_IMS_PDN_PREFERENCE                   (0x40000002)
// WParam
#define IMS_PDN_PREFERENCE_CELLULAR                    (0x01)
#define IMS_PDN_PREFERENCE_WLAN                        (0x02)
#define IMS_PDN_PREFERENCE_WLAN_WAITING                (0x03)
#define IMS_PDN_PREFERENCE_WLAN_WAITING_DONE           (0x04)

#define IMS_EVENT_NETWORK_CAPABILITY                   (0x40000004)
// WParam, service type
#define IMS_SERVICE_CAPA_VIDEO                         (0)
// LParam, capability
#define IMS_CAPA_OFF                                   (0)
#define IMS_CAPA_ON                                    (1)

#define IMS_EVENT_MEDIA_MOCA_ENABLE_CHECK              (0x40000011)
#define IMS_MEDIA_MOCA_OFF                             (0)
#define IMS_MEDIA_MOCA_ON                              (1)

#define IMS_EVENT_RTT_SETTING                          (0x40000020)
// WParam
#define IMS_RTT_MODE_NONE                              (-1)
#define IMS_RTT_CAPABLE_OFF                            (0)
#define IMS_RTT_VISIBLE_DURING_CALLS                   (1)
#define IMS_RTT_ALWAYS_VISIBLE                         (2)

#define IMS_EVENT_DDS_CHANGED                          (0x50000000)
// WParam : slot id

// LParam
#define IMS_COUNTRY_INFO_AVAILABLE                     (1)

// CELLULAR Signal Strength
#define IMS_EVENT_CELL_SIGNAL_STRENGTH_CHANGE          (0x60020000)
// WParam
#define IMS_CELL_SIGNAL_BAD                            (0)
#define IMS_CELL_SIGNAL_GOOD                           (1)

#define IMS_EVENT_AVAIL_RAT_INFO_CHANGED               (0x70100000)
// WParam : Available RAT INFO
#define IMS_AVAIL_RAT_INFO_NONE                        (0x0)
#define IMS_AVAIL_RAT_INFO_2G                          (0x1)
#define IMS_AVAIL_RAT_INFO_3G                          (0x2)
#define IMS_AVAIL_RAT_INFO_LTE                         (0x4)
#define IMS_AVAIL_RAT_INFO_WIFI                        (0x8)

#define IMS_EVENT_PHONE_RESTARTED                      (0x71000000)

// 1  MUST NOT use the above(>=) 0x80000000 from java to native

// Java to Native }

// Native to Java {
// VT REG
#define IMS_EVENT_REG_STATE                            (0x00000002)
// wPARAM (HWORD) network type
#define IMS_NET_LTE                                    (1)
#define IMS_NET_WIFI                                   (2)
// wPARAM (LWORD) service type
// refer IUIMS
// lPARAM
#define IMS_REG_OFF                                    (0)
#define IMS_REG_ON                                     (1)
#define IMS_REG_OFF_DONE                               (2)
#define IMS_REG_FAILED                                 (3)

// VoLTE Indicator
#define IMS_EVENT_VOLTE_INDICATOR                      (0x00000006)
// wPARAM
#define IMS_VOLTE_INDICATOR_OFF                        (0)
#define IMS_VOLTE_INDICATOR_ON                         (1)
#define IMS_VOLTE_INDICATOR_UNSUPPORTED                (2)

// VoWIFI Indicator
#define IMS_EVENT_VOWIFI_INDICATOR                     (0x00000007)
// wPARAM
#define IMS_VOWIFI_INDICATOR_OFF                       (0)
#define IMS_VOWIFI_INDICATOR_ON                        (1)
#define IMS_VOWIFI_INDICATOR_UNSUPPORTED               (2)

// VZW
#define IMS_EVENT_REG_SERVICE                          (0x00000010)
// wPARAM
#define IMS_REG_SERVICE_NONE                           (0)
#define IMS_REG_SERVICE_VOIP                           (1)
#define IMS_REG_SERVICE_SMS                            (2)
// lPARAM
#define IMS_REG_SERVICE_TYPE_NORMAL                    (0)
#define IMS_REG_SERVICE_TYPE_UPDATE                    (1)
#define IMS_REG_SERVICE_TYPE_SYNCUP                    (2)
#define IMS_REG_SERVICE_TYPE_FORCE_TO_VOIP             (3)

#define IMS_EVENT_REG_FAILURE                          (0x00000011)
// wPARAM (response code)
// lPARAM (consecutive failure count)

// Notify native booting completed
#define IMS_EVENT_NATIVE_BOOT_COMPLETED                (0x00000012)

#define IMS_EVENT_WAKE_LOCK                            (0x00000013)

#define IMS_EVENT_DATA_CONNECTION                      (0x00000015)
// wPARAM
#define IMS_DEACTIVATE_REQ                             (1)
// lPARAM
#define IMS_DETACH_TIMER                               (1)
#define IMS_DETACH_PERMANENT                           (2)
#define IMS_DETACH_AND_ATTACH                          (3)
#define IMS_REFRESH_LEAVE_LTE_TIMER                    (4)

#define IMS_EVENT_OBTAIN_PHONE_NUMBER                  (0x00000017)
// WParam
#define IMS_OBTAIN_PHONE_NUMBER_INITIAL                (0)
#define IMS_OBTAIN_PHONE_NUMBER_REFRESH                (1)
#define IMS_OBTAIN_PHONE_NUMBER_CLEAR                  (2)

#define IMS_EVENT_DCN                                  (0x00000019)
// WParam
#define IMS_DCN_NOT_SENDING                            (0)  // IMS Deregistered
#define IMS_DCN_SENDING                                (1)  // IMS Registered
#define IMS_DCN_FORCE_SENDING                          (2)

#define IMS_EVENT_DM_SYNC                              (0x00000020)
// WParam
#define IMS_DM_SYNC_ONLY                               (1)
#define IMS_DM_SYNC_VLT_DISABLED                       (2)

#define IMS_EVENT_MLT                                  (0x00000021)
// WParam
#define IMS_MLT_REGISTRATION_SUCCESS                   (10)
#define IMS_MLT_REREGISTRATION_SUCCESS                 (11)
#define IMS_MLT_LDB_LOSS_PACKET_COUNT                  (12)
#define IMS_MLT_LDB_TOTAL_PACKET_COUNT                 (13)
#define IMS_MLT_MOCA_PACKET_JITTER_REPORT              (14)

#define IMS_MLT_CALL_FAILURE                           (20)

#define IMS_EVENT_REG_DESTROYED                        (0x00000022)
// WParam
#define IMS_REG_DESTROYED_SILENT_REDIAL                (1)

#define IMS_EVENT_NOTIFY_STATE                         (0x00000023)
// WParam
#define IMS_REG_NOTIFY_STATE_ACTIVE                    (1)
#define IMS_REG_NOTIFY_STATE_INVALID                   (2)

// For explicit DM configuration retrieval
#define IMS_EVENT_DM_CONFIGURATION_RETRIEVAL           (0x00000071)
#define FLAG_STARTAOS_NOTFORCED                        (0)
#define FLAG_STARTAOS_FORCED                           (1)

// For IMS message display to notify something to user
#define IMS_EVENT_SHOW_MESSAGE                         (0x00000080)
// WParam
// LParam
#define IMS_MESSAGE_REGISTRATION_FAILED                (1)
#define IMS_MESSAGE_SERVICE_NOT_PROVISIONED            (2)

#define IMS_EVENT_REGISTRATION                         (0x00000100)
/* WPARAM (HWORD) state
    0 : not registered
    1 : registered
*/
// WPARAM (LWORD) service state (bit masking)
#define IMS_REGISTRATION_SERVICE_NONE                  (0x0)
#define IMS_REGISTRATION_SERVICE_VOICE                 (0x1)
#define IMS_REGISTRATION_SERVICE_VIDEO                 (0x2)
#define IMS_REGISTRATION_SERVICE_SMS                   (0x4)

/* LPARAM (HWORD) reason
0 : Unspecified
1 : Power off (N/A MTK)
2 : RF off (N/A MTK)
3 : PLMN blocked
4 : T3402 blocked
5 : IMS initialization
*/
#define IMS_REGISTRATION_REASON_UNSPECIFIED            (0x0)
#define IMS_REGISTRATION_REASON_PLMN_BLOCKED           (0x3)
#define IMS_REGISTRATION_REASON_T3402_BLOCKED          (0x4)
#define IMS_REGISTRATION_REASON_IMS_INITIALIZATION     (0x5)
#define IMS_REGISTRATION_REASON_BLOCK_NOTIFICATION     (0x100)

// LPARAM (LWORD) detail state
#define IMS_REGISTRATION_INVALID                       (-1)
#define IMS_REGISTRATION_OFFLINE                       (0)
#define IMS_REGISTRATION_REGISTERING                   (1)
#define IMS_REGISTRATION_REGISTERED                    (2)
#define IMS_REGISTRATION_REREGISTERING                 (3)
#define IMS_REGISTRATION_DEREGISTERING                 (4)
#define IMS_REGISTRATION_STOP                          (5)

// LGU+ Specific event
// for IMS_MOCA_LOG_TRACE
#define IMS_EVENT_TRACE_MOCA                           (0x00000500)
// WParam
#define IMS_TRACE_MOCA_STOP                            (1)

#define IMS_EVENT_ISIM_STATE                           (0x00000700)
// wParam
#define IMS_ISIM_STATE_INVALID                         (0)
#define IMS_ISIM_STATE_VALID                           (1)
#define IMS_ISIM_STATE_REFRESH_STARTED                 (2)
#define IMS_ISIM_STATE_REFRESH_COMPLETE                (3)

// Preconditions that it must check before connecting or maintaining IMS APN
#define IMS_EVENT_PDN_PRECONDITION_CHANGED             (0x00000701)
// wParam
#define PRECONDITION_ISIMINVALID                       (1)
#define PRECONDITION_ISIMVALID                         (2)
#define PRECONDITION_ISIMREFRESHSTARTED                (3)
#define PRECONDITION_ISIMREFRESHED                     (4)
#define PRECONDITION_403FORBIDDEN                      (11)
#define IMS_PDN_BLOCK_REASON_NONE                      (100)
#define IMS_PDN_BLOCK_REASON_REGI_FAIL                 (111)
#define IMS_PDN_BLOCK_REASON_OTHERS                    (121)
#define SERVICE_CHECKED_AVAILABLE                      (122)
#define SERVICE_CHECKED_NOTAVAILABLE                   (123)
#define IMS_PDN_RELEASE_DEREG_SUCCESS                  (124)
#define IMS_PDN_RECONNECT                              (125)

#define IMS_EVENT_DEBUG                                (0x00000800)
// wParam
//    - HIWORD : Category (SIP - 0, Media - 1, ...)
//    - LOWORD : Free format according to the category
// lParam
//    - Free format according to the category

// For TTA Registration Retry Debug
#define IMS_EVENT_DEBUG_AWT_UPDATED                    (0x00000801)

#define IMS_EVENT_NO_RTP_AND_PING_FAIL                 (0x00040010)

#define IMS_EVENT_REGI_REPORT_TO_WFC                   (0x04000000)
// wparam
#define REPORT_REG_STATUS_NOT_READY                    (1)
#define REPORT_REG_STATUS_IDLE                         (2)
#define REPORT_REG_STATUS_REGISTERING                  (3)
#define REPORT_REG_STATUS_REGFAILED                    (4)
#define REPORT_REG_STATUS_REGISTERED                   (5)
#define REPORT_REG_STATUS_REFRESHING                   (6)
#define REPORT_REG_STATUS_REFRESHFAILED                (7)
#define REPORT_REG_STATUS_DEREGISTERING                (8)
// lparam - upper
#define IPCAN_CAT_NONE                                 (0x00010000)
#define IPCAN_CAT_MOBILE                               (0x00020000)
#define IPCAN_CAT_EPDG                                 (0x00040000)
// lparam - lower
#define REASON_PCSCF_FAILED                            (0x00000001)
#define REASON_LOCAL_IP_FAILED                         (0x00000002)
#define REASON_NO_USER_INFO                            (0x00000004)
#define REASON_NO_SERVICE_RAT                          (0x00000008)
#define REASON_IMS_DISABLED                            (0x00000010)
#define REASON_GBA_NOT_SUPPORTED                       (0x00000040)
#define REASON_AIRPLANE_MODE_ENABLED                   (0x00000080)
#define REASON_IPSEC_FAILED                            (0x00000100)
#define REASON_TXN_TIMEOUT                             (0x00000200)
#define REASON_NOTIFICATION_TERMINATED                 (0x00000400)
#define REASON_SUBSCRIPTION_FORBIDDEN                  (0x00000800)
#define REASON_ALL_OTHER_FAILURES                      (0x00001000)
#define REASON_SUBSCRIPTION_FORBIDDEN_TERMINATE        (0x00002000)

// set RAT preference to ePDG
#define IMS_EVENT_EPDG_PREFERENCE                      (0x05000000)
// wPARAM
#define EPDG_PREFERRED                                 (0)
#define EPDG_ONLY                                      (1)
#define CELLULAR_PREFERRED                             (2)
#define CELLULAR_ONLY                                  (3)

// set Service status for ePDG
#define IMS_EVENT_SERVICE_STATUS                       (0x05000001)
// wPARAM
#define IMS_SERVICE_UC                                 (1)
#define IMS_SERVICE_SMS                                (2)
#define IMS_SERVICE_UCE                                (3)
// lPARAM
#define IMS_SERVICE_STOP                               (0)
#define IMS_SERVICE_START                              (1)

#define IMS_EVENT_REPORT_BAD_NETWORK                   (0x05000002)
// wParam upper - access network
#define CAT_CELLULAR                                   (0x00010000)
#define CAT_IWLAN                                      (0x00020000)
// wParam lower - capability
#define PDN_TYPE_IMS                                   (0x00000001)
#define PDN_TYPE_EMERGENCY                             (0x00000002)
#define PDN_TYPE_INTERNET                              (0x00000004)
#define PDN_TYPE_XCAP                                  (0x00000008)

// lParam - score
#define QUALITY_BAD                                    (20)
#define QUALITY_GOOD                                   (100)

// IMS to IMS Phone Manager
#define IMS_EVENT_WIFI_SERVICE                         (0x10000000)
// WParam
#define IMS_WIFI_OFF                                   (0)
#define IMS_WIFI_ON                                    (1)

// Call type for WFC icon,  native -> java -> WFC module, go2071, red.kim
#define IMS_EVENT_CALL_INFO_TO_WFC                     (0x30000000)
// wPARAM
#define IMS_EVENT_CALL_STATE_INACTIVE                  (0)
#define IMS_EVENT_CALL_STATE_ACTIVE                    (1)
// lPARAM
#define IMS_EVENT_CALL_TYPE_WIFI                       (0)
#define IMS_EVENT_CALL_TYPE_LTE                        (1)

#define IMS_EVENT_SEND_DATA_TO_MODEM                   (0x80000001)
// wPARAM
#define IMS_EVENT_DATA_FLUSH_ENABLED                   (1)
#define IMS_EVENT_ACB_SKIP_NOTI                        (2)
// lParam
#define IMS_EVENT_REGISTRATION_START                   (0)
#define IMS_EVENT_REGISTRATION_END                     (1)
#define IMS_EVENT_REG_EVENT_START                      (2)
#define IMS_EVENT_REG_EVENT_END                        (3)

// For SCM
#define IMS_EVENT_SEND_SCM_TO_MODEM                    (0x80000002)
// for legacy interface
// wPARAM
#define IMS_EVENT_CALL_START                           (0)
#define IMS_EVENT_CALL_END                             (1)
// lParam
#define IMS_EVENT_CALL_TYPE_VOICE                      (0)
#define IMS_EVENT_CALL_TYPE_VIDEO                      (1)

// for IMS RIL integration
// wPARAM
#define IMS_SCM_START                                  (0)
#define IMS_SCM_END                                    (1)
// lParam
#define IMS_SCM_TYPE_VOICE                             (1)
#define IMS_SCM_TYPE_VIDEO                             (2)
#define IMS_SCM_TYPE_SMS                               (3)
#define IMS_SCM_TYPE_REGISTRATION                      (4)
#define IMS_SCM_TYPE_REG_EVENT                         (11)

#define IMS_EVENT_CALL_INFO                            (0x90000000)
// wPARAM - SessionKey
// lParam
// #define IMS_CALL_STATE_XXX (0x0000000f)
// #define IMS_CALL_STATE_REASON (0x0000fff0)
// #define IMS_CALL_TYPE (0x000f0000)
// #define IMS_CALL_RAT (0x00f00000)

#define IMS_EVENT_CALL_MEIDA_INFO                      (0x90000001)
// wPARAM - SessionKey
// lParam
// #define IMS_CALL_MEDIA_TYPE (0x0000000f)
// #define IMS_CALL_MEDIA_CODEC (0x000000f0)
// #define IMS_CALL_MEDIA_BANDWIDTH (0x00000f00)
// #define IMS_CALL_MEDIA_BITRATE (0x00ff0000)

// Native to Java }

// Macro definition for event parameters
#define IMS_MAKEPARAM(HIWORD, LOWORD)                  (IMS_UINT32)(((HIWORD) << 16) | (LOWORD))

#define IMS_HIWORD(LPARAM)                             (IMS_UINT16)(((LPARAM) >> 16) & (0xFFFF))

#define IMS_LOWORD(LPARAM)                             (IMS_UINT16)((LPARAM)&0xFFFF)

// Test Mask
#define TEST_MASK_NONE                                 (0x0000)
#define TEST_MASK_IGNORE_IMPU_VALIDATION               (0x0001)
#define TEST_MASK_ROAMING_CONDITION                    (0x0002)
#define TEST_MASK_VOLTE_INDICATOR                      (0x0004)
#define TEST_MASK_IMS_STATUS_TO_UICC_OFF               (0x0008)
#define TEST_MASK_NV_SYNC_OFF                          (0x0010)
#define TEST_MASK_DISABLE_2SEC_EREG_RETRY              (0x0020)
#define TEST_MASK_HVOLTE_TEST_PLAN                     (0x0040)
#define TEST_MASK_IGNORE_FORBIDDEN_RESPONSE            (0x0080)
#define TEST_MASK_UNLOCK_EVS_NEGO_LIMIT                (0x0100)

#endif
