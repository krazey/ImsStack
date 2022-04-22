/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20170704  hwangoo.park@             Created
    </table>

    Description
*/

#ifndef _GEOLOCATION_PIDF_CREATOR_H_
#define _GEOLOCATION_PIDF_CREATOR_H_

#include "ByteArray.h"
#include "ImsSlot.h"
#include "IPhoneInfoLocation.h"

class ILocationProperties;

class GeolocationPidfCreator
    : public IMSSlot
{
public:
    GeolocationPidfCreator(IN IMS_SINT32 nSlotId_);
    virtual ~GeolocationPidfCreator();

private:
    GeolocationPidfCreator(IN const GeolocationPidfCreator &objRHS);
    GeolocationPidfCreator& operator=(IN const GeolocationPidfCreator &objRHS);

public:
    // This method creates PIDF for Geolocation with country only.
    IMS_BOOL Create(IN const AString &strEntityUri,
            IN const AString &strCountry, OUT ByteArray &objContent);
    // This method creates PIDF for Geolocation based on the option (country info.),
    // but if country is not determined, then don't create PIDF.
    IMS_BOOL Create(IN const AString &strEntityUri,
            IN IMS_BOOL bUnknownCountryAllowed, OUT ByteArray &objContent);
    // This method creates PIDF for Geolocation with country only or country and state.
    // but if country is not determined, then don't create PIDF.
    IMS_BOOL CreateWithoutPosition(IN const AString &strEntityUri,
            IN IMS_BOOL bUnknownCountryAllowed, IN IMS_BOOL bIncludeState,
            OUT ByteArray &objContent);
    // This method creates PIDF for Geolocation with position information.
    // If position is not available, it returns IMS_FALSE.
    IMS_BOOL CreateWithPosition(IN const AString &strEntityUri,
            OUT ByteArray &objContent, IN IMS_SINT32 nConfidence = 0);
    // This method creates PIDF for Geolocation with position and country information.
    // The PIDF will not have City, State, and Zip code information.
    // If position is not available, it returns IMS_FALSE.
    IMS_BOOL CreateWithPositionAndCountry(IN const AString &strEntityUri,
            OUT ByteArray &objContent, IN IMS_SINT32 nConfidence = 0);
    // This method creates PIDF for Geolocation with position information without CIVIC.
    // If position is not available, it returns IMS_FALSE.
    IMS_BOOL CreateWithoutCivic(IN const AString &strEntityUri,
            OUT ByteArray &objContent, IN IMS_SINT32 nConfidence = 0);

    // FEATURE_XXX
    inline void ClearFeatures(IN IMS_SINT32 nFeatures)
    { this->nFeatures &= ~nFeatures; }
    inline IMS_SINT32 GetFeatures() const
    { return nFeatures; }
    inline IMS_BOOL IsFeatureSet(IN IMS_SINT32 nFeature) const
    { return (nFeatures & nFeature) == nFeature; }
    inline void SetFeatures(IN IMS_SINT32 nFeatures)
    { this->nFeatures |= nFeatures; }

    // IMEI URN or UUID
    void SetDeviceId(IN const AString &strId);
    // "id" attribute of tuple element
    void SetTupleId(IN const AString& strId);

    // It's created with "pres:" URI scheme using IMPI of specified slot
    static AString CreateEntityUri(IN IMS_SINT32 nSlotId);

private:
    const AString& GetDeviceId() const;
    const AString& GetDeviceName() const;
    ILocationProperties* GetLocationProperties(IN IMS_SINT32 nType = ILocationInfo::LOCATION_ALL) const;
    const AString& GetTupleId() const;

public:
    // Additional features to be used when creating Geolocation PIDF XML
    enum
    {
        // "device" PIDF XML is a default,
        // but the application wants to use "tuple" PIDF XML, then it's set by the application.
        FEATURE_FORMAT_TUPLE = 0x00000001,
        // Identifies that "method" element needs to be included or not in PIDF XML.
        FEATURE_NO_METHOD = 0x00000002,
        // Identifies that "country" element needs to be included or not in PIDF XML
        // when it's unknown ("ZZ").
        FEATURE_NO_COUNTRY_IF_UNKNOWN = 0x00000004,

        FEATURE_ALL = 0x7FFFFFFF
    };

    // Namespaces for PIDF-LO
    enum
    {
        NAMESPACE_NONE = 0x0000,

        // urn:ietf:params:xml:ns:pidf:data-model
        NAMESPACE_DM = 0x0001,
        // urn:ietf:params:xml:ns:pidf:geopriv10
        NAMESPACE_GP = 0x0002,
        // http://www.opengis.net/gml
        NAMESPACE_GML = 0x0004,
        // http://www.opengis.net/pidflo/1.0
        NAMESPACE_GS = 0x0008,
        // urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr
        NAMESPACE_CL = 0x0010,
        // urn:ietf:params:xml:ns:geopriv:conf
        NAMESPACE_CON = 0x0020,

        NAMESPACE_ALL = 0xFFFF,

        // Namespace for country information only when forming PIDF XML
        NAMESPACE_COUNTRY = (NAMESPACE_DM | NAMESPACE_GP | NAMESPACE_CL),
    };

private:
    // FEATURE_XXX
    IMS_SINT32 nFeatures;
    // "id" attribute of "device" element
    AString strDeviceName;
    // IMEI URN or UUID
    AString strDeviceId;
    // "id" attribute of "tuple" element
    // It's required when FEATURE_FORMAT_TUPLE is enabled.
    AString strTupleId;
};

#endif // _GEOLOCATION_PIDF_CREATOR_H_
