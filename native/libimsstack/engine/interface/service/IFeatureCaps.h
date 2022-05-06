#ifndef _INTERFACE_FEATURE_CAPS_H_
#define _INTERFACE_FEATURE_CAPS_H_

#include "AString.h"

/**
 * @brief This class provides an interface to manage/control the feature capabilities.
 */
class IFeatureCaps
{
public:
    /**
     * @brief Adds an additional feature parameter for a Contact header.
     *
     * NOTE: If the feature is a boolean type, strValue is "true" (or empty) / "false".
     *
     * @param strName the feature name
     * @param strValue the feature value
     */
    virtual void AddFeature(IN CONST AString& strName, IN CONST AString& strValue) = 0;

    /**
     * @brief Adds an additional feature parameter for a Contact header
     *        according the specified method or message type.
     *
     * NOTE: If the feature is a boolean type, strValue is "true" (or empty) / "false".
     *
     * @param strName the feature name
     * @param strValue the feature value
     * @param nSIPMethod an SIP method; refer to @ref SipMethod class\n
     *                   (Allowed for INVITE, SUBSCRIBE, REFER, NOTIFY, OPTIONS, PUBLISH)
     * @param nMessageType an SIP message type (0: request, 1: response, 2: any);
     *                     refer to @ref ISipMessage class
     */
    virtual void AddFeature(IN CONST AString& strName, IN CONST AString& strValue,
            IN IMS_SINT32 nSIPMethod, IN IMS_SINT32 nMessageType = 2 /* ANY */) = 0;

    /**
     * @brief Removes an added feature parameter for a Contact header.
     *
     * NOTE: If the feature is a boolean type, strValue is "true" (or empty) / "false".
     *
     * @param strName the feature name
     * @param strValue the feature value
     */
    virtual void RemoveFeature(IN CONST AString& strName, IN CONST AString& strValue) = 0;

    /**
     * @brief Removes an added feature parameter for a Contact header
     *        according the specified method and message type.
     *
     * NOTE: If the feature is a boolean type, strValue is "true" (or empty) / "false".
     *
     * @param strName the feature name
     * @param strValue the feature value
     * @param nSIPMethod an SIP method; refer to @ref SipMethod class\n
     *                   (Allowed for INVITE, SUBSCRIBE, REFER, NOTIFY, OPTIONS, PUBLISH)
     * @param nMessageType an SIP message type (0: request, 1: response, 2: any);
     *                     refer to @ref ISipMessage class
     */
    virtual void RemoveFeature(IN CONST AString& strName, IN CONST AString& strValue,
            IN IMS_SINT32 nSIPMethod, IN IMS_SINT32 nMessageType = 2 /* ANY */) = 0;

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
            IN CONST AString& strName, IN CONST AString& strValue) = 0;

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
            IN CONST AString& strName, IN CONST AString& strValue) = 0;

    /**
     * @brief Removes all the excluded features for the RegCaps.
     */
    virtual void RemoveAllExcludedFeaturesForRegCaps() = 0;
};

#endif  // _INTERFACE_FEATURE_CAPS_H_
