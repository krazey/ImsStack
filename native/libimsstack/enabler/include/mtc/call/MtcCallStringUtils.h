/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef MTC_CALL_STRING_UTILS_H_
#define MTC_CALL_STRING_UTILS_H_

#include "IImsRadio.h"
#include "IIpcan.h"
#include "ImsAosReason.h"
#include "ImsTypeDef.h"
#include "INetworkWatcher.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "call/state/IMtcCallState.h"
#include "call/state/MtcCallState.h"
#include "helper/IMtcAosStateListener.h"
#include "helper/ISrvccStateListener.h"
#include <unordered_map>

class MtcCallStringUtils
{
public:
    inline static const IMS_CHAR* ConvertCallType(IN CallType eCallType)
    {
        static const std::unordered_map<CallType, const IMS_CHAR*> objCallTypeStrings = {
                {CallType::UNKNOWN,   "unknown"  },
                {CallType::VOIP,      "voip"     },
                {CallType::VT,        "vt"       },
                {CallType::RTT,       "rtt"      },
                {CallType::VIDEO_RTT, "video_rtt"},
        };

        auto it = objCallTypeStrings.find(eCallType);
        return it->second;
    }

    inline static const IMS_CHAR* ConvertMediaType(IN IMS_UINT32 eMediaType)
    {
        static const std::unordered_map<IMS_UINT32, const IMS_CHAR*> objMediaTypeStrings = {
                {MEDIATYPE_NONE,  "none" },
                {MEDIATYPE_AUDIO, "audio"},
                {MEDIATYPE_VIDEO, "video"},
                {MEDIATYPE_TEXT,  "text" },
        };

        auto it = objMediaTypeStrings.find(eMediaType);
        return it != objMediaTypeStrings.end() ? it->second : "OUT_OF_RANGE";
    }

    inline static const IMS_CHAR* ConvertUpdateType(IN UpdateType eUpdateType)
    {
        static const std::unordered_map<UpdateType, const IMS_CHAR*> objUpdateTypeStrings = {
                {UpdateType::NONE,                    "none"                   },
                {UpdateType::NORMAL,                  "normal"                 },
                {UpdateType::HOLD,                    "hold"                   },
                {UpdateType::RESUME,                  "resume"                 },
                {UpdateType::SESSION,                 "session"                },
                {UpdateType::CONF,                    "conf"                   },
                {UpdateType::REFRESH,                 "refresh"                },
                {UpdateType::SRVCC_RECOVERED_CANCEL,  "srvcc recovered cancel" },
                {UpdateType::SRVCC_RECOVERED_FAILURE, "srvcc recovered failure"},
                {UpdateType::LOCATION,                "location"               },
        };

        auto it = objUpdateTypeStrings.find(eUpdateType);
        return it->second;
    }

    inline static const IMS_CHAR* ConvertTimerType(IN IMS_SINT32 nTimerType)
    {
        static const std::unordered_map<IMS_SINT32, const IMS_CHAR*> objTimerTypeStrings = {
                {MtcCallState::TIMER_MO_REGISTRATION_FOR_SILENT_REDIAL,
                 "TIMER_MO_REGISTRATION_FOR_SILENT_REDIAL"                                             },
                {MtcCallState::TIMER_MO_CALL_INITIATION_TO_18X_WAIT,
                 "TIMER_MO_CALL_INITIATION_TO_18X_WAIT"                                                },
                {MtcCallState::TIMER_MO_18X_WAIT,                       "TIMER_MO_18X_WAIT"            },
                {MtcCallState::TIMER_MO_NOANSWER,                       "TIMER_MO_NOANSWER"            },
                {MtcCallState::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON,
                 "TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON"                                                },
                {MtcCallState::TIMER_MT_ALERTING,                       "TIMER_MT_ALERTING"            },
                {MtcCallState::TIMER_MT_PRACK_WAIT,                     "TIMER_MT_PRACK_WAIT"          },
                {MtcCallState::TIMER_RETRY_UPDATE,                      "TIMER_RETRY_UPDATE"           },
                {MtcCallState::TIMER_CONVERT_USER_RESPONSE,             "TIMER_CONVERT_USER_RESPONSE"  },
                {MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE,           "TIMER_CONVERT_REMOTE_RESPONSE"},
                {MtcCallState::TIMER_E911_WAIT_SESSION_RELEASED,
                 "TIMER_E911_WAIT_SESSION_RELEASED"                                                    },
                {MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED,
                 "TIMER_DELAY_UPDATE_AFTER_CONNECTED"                                                  },
        };

        auto it = objTimerTypeStrings.find(nTimerType);
        return it != objTimerTypeStrings.end() ? it->second : "OUT_OF_RANGE";
    }

    inline static const IMS_CHAR* ConvertBlockStatus(IN IMtcBlockChecker::Result::Status eStatus)
    {
        static const std::unordered_map<IMtcBlockChecker::Result::Status, const IMS_CHAR*>
                objBlockStatusStrings = {
                        {IMtcBlockRule::Result::Status::UNBLOCKED, "unblocked"},
                        {IMtcBlockRule::Result::Status::BLOCKED,   "blocked"  },
                        {IMtcBlockRule::Result::Status::PENDING,   "pending"  },
        };

        auto it = objBlockStatusStrings.find(eStatus);
        return it->second;
    }

    inline static const IMS_CHAR* ConvertIpcanType(IN IMS_UINT32 eIpcan)
    {
        static const std::unordered_map<IMS_UINT32, const IMS_CHAR*> objIpcanTypeStrings = {
                {IIpcan::CATEGORY_MOBILE, "mobile"},
                {IIpcan::CATEGORY_WLAN,   "wlan"  },
                {IIpcan::CATEGORY_ANY,    "any"   },
        };

        auto it = objIpcanTypeStrings.find(eIpcan);
        return it != objIpcanTypeStrings.end() ? it->second : "OUT_OF_RANGE";
    }

    inline static const IMS_CHAR* ConvertAosState(IN MtcAosState eState)
    {
        static const std::unordered_map<MtcAosState, const IMS_CHAR*> objAosStateStrings = {
                {MtcAosState::CONNECTED,     "connected"    },
                {MtcAosState::SUSPENDED,     "suspended"    },
                {MtcAosState::DISCONNECTING, "disconnecting"},
                {MtcAosState::DISCONNECTED,  "disconnected" },
        };

        auto it = objAosStateStrings.find(eState);
        return it->second;
    }

    inline static const IMS_CHAR* ConvertRatType(IN IMS_SINT32 eRatType)
    {
        static const std::unordered_map<IMS_SINT32, const IMS_CHAR*> objRatTypeStrings = {
                {INetworkWatcher::RADIOTECH_TYPE_INVALID,  "invalid"    },
                {INetworkWatcher::RADIOTECH_TYPE_UNKNOWN,  "unknown"    },
                {INetworkWatcher::RADIOTECH_TYPE_GPRS,     "GPRS"       },
                {INetworkWatcher::RADIOTECH_TYPE_EDGE,     "EDGE"       },
                {INetworkWatcher::RADIOTECH_TYPE_UMTS,     "UMTS"       },
                {INetworkWatcher::RADIOTECH_TYPE_CDMA,     "CDMA"       },
                {INetworkWatcher::RADIOTECH_TYPE_EVDO_0,   "EVDO_0"     },
                {INetworkWatcher::RADIOTECH_TYPE_EVDO_A,   "EVDO_A"     },
                {INetworkWatcher::RADIOTECH_TYPE_1xRTT,    "1xRTT"      },
                {INetworkWatcher::RADIOTECH_TYPE_HSDPA,    "HSDPA"      },
                {INetworkWatcher::RADIOTECH_TYPE_HSUPA,    "HSUPA"      },
                {INetworkWatcher::RADIOTECH_TYPE_HSPA,     "HSPA"       },
                {INetworkWatcher::RADIOTECH_TYPE_IDEN,     "IDEN"       },
                {INetworkWatcher::RADIOTECH_TYPE_EVDO_B,   "EVDO_B"     },
                {INetworkWatcher::RADIOTECH_TYPE_LTE,      "LTE"        },
                {INetworkWatcher::RADIOTECH_TYPE_EHRPD,    "EHRPD"      },
                {INetworkWatcher::RADIOTECH_TYPE_HSPAP,    "HSPAP"      },
                {INetworkWatcher::RADIOTECH_TYPE_TD_SCDMA, "TD_SCDMA"   },
                {INetworkWatcher::RADIOTECH_TYPE_IWLAN,    "IWLAN"      },
                {INetworkWatcher::RADIOTECH_TYPE_LTE_CA,   "LTE_CA"     },
                {INetworkWatcher::RADIOTECH_TYPE_NR,       "NR"         },
                {INetworkWatcher::RADIOTECH_TYPE_MAX,      "invalid max"},
        };

        auto it = objRatTypeStrings.find(eRatType);
        return it != objRatTypeStrings.end() ? it->second : "OUT_OF_RANGE";
    }

    inline static const IMS_CHAR* ConvertFailureReason(IN IMS_UINT32 nFailureReason)
    {
        static const std::unordered_map<IMS_UINT32, const IMS_CHAR*> obFailureReasonStrings = {
                {IImsRadio::REASON_ACCESS_DENIED,     "Access denied"    },
                {IImsRadio::REASON_NAS_FAILURE,       "NAS failure"      },
                {IImsRadio::REASON_RACH_FAILURE,      "RACH failure"     },
                {IImsRadio::REASON_RLC_FAILURE,       "RLC failure"      },
                {IImsRadio::REASON_RRC_REJECT,        "RRC reject"       },
                {IImsRadio::REASON_RRC_TIMEOUT,       "RRC timeout"      },
                {IImsRadio::REASON_NO_SERVICE,        "No service"       },
                {IImsRadio::REASON_PDN_NOT_AVAILABLE, "PDN not available"},
                {IImsRadio::REASON_RF_BUSY,           "RF busy"          },
                {IImsRadio::REASON_INTERNAL_ERROR,    "Internal error"   },
        };

        auto it = obFailureReasonStrings.find(nFailureReason);
        return it != obFailureReasonStrings.end() ? it->second : "OUT_OF_RANGE";
    }

    inline static const IMS_CHAR* ConvertSrvccState(IN SrvccState eState)
    {
        static const std::unordered_map<SrvccState, const IMS_CHAR*> objSrvccStateStrings = {
                {SrvccState::IDLE,      "IDLE"     },
                {SrvccState::STARTED,   "STARTED"  },
                {SrvccState::SUCCEEDED, "SUCCEEDED"},
                {SrvccState::FAILED,    "FAILED"   },
                {SrvccState::CANCELED,  "CANCELED" },
        };

        auto it = objSrvccStateStrings.find(eState);
        return it->second;
    }

    inline static const IMS_CHAR* ConvertAosReason(IN IMS_UINT32 eAosReason)
    {
        static const std::unordered_map<IMS_UINT32, const IMS_CHAR*> objAosReasonStrings = {
                {ImsAosReason::NONE,              "none"                     },
                {ImsAosReason::POWER_OFF,         "power off"                },
                {ImsAosReason::SERVICE_POLICY,    "service policy"           },
                {ImsAosReason::DATA_DISCONNECTED, "data disconnected"        },
                {ImsAosReason::REG_TERMINATED,    "registration terminated"  },
                {ImsAosReason::REG_NEW_REQUIRED,  "new registration required"},
                {ImsAosReason::REG_TERMINATING,   "registration terminating" },
                {ImsAosReason::IP_CHANGED,        "ip changed"               },
                {ImsAosReason::NOT_SPECIFIED,     "not specified"            },
        };

        auto it = objAosReasonStrings.find(eAosReason);
        return it != objAosReasonStrings.end() ? it->second : "OUT_OF_RANGE";
    }

    inline static const IMS_CHAR* ConvertCallState(IN CallStateName eState)
    {
        static const std::unordered_map<CallStateName, const IMS_CHAR*> objCallStateStrings = {
                {CallStateName::IDLE,        "IDLE"       },
                {CallStateName::OUTGOING,    "OUTGOING"   },
                {CallStateName::INCOMING,    "INCOMING"   },
                {CallStateName::ALERTING,    "ALERTING"   },
                {CallStateName::ESTABLISHED, "ESTABLISHED"},
                {CallStateName::UPDATING,    "UPDATING"   },
                {CallStateName::TERMINATING, "TERMINATING"},
        };

        auto it = objCallStateStrings.find(eState);
        return it->second;
    }
};

#endif
