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

#ifndef MTC_MEDIA_STRING_UTILS_H_
#define MTC_MEDIA_STRING_UTILS_H_

#include "IJniMedia.h"
#include "ImsTypeDef.h"
#include "MediaDef.h"
#include "MediaNego.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include <unordered_map>

class MtcMediaStringUtils
{
public:
    inline static const IMS_CHAR* ConvertReportType(IN IMS_UINT32 eReportType)
    {
        static const std::unordered_map<IMS_UINT32, const IMS_CHAR*> objReportTypeStrings = {
                {REPORT_TYPE::REPORT_SUCCESS,                     "success"                     },
                {REPORT_TYPE::REPORT_FAILURE,                     "failure"                     },
                {REPORT_TYPE::REPORT_DATA_RECEIVE_FAILED,         "data receive failed"         },
                {REPORT_TYPE::REPORT_DATA_RECEIVE_STARTED,        "data receive started"        },
                {REPORT_TYPE::REPORT_QOS,                         "qos"                         },
                {REPORT_TYPE::REPORT_VIDEO_LOWEST_BITRATE,        "video lowest bitrate"        },
                {REPORT_TYPE::REPORT_CHECK_RADIO_CONNECTION,      "check radio condition"       },
                {REPORT_TYPE::REPORT_NW_TONE_RTP_RECEIVE_STARTED, "network tone receive started"},
                {REPORT_TYPE::REPORT_NW_TONE_RTP_RECEIVE_FAILED,  "network tone receive failed" },
                {REPORT_TYPE::REPORT_RECEIVED_DTMF_EVENT,         "dtmf event received"         },
                {REPORT_TYPE::REPORT_MEDIA_DETACH,                "media detach"                },
                {REPORT_TYPE::REPORT_TRIGGER_ANBR_QUERY,          "trigger anbr query"          },
                {REPORT_TYPE::REPORT_ANBR_NEGOTIATION_RESULT,     "anbr negotiation result"     },
                {REPORT_TYPE::REPORT_NOTUSED,                     "not used"                    },
        };

        auto it = objReportTypeStrings.find(eReportType);
        return it != objReportTypeStrings.end() ? it->second : "OUT_OF_RANGE";
    }

    inline static const IMS_CHAR* ConvertContentType(IN IMS_UINT32 eContentType)
    {
        static const std::unordered_map<IMS_UINT32, const IMS_CHAR*> objContentTypeStrings = {
                {MEDIA_TYPE_INVALID,        "invalid"             },
                {MEDIA_TYPE_AUDIO,          "audio"               },
                {MEDIA_TYPE_VIDEO,          "video"               },
                {MEDIA_TYPE_AUDIOVIDEO,     "audio & video"       },
                {MEDIA_TYPE_TEXT,           "text"                },
                {MEDIA_TYPE_AUDIOTEXT,      "audio & text"        },
                {MEDIA_TYPE_VIDEOTEXT,      "video & text"        },
                {MEDIA_TYPE_AUDIOVIDEOTEXT, "audio & video & text"},
                {MEDIA_TYPE_NOTUSED,        "not used"            },
        };

        auto it = objContentTypeStrings.find(eContentType);
        return it != objContentTypeStrings.end() ? it->second : "OUT_OF_RANGE";
    }

    inline static const IMS_CHAR* ConvertErrorType(IN IMS_SINT32 eErrorType)
    {
        static const std::unordered_map<IMS_SINT32, const IMS_CHAR*> objErrorTypeStrings = {
                {RtpError::NO_ERROR,              "no error"             },
                {RtpError::INVALID_PARAM,         "invalid param"        },
                {RtpError::NOT_READY,             "not ready"            },
                {RtpError::NO_MEMORY,             "no memory"            },
                {RtpError::NO_RESOURCES,          "no resources"         },
                {RtpError::PORT_UNAVAILABLE,      "port unavailable"     },
                {RtpError::REQUEST_NOT_SUPPORTED, "request not supported"},
                {RtpError::RESPONSE_WAIT_TIMEOUT, "response wait timeout"},
        };

        auto it = objErrorTypeStrings.find(eErrorType);
        return it != objErrorTypeStrings.end() ? it->second : "OUT_OF_RANGE";
    }

    inline static const IMS_CHAR* ConvertNegoType(IN MediaNego::MediaNegoResult eNegoResult)
    {
        static const std::unordered_map<MediaNego::MediaNegoResult, const IMS_CHAR*>
                objNegoResultStrings = {
                        {MediaNego::MediaNegoResult::NO_ERROR,                 "no error"        },
                        {MediaNego::MediaNegoResult::ERROR_INVALID_DESCRIPTOR,
                         "invalid descriptor"                                                    },
                        {MediaNego::MediaNegoResult::ERROR_NO_CODEC_MATCHED,   "no codec matched"},
                        {MediaNego::MediaNegoResult::ERROR_IP_MISMATCH,        "ip mismatch"     },
                        {MediaNego::MediaNegoResult::ERROR_NO_AUDIO,           "no audio"        },
                        {MediaNego::MediaNegoResult::ERROR_NO_VIDEO,           "no video"        },
                        {MediaNego::MediaNegoResult::ERROR_NO_TEXT,            "no text"         },
        };

        auto it = objNegoResultStrings.find(eNegoResult);
        return it->second;
    }

    inline static const IMS_CHAR* ConvertPemType(IN PemType ePemType)
    {
        static const std::unordered_map<PemType, const IMS_CHAR*> objPemTypeStrings = {
                {PemType::NONE,     "none"    },
                {PemType::SENDRECV, "sendrecv"},
                {PemType::SENDONLY, "sendonly"},
                {PemType::RECVONLY, "recvonly"},
                {PemType::INACTIVE, "inactive"},
        };

        auto it = objPemTypeStrings.find(ePemType);
        return it->second;
    }

    inline static const IMS_CHAR* ConvertQuality(IN IMS_SINT32 eCodecType)
    {
        if (eCodecType < MEDIA_QUALITY_NONE || eCodecType > MEDIA_QUALITY_NOTUSED)
        {
            return "OUT_OF_RANGE";
        }
        else if (eCodecType == MEDIA_QUALITY_NONE || eCodecType == MEDIA_QUALITY_NOTUSED)
        {
            return "none";
        }
        else  // (eCodecType > MEDIA_QUALITY_NONE && eCodecType < MEDIA_QUALITY_NOTUSED)
        {
            return "valid";
        }
    }

    inline static const IMS_CHAR* ConvertNegoState(IN NEGO_STATE eNegoState)
    {
        static const std::unordered_map<NEGO_STATE, const IMS_CHAR*> objNegoStateStrings = {
                {STATE_IDLE,           "idle"          },
                {STATE_OFFER_RECEIVED, "offer received"},
                {STATE_OFFER_SENT,     "offer sent"    },
                {STATE_NEGOTIATED,     "negotiated"    },
                {STATE_NOTUSED,        "not used"      },
        };

        auto it = objNegoStateStrings.find(eNegoState);
        return it->second;
    }

    inline static const IMS_CHAR* ConvertDirection(IN IMS_SINT32 eDirection)
    {
        static const std::unordered_map<IMS_SINT32, const IMS_CHAR*> objMediaDirectionStrings = {
                {MEDIA_DIRECTION_INVALID,      "invalid" },
                {MEDIA_DIRECTION_INACTIVE,     "inactive"},
                {MEDIA_DIRECTION_RECEIVE,      "receive" },
                {MEDIA_DIRECTION_SEND,         "send"    },
                {MEDIA_DIRECTION_SEND_RECEIVE, "sendrecv"},
        };

        auto it = objMediaDirectionStrings.find(eDirection);
        return it != objMediaDirectionStrings.end() ? it->second : "OUT_OF_RANGE";
    }

    inline static const IMS_CHAR* ConvertProtocolType(IN IMS_UINT32 eProtocolType)
    {
        static const std::unordered_map<IMS_UINT32, const IMS_CHAR*> objProtocolTypeStrings = {
                {MEDIA_PROTOCOL_NONE,      "none"     },
                {MEDIA_PROTOCOL_ANY,       "any"      },
                {MEDIA_PROTOCOL_RTP,       "rtp"      },
                {MEDIA_PROTOCOL_RTCP,      "rtcp"     },
                {MEDIA_PROTOCOL_NO_CHANGE, "no change"},
        };

        auto it = objProtocolTypeStrings.find(eProtocolType);
        return it != objProtocolTypeStrings.end() ? it->second : "OUT_OF_RANGE";
    }
};

#endif
