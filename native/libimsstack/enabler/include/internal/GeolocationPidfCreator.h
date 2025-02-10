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
#ifndef GEOLOCATION_PIDF_CREATOR_H_
#define GEOLOCATION_PIDF_CREATOR_H_

#include "ByteArray.h"
#include "IPhoneInfoLocation.h"
#include "ImsSlot.h"
#include <initializer_list>

namespace enabler
{
class Element;
}

class ILocationProperties;

class GeolocationPidfCreator : public ImsSlot
{
public:
    explicit GeolocationPidfCreator(IN IMS_SINT32 nSlotId);
    virtual ~GeolocationPidfCreator();

    GeolocationPidfCreator(IN const GeolocationPidfCreator&) = delete;
    GeolocationPidfCreator& operator=(IN const GeolocationPidfCreator&) = delete;

public:
    /**
     * Creates PIDF-LO XML for geolocation with country and state only.
     *
     * @param strEntityUri "entity" attribute of <presence> element.
     * @param bUnknownCountryAllowed If true, it fails when the country is empty or "ZZ".
     * @param bIncludeState "cl:A1" element is added with the state if true.
     * @param objContent ByteArray to be set the result XML.
     * @return True if success.
     */
    IMS_BOOL CreateWithoutPosition(IN const AString& strEntityUri,
            IN IMS_BOOL bUnknownCountryAllowed, IN IMS_BOOL bIncludeState,
            OUT ByteArray& objContent) const;

    /**
     * Creates PIDF-LO XML for geolocation with position, country, state, city and postal.
     * Fails if the position is not available.
     *
     * @param strEntityUri "entity" attribute of <presence> element.
     * @param objContent ByteArray to be set the result XML.
     * @param nConfidence Text of <con:confidence> element. Use the location's if <= 0.
     * @return True if success.
     */
    IMS_BOOL CreateWithPosition(IN const AString& strEntityUri, OUT ByteArray& objContent,
            IN IMS_SINT32 nConfidence = 0) const;

    /**
     * Creates PIDF-LO XML for geolocation with position and country.
     * Fails if the position is not available.
     *
     * @param strEntityUri "entity" attribute of <presence> element.
     * @param objContent ByteArray to be set the result XML.
     * @param nConfidence Text of <con:confidence> element. Use the location's if <= 0.
     * @return True if success.
     */
    IMS_BOOL CreateWithPositionAndCountry(IN const AString& strEntityUri, OUT ByteArray& objContent,
            IN IMS_SINT32 nConfidence = 0) const;

    /**
     * Creates PIDF-LO XML for geolocation with position only.
     * Fails if the position is not available.
     *
     * @param strEntityUri "entity" attribute of <presence> element.
     * @param objContent ByteArray to be set the result XML.
     * @param nConfidence Text of <con:confidence> element. Use the location's if <= 0.
     * @return True if success.
     */
    IMS_BOOL CreateWithoutCivic(IN const AString& strEntityUri, OUT ByteArray& objContent,
            IN IMS_SINT32 nConfidence = 0) const;

    /** Resets all the features. */
    inline void ClearFeatures(IN IMS_SINT32 nFeatures) { m_nFeatures &= ~nFeatures; }
    /** Gets the current features. */
    inline IMS_SINT32 GetFeatures() const { return m_nFeatures; }
    /** Checks if the given features are set. */
    inline IMS_BOOL IsFeatureSet(IN IMS_SINT32 nFeature) const { return m_nFeatures & nFeature; }
    /** Sets the features. */
    inline void SetFeatures(IN IMS_SINT32 nFeatures) { m_nFeatures |= nFeatures; }

    /**
     * Sets the text of <dm:deviceID> element when PIDF-LO is generated using <device> element.
     * Auto-generated UUID will be used if empty.
     *
     * @param strId Can be an IMEI URN or a UUID.
     */
    inline void SetDeviceId(IN const AString& strId) { m_strDeviceId = strId; }

    /**
     * Sets the "id" attribute of <tuple> element when PIDF-LO is generated using it.
     * Default value will be used if empty.
     *
     * @param strId ID to distinguish the tuple.
     */
    inline void SetTupleId(IN const AString& strId) { m_strTupleId = strId; }

    /**
     * Sets the text of <retransmission-allowed> element.
     * The element will be omitted if empty.
     *
     * @param strRetransmissionAllowed Text to be set.
     */
    inline void SetRetransmissionAllowed(IN const AString& strRetransmissionAllowed)
    {
        m_strRetransmissionAllowed = strRetransmissionAllowed;
    }

private:
    ILocationProperties* GetLocationProperties(
            IN IMS_SINT32 nType = ILocationInfo::LOCATION_ALL) const;

    AString GetTupleId() const;
    AString GetDeviceId() const;
    AString GetDeviceName() const;

    /**
     * Creates "entity" attribute of <presence> element with "pres:" URI scheme using IMPI of
     * the specified slot.
     */
    AString CreateEntityUri() const;
    enabler::Element* CreateLocationElement(IN const AString& strTimestamp,
            IN std::initializer_list<enabler::Element*> lstChildren) const;
    enabler::Element* CreateShapeElement(IN const ILocationProperties& objLocation) const;

    IMS_BOOL IsPositionAvailable(IN const ILocationProperties* piLocation) const;

public:
    enum
    {
        /** <tuple> element is used instead of <device> element if set. */
        FEATURE_FORMAT_TUPLE = 0x00000001,
        /** <method> element is excluded if set. */
        FEATURE_NO_METHOD = 0x00000002,
        /** Unknown("ZZ") <country> element is excluded if set. */
        FEATURE_NO_COUNTRY_IF_UNKNOWN = 0x00000004,

        FEATURE_ALL = 0x7FFFFFFF
    };

private:
    IMS_SINT32 m_nFeatures;

    AString m_strDeviceName;
    AString m_strDeviceId;
    AString m_strTupleId;
    AString m_strRetransmissionAllowed;
};

#endif
