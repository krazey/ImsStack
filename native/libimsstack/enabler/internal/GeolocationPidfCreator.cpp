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
#include "ImsUuid.h"
#include "ServiceMemory.h"
#include "ServicePhoneInfo.h"
#include "ServiceSystemTime.h"
#include "ServiceTrace.h"
#include "TextParser.h"

#include "Engine.h"
#include "IConfiguration.h"
#include "ISubscriberConfig.h"

#include "IXmlStreamWriter.h"
#include "XmlFactory.h"

#include "GeolocationPidfCreator.h"

__IMS_TRACE_TAG_BASE__;

#define DEFAULT_TUPLE_ID "VoLte"

LOCAL void geolocationPidfCreator_CreateEndElement(IN_OUT IXmlStreamWriter* piWriter)
{
    piWriter->WriteEndElement();
    piWriter->WriteCharacters(TextParser::STR_LF);
}

LOCAL void geolocationPidfCreator_CreateRootElement(IN_OUT IXmlStreamWriter* piWriter,
        IN const AString& strEntityUri, IN IMS_SINT32 nNamespaces)
{
    piWriter->WriteStartElement("presence");

    piWriter->WriteDefaultNamespace("urn:ietf:params:xml:ns:pidf");

    if ((nNamespaces & GeolocationPidfCreator::NAMESPACE_DM) != 0)
    {
        piWriter->WriteNamespace("dm", "urn:ietf:params:xml:ns:pidf:data-model");
    }

    if ((nNamespaces & GeolocationPidfCreator::NAMESPACE_GP) != 0)
    {
        piWriter->WriteNamespace("gp", "urn:ietf:params:xml:ns:pidf:geopriv10");
    }

    if ((nNamespaces & GeolocationPidfCreator::NAMESPACE_GML) != 0)
    {
        piWriter->WriteNamespace("gml", "http://www.opengis.net/gml");
    }

    if ((nNamespaces & GeolocationPidfCreator::NAMESPACE_GS) != 0)
    {
        piWriter->WriteNamespace("gs", "http://www.opengis.net/pidflo/1.0");
    }

    if ((nNamespaces & GeolocationPidfCreator::NAMESPACE_CL) != 0)
    {
        piWriter->WriteNamespace("cl", "urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr");
    }

    if ((nNamespaces & GeolocationPidfCreator::NAMESPACE_CON) != 0)
    {
        piWriter->WriteNamespace("con", "urn:ietf:params:xml:ns:geopriv:conf");
    }

    piWriter->WriteAttribute("entity", strEntityUri);
    piWriter->WriteCharacters(TextParser::STR_LF);
}

LOCAL void geolocationPidfCreator_CreateXMLStartLine(IN_OUT IXmlStreamWriter* piWriter)
{
    piWriter->WriteStartDocument("UTF-8", "1.0");
    piWriter->WriteCharacters(TextParser::STR_LF);
}

LOCAL void geolocationPidfCreator_CreateCirclePosition(IN_OUT IXmlStreamWriter* piWriter,
        IN const AString& strPosition, IN const AString& strRadius, IN const AString& strConfidence)
{
    if ((strPosition.GetLength() != 0) || (strRadius.GetLength() != 0))
    {
        // gs:Circle element
        piWriter->WriteStartElement("gs:Circle");
        piWriter->WriteAttribute("srsName", "urn:ogc:def:crs:EPSG::4326");
        piWriter->WriteCharacters(TextParser::STR_LF);

        if (strPosition.GetLength() != 0)
        {
            // gml:pos element
            piWriter->WriteStartElement("gml:pos");
            piWriter->WriteCharacters(strPosition);
            geolocationPidfCreator_CreateEndElement(piWriter);
        }

        if (strRadius.GetLength() != 0)
        {
            // gs:radius element
            piWriter->WriteStartElement("gs:radius");
            piWriter->WriteAttribute("uom", "urn:ogc:def:uom:EPSG::9001");
            piWriter->WriteCharacters(strRadius);
            geolocationPidfCreator_CreateEndElement(piWriter);
        }

        // end of gs:Circle
        geolocationPidfCreator_CreateEndElement(piWriter);
    }

    if (strConfidence.GetLength() != 0)
    {
        // con:confidence element
        piWriter->WriteStartElement("con:confidence");
        piWriter->WriteAttribute("pdf", "normal");
        piWriter->WriteCharacters(strConfidence);
        geolocationPidfCreator_CreateEndElement(piWriter);
    }
}

LOCAL void geolocationPidfCreator_CreateEllipsoidPosition(IN_OUT IXmlStreamWriter* piWriter,
        IN const AString& strPosition, IN const AString& strRadius,
        IN const AString& strVerticalAccuracy, IN const AString& strConfidence)
{
    if (strPosition.GetLength() != 0 ||
            (strRadius.GetLength() != 0 && strVerticalAccuracy.GetLength() != 0))
    {
        // gs:Ellipsoid element
        piWriter->WriteStartElement("gs:Ellipsoid");
        piWriter->WriteAttribute("srsName", "urn:ogc:def:crs:EPSG::4979");
        piWriter->WriteCharacters(TextParser::STR_LF);

        if (strPosition.GetLength() != 0)
        {
            // gml:pos element
            piWriter->WriteStartElement("gml:pos");
            piWriter->WriteCharacters(strPosition);
            geolocationPidfCreator_CreateEndElement(piWriter);
        }

        if (strRadius.GetLength() != 0 && strVerticalAccuracy.GetLength() != 0)
        {
            // gs:semiMajorAxis element
            piWriter->WriteStartElement("gs:semiMajorAxis");
            piWriter->WriteAttribute("uom", "urn:ogc:def:uom:EPSG::9001");
            piWriter->WriteCharacters(strRadius);
            geolocationPidfCreator_CreateEndElement(piWriter);

            // gs:semiMinorAxis element
            piWriter->WriteStartElement("gs:semiMinorAxis");
            piWriter->WriteAttribute("uom", "urn:ogc:def:uom:EPSG::9001");
            piWriter->WriteCharacters(strRadius);
            geolocationPidfCreator_CreateEndElement(piWriter);

            // gs:verticalAxis element
            piWriter->WriteStartElement("gs:verticalAxis");
            piWriter->WriteAttribute("uom", "urn:ogc:def:uom:EPSG::9001");
            piWriter->WriteCharacters(strVerticalAccuracy);
            geolocationPidfCreator_CreateEndElement(piWriter);
        }

        // gs:orientation element
        piWriter->WriteStartElement("gs:orientation");
        piWriter->WriteAttribute("uom", "urn:ogc:def:uom:EPSG::9102");
        piWriter->WriteCharacters("0");
        geolocationPidfCreator_CreateEndElement(piWriter);

        // end of gs:Ellipsoid
        geolocationPidfCreator_CreateEndElement(piWriter);
    }

    if (strConfidence.GetLength() != 0)
    {
        // con:confidence element
        piWriter->WriteStartElement("con:confidence");
        piWriter->WriteAttribute("pdf", "normal");
        piWriter->WriteCharacters(strConfidence);
        geolocationPidfCreator_CreateEndElement(piWriter);
    }
}

LOCAL void geolocationPidfCreator_CreateCivicAddress(IN_OUT IXmlStreamWriter* piWriter,
        IN const AString& strCountry, IN const AString& strState, IN const AString& strCity,
        IN const AString& strPostal)
{
    if ((strCountry.GetLength() != 0) || (strState.GetLength() != 0) ||
            (strCity.GetLength() != 0) || (strPostal.GetLength() != 0))
    {
        // cl:civicAddress element
        piWriter->WriteStartElement("cl:civicAddress");
        piWriter->WriteCharacters(TextParser::STR_LF);

        if (strCountry.GetLength() != 0)
        {
            // cl:country element
            piWriter->WriteStartElement("cl:country");
            piWriter->WriteCharacters(strCountry);
            geolocationPidfCreator_CreateEndElement(piWriter);
        }

        if (strState.GetLength() != 0)
        {
            // cl:A1 element
            piWriter->WriteStartElement("cl:A1");
            piWriter->WriteCharacters(strState);
            geolocationPidfCreator_CreateEndElement(piWriter);
        }

        if (strCity.GetLength() != 0)
        {
            // cl:A2 element
            piWriter->WriteStartElement("cl:A2");
            piWriter->WriteCharacters(strCity);
            geolocationPidfCreator_CreateEndElement(piWriter);
        }

        if (strPostal.GetLength() != 0)
        {
            // cl:PC element
            piWriter->WriteStartElement("cl:PC");
            piWriter->WriteCharacters(strPostal);
            geolocationPidfCreator_CreateEndElement(piWriter);
        }

        // end of cl:civicAddress
        geolocationPidfCreator_CreateEndElement(piWriter);
    }
}

LOCAL void geolocationPidfCreator_CreatePIDFForDevice(IN_OUT IXmlStreamWriter* piWriter,
        IN const AString& strDeviceName, IN const AString& strDeviceId,
        IN const AString& strCountry, IN const AString& strState, IN const AString& strCity,
        IN const AString& strPostal, IN const AString& strPosition, IN const AString& strRadius,
        IN const AString& strVerticalAccuracy, IN const AString& strShape,
        IN const AString& strConfidence, IN const AString& strMethod,
        IN const AString& strTimeStamp)
{
    // dm:device element
    piWriter->WriteStartElement("dm:device");
    piWriter->WriteAttribute("id", strDeviceName);
    piWriter->WriteCharacters(TextParser::STR_LF);

    // gp:geopriv element
    piWriter->WriteStartElement("gp:geopriv");
    piWriter->WriteCharacters(TextParser::STR_LF);

    // gp:location-info element
    piWriter->WriteStartElement("gp:location-info");
    piWriter->WriteCharacters(TextParser::STR_LF);

    // cl:civicAddress
    geolocationPidfCreator_CreateCivicAddress(piWriter, strCountry, strState, strCity, strPostal);

    // Geodetic Shape Representation (gs:...)
    if (strShape.EqualsIgnoreCase("Circle"))
    {
        geolocationPidfCreator_CreateCirclePosition(
                piWriter, strPosition, strRadius, strConfidence);
    }
    else if (strShape.EqualsIgnoreCase("Ellipsoid"))
    {
        geolocationPidfCreator_CreateEllipsoidPosition(
                piWriter, strPosition, strRadius, strVerticalAccuracy, strConfidence);
    }

    // end of gp:location-info
    geolocationPidfCreator_CreateEndElement(piWriter);

    // gp:usage-rules element
    piWriter->WriteEmptyElement("gp:usage-rules");
    piWriter->WriteCharacters(TextParser::STR_LF);

    if (strMethod.GetLength() != 0)
    {
        // gp:method element
        piWriter->WriteStartElement("gp:method");
        piWriter->WriteCharacters(strMethod);
        geolocationPidfCreator_CreateEndElement(piWriter);
    }

    // end of gp:geopriv
    geolocationPidfCreator_CreateEndElement(piWriter);

    // dm:deviceID element
    piWriter->WriteStartElement("dm:deviceID");
    piWriter->WriteCharacters(strDeviceId);
    geolocationPidfCreator_CreateEndElement(piWriter);

    if (strTimeStamp.GetLength() != 0)
    {
        // dm:timestamp element
        piWriter->WriteStartElement("dm:timestamp");
        piWriter->WriteCharacters(strTimeStamp);
        geolocationPidfCreator_CreateEndElement(piWriter);
    }

    // end of dm:device
    geolocationPidfCreator_CreateEndElement(piWriter);
}

LOCAL void geolocationPidfCreator_CreatePIDFForTuple(IN_OUT IXmlStreamWriter* piWriter,
        IN const AString& strTupleId, IN const AString& strCountry, IN const AString& strState,
        IN const AString& strCity, IN const AString& strPostal, IN const AString& strPosition,
        IN const AString& strRadius, IN const AString& strVerticalAccuracy,
        IN const AString& strShape, IN const AString& strConfidence, IN const AString& strMethod,
        IN const AString& strTimeStamp)
{
    // tuple element
    piWriter->WriteStartElement("tuple");
    piWriter->WriteAttribute("id", strTupleId);
    piWriter->WriteCharacters(TextParser::STR_LF);

    // status element
    piWriter->WriteStartElement("status");
    piWriter->WriteCharacters(TextParser::STR_LF);

    // gp:geopriv element
    piWriter->WriteStartElement("gp:geopriv");
    piWriter->WriteCharacters(TextParser::STR_LF);

    // gp:location-info element
    piWriter->WriteStartElement("gp:location-info");
    piWriter->WriteCharacters(TextParser::STR_LF);

    // cl:civicAddress
    geolocationPidfCreator_CreateCivicAddress(piWriter, strCountry, strState, strCity, strPostal);

    // Geodetic Shape Representation (gs:...)
    if (strShape.EqualsIgnoreCase("Circle"))
    {
        geolocationPidfCreator_CreateCirclePosition(
                piWriter, strPosition, strRadius, strConfidence);
    }
    else if (strShape.EqualsIgnoreCase("Ellipsoid"))
    {
        geolocationPidfCreator_CreateEllipsoidPosition(
                piWriter, strPosition, strRadius, strVerticalAccuracy, strConfidence);
    }

    // end of gp:location-info
    geolocationPidfCreator_CreateEndElement(piWriter);

    // gp:usage-rules element
    piWriter->WriteEmptyElement("gp:usage-rules");
    piWriter->WriteCharacters(TextParser::STR_LF);

    if (strMethod.GetLength() != 0)
    {
        // gp:method element
        piWriter->WriteStartElement("gp:method");
        piWriter->WriteCharacters(strMethod);
        geolocationPidfCreator_CreateEndElement(piWriter);
    }

    // end of gp:geopriv
    geolocationPidfCreator_CreateEndElement(piWriter);

    // end of status
    geolocationPidfCreator_CreateEndElement(piWriter);

    if (strTimeStamp.GetLength() != 0)
    {
        // dm:timestamp element
        piWriter->WriteStartElement("dm:timestamp");
        piWriter->WriteCharacters(strTimeStamp);
        geolocationPidfCreator_CreateEndElement(piWriter);
    }

    // end of tuple
    geolocationPidfCreator_CreateEndElement(piWriter);
}

LOCAL void geolocationPidfCreator_CreatePIDF(IN const AString& strEntityUri,
        IN const AString& strTupleId, IN const AString& strDeviceName,
        IN const AString& strPredefinedDeviceId, IN const AString& strCountry,
        IN const AString& strState, IN const AString& strCity, IN const AString& strPostal,
        IN const AString& strPosition, IN const AString& strRadius,
        IN const AString& strVerticalAccuracy, IN const AString& strShape,
        IN const AString& strConfidence, IN const AString& strMethod,
        IN const AString& strTimeStamp, OUT ByteArray& objContent,
        IN IMS_SINT32 nNamespaces = GeolocationPidfCreator::NAMESPACE_ALL)
{
    XmlFactory* pXmlFactory = XmlFactory::GetInstance();
    IXmlStreamWriter* piWriter = pXmlFactory->CreateStreamWriter();

    if (piWriter == IMS_NULL)
    {
        return;
    }

    geolocationPidfCreator_CreateXMLStartLine(piWriter);

    // "presence" element
    geolocationPidfCreator_CreateRootElement(piWriter, strEntityUri, nNamespaces);

    if (strTupleId.GetLength() > 0)
    {
        geolocationPidfCreator_CreatePIDFForTuple(piWriter, strTupleId, strCountry, strState,
                strCity, strPostal, strPosition, strRadius, strVerticalAccuracy, strShape,
                strConfidence, strMethod, strTimeStamp);
    }
    else
    {
        AString strDeviceId;

        if (strPredefinedDeviceId.GetLength() > 0)
        {
            strDeviceId = strPredefinedDeviceId;
        }
        else
        {
            strDeviceId = ImsUuid::GetUuid(ImsUuid::VERSION_1);
            strDeviceId.Prepend("urn:uuid:");
        }

        geolocationPidfCreator_CreatePIDFForDevice(piWriter, strDeviceName, strDeviceId, strCountry,
                strState, strCity, strPostal, strPosition, strRadius, strVerticalAccuracy, strShape,
                strConfidence, strMethod, strTimeStamp);
    }

    // end of presence
    geolocationPidfCreator_CreateEndElement(piWriter);

    piWriter->WriteEndDocument();

    // Set content
    IMS_CHAR* pszXml = piWriter->Flush();

    if (pszXml != IMS_NULL)
    {
        objContent.Attach(reinterpret_cast<IMS_BYTE*>(pszXml), piWriter->GetContentLength());
        objContent.Detach();

        IMS_MEM_Free(pszXml);
    }

    piWriter->Close();

    pXmlFactory->DestroyStreamWriter(piWriter);
}

PUBLIC
GeolocationPidfCreator::GeolocationPidfCreator(IN IMS_SINT32 nSlotId) :
        ImsSlot(nSlotId),
        m_nFeatures(0),
        m_strDeviceName(AString::ConstNull()),
        m_strDeviceId(AString::ConstNull()),
        m_strTupleId(DEFAULT_TUPLE_ID)
{
    PhoneInfoService::GetPhoneInfoService()->GetDeviceInfo()->GetDeviceName(m_strDeviceName);

    if (m_strDeviceName.GetLength() == 0)
    {
        m_strDeviceName = "Phone";
    }
}

PUBLIC VIRTUAL GeolocationPidfCreator::~GeolocationPidfCreator() {}

// This method creates PIDF for Geolocation with country only or country and state.
// but if country is not determined, then don't create PIDF.
PUBLIC
IMS_BOOL GeolocationPidfCreator::CreateWithoutPosition(IN const AString& strEntityUri,
        IN IMS_BOOL bUnknownCountryAllowed, IN IMS_BOOL bIncludeState,
        OUT ByteArray& objContent) const
{
    IMS_SINT32 nType = ILocationInfo::LOCATION_ALL;

    if (bIncludeState == IMS_FALSE)
    {
        nType = ILocationInfo::LOCATION_POSITION_N_COUNTRY;
    }

    ILocationProperties* piLocation = GetLocationProperties(nType);

    if (piLocation == IMS_NULL)
    {
        IMS_TRACE_E(0, "ILocationProperties is not available", 0, 0, 0);
        return IMS_FALSE;
    }

    const AString& strCountry = piLocation->GetCountry();

    if (!bUnknownCountryAllowed && ((strCountry.GetLength() == 0) || strCountry.Equals("ZZ")))
    {
        IMS_TRACE_I("Country is empty or unknown", 0, 0, 0);
        return IMS_FALSE;
    }

    const AString& strCurrentTime = piLocation->GetCurrentTime();
    const AString& strState = bIncludeState ? piLocation->GetState() : AString::ConstNull();

    geolocationPidfCreator_CreatePIDF(
            (strEntityUri.GetLength() > 0) ? strEntityUri : CreateEntityUri(GetSlotId()),
            GetTupleId(), GetDeviceName(), GetDeviceId(), strCountry, strState,
            AString::ConstNull(), AString::ConstNull(), AString::ConstNull(), AString::ConstNull(),
            AString::ConstNull(), AString::ConstNull(), AString::ConstNull(), AString::ConstNull(),
            strCurrentTime, objContent, NAMESPACE_COUNTRY);

    return objContent.GetLength() != 0;
}

// This method creates PIDF for Geolocation with position information.
// If position is not available, it returns IMS_FALSE.
PUBLIC
IMS_BOOL GeolocationPidfCreator::CreateWithPosition(IN const AString& strEntityUri,
        OUT ByteArray& objContent, IN IMS_SINT32 nConfidence /*= 0*/) const
{
    ILocationProperties* piLocation = GetLocationProperties();

    if (piLocation == IMS_NULL)
    {
        IMS_TRACE_E(0, "ILocationProperties is not available", 0, 0, 0);
        return IMS_FALSE;
    }

    const AString& strLatitude = piLocation->GetLatitude();
    const AString& strLongitude = piLocation->GetLongitude();

    if (((strLatitude.GetLength() == 0) || (strLongitude.GetLength() == 0)) ||
            (strLatitude.Equals("0.0") && strLongitude.Equals("0.0")))
    {
        IMS_TRACE_I("Lat/Lon is not available", 0, 0, 0);
        return IMS_FALSE;
    }

    const AString& strRadius = piLocation->GetRadius();
    const AString& strShape = piLocation->GetShape();
    AString strConfidence = piLocation->GetConfidence();
    const AString& strCurrentTime = piLocation->GetCurrentTime();
    const AString& strMethod =
            IsFeatureSet(FEATURE_NO_METHOD) ? AString::ConstNull() : piLocation->GetMethod();
    const AString& strCountry =
            IsFeatureSet(FEATURE_NO_COUNTRY_IF_UNKNOWN) && piLocation->GetCountry().Equals("ZZ")
            ? AString::ConstNull()
            : piLocation->GetCountry();
    const AString& strState = piLocation->GetState();
    const AString& strCity = piLocation->GetCity();
    const AString& strPostal = piLocation->GetPostal();
    const AString& strAltitude = piLocation->GetAltitude();
    const AString& strVerticalAccuracy = piLocation->GetVerticalAccuracy();
    AString strPosition;

    strPosition.Sprintf("%s %s", strLatitude.GetStr(), strLongitude.GetStr());
    if (strAltitude.GetLength() != 0 && strShape.EqualsIgnoreCase("Ellipsoid"))
    {
        strPosition.Append(" ");
        strPosition.Append(strAltitude);
    }

    if (nConfidence > 0)
    {
        strConfidence.Sprintf("%d", nConfidence);
    }

    geolocationPidfCreator_CreatePIDF(
            (strEntityUri.GetLength() > 0) ? strEntityUri : CreateEntityUri(GetSlotId()),
            GetTupleId(), GetDeviceName(), GetDeviceId(), strCountry, strState, strCity, strPostal,
            strPosition, strRadius, strVerticalAccuracy, strShape, strConfidence, strMethod,
            strCurrentTime, objContent);

    return objContent.GetLength() != 0;
}

// This method creates PIDF for Geolocation with position and country information.
// The PIDF will not have City, State, and Zip code information.
// If position is not available, it returns IMS_FALSE.
PUBLIC
IMS_BOOL GeolocationPidfCreator::CreateWithPositionAndCountry(IN const AString& strEntityUri,
        OUT ByteArray& objContent, IN IMS_SINT32 nConfidence /*= 0*/) const
{
    ILocationProperties* piLocation =
            GetLocationProperties(ILocationInfo::LOCATION_POSITION_N_COUNTRY);

    if (piLocation == IMS_NULL)
    {
        IMS_TRACE_E(0, "ILocationProperties is not available", 0, 0, 0);
        return IMS_FALSE;
    }

    const AString& strLatitude = piLocation->GetLatitude();
    const AString& strLongitude = piLocation->GetLongitude();

    if (((strLatitude.GetLength() == 0) || (strLongitude.GetLength() == 0)) ||
            (strLatitude.Equals("0.0") && strLongitude.Equals("0.0")))
    {
        IMS_TRACE_I("Lat/Lon is not available", 0, 0, 0);
        return IMS_FALSE;
    }

    const AString& strRadius = piLocation->GetRadius();
    const AString& strShape = piLocation->GetShape();
    AString strConfidence = piLocation->GetConfidence();
    const AString& strCurrentTime = piLocation->GetCurrentTime();
    const AString& strMethod =
            IsFeatureSet(FEATURE_NO_METHOD) ? AString::ConstNull() : piLocation->GetMethod();
    const AString& strCountry =
            IsFeatureSet(FEATURE_NO_COUNTRY_IF_UNKNOWN) && piLocation->GetCountry().Equals("ZZ")
            ? AString::ConstNull()
            : piLocation->GetCountry();
    const AString& strAltitude = piLocation->GetAltitude();
    const AString& strVerticalAccuracy = piLocation->GetVerticalAccuracy();
    AString strPosition;

    strPosition.Sprintf("%s %s", strLatitude.GetStr(), strLongitude.GetStr());
    if (strAltitude.GetLength() != 0 && strShape.EqualsIgnoreCase("Ellipsoid"))
    {
        strPosition.Append(" ");
        strPosition.Append(strAltitude);
    }

    if (nConfidence > 0)
    {
        strConfidence.Sprintf("%d", nConfidence);
    }

    geolocationPidfCreator_CreatePIDF(
            (strEntityUri.GetLength() > 0) ? strEntityUri : CreateEntityUri(GetSlotId()),
            GetTupleId(), GetDeviceName(), GetDeviceId(), strCountry, AString::ConstNull(),
            AString::ConstNull(), AString::ConstNull(), strPosition, strRadius, strVerticalAccuracy,
            strShape, strConfidence, strMethod, strCurrentTime, objContent);

    return objContent.GetLength() != 0;
}

// This method creates PIDF for Geolocation with position information without CIVIC.
// If position is not available, it returns IMS_FALSE.
PUBLIC
IMS_BOOL GeolocationPidfCreator::CreateWithoutCivic(IN const AString& strEntityUri,
        OUT ByteArray& objContent, IN IMS_SINT32 nConfidence /*= 0*/) const
{
    ILocationProperties* piLocation = GetLocationProperties(ILocationInfo::LOCATION_POSITION);

    if (piLocation == IMS_NULL)
    {
        IMS_TRACE_E(0, "ILocationProperties is not available", 0, 0, 0);
        return IMS_FALSE;
    }

    const AString& strLatitude = piLocation->GetLatitude();
    const AString& strLongitude = piLocation->GetLongitude();

    if (((strLatitude.GetLength() == 0) || (strLongitude.GetLength() == 0)) ||
            (strLatitude.Equals("0.0") && strLongitude.Equals("0.0")))
    {
        IMS_TRACE_I("Lat/Lon is not available", 0, 0, 0);
        return IMS_FALSE;
    }

    const AString& strRadius = piLocation->GetRadius();
    const AString& strShape = piLocation->GetShape();
    AString strConfidence = piLocation->GetConfidence();
    const AString& strCurrentTime = piLocation->GetCurrentTime();
    const AString& strMethod =
            IsFeatureSet(FEATURE_NO_METHOD) ? AString::ConstNull() : piLocation->GetMethod();
    const AString& strAltitude = piLocation->GetAltitude();
    const AString& strVerticalAccuracy = piLocation->GetVerticalAccuracy();
    AString strPosition;

    strPosition.Sprintf("%s %s", strLatitude.GetStr(), strLongitude.GetStr());
    if (strAltitude.GetLength() != 0 && strShape.EqualsIgnoreCase("Ellipsoid"))
    {
        strPosition.Append(" ");
        strPosition.Append(strAltitude);
    }

    if (nConfidence > 0)
    {
        strConfidence.Sprintf("%d", nConfidence);
    }

    geolocationPidfCreator_CreatePIDF(
            (strEntityUri.GetLength() > 0) ? strEntityUri : CreateEntityUri(GetSlotId()),
            GetTupleId(), GetDeviceName(), GetDeviceId(), AString::ConstNull(),
            AString::ConstNull(), AString::ConstNull(), AString::ConstNull(), strPosition,
            strRadius, strVerticalAccuracy, strShape, strConfidence, strMethod, strCurrentTime,
            objContent);

    return objContent.GetLength() != 0;
}

// IMEI URN or UUID
PUBLIC
void GeolocationPidfCreator::SetDeviceId(IN const AString& strId)
{
    if (!m_strDeviceId.Equals(strId))
    {
        IMS_TRACE_D("DeviceId :: %s >> %s", m_strDeviceId.GetStr(), strId.GetStr(), 0);

        m_strDeviceId = strId;
    }
}

// "id" attribute of tuple element
PUBLIC
void GeolocationPidfCreator::SetTupleId(IN const AString& strId)
{
    if (!m_strTupleId.Equals(strId))
    {
        IMS_TRACE_D("TupleId :: %s >> %s", m_strTupleId.GetStr(), strId.GetStr(), 0);

        m_strTupleId = strId;

        if (m_strTupleId.GetLength() == 0)
        {
            m_strTupleId = DEFAULT_TUPLE_ID;
        }
    }
}

PUBLIC GLOBAL AString GeolocationPidfCreator::CreateEntityUri(IN IMS_SINT32 nSlotId)
{
    const ISubscriberConfig* piSubsConfig =
            Engine::GetConfiguration()->GetSubscriberConfig(nSlotId);

    if (piSubsConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "ISubscriberConfig is null", 0, 0, 0);
        return AString::ConstNull();
    }

    AString strUri;
    strUri.Sprintf("pres:%s", piSubsConfig->GetPrivateUserId().GetStr());

    return strUri;
}

PRIVATE
ILocationProperties* GeolocationPidfCreator::GetLocationProperties(IN IMS_SINT32 nType) const
{
    ILocationInfo* piLocationInfo =
            PhoneInfoService::GetPhoneInfoService()->GetLocationInfo(GetSlotId());

    return (piLocationInfo != IMS_NULL) ? piLocationInfo->GetLocationProperties(nType) : IMS_NULL;
}

PRIVATE
const AString& GeolocationPidfCreator::GetTupleId() const
{
    if (IsFeatureSet(FEATURE_FORMAT_TUPLE))
    {
        return m_strTupleId;
    }

    return AString::ConstNull();
}
