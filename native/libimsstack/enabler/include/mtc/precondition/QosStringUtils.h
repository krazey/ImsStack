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

#ifndef QOS_STRING_UTILS_H_
#define QOS_STRING_UTILS_H_

#include "ImsTypeDef.h"
#include "SdpAttribute.h"
#include "SdpMedia.h"
#include "offeranswer/SdpPrecondition.h"
#include "precondition/QosDef.h"
#include <unordered_map>

class QosStringUtils
{
public:
    inline static const IMS_CHAR* ConvertSdpMediaType(IN IMS_SINT32 eSdpMediaType)
    {
        static const std::unordered_map<IMS_SINT32, const IMS_CHAR*> objSdpMediaTypeStrings = {
                {SdpMedia::TYPE_INVALID,     "invalid"    },
                {SdpMedia::TYPE_AUDIO,       "audio"      },
                {SdpMedia::TYPE_VIDEO,       "video"      },
                {SdpMedia::TYPE_TEXT,        "text"       },
                {SdpMedia::TYPE_APPLICATION, "application"},
                {SdpMedia::TYPE_MESSAGE,     "message"    },
                {SdpMedia::TYPE_OTHER,       "other"      },
                {SdpMedia::TYPE_MAX,         "invalid max"},
        };

        auto it = objSdpMediaTypeStrings.find(eSdpMediaType);
        return it != objSdpMediaTypeStrings.end() ? it->second : "OUT_OF_RANGE";
    }

    inline static const IMS_CHAR* ConvertQosAttribute(IN IMS_SINT32 eAttrType)
    {
        switch (eAttrType)
        {
            case SdpAttribute::CURR:
                return "curr";
            case SdpAttribute::DES:
                return "des";
            case SdpAttribute::CONF:
                return "conf";
            default:
                return "invalid";
        }
    }

    inline static const IMS_CHAR* ConvertQosDir(IN IMS_SINT32 eDirTag)
    {
        static const std::unordered_map<IMS_SINT32, const IMS_CHAR*> objQosDirStrings = {
                {SdpPrecondition::DIRECTION_INVALID,  "invalid"    },
                {SdpPrecondition::DIRECTION_NONE,     "none"       },
                {SdpPrecondition::DIRECTION_SEND,     "send"       },
                {SdpPrecondition::DIRECTION_RECV,     "recv"       },
                {SdpPrecondition::DIRECTION_SENDRECV, "sendrecv"   },
                {SdpPrecondition::DIRECTION_MAX,      "invalid max"},
        };

        auto it = objQosDirStrings.find(eDirTag);
        return it != objQosDirStrings.end() ? it->second : "OUT_OF_RANGE";
    }

    inline static const IMS_CHAR* ConvertQosStrength(IN IMS_SINT32 eStrengthTag)
    {
        static const std::unordered_map<IMS_SINT32, const IMS_CHAR*> objQosStrengthStrings = {
                {SdpPrecondition::STRENGTH_INVALID,   "invalid"    },
                {SdpPrecondition::STRENGTH_MANDATORY, "mandatory"  },
                {SdpPrecondition::STRENGTH_OPTIONAL,  "optional"   },
                {SdpPrecondition::STRENGTH_NONE,      "none"       },
                {SdpPrecondition::STRENGTH_FAILURE,   "failure"    },
                {SdpPrecondition::STRENGTH_UNKNOWN,   "unknown"    },
                {SdpPrecondition::STRENGTH_NOTUSED,   "not used"   },
                {SdpPrecondition::STRENGTH_MAX,       "invalid max"},
        };

        auto it = objQosStrengthStrings.find(eStrengthTag);
        return it != objQosStrengthStrings.end() ? it->second : "OUT_OF_RANGE";
    }

    inline static const IMS_CHAR* ConvertQosLossPolicy(IN QosLossPolicy ePolicy)
    {
        static const std::unordered_map<QosLossPolicy, const IMS_CHAR*> objQosLossPolicyStrings = {
                {QosLossPolicy::MAINTAIN, "maintain"},
                {QosLossPolicy::MODIFY,   "modify"  },
                {QosLossPolicy::RELEASE,  "release" },
        };

        auto it = objQosLossPolicyStrings.find(ePolicy);
        return it->second;
    }

    inline static const IMS_CHAR* ConvertQosStatus(IN QosStatus eStatus)
    {
        static const std::unordered_map<QosStatus, const IMS_CHAR*> objQosStatusStrings = {
                {QosStatus::IDLE,      "idle"     },
                {QosStatus::AVAILABLE, "available"},
                {QosStatus::LOST,      "lost"     },
        };

        auto it = objQosStatusStrings.find(eStatus);
        return it->second;
    }

    inline static const IMS_CHAR* ConvertQosTimerType(IN QosTimerType eTimerType)
    {
        static const std::unordered_map<QosTimerType, const IMS_CHAR*> objQosTimerTypeStrings = {
                {QosTimerType::WAIT_AUDIO_DEDICATED_BEARER,       "wait audio dedicated bearer"},
                {QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER,
                 "wait available after w2l handover"                                           },
                {QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE,         "wait video text available"  },
                {QosTimerType::GUARD_AFTER_LOST,                  "guard after lost"           },
                {QosTimerType::FORCE_AVAILABLE,                   "force available"            },
        };

        auto it = objQosTimerTypeStrings.find(eTimerType);
        return it->second;
    }

    inline static const IMS_CHAR* ConvertQosStatusType(IN IMS_SINT32 eStatusType)
    {
        static const std::unordered_map<IMS_SINT32, const IMS_CHAR*> objQosStatusTypeStrings = {
                {SdpPrecondition::STATUS_INVALID, "invalid"    },
                {SdpPrecondition::STATUS_E2E,     "e2e"        },
                {SdpPrecondition::STATUS_LOCAL,   "local"      },
                {SdpPrecondition::STATUS_REMOTE,  "remote"     },
                {SdpPrecondition::STATUS_MAX,     "invalid max"},
        };

        auto it = objQosStatusTypeStrings.find(eStatusType);
        return it != objQosStatusTypeStrings.end() ? it->second : "OUT_OF_RANGE";
    }
};

#endif
