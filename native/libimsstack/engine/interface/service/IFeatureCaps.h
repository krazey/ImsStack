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
#ifndef INTERFACE_FEATURE_CAPS_H_
#define INTERFACE_FEATURE_CAPS_H_

#include "AString.h"

#include "ISipMessage.h"

/**
 * @brief This class provides an interface to manage/control the feature capabilities.
 */
class IFeatureCaps
{
protected:
    virtual ~IFeatureCaps() = default;

public:
    /**
     * @brief Adds an additional feature parameter for a Contact header.
     *
     * NOTE: If the feature is a boolean type, strValue is "true" (or empty) / "false".
     *
     * @param strName the feature name
     * @param strValue the feature value
     */
    virtual void AddFeature(IN const AString& strName, IN const AString& strValue) = 0;

    /**
     * @brief Adds an additional feature parameter for a Contact header
     *        according the specified method or message type.
     *
     * NOTE: If the feature is a boolean type, strValue is "true" (or empty) / "false".
     *
     * @param strName the feature name
     * @param strValue the feature value
     * @param nSipMethod an SIP method; refer to @ref SipMethod class\n
     *                   (Allowed for INVITE, SUBSCRIBE, REFER, NOTIFY, OPTIONS, PUBLISH)
     * @param nMessageType an SIP message type (0: request, 1: response, 2: any);
     *                     refer to @ref ISipMessage class
     */
    virtual void AddFeature(IN const AString& strName, IN const AString& strValue,
            IN IMS_SINT32 nSipMethod, IN IMS_SINT32 nMessageType = ISipMessage::TYPE_ANY) = 0;

    /**
     * @brief Removes an added feature parameter for a Contact header.
     *
     * NOTE: If the feature is a boolean type, strValue is "true" (or empty) / "false".
     *
     * @param strName the feature name
     * @param strValue the feature value
     */
    virtual void RemoveFeature(IN const AString& strName, IN const AString& strValue) = 0;

    /**
     * @brief Removes an added feature parameter for a Contact header
     *        according the specified method and message type.
     *
     * NOTE: If the feature is a boolean type, strValue is "true" (or empty) / "false".
     *
     * @param strName the feature name
     * @param strValue the feature value
     * @param nSipMethod an SIP method; refer to @ref SipMethod class\n
     *                   (Allowed for INVITE, SUBSCRIBE, REFER, NOTIFY, OPTIONS, PUBLISH)
     * @param nMessageType an SIP message type (0: request, 1: response, 2: any);
     *                     refer to @ref ISipMessage class
     */
    virtual void RemoveFeature(IN const AString& strName, IN const AString& strValue,
            IN IMS_SINT32 nSipMethod, IN IMS_SINT32 nMessageType = ISipMessage::TYPE_ANY) = 0;

    /**
     * @brief Removes all the features for a Contact header.
     */
    virtual void RemoveAllFeatures() = 0;

    /**
     * @brief Adds a feature parameter to exclude a specific feature parameter from a Contact
     *        header which is used on the IMS-REG procedure.
     *
     * If the application doesn't want to include a specific feature parameter,
     * then it can use this method.\n
     * NOTE: If the feature is a boolean type, strValue is "true" (or empty) / "false".
     *
     * @param strName the feature name
     * @param strValue the feature value
     */
    virtual void AddExcludedFeatureForRegCaps(
            IN const AString& strName, IN const AString& strValue) = 0;

    /**
     * @brief Removes an added feature parameter to exclude a specific feature parameter
     *        from a Contact header.
     *
     * NOTE: If the feature is a boolean type, strValue is "true" (or empty) / "false".
     *
     * @param strName the feature name
     * @param strValue the feature value
     */
    virtual void RemoveExcludedFeatureForRegCaps(
            IN const AString& strName, IN const AString& strValue) = 0;

    /**
     * @brief Removes all the excluded features for the RegCaps.
     */
    virtual void RemoveAllExcludedFeaturesForRegCaps() = 0;
};

#endif
