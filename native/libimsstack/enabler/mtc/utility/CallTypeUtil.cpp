/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include "ImsAosParameter.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/IMtcAosConnector.h"
#include "utility/CallTypeUtil.h"

PUBLIC GLOBAL CallType CallTypeUtil::RestrictCallTypeByRegisteredFeature(
        IN CallType eCallType, IN const IMtcAosConnector* pAosConnector)
{
    if (pAosConnector == IMS_NULL)
    {
        return CallType::VOIP;
    }

    IMS_BOOL bVideoFeature = pAosConnector->GetFeatures() & ImsAosFeature::VIDEO;
    IMS_BOOL bTextFeature = pAosConnector->GetFeatures() & ImsAosFeature::TEXT;
    return RestrictCallTypeByCapability(eCallType, bVideoFeature, bTextFeature);
}

PUBLIC GLOBAL CallType CallTypeUtil::RestrictCallTypeByCapability(
        IN CallType eCallType, IN IMS_BOOL bVideoCapable, IN IMS_BOOL bRttCapable)
{
    CallType eCapableCallType = eCallType;

    if (!bVideoCapable)
    {
        if (eCapableCallType == CallType::VT)
        {
            eCapableCallType = CallType::VOIP;
        }
        else if (eCapableCallType == CallType::VIDEO_RTT)
        {
            eCapableCallType = CallType::RTT;
        }
    }
    if (!bRttCapable)
    {
        if (eCapableCallType == CallType::RTT)
        {
            eCapableCallType = CallType::VOIP;
        }
        else if (eCapableCallType == CallType::VIDEO_RTT)
        {
            eCapableCallType = CallType::VT;
        }
    }

    return eCapableCallType;
}

PUBLIC GLOBAL CallType CallTypeUtil::GetCallTypeByRegisteredFeature(
        IN const IMtcAosConnector* pAosConnector, IN const MtcConfigurationProxy& objConfig)
{
    if (pAosConnector == IMS_NULL)
    {
        return CallType::VOIP;
    }

    IMS_BOOL bVideoFeature = pAosConnector->GetFeatures() & ImsAosFeature::VIDEO;
    IMS_BOOL bTextFeature = pAosConnector->GetFeatures() & ImsAosFeature::TEXT;

    if (bVideoFeature && !bTextFeature)
    {
        return CallType::VT;
    }
    else if (!bVideoFeature && bTextFeature)
    {
        return CallType::RTT;
    }
    else if (bVideoFeature && bTextFeature)
    {
        // Video && RTT
        IMS_SINT32 nPolicyForTextAndVideo =
                objConfig.GetInt(ConfigVt::KEY_POLICY_FOR_TEXT_WITH_VIDEO_INT);
        if (nPolicyForTextAndVideo == ConfigVt::TEXT_VIDEO_NOT_ALLOWED ||
                nPolicyForTextAndVideo == ConfigVt::TEXT_VIDEO_NOT_ALLOWED_IF_ACTIVE)
        {
            return CallType::VT;
        }
        else
        {
            // TEXT_VIDEO_ALLOWED
            return CallType::VIDEO_RTT;
        }
    }

    return CallType::VOIP;
}
