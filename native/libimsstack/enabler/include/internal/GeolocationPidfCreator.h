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

class ILocationProperties;

class GeolocationPidfCreator : public ImsSlot
{
public:
    explicit GeolocationPidfCreator(IN IMS_SINT32 nSlotId);
    virtual ~GeolocationPidfCreator();

    GeolocationPidfCreator(IN const GeolocationPidfCreator&) = delete;
    GeolocationPidfCreator& operator=(IN const GeolocationPidfCreator&) = delete;

public:
    // This method creates PIDF for Geolocation with country only or country and state.
    // but if country is not determined, then don't create PIDF.
    IMS_BOOL CreateWithoutPosition(IN const AString& strEntityUri,
            IN IMS_BOOL bUnknownCountryAllowed, IN IMS_BOOL bIncludeState,
            OUT ByteArray& objContent) const;
    // This method creates PIDF for Geolocation with position information.
    // If position is not available, it returns IMS_FALSE.
    IMS_BOOL CreateWithPosition(IN const AString& strEntityUri, OUT ByteArray& objContent,
            IN IMS_SINT32 nConfidence = 0) const;
    // This method creates PIDF for Geolocation with position and country information.
    // The PIDF will not have City, State, and Zip code information.
    // If position is not available, it returns IMS_FALSE.
    IMS_BOOL CreateWithPositionAndCountry(IN const AString& strEntityUri, OUT ByteArray& objContent,
            IN IMS_SINT32 nConfidence = 0) const;
    // This method creates PIDF for Geolocation with position information without CIVIC.
    // If position is not available, it returns IMS_FALSE.
    IMS_BOOL CreateWithoutCivic(IN const AString& strEntityUri, OUT ByteArray& objContent,
            IN IMS_SINT32 nConfidence = 0) const;

    // FEATURE_XXX
    inline void ClearFeatures(IN IMS_SINT32 nFeatures) { m_nFeatures &= ~nFeatures; }
    inline IMS_SINT32 GetFeatures() const { return m_nFeatures; }
    inline IMS_BOOL IsFeatureSet(IN IMS_SINT32 nFeature) const
    {
        return (m_nFeatures & nFeature) == nFeature;
    }
    inline void SetFeatures(IN IMS_SINT32 nFeatures) { m_nFeatures |= nFeatures; }

    // IMEI URN or UUID
    void SetDeviceId(IN const AString& strId);
    // "id" attribute of tuple element
    void SetTupleId(IN const AString& strId);

    // It's created with "pres:" URI scheme using IMPI of specified slot
    static AString CreateEntityUri(IN IMS_SINT32 nSlotId);

private:
    inline const AString& GetDeviceId() const { return m_strDeviceId; }
    inline const AString& GetDeviceName() const { return m_strDeviceName; }
    ILocationProperties* GetLocationProperties(
            IN IMS_SINT32 nType = ILocationInfo::LOCATION_ALL) const;
    const AString& GetTupleId() const;

public:
    /// Additional features to be used when creating Geolocation PIDF XML
    enum
    {
        /// "device" PIDF XML is a default,
        /// but the application wants to use "tuple" PIDF XML, then it's set by the application.
        FEATURE_FORMAT_TUPLE = 0x00000001,
        /// Identifies that "method" element needs to be included or not in PIDF XML.
        FEATURE_NO_METHOD = 0x00000002,
        /// Identifies that "country" element needs to be included or not in PIDF XML
        /// when it's unknown ("ZZ").
        FEATURE_NO_COUNTRY_IF_UNKNOWN = 0x00000004,

        FEATURE_ALL = 0x7FFFFFFF
    };

    /// Namespaces for PIDF-LO
    enum
    {
        NAMESPACE_NONE = 0x0000,

        /// urn:ietf:params:xml:ns:pidf:data-model
        NAMESPACE_DM = 0x0001,
        /// urn:ietf:params:xml:ns:pidf:geopriv10
        NAMESPACE_GP = 0x0002,
        /// http://www.opengis.net/gml
        NAMESPACE_GML = 0x0004,
        /// http://www.opengis.net/pidflo/1.0
        NAMESPACE_GS = 0x0008,
        /// urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr
        NAMESPACE_CL = 0x0010,
        /// urn:ietf:params:xml:ns:geopriv:conf
        NAMESPACE_CON = 0x0020,

        NAMESPACE_ALL = 0xFFFF,

        /// Namespace for country information only when forming PIDF XML
        NAMESPACE_COUNTRY = (NAMESPACE_DM | NAMESPACE_GP | NAMESPACE_CL),
    };

private:
    // FEATURE_XXX
    IMS_SINT32 m_nFeatures;
    // "id" attribute of "device" element
    AString m_strDeviceName;
    // IMEI URN or UUID
    AString m_strDeviceId;
    // "id" attribute of "tuple" element
    // It's required when FEATURE_FORMAT_TUPLE is enabled.
    AString m_strTupleId;
};

#endif
