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

#include "IMSTypeDef.h"
#include "SdpMedia.h"
#include "SdpAttribute.h"
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

    inline static const IMS_CHAR* PS_QosCheckType(IN QosCheckType eCheckType)
    {
        switch (eCheckType)
        {
            case QosCheckType::ALL_STATUS:
                return "all";
            case QosCheckType::LOCAL_STATUS:
                return "local";
            case QosCheckType::REMOTE_STATUS:
                return "remote";
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
            case QosLossPolicy::RELEASE:
                return "release";
            default:
                return "invalid";
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
            case QosStatus::LOST:
                return "lost";
            default:
                return "invalid";
        }
    }

    inline static const IMS_CHAR* PS_QosTimerType(IN QosTimerType eTimerType)
    {
        switch (eTimerType)
        {
            case QosTimerType::WAIT_AVAILABLE:
                return "wait available";
            case QosTimerType::GUARD_INACTIVE:
                return "guard inactive";
            case QosTimerType::FORCE_AVAILABLE:
                return "force available";
            default:
                return "invalid";
        }
    }
};

#ifndef PS_SdpMediaType
#define PS_SdpMediaType(A) QosStringDef::PS_SdpMediaType(A)
#endif

#ifndef PS_QosAttribute
#define PS_QosAttribute(A) QosStringDef::PS_QosAttribute(A)
#endif

#ifndef PS_QosCheckType
#define PS_QosCheckType(A) QosStringDef::PS_QosCheckType(A)
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
