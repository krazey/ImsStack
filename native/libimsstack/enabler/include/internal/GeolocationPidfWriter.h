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

#ifndef GEOLOCATION_PIDF_WRITER_H_
#define GEOLOCATION_PIDF_WRITER_H_

#include "AString.h"
#include "ByteArray.h"
#include "ImsTypeDef.h"
#include <initializer_list>
#include <vector>

class IXmlStreamWriter;

namespace enabler
{

class Element
{
public:
    virtual ~Element();
    Element(IN const Element&) = delete;
    Element& operator=(IN const Element&) = delete;

    virtual void Write(IN_OUT IXmlStreamWriter& objWriter) const;
    virtual void Append(IN Element* pElement);

    static Element* const s_pEmptyElement;

protected:
    explicit inline Element(IN std::initializer_list<Element*> lstChildren) :
            m_lstChildren(lstChildren)
    {
    }

private:
    std::vector<Element*> m_lstChildren;
};

class PidfLoXml : public Element
{
public:
    explicit inline PidfLoXml(IN std::initializer_list<Element*> lstChildren) :
            Element(lstChildren)
    {
    }

    void Write(IN_OUT IXmlStreamWriter& objWriter) const override;

    /**
     * Convenience function that uses `Write(IN_OUT IXmlStreamWriter& objWriter)` internally.
     *
     * @return PIDF-LO XML. Empty if it fails.
     */
    ByteArray Write() const;
};

class Presence : public Element
{
public:
    enum Namespace
    {
        NONE = 0,
        DM = 1 << 1,
        GP = 1 << 2,
        GML = 1 << 3,
        GS = 1 << 4,
        CL = 1 << 5,
        CON = 1 << 6,
        GBP = 1 << 7,
        ALL = ~0,

        COUNTRY = DM | GP | CL,
    };

    inline Presence(IN IMS_SINT32 nNamespaces, IN const AString& strEntityUri,
            std::initializer_list<Element*> lstChildren) :
            Element(lstChildren),
            m_nNamespaces(nNamespaces),
            m_strEntityUri(strEntityUri)
    {
    }

    void Write(IN_OUT IXmlStreamWriter& objWriter) const override;

private:
    const IMS_SINT32 m_nNamespaces;
    const AString m_strEntityUri;
};

class Person : public Element
{
public:
    inline Person(IN const AString& strPersonId, IN std::initializer_list<Element*> lstChildren) :
            Element(lstChildren),
            m_strPersonId(strPersonId)
    {
    }

    void Write(IN_OUT IXmlStreamWriter& objWriter) const override;

private:
    const AString m_strPersonId;
};

class Device : public Element
{
public:
    inline Device(IN const AString& strDeviceId, IN std::initializer_list<Element*> lstChildren) :
            Element(lstChildren),
            m_strDeviceId(strDeviceId)
    {
    }

    void Write(IN_OUT IXmlStreamWriter& objWriter) const override;

private:
    const AString m_strDeviceId;
};

class Tuple : public Element
{
public:
    inline Tuple(IN const AString& strTupleId, IN std::initializer_list<Element*> lstChildren) :
            Element(lstChildren),
            m_strTupleId(strTupleId)
    {
    }

    void Write(IN_OUT IXmlStreamWriter& objWriter) const override;

private:
    const AString m_strTupleId;
};

class Status : public Element
{
public:
    explicit inline Status(IN std::initializer_list<Element*> lstChildren) :
            Element(lstChildren)
    {
    }

    void Write(IN_OUT IXmlStreamWriter& objWriter) const override;
};

class Geopriv : public Element
{
public:
    explicit inline Geopriv(IN std::initializer_list<Element*> lstChildren) :
            Element(lstChildren)
    {
    }

    void Write(IN_OUT IXmlStreamWriter& objWriter) const override;
};

class LocationInfo : public Element
{
public:
    explicit inline LocationInfo(IN std::initializer_list<Element*> lstChildren) :
            Element(lstChildren)
    {
    }

    void Write(IN_OUT IXmlStreamWriter& objWriter) const override;
};

class UsageRules : public Element
{
public:
    explicit inline UsageRules(IN std::initializer_list<Element*> lstChildren) :
            Element(lstChildren)
    {
    }

    void Write(IN_OUT IXmlStreamWriter& objWriter) const override;
};

class RetransmissionAllowed : public Element
{
public:
    explicit inline RetransmissionAllowed(IN const AString& strRetransmissionAllowed) :
            Element({}),
            m_strRetransmissionAllowed(strRetransmissionAllowed)
    {
    }

    void Write(IN_OUT IXmlStreamWriter& objWriter) const override;

private:
    const AString m_strRetransmissionAllowed;
};

class Method : public Element
{
public:
    explicit inline Method(IN const AString& strMethod) :
            Element({}),
            m_strMethod(strMethod)
    {
    }

    void Write(IN_OUT IXmlStreamWriter& objWriter) const override;

private:
    const AString m_strMethod;
};

class Timestamp : public Element
{
public:
    explicit inline Timestamp(IN const AString& strTimestamp) :
            Element({}),
            m_strTimestamp(strTimestamp)
    {
    }

    void Write(IN_OUT IXmlStreamWriter& objWriter) const override;

private:
    const AString m_strTimestamp;
};

class DeviceId : public Element
{
public:
    explicit inline DeviceId(IN const AString& strDeviceId) :
            Element({}),
            m_strDeviceId(strDeviceId)
    {
    }

    void Write(IN_OUT IXmlStreamWriter& objWriter) const override;

private:
    const AString m_strDeviceId;
};

class Circle : public Element
{
public:
    inline Circle(IN const AString& strLatitude, IN const AString& strLongitude,
            IN const AString& strRadius) :
            Element({}),
            m_strLatitude(strLatitude),
            m_strLongitude(strLongitude),
            m_strRadius(strRadius)
    {
    }

    void Write(IN_OUT IXmlStreamWriter& objWriter) const override;

private:
    const AString m_strLatitude;
    const AString m_strLongitude;
    const AString m_strRadius;
};

class Ellipsoid : public Element
{
public:
    inline Ellipsoid(IN const AString& strLatitude, IN const AString& strLongitude,
            IN const AString& strAltitude, IN const AString& strRadius,
            IN const AString& strVerticalAccuracy) :
            Element({}),
            m_strLatitude(strLatitude),
            m_strLongitude(strLongitude),
            m_strAltitude(strAltitude),
            m_strRadius(strRadius),
            m_strVerticalAccuracy(strVerticalAccuracy)
    {
    }

    void Write(IN_OUT IXmlStreamWriter& objWriter) const override;

private:
    const AString m_strLatitude;
    const AString m_strLongitude;
    const AString m_strAltitude;
    const AString m_strRadius;
    const AString m_strVerticalAccuracy;
};

class Confidence : public Element
{
public:
    explicit inline Confidence(IN const AString& strConfidence) :
            Element({}),
            m_strConfidence(strConfidence)
    {
    }

    void Write(IN_OUT IXmlStreamWriter& objWriter) const override;

private:
    const AString m_strConfidence;
};

class CivicAddress : public Element
{
public:
    inline CivicAddress(IN const AString& strCountry,
            IN const AString& strState = AString::ConstNull(),
            IN const AString& strCity = AString::ConstNull(),
            IN const AString& strPostal = AString::ConstNull()) :
            Element({}),
            m_strCountry(strCountry),
            m_strState(strState),
            m_strCity(strCity),
            m_strPostal(strPostal)
    {
    }

    void Write(IN_OUT IXmlStreamWriter& objWriter) const override;

private:
    const AString m_strCountry;
    const AString m_strState;
    const AString m_strCity;
    const AString m_strPostal;
};

}  // namespace enabler

#endif
