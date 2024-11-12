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
#include "ByteArray.h"
#include "GeolocationPidfWriter.h"
#include "IXmlStreamWriter.h"
#include "ImsTypeDef.h"
#include "ServiceMemory.h"
#include "TextParser.h"
#include "XmlFactory.h"
#include <initializer_list>
#include <vector>

namespace enabler
{

Element* const Element::s_pEmptyElement = new Element{};

PUBLIC VIRTUAL Element::~Element()
{
    for (const Element* pElement : m_lstChildren)
    {
        delete pElement;
    }
}

PUBLIC VIRTUAL void Element::Write(IN_OUT IXmlStreamWriter& objWriter) const
{
    for (const Element* pElement : m_lstChildren)
    {
        pElement->Write(objWriter);
    }
}

PUBLIC VIRTUAL void Element::Append(IN Element* pElement)
{
    m_lstChildren.push_back(pElement);
}

PUBLIC VIRTUAL void PidfLoXml::Write(IN_OUT IXmlStreamWriter& objWriter) const
{
    objWriter.WriteStartDocument("UTF-8", "1.0");
    objWriter.WriteCharacters(TextParser::STR_LF);

    Element::Write(objWriter);

    objWriter.WriteEndDocument();
}

PUBLIC ByteArray PidfLoXml::Write() const
{
    ByteArray objContent;

    XmlFactory* pXmlFactory = XmlFactory::GetInstance();
    IXmlStreamWriter* pWriter = pXmlFactory->CreateStreamWriter();

    Write(*pWriter);

    IMS_CHAR* pszXml = pWriter->Flush();
    if (pszXml != IMS_NULL)
    {
        objContent.Attach(reinterpret_cast<IMS_BYTE*>(pszXml), pWriter->GetContentLength());
        objContent.Detach();
        IMS_MEM_Free(pszXml);
    }

    pWriter->Close();
    pXmlFactory->DestroyStreamWriter(pWriter);

    return objContent;
}

PUBLIC VIRTUAL void Presence::Write(IN_OUT IXmlStreamWriter& objWriter) const
{
    objWriter.WriteStartElement("presence");
    objWriter.WriteDefaultNamespace("urn:ietf:params:xml:ns:pidf");

    if (m_nNamespaces & Namespace::DM)
    {
        objWriter.WriteNamespace("dm", "urn:ietf:params:xml:ns:pidf:data-model");
    }
    if (m_nNamespaces & Namespace::GP)
    {
        objWriter.WriteNamespace("gp", "urn:ietf:params:xml:ns:pidf:geopriv10");
    }
    if (m_nNamespaces & Namespace::GML)
    {
        objWriter.WriteNamespace("gml", "http://www.opengis.net/gml");
    }
    if (m_nNamespaces & Namespace::GS)
    {
        objWriter.WriteNamespace("gs", "http://www.opengis.net/pidflo/1.0");
    }
    if (m_nNamespaces & Namespace::CL)
    {
        objWriter.WriteNamespace("cl", "urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr");
    }
    if (m_nNamespaces & Namespace::CON)
    {
        objWriter.WriteNamespace("con", "urn:ietf:params:xml:ns:geopriv:conf");
    }
    if (m_nNamespaces & Namespace::GBP)
    {
        objWriter.WriteNamespace("gbp", "urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy");
    }

    objWriter.WriteAttribute("entity", m_strEntityUri);
    objWriter.WriteCharacters(TextParser::STR_LF);

    Element::Write(objWriter);

    objWriter.WriteEndElement();
    objWriter.WriteCharacters(TextParser::STR_LF);
}

PUBLIC VIRTUAL void Person::Write(IN_OUT IXmlStreamWriter& objWriter) const
{
    objWriter.WriteStartElement("dm:person");
    objWriter.WriteAttribute("id", m_strPersonId);
    objWriter.WriteCharacters(TextParser::STR_LF);

    Element::Write(objWriter);

    objWriter.WriteEndElement();
    objWriter.WriteCharacters(TextParser::STR_LF);
}

PUBLIC VIRTUAL void Device::Write(IN_OUT IXmlStreamWriter& objWriter) const
{
    objWriter.WriteStartElement("dm:device");
    objWriter.WriteAttribute("id", m_strDeviceId);
    objWriter.WriteCharacters(TextParser::STR_LF);

    Element::Write(objWriter);

    objWriter.WriteEndElement();
    objWriter.WriteCharacters(TextParser::STR_LF);
}

PUBLIC VIRTUAL void Tuple::Write(IN_OUT IXmlStreamWriter& objWriter) const
{
    objWriter.WriteStartElement("tuple");
    objWriter.WriteAttribute("id", m_strTupleId);
    objWriter.WriteCharacters(TextParser::STR_LF);

    Element::Write(objWriter);

    objWriter.WriteEndElement();
    objWriter.WriteCharacters(TextParser::STR_LF);
}

void Status::Write(IN_OUT IXmlStreamWriter& objWriter) const
{
    objWriter.WriteStartElement("status");
    objWriter.WriteCharacters(TextParser::STR_LF);

    Element::Write(objWriter);

    objWriter.WriteEndElement();
    objWriter.WriteCharacters(TextParser::STR_LF);
}

void Geopriv::Write(IN_OUT IXmlStreamWriter& objWriter) const
{
    objWriter.WriteStartElement("gp:geopriv");
    objWriter.WriteCharacters(TextParser::STR_LF);

    Element::Write(objWriter);

    objWriter.WriteEndElement();
    objWriter.WriteCharacters(TextParser::STR_LF);
}

void LocationInfo::Write(IN_OUT IXmlStreamWriter& objWriter) const
{
    objWriter.WriteStartElement("gp:location-info");
    objWriter.WriteCharacters(TextParser::STR_LF);

    Element::Write(objWriter);

    objWriter.WriteEndElement();
    objWriter.WriteCharacters(TextParser::STR_LF);
}

void UsageRules::Write(IN_OUT IXmlStreamWriter& objWriter) const
{
    objWriter.WriteStartElement("gp:usage-rules");
    objWriter.WriteCharacters(TextParser::STR_LF);

    Element::Write(objWriter);

    objWriter.WriteEndElement();
    objWriter.WriteCharacters(TextParser::STR_LF);
}

void RetransmissionAllowed::Write(IN_OUT IXmlStreamWriter& objWriter) const
{
    if (m_strRetransmissionAllowed.GetLength() > 0)
    {
        objWriter.WriteStartElement("gbp:retransmission-allowed");
        objWriter.WriteCharacters(m_strRetransmissionAllowed);
        objWriter.WriteEndElement();
        objWriter.WriteCharacters(TextParser::STR_LF);
    }
}

void Method::Write(IN_OUT IXmlStreamWriter& objWriter) const
{
    if (m_strMethod.GetLength() > 0)
    {
        objWriter.WriteStartElement("gp:method");
        objWriter.WriteCharacters(m_strMethod);
        objWriter.WriteEndElement();
        objWriter.WriteCharacters(TextParser::STR_LF);
    }
}

void Timestamp::Write(IN_OUT IXmlStreamWriter& objWriter) const
{
    if (m_strTimestamp.GetLength() > 0)
    {
        objWriter.WriteStartElement("dm:timestamp");
        objWriter.WriteCharacters(m_strTimestamp);
        objWriter.WriteEndElement();
        objWriter.WriteCharacters(TextParser::STR_LF);
    }
}

void DeviceId::Write(IN_OUT IXmlStreamWriter& objWriter) const
{
    if (m_strDeviceId.GetLength() > 0)
    {
        objWriter.WriteStartElement("dm:deviceID");
        objWriter.WriteCharacters(m_strDeviceId);
        objWriter.WriteEndElement();
        objWriter.WriteCharacters(TextParser::STR_LF);
    }
}

void Circle::Write(IN_OUT IXmlStreamWriter& objWriter) const
{
    if (m_strLatitude.GetLength() <= 0 || m_strLongitude.GetLength() <= 0)
    {
        return;
    }

    objWriter.WriteStartElement("gs:Circle");
    objWriter.WriteAttribute("srsName", "urn:ogc:def:crs:EPSG::4326");
    objWriter.WriteCharacters(TextParser::STR_LF);

    AString strPosition;
    strPosition.Sprintf("%s %s", m_strLatitude.GetStr(), m_strLongitude.GetStr());
    objWriter.WriteStartElement("gml:pos");
    objWriter.WriteCharacters(strPosition);
    objWriter.WriteEndElement();
    objWriter.WriteCharacters(TextParser::STR_LF);

    if (m_strRadius.GetLength() > 0)
    {
        objWriter.WriteStartElement("gs:radius");
        objWriter.WriteAttribute("uom", "urn:ogc:def:uom:EPSG::9001");
        objWriter.WriteCharacters(m_strRadius);
        objWriter.WriteEndElement();
        objWriter.WriteCharacters(TextParser::STR_LF);
    }

    objWriter.WriteEndElement();
    objWriter.WriteCharacters(TextParser::STR_LF);
}

void Ellipsoid::Write(IN_OUT IXmlStreamWriter& objWriter) const
{
    if (m_strLatitude.GetLength() <= 0 || m_strLongitude.GetLength() <= 0 ||
            m_strAltitude.GetLength() <= 0)
    {
        return;
    }

    objWriter.WriteStartElement("gs:Ellipsoid");
    objWriter.WriteAttribute("srsName", "urn:ogc:def:crs:EPSG::4979");
    objWriter.WriteCharacters(TextParser::STR_LF);

    AString strPosition;
    strPosition.Sprintf(
            "%s %s %s", m_strLatitude.GetStr(), m_strLongitude.GetStr(), m_strAltitude.GetStr());
    objWriter.WriteStartElement("gml:pos");
    objWriter.WriteCharacters(strPosition);
    objWriter.WriteEndElement();
    objWriter.WriteCharacters(TextParser::STR_LF);

    if (m_strRadius.GetLength() > 0 && m_strVerticalAccuracy.GetLength() > 0)
    {
        objWriter.WriteStartElement("gs:semiMajorAxis");
        objWriter.WriteAttribute("uom", "urn:ogc:def:uom:EPSG::9001");
        objWriter.WriteCharacters(m_strRadius);
        objWriter.WriteEndElement();

        objWriter.WriteStartElement("gs:semiMinorAxis");
        objWriter.WriteAttribute("uom", "urn:ogc:def:uom:EPSG::9001");
        objWriter.WriteCharacters(m_strRadius);
        objWriter.WriteEndElement();

        objWriter.WriteStartElement("gs:verticalAxis");
        objWriter.WriteAttribute("uom", "urn:ogc:def:uom:EPSG::9001");
        objWriter.WriteCharacters(m_strVerticalAccuracy);
        objWriter.WriteEndElement();

        objWriter.WriteStartElement("gs:orientation");
        objWriter.WriteAttribute("uom", "urn:ogc:def:uom:EPSG::9102");
        objWriter.WriteCharacters("0");
        objWriter.WriteEndElement();

        objWriter.WriteCharacters(TextParser::STR_LF);
    }

    objWriter.WriteEndElement();
    objWriter.WriteCharacters(TextParser::STR_LF);
}

void Confidence::Write(IN_OUT IXmlStreamWriter& objWriter) const
{
    if (m_strConfidence.GetLength() > 0)
    {
        objWriter.WriteStartElement("con:confidence");
        objWriter.WriteAttribute("pdf", "normal");
        objWriter.WriteCharacters(m_strConfidence);
        objWriter.WriteEndElement();
        objWriter.WriteCharacters(TextParser::STR_LF);
    }
}

void CivicAddress::Write(IN_OUT IXmlStreamWriter& objWriter) const
{
    if (m_strCountry.GetLength() <= 0 && m_strState.GetLength() <= 0 &&
            m_strCity.GetLength() <= 0 && m_strPostal.GetLength() <= 0)
    {
        return;
    }

    objWriter.WriteStartElement("cl:civicAddress");
    objWriter.WriteCharacters(TextParser::STR_LF);

    if (m_strCountry.GetLength() > 0)
    {
        objWriter.WriteStartElement("cl:country");
        objWriter.WriteCharacters(m_strCountry);
        objWriter.WriteEndElement();
        objWriter.WriteCharacters(TextParser::STR_LF);
    }
    if (m_strState.GetLength() > 0)
    {
        objWriter.WriteStartElement("cl:A1");
        objWriter.WriteCharacters(m_strState);
        objWriter.WriteEndElement();
        objWriter.WriteCharacters(TextParser::STR_LF);
    }
    if (m_strCity.GetLength() > 0)
    {
        objWriter.WriteStartElement("cl:A2");
        objWriter.WriteCharacters(m_strCity);
        objWriter.WriteEndElement();
        objWriter.WriteCharacters(TextParser::STR_LF);
    }
    if (m_strPostal.GetLength() > 0)
    {
        objWriter.WriteStartElement("cl:PC");
        objWriter.WriteCharacters(m_strPostal);
        objWriter.WriteEndElement();
        objWriter.WriteCharacters(TextParser::STR_LF);
    }

    objWriter.WriteEndElement();
    objWriter.WriteCharacters(TextParser::STR_LF);
}

}  // namespace enabler
