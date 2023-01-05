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
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "Feature.h"
#include "QosProperty.h"
#include "private/CoreServiceConfig.h"

__IMS_TRACE_TAG_CONF__;

class CoreServiceConfigPrivate
{
public:
    explicit CoreServiceConfigPrivate(IN const AString& strServiceId);
    ~CoreServiceConfigPrivate();

    CoreServiceConfigPrivate(IN const CoreServiceConfigPrivate&) = delete;
    CoreServiceConfigPrivate& operator=(IN const CoreServiceConfigPrivate&) = delete;

public:
    static IMS_BOOL AddCoreServiceProperty(
            IN const AStringArray& objProperty, IN_OUT CoreServiceConfigPrivate* pConfigPrivate);
    static IMS_BOOL AddQosProperty(
            IN const AStringArray& objProperty, IN_OUT CoreServiceConfigPrivate* pConfigPrivate);
    static IMS_BOOL AddRegistrationHeaderProperty(
            IN const AStringArray& objProperty, IN_OUT CoreServiceConfigPrivate* pConfigPrivate);
    static IMS_BOOL AddMediaProfileProperty(
            IN const AStringArray& objProperty, IN_OUT CoreServiceConfigPrivate* pConfigPrivate);
    static IMS_BOOL AddConnectionModelProperty(
            IN const AStringArray& objProperty, IN_OUT CoreServiceConfigPrivate* pConfigPrivate);

private:
    friend class CoreServiceConfig;

    AString m_strServiceId;

    // IARI of CoreService property
    ServiceIdentifier m_objIari;
    // ICSIs of CoreService property
    IMSList<ServiceIdentifier> m_objIcsis;
    // Features of CoreService property
    IMSList<ServiceIdentifier> m_objFeatureTags;
    IMSList<FeatureSet*> m_objFeatureSets;

    // SEND flowspec of Qos property
    IMSList<QosProperty> m_objFlowSpecSends;
    // RECEIVE flowspec of Qos property
    IMSList<QosProperty> m_objFlowSpecReceives;

    // Registration headers property
    AStringArray m_objRegHeaders;

    // Media profile property
    AString m_strMediaProfile;

    // Connection model selection
    //    If present, MSRP connection model is that of the standard MSRP procedure
    //    as defined in RFC 4975.
    //    If not present, the model follows that of [draft-ietf-simple-msrp-acm-00].
    IMS_BOOL m_bConnectionModelSupported;
};

PUBLIC
CoreServiceConfigPrivate::CoreServiceConfigPrivate(IN const AString& strServiceId) :
        m_strServiceId(strServiceId),
        m_strMediaProfile(AString::ConstNull()),
        m_bConnectionModelSupported(IMS_FALSE)
{
}

PUBLIC
CoreServiceConfigPrivate::~CoreServiceConfigPrivate()
{
    for (IMS_UINT32 i = 0; i < m_objFeatureSets.GetSize(); ++i)
    {
        FeatureSet* pFeatureSet = m_objFeatureSets.GetAt(i);

        if (pFeatureSet != IMS_NULL)
        {
            delete pFeatureSet;
        }
    }

    m_objFeatureSets.Clear();
}

PUBLIC GLOBAL IMS_BOOL CoreServiceConfigPrivate::AddCoreServiceProperty(
        IN const AStringArray& objProperty, IN_OUT CoreServiceConfigPrivate* pConfigPrivate)
{
    // { "CoreService", "ServiceId", "zero or one IARI", "zero or more ICSIs", "Feature Tags" }

    // NOTE: Here, we treat ICSI and IARI as case-insensitive which is not strictly true.
    // In reality, they can be partially case-sensitive and partially case-insensitive.
    // To correctly handle the case would require a parsing of the ICSI and IARI
    // which would add considerable complexity for little gain.

    // IARI : zero or one
    const AString& strIari = objProperty.GetElementAt(2);

    if (strIari.GetLength() != 0)
    {
        if (!ServiceIdentifier::CheckFeatureFlags(strIari, IMS_FALSE))
        {
            return IMS_FALSE;
        }

        pConfigPrivate->m_objIari = ServiceIdentifier::Create(strIari);
    }

    // ICSI : zero or more
    IMS_SINT32 i;
    AStringArray objTokens = ImsProperty::Decode(objProperty.GetElementAt(3));

    for (i = 0; i < objTokens.GetCount(); ++i)
    {
        if (!ServiceIdentifier::CheckFeatureFlags(objTokens.GetElementAt(i), IMS_TRUE))
        {
            return IMS_FALSE;
        }

        // Add ICSI to CoreServiceConfig
        pConfigPrivate->m_objIcsis.Append(ServiceIdentifier::Create(objTokens.GetElementAt(i)));
    }

    // Feature tags : zero or more
    objTokens = ImsProperty::Decode(objProperty.GetElementAt(4));

    for (i = 0; i < objTokens.GetCount(); ++i)
    {
        if (!ServiceIdentifier::CheckFeatureFlags(objTokens.GetElementAt(i), IMS_TRUE))
        {
            return IMS_FALSE;
        }

        // Add feature-tag to CoreServiceConfig
        pConfigPrivate->m_objFeatureTags.Append(
                ServiceIdentifier::Create(objTokens.GetElementAt(i)));
    }

    // Create a feature sets from feature tags
    for (IMS_UINT32 j = 0; j < pConfigPrivate->m_objFeatureTags.GetSize(); ++j)
    {
        FeatureSet* pFeatureSet =
                FeatureSet::FromServiceIdentifier(pConfigPrivate->m_objFeatureTags.GetAt(j));

        if (pFeatureSet != IMS_NULL)
        {
            pConfigPrivate->m_objFeatureSets.Append(pFeatureSet);
        }
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL CoreServiceConfigPrivate::AddQosProperty(
        IN const AStringArray& objProperty, IN_OUT CoreServiceConfigPrivate* pConfigPrivate)
{
    // { "Qos", "ServiceId", "MIME Content Type", "Flowspec_Send", "Flowspec_Receive" }

    if (objProperty.GetCount() != 5)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    QosProperty objFlowSpecSend(objProperty.GetElementAt(2));
    QosProperty objFlowSpecReceive(objProperty.GetElementAt(2));

    objFlowSpecSend.SetQos(objProperty.GetElementAt(3));

    if (objProperty.GetElementAt(4).IsEmpty())
    {
        objFlowSpecReceive.SetQos(objProperty.GetElementAt(3));
    }
    else
    {
        objFlowSpecReceive.SetQos(objProperty.GetElementAt(4));
    }

    pConfigPrivate->m_objFlowSpecSends.Append(objFlowSpecSend);
    pConfigPrivate->m_objFlowSpecReceives.Append(objFlowSpecSend);

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL CoreServiceConfigPrivate::AddRegistrationHeaderProperty(
        IN const AStringArray& objProperty, IN_OUT CoreServiceConfigPrivate* pConfigPrivate)
{
    // { "Reg", "ServiceId", "Header1", "Header2", ... }

    if (pConfigPrivate->m_objRegHeaders.GetCount() > 0)
    {
        IMS_TRACE_E(0,
                "Property contains a Reg entry for a core service that already has "
                "a Reg entry defined: %s, %s",
                ImsProperty::ToString(pConfigPrivate->m_objRegHeaders).GetStr(),
                ImsProperty::ToString(objProperty).GetStr(), 0);
        return IMS_FALSE;
    }

    if (objProperty.GetCount() < 3)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    for (IMS_SINT32 i = 2; i < objProperty.GetCount(); ++i)
    {
        const AString& strValue = objProperty.GetElementAt(i);

        if (!strValue.Contains(':'))
        {
            IMS_TRACE_E(0, "Property is malformed, SIP headers must contains a COLON : %s",
                    ImsProperty::ToString(objProperty).GetStr(), 0, 0);
            return IMS_FALSE;
        }

        pConfigPrivate->m_objRegHeaders.AddElement(strValue);
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL CoreServiceConfigPrivate::AddMediaProfileProperty(
        IN const AStringArray& objProperty, IN_OUT CoreServiceConfigPrivate* pConfigPrivate)
{
    // { "Mprof", "ServiceId", "Media profile short name" }

    if (objProperty.GetCount() != 3)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    pConfigPrivate->m_strMediaProfile = objProperty.GetElementAt(2);

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL CoreServiceConfigPrivate::AddConnectionModelProperty(
        IN const AStringArray& objProperty, IN_OUT CoreServiceConfigPrivate* pConfigPrivate)
{
    // { "Connection", "ServiceId" }

    if (objProperty.GetCount() != 2)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    pConfigPrivate->m_bConnectionModelSupported = IMS_TRUE;

    return IMS_TRUE;
}

PUBLIC
CoreServiceConfig::CoreServiceConfig(IN const AString& strServiceId) :
        m_pConfigPrivate(new CoreServiceConfigPrivate(strServiceId))
{
}

PUBLIC VIRTUAL CoreServiceConfig::~CoreServiceConfig()
{
    if (m_pConfigPrivate != IMS_NULL)
    {
        delete m_pConfigPrivate;
    }
}

PUBLIC VIRTUAL const AString& CoreServiceConfig::GetServiceId() const
{
    return m_pConfigPrivate->m_strServiceId;
}

PUBLIC VIRTUAL IMS_BOOL CoreServiceConfig::IsIariSupported() const
{
    if (m_pConfigPrivate->m_objIari.GetName().GetLength() == 0)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL const ServiceIdentifier& CoreServiceConfig::GetIari() const
{
    return m_pConfigPrivate->m_objIari;
}

PUBLIC VIRTUAL const IMSList<ServiceIdentifier>& CoreServiceConfig::GetIcsis() const
{
    return m_pConfigPrivate->m_objIcsis;
}

PUBLIC VIRTUAL const IMSList<ServiceIdentifier>& CoreServiceConfig::GetFeatureTags() const
{
    return m_pConfigPrivate->m_objFeatureTags;
}

PUBLIC VIRTUAL const AString& CoreServiceConfig::GetMediaProfile() const
{
    return m_pConfigPrivate->m_strMediaProfile;
}

PUBLIC
IMS_BOOL CoreServiceConfig::Create(IN const AStringArray& objCoreServiceProperty)
{
    if (!CoreServiceConfigPrivate::AddCoreServiceProperty(objCoreServiceProperty, m_pConfigPrivate))
    {
        IMS_TRACE_E(0, "Adding CoreService property failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL CoreServiceConfig::AddProperty(IN const AStringArray& objProperty)
{
    IMS_SINT32 nKey = ImsProperty::StringToKey(objProperty.GetElementAt(0));

    switch (nKey)
    {
        case ImsProperty::PKEY_QOS:
            return CoreServiceConfigPrivate::AddQosProperty(objProperty, m_pConfigPrivate);
        case ImsProperty::PKEY_REG:
            return CoreServiceConfigPrivate::AddRegistrationHeaderProperty(
                    objProperty, m_pConfigPrivate);
        case ImsProperty::PKEY_MPROF:
            return CoreServiceConfigPrivate::AddMediaProfileProperty(objProperty, m_pConfigPrivate);
        case ImsProperty::PKEY_CONNECTION:
            return CoreServiceConfigPrivate::AddConnectionModelProperty(
                    objProperty, m_pConfigPrivate);
        default:
            return IMS_FALSE;
    }
}

PUBLIC
const IMSList<FeatureSet*>& CoreServiceConfig::GetFeatureSets() const
{
    return m_pConfigPrivate->m_objFeatureSets;
}

PUBLIC
AStringArray CoreServiceConfig::GetQosContentTypes() const
{
    AStringArray objContentTypes;

    for (IMS_UINT32 i = 0; i < m_pConfigPrivate->m_objFlowSpecSends.GetSize(); ++i)
    {
        const QosProperty& objQos = m_pConfigPrivate->m_objFlowSpecSends.GetAt(i);

        objContentTypes.AddElement(objQos.GetContentType());
    }

    return objContentTypes;
}

PUBLIC
const QosProperty* CoreServiceConfig::GetFlowSpecSend(IN const AString& strContentType) const
{
    for (IMS_UINT32 i = 0; i < m_pConfigPrivate->m_objFlowSpecSends.GetSize(); ++i)
    {
        const QosProperty& objQos = m_pConfigPrivate->m_objFlowSpecSends.GetAt(i);

        if (objQos.Equals(strContentType))
        {
            return &objQos;
        }
    }

    return IMS_NULL;
}

PUBLIC
const QosProperty* CoreServiceConfig::GetFlowSpecReceive(IN const AString& strContentType) const
{
    for (IMS_UINT32 i = 0; i < m_pConfigPrivate->m_objFlowSpecReceives.GetSize(); ++i)
    {
        const QosProperty& objQos = m_pConfigPrivate->m_objFlowSpecReceives.GetAt(i);

        if (objQos.Equals(strContentType))
        {
            return &objQos;
        }
    }

    return IMS_NULL;
}

PUBLIC
const AStringArray& CoreServiceConfig::GetRegistrationHeaders() const
{
    return m_pConfigPrivate->m_objRegHeaders;
}

PUBLIC
IMS_BOOL CoreServiceConfig::IsConnectionModelSupported() const
{
    return m_pConfigPrivate->m_bConnectionModelSupported;
}

PUBLIC
void CoreServiceConfig::ToRegistry(IN_OUT ImsRegistry*& pRegistry) const
{
    AStringArray objProperty;
    AStringArray objValues;

    // CoreService property
    objProperty.AddElement(ImsProperty::PKEY_STRING[ImsProperty::PKEY_CORE_SERVICE]);
    objProperty.AddElement(GetServiceId());

    if (IsIariSupported())
    {
        const ServiceIdentifier& objIARI = GetIari();

        objProperty.AddElement(objIARI.ToString());
    }
    else
    {
        objProperty.AddElement(AString::ConstEmpty());
    }

    IMS_UINT32 i;

    for (i = 0; i < m_pConfigPrivate->m_objIcsis.GetSize(); ++i)
    {
        const ServiceIdentifier& objServiceId = m_pConfigPrivate->m_objIcsis.GetAt(i);

        objValues.AddElement(objServiceId.ToString());
    }

    if (objValues.GetCount() > 0)
    {
        objProperty.AddElement(ImsProperty::Encode(objValues));
    }
    else
    {
        objProperty.AddElement(AString::ConstEmpty());
    }

    for (i = 0; i < m_pConfigPrivate->m_objFeatureTags.GetSize(); ++i)
    {
        const ServiceIdentifier& objServiceId = m_pConfigPrivate->m_objFeatureTags.GetAt(i);

        objValues.AddElement(objServiceId.ToString());
    }

    if (objValues.GetCount() > 0)
    {
        objProperty.AddElement(ImsProperty::Encode(objValues));
    }
    else
    {
        objProperty.AddElement(AString::ConstEmpty());
    }

    pRegistry->Add(objProperty);

    // QosProfile property
    IMS_SINT32 j;

    objValues.RemoveAllElements();

    objValues = GetQosContentTypes();

    for (j = 0; j < objValues.GetCount(); ++j)
    {
        const AString& strContentType = objValues.GetElementAt(j);
        const QosProperty* pFlowspecSend = GetFlowSpecSend(strContentType);
        const QosProperty* pFlowspecReceive = GetFlowSpecReceive(strContentType);

        if ((pFlowspecSend != IMS_NULL) && (pFlowspecReceive != IMS_NULL))
        {
            objProperty.RemoveAllElements();

            objProperty.AddElement(ImsProperty::PKEY_STRING[ImsProperty::PKEY_QOS]);
            objProperty.AddElement(GetServiceId());
            objProperty.AddElement(strContentType);
            objProperty.AddElement(pFlowspecSend->GetQosString());
            objProperty.AddElement(pFlowspecReceive->GetQosString());

            pRegistry->Add(objProperty);
        }
    }

    // Registration header property
    objProperty.RemoveAllElements();

    objProperty.AddElement(ImsProperty::PKEY_STRING[ImsProperty::PKEY_REG]);
    objProperty.AddElement(GetServiceId());

    for (j = 0; j < m_pConfigPrivate->m_objRegHeaders.GetCount(); ++j)
    {
        objProperty.AddElement(m_pConfigPrivate->m_objRegHeaders.GetElementAt(j));
    }

    pRegistry->Add(objProperty);

    // Media profile property
    objProperty.RemoveAllElements();

    objProperty.AddElement(ImsProperty::PKEY_STRING[ImsProperty::PKEY_MPROF]);
    objProperty.AddElement(GetServiceId());
    objProperty.AddElement(GetMediaProfile());

    pRegistry->Add(objProperty);

    // Connection property
    if (IsConnectionModelSupported())
    {
        objProperty.RemoveAllElements();

        objProperty.AddElement(ImsProperty::PKEY_STRING[ImsProperty::PKEY_CONNECTION]);
        objProperty.AddElement(GetServiceId());

        pRegistry->Add(objProperty);
    }
}
