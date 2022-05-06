/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090531  toastops@                 Created
    </table>

    Description

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
    CoreServiceConfigPrivate(IN const AString& strServiceId_);
    ~CoreServiceConfigPrivate();

public:
    static IMS_BOOL AddCoreServiceProperty(
            IN const AStringArray& objProperty, IN_OUT CoreServiceConfigPrivate* pConfigP);
    static IMS_BOOL AddQosProperty(
            IN const AStringArray& objProperty, IN_OUT CoreServiceConfigPrivate* pConfigP);
    static IMS_BOOL AddRegistrationHeaderProperty(
            IN const AStringArray& objProperty, IN_OUT CoreServiceConfigPrivate* pConfigP);
    static IMS_BOOL AddMediaProfileProperty(
            IN const AStringArray& objProperty, IN_OUT CoreServiceConfigPrivate* pConfigP);
    static IMS_BOOL AddConnectionModelProperty(
            IN const AStringArray& objProperty, IN_OUT CoreServiceConfigPrivate* pConfigP);

private:
    friend class CoreServiceConfig;

    AString strServiceId;

    // IARI of CoreService property
    ServiceIdentifier objIARI;
    // ICSIs of CoreService property
    IMSList<ServiceIdentifier> objICSIs;
    // Features of CoreService property
    IMSList<ServiceIdentifier> objFeatureTags;
    IMSList<FeatureSet*> objFeatureSets;

    // SEND flowspec of Qos property
    IMSList<QosProperty> objFlowSpecSends;
    // RECEIVE flowspec of Qos property
    IMSList<QosProperty> objFlowSpecReceives;

    // Registration headers property
    AStringArray objRegHeaders;

    // Media profile property
    AString strMediaProfile;

    // Connection model selection
    //    If present, MSRP connection model is that of the standard MSRP procedure
    //    as defined in RFC 4975.
    //    If not present, the model follows that of [draft-ietf-simple-msrp-acm-00].
    IMS_BOOL bConnectionModelSupported;
};

PUBLIC
CoreServiceConfigPrivate::CoreServiceConfigPrivate(IN const AString& strServiceId_) :
        strServiceId(strServiceId_),
        strMediaProfile(AString::ConstNull()),
        bConnectionModelSupported(IMS_FALSE)
{
}

PUBLIC
CoreServiceConfigPrivate::~CoreServiceConfigPrivate()
{
    for (IMS_UINT32 i = 0; i < objFeatureSets.GetSize(); ++i)
    {
        FeatureSet* pFeatureSet = objFeatureSets.GetAt(i);

        if (pFeatureSet != IMS_NULL)
        {
            delete pFeatureSet;
        }
    }
}

PUBLIC GLOBAL IMS_BOOL CoreServiceConfigPrivate::AddCoreServiceProperty(
        IN const AStringArray& objProperty, IN_OUT CoreServiceConfigPrivate* pConfigP)
{
    // { "CoreService", "ServiceId", "zero or one IARI", "zero or more ICSIs", "Feature Tags" }

    // NOTE: Here, we treat ICSI and IARI as case-insensitive which is not strictly true.
    // In reality, they can be partially case-sensitive and partially case-insensitive.
    // To correctly handle the case would require a parsing of the ICSI and IARI
    // which would add considerable complexity for little gain.

    // IARI : zero or one
    const AString& strIARI = objProperty.GetElementAt(2);

    if (strIARI.GetLength() != 0)
    {
        if (!ServiceIdentifier::CheckFeatureFlags(strIARI, IMS_FALSE))
        {
            return IMS_FALSE;
        }

        pConfigP->objIARI = ServiceIdentifier::Create(strIARI);
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
        pConfigP->objICSIs.Append(ServiceIdentifier::Create(objTokens.GetElementAt(i)));
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
        pConfigP->objFeatureTags.Append(ServiceIdentifier::Create(objTokens.GetElementAt(i)));
    }

    // Create a feature sets from feature tags
    for (IMS_UINT32 i = 0; i < pConfigP->objFeatureTags.GetSize(); ++i)
    {
        FeatureSet* pFeatureSet =
                FeatureSet::FromServiceIdentifier(pConfigP->objFeatureTags.GetAt(i));

        if (pFeatureSet != IMS_NULL)
        {
            pConfigP->objFeatureSets.Append(pFeatureSet);
        }
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL CoreServiceConfigPrivate::AddQosProperty(
        IN const AStringArray& objProperty, IN_OUT CoreServiceConfigPrivate* pConfigP)
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

    pConfigP->objFlowSpecSends.Append(objFlowSpecSend);
    pConfigP->objFlowSpecReceives.Append(objFlowSpecSend);

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL CoreServiceConfigPrivate::AddRegistrationHeaderProperty(
        IN const AStringArray& objProperty, IN_OUT CoreServiceConfigPrivate* pConfigP)
{
    // { "Reg", "ServiceId", "Header1", "Header2", ... }

    if (pConfigP->objRegHeaders.GetCount() > 0)
    {
        IMS_TRACE_E(0,
                "Property contains a Reg entry for a core service that already has "
                "a Reg entry defined: %s, %s",
                ImsProperty::ToString(pConfigP->objRegHeaders).GetStr(),
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

        pConfigP->objRegHeaders.AddElement(strValue);
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL CoreServiceConfigPrivate::AddMediaProfileProperty(
        IN const AStringArray& objProperty, IN_OUT CoreServiceConfigPrivate* pConfigP)
{
    // { "Mprof", "ServiceId", "Media profile short name" }

    if (objProperty.GetCount() != 3)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    pConfigP->strMediaProfile = objProperty.GetElementAt(2);

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL CoreServiceConfigPrivate::AddConnectionModelProperty(
        IN const AStringArray& objProperty, IN_OUT CoreServiceConfigPrivate* pConfigP)
{
    // { "Connection", "ServiceId" }

    if (objProperty.GetCount() != 2)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    pConfigP->bConnectionModelSupported = IMS_TRUE;

    return IMS_TRUE;
}

PUBLIC
CoreServiceConfig::CoreServiceConfig(IN const AString& strServiceId_) :
        pConfigP(new CoreServiceConfigPrivate(strServiceId_))
{
}

PUBLIC VIRTUAL CoreServiceConfig::~CoreServiceConfig()
{
    if (pConfigP != IMS_NULL)
    {
        delete pConfigP;
    }
}

PUBLIC VIRTUAL const AString& CoreServiceConfig::GetServiceId() const
{
    return pConfigP->strServiceId;
}

PUBLIC VIRTUAL IMS_BOOL CoreServiceConfig::IsIARISupported() const
{
    if (pConfigP->objIARI.GetName().GetLength() == 0)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL const ServiceIdentifier& CoreServiceConfig::GetIARI() const
{
    return pConfigP->objIARI;
}

PUBLIC VIRTUAL const IMSList<ServiceIdentifier>& CoreServiceConfig::GetICSIs() const
{
    return pConfigP->objICSIs;
}

PUBLIC VIRTUAL const IMSList<ServiceIdentifier>& CoreServiceConfig::GetFeatureTags() const
{
    return pConfigP->objFeatureTags;
}

PUBLIC VIRTUAL const AString& CoreServiceConfig::GetMediaProfile() const
{
    return pConfigP->strMediaProfile;
}

PUBLIC
IMS_BOOL CoreServiceConfig::Create(IN const AStringArray& objCoreServiceProperty)
{
    if (!CoreServiceConfigPrivate::AddCoreServiceProperty(objCoreServiceProperty, pConfigP))
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
            return CoreServiceConfigPrivate::AddQosProperty(objProperty, pConfigP);

        case ImsProperty::PKEY_REG:
            return CoreServiceConfigPrivate::AddRegistrationHeaderProperty(objProperty, pConfigP);

        case ImsProperty::PKEY_MPROF:
            return CoreServiceConfigPrivate::AddMediaProfileProperty(objProperty, pConfigP);

        case ImsProperty::PKEY_CONNECTION:
            return CoreServiceConfigPrivate::AddConnectionModelProperty(objProperty, pConfigP);

        default:
            return IMS_FALSE;
    }
}

PUBLIC
const IMSList<FeatureSet*>& CoreServiceConfig::GetFeatureSets() const
{
    return pConfigP->objFeatureSets;
}

PUBLIC
AStringArray CoreServiceConfig::GetQosContentTypes() const
{
    AStringArray objContentTypes;

    for (IMS_UINT32 i = 0; i < pConfigP->objFlowSpecSends.GetSize(); ++i)
    {
        const QosProperty& objQos = pConfigP->objFlowSpecSends.GetAt(i);

        objContentTypes.AddElement(objQos.GetContentType());
    }

    return objContentTypes;
}

PUBLIC
const QosProperty* CoreServiceConfig::GetFlowSpecSend(IN const AString& strContentType) const
{
    for (IMS_UINT32 i = 0; i < pConfigP->objFlowSpecSends.GetSize(); ++i)
    {
        const QosProperty& objQos = pConfigP->objFlowSpecSends.GetAt(i);

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
    for (IMS_UINT32 i = 0; i < pConfigP->objFlowSpecReceives.GetSize(); ++i)
    {
        const QosProperty& objQos = pConfigP->objFlowSpecReceives.GetAt(i);

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
    return pConfigP->objRegHeaders;
}

PUBLIC
IMS_BOOL CoreServiceConfig::IsConnectionModelSupported() const
{
    return pConfigP->bConnectionModelSupported;
}

PUBLIC
void CoreServiceConfig::ToRegistry(IN_OUT ImsRegistry*& pRegistry) const
{
    AStringArray objProperty;
    AStringArray objValues;

    // CoreService property
    objProperty.AddElement(ImsProperty::PKEY_STRING[ImsProperty::PKEY_CORE_SERVICE]);
    objProperty.AddElement(GetServiceId());

    if (IsIARISupported())
    {
        const ServiceIdentifier& objIARI = GetIARI();

        objProperty.AddElement(objIARI.ToString());
    }
    else
    {
        objProperty.AddElement(AString::ConstEmpty());
    }

    IMS_UINT32 i;

    for (i = 0; i < pConfigP->objICSIs.GetSize(); ++i)
    {
        const ServiceIdentifier& objSI = pConfigP->objICSIs.GetAt(i);

        objValues.AddElement(objSI.ToString());
    }

    if (objValues.GetCount() > 0)
    {
        objProperty.AddElement(ImsProperty::Encode(objValues));
    }
    else
    {
        objProperty.AddElement(AString::ConstEmpty());
    }

    for (i = 0; i < pConfigP->objFeatureTags.GetSize(); ++i)
    {
        const ServiceIdentifier& objSI = pConfigP->objFeatureTags.GetAt(i);

        objValues.AddElement(objSI.ToString());
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

    for (j = 0; j < pConfigP->objRegHeaders.GetCount(); ++j)
    {
        objProperty.AddElement(pConfigP->objRegHeaders.GetElementAt(j));
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
