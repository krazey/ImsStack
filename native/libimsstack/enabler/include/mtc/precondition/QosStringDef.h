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

#ifndef QOS_STRING_DEF_H_
#define QOS_STRING_DEF_H_

#include "ImsTypeDef.h"
#include "SdpAttribute.h"
#include "SdpMedia.h"
#include "offeranswer/SdpPrecondition.h"
#include "precondition/QosDef.h"

class QosStringDef
{
public:
    inline static const IMS_CHAR* PS_SdpMediaType(IN IMS_SINT32 eSdpMediaType)
    {
        switch (eSdpMediaType)
        {
            case SdpMedia::TYPE_AUDIO:
                return "audio";
            case SdpMedia::TYPE_VIDEO:
                return "video";
            case SdpMedia::TYPE_TEXT:
                return "text";
            default:
                return "invalid";
        }
    }

    inline static const IMS_CHAR* PS_QosAttribute(IN IMS_SINT32 eAttrType)
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

    inline static const IMS_CHAR* PS_QosDir(IN IMS_SINT32 eDirTag)
    {
        switch (eDirTag)
        {
            case SdpPrecondition::DIRECTION_NONE:
                return "none";
            case SdpPrecondition::DIRECTION_SEND:
                return "send";
            case SdpPrecondition::DIRECTION_RECV:
                return "recv";
            case SdpPrecondition::DIRECTION_SENDRECV:
                return "sendrecv";
            default:
                return "invalid";
        }
    }

    inline static const IMS_CHAR* PS_QosStrength(IN IMS_SINT32 eStrengthTag)
    {
        switch (eStrengthTag)
        {
            case SdpPrecondition::STRENGTH_MANDATORY:
                return "mandatory";
            case SdpPrecondition::STRENGTH_OPTIONAL:
                return "optional";
            case SdpPrecondition::STRENGTH_NONE:
                return "none";
            case SdpPrecondition::STRENGTH_FAILURE:
                return "failure";
            case SdpPrecondition::STRENGTH_UNKNOWN:
                return "unknown";
            case SdpPrecondition::STRENGTH_NOTUSED:
                return "not used";
            default:
                return "invalid";
        }
    }

    inline static const IMS_CHAR* PS_QosLossPolicy(IN QosLossPolicy ePolicy)
    {
        switch (ePolicy)
        {
            case QosLossPolicy::MAINTAIN:
                return "maintain";
            case QosLossPolicy::MODIFY:
                return "modify";
            default:  // QosLossPolicy::RELEASE:
                return "release";
        }
    }

    inline static const IMS_CHAR* PS_QosStatus(IN QosStatus eStatus)
    {
        switch (eStatus)
        {
            case QosStatus::IDLE:
                return "idle";
            case QosStatus::AVAILABLE:
                return "available";
            default:  // QosStatus::LOST:
                return "lost";
        }
    }

    inline static const IMS_CHAR* PS_QosTimerType(IN QosTimerType eTimerType)
    {
        switch (eTimerType)
        {
            case QosTimerType::WAIT_AUDIO_AVAILABLE:
                return "wait audio available";
            case QosTimerType::GUARD_AVAILABLE:
                return "guard available";
            case QosTimerType::GUARD_AFTER_LOST:
                return "guard after lost";
            case QosTimerType::WAIT_AVAILABLE_AFTER_HANDOVER:
                return "wait available after handover";
            default:  // QosTimerType::FORCE_AVAILABLE:
                return "force available";
        }
    }
};

#ifndef PS_SdpMediaType
#define PS_SdpMediaType(A) QosStringDef::PS_SdpMediaType(A)
#endif

#ifndef PS_QosAttribute
#define PS_QosAttribute(A) QosStringDef::PS_QosAttribute(A)
#endif

#ifndef PS_QosDir
#define PS_QosDir(A) QosStringDef::PS_QosDir(A)
#endif

#ifndef PS_QosStrength
#define PS_QosStrength(A) QosStringDef::PS_QosStrength(A)
#endif

#ifndef PS_QosLossPolicy
#define PS_QosLossPolicy(A) QosStringDef::PS_QosLossPolicy(A)
#endif

#ifndef PS_QosStatus
#define PS_QosStatus(A) QosStringDef::PS_QosStatus(A)
#endif

#ifndef PS_QosTimerType
#define PS_QosTimerType(A) QosStringDef::PS_QosTimerType(A)
#endif

#endif
