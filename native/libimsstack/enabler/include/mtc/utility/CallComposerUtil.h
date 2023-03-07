/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "AString.h"
#include "ImsTypeDef.h"
#include <utility>

class IMessage;
class IMtcCallContext;

/**
 * This class contains utility functions for set/get call composer elements to/from the message.
 */
class CallComposerUtil final
{
public:
    CallComposerUtil() = delete;
    ~CallComposerUtil() = delete;
    CallComposerUtil(IN const CallComposerUtil&) = delete;
    CallComposerUtil& operator=(IN const CallComposerUtil&) = delete;

    /**
     * Gets the call composer priority from the message.
     *
     * @param objMessage Source message.
     * @return CALL_COMPOSER_PRIORITY_NONE or CALL_COMPOSER_PRIORITY_URGENT for valid value.
     *         < 0 if there's no valid priority.
     */
    static IMS_SINT32 GetPriority(IN const IMessage& objMessage);

    /**
     * Gets the call composer subject from the message.
     *
     * @param objMessage Source message.
     * @return Subject string. Empty string if there's no value.
     */
    static AString GetSubject(IN const IMessage& objMessage);

    /**
     * Gets the call composer picture URL from the message.
     *
     * @param objMessage Source message.
     * @return URL of the picture. Empty string if there's no value.
     */
    static AString GetPicture(IN const IMessage& objMessage);

    /**
     * Gets the call composer location from the message.
     *
     * @param objMessage Source message.
     * @return Pair of latitude and longitude. Empty strings if there's no value.
     */
    static std::pair<AString, AString> GetLocation(IN const IMessage& objMessage);

    /**
     * Sets the call composer priority to the message.
     *
     * @param nPriority CALL_COMPOSER_PRIORITY_NONE or CALL_COMPOSER_PRIORITY_URGENT.
     *                  Otherwise does nothing.
     * @param objMessage Message to set the element.
     */
    static void SetPriority(IN IMS_SINT32 nPriority, OUT IMessage& objMessage);

    /**
     * Sets the call composer subject to the message if not empty.
     *
     * @param strSubject Subject.
     * @param objMessage Message to set the element.
     */
    static void SetSubject(IN const AString& strSubject, OUT IMessage& objMessage);

    /**
     * Sets the call composer picture URL to the message if not empty.
     *
     * @param strUrl HTTP URL of the picture.
     * @param objMessage Message to set the element.
     */
    static void SetPicture(IN const AString& strUrl, OUT IMessage& objMessage);

    /**
     * Sets the location to the message if both latitude and longitude are not empty.
     *
     * @param strLatitude Latitude.
     * @param strLongitude Longitude.
     * @param objContext Context to generate the location body.
     * @param objMessage Message to set the element.
     */
    static void SetLocation(IN const AString& strLatitude, IN const AString& strLongitude,
            IN IMtcCallContext& objContext, OUT IMessage& objMessage);
};
