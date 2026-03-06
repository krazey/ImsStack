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

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"

class IMtcAosConnector;
class MtcConfigurationProxy;

/**
 * This class contains utility functions for call type related operation.
 */
class CallTypeUtil final
{
public:
    CallTypeUtil() = delete;
    ~CallTypeUtil() = delete;
    CallTypeUtil(IN const CallTypeUtil&) = delete;
    CallTypeUtil& operator=(IN const CallTypeUtil&) = delete;

    /**
     * @brief Restricts the call type based on the registered AoS features.
     *
     * @param eCallType The original call type.
     * @param pAosConnector The AoS connector to get registered features from.
     * @return The restricted call type.
     */
    static CallType RestrictCallTypeByRegisteredFeature(
            IN CallType eCallType, IN const IMtcAosConnector* pAosConnector);

    /**
     * @brief Restricts the call type based on video and RTT capabilities.
     *
     * @param eCallType The original call type.
     * @param bVideoCapable Whether video is capable.
     * @param bRttCapable Whether RTT is capable.
     * @return The restricted call type.
     */
    static CallType RestrictCallTypeByCapability(
            IN CallType eCallType, IN IMS_BOOL bVideoCapable, IN IMS_BOOL bRttCapable);

    /**
     * @brief Gets the most appropriate call type based on registered features and configuration.
     *
     * @param pAosConnector The AoS connector to get registered features from.
     * @param objConfig The configuration to check policies.
     * @return The determined call type.
     */
    static CallType GetCallTypeByRegisteredFeature(
            IN const IMtcAosConnector* pAosConnector, IN const MtcConfigurationProxy& objConfig);

    /**
     * @brief Checks if the given call type is a video call.
     *
     * @param eCallType The call type to check.
     * @return True if it is a video call, otherwise false.
     */
    inline static IMS_BOOL IsVideoCall(IN CallType eCallType)
    {
        return eCallType == CallType::VT || eCallType == CallType::VIDEO_RTT;
    }

    /**
     * @brief Checks if the given call type is an RTT call.
     *
     * @param eCallType The call type to check.
     * @return True if it is a RTT call, otherwise false.
     */
    inline static IMS_BOOL IsRttCall(IN CallType eCallType)
    {
        return eCallType == CallType::RTT || eCallType == CallType::VIDEO_RTT;
    }
};
