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
#include "RCObject.h"
#include "private/CapProperty.h"
#include "private/CoreServiceConfig.h"
#include "private/AppConfig.h"

__IMS_TRACE_TAG_CONF__;

class AppConfigPrivate : public RCObject
{
public:
    explicit AppConfigPrivate(IN const AString& strAppId_);
    ~AppConfigPrivate();

private:
    AppConfigPrivate(IN const AppConfigPrivate& objRHS);
    AppConfigPrivate& operator=(IN const AppConfigPrivate& objRHS);

public:
    IMS_BOOL CheckMandatoryProperty() const;
    CapProperty* GetCapProperty(IN IMS_SINT32 nSector, IN IMS_SINT32 nMessageType) const;
    CoreServiceConfig* GetCoreServiceConfig(IN const AString& strServiceId) const;

    IMS_BOOL AddCapabilitySDP(
            IN IMS_SINT32 nSector, IN IMS_SINT32 nMessageType, IN const AString& strSDPField);

    static IMS_BOOL AddStreamMediaProperty(
            IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigP);
    static IMS_BOOL AddFramedMediaProperty(
            IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigP);
    static IMS_BOOL AddBasicMediaProperty(
            IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigP);
    static IMS_BOOL AddEventProperty(
            IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigP);
    static IMS_BOOL AddWriteHeaderProperty(
            IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigP);
    static IMS_BOOL AddReadHeaderProperty(
            IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigP);
    static IMS_BOOL AddCapabilityProperty(
            IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigP);
    static IMS_BOOL AddCoreServiceProperty(
            IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigP);
    static IMS_BOOL AddCoreServiceRelatedProperty(
            IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigP);

private:
    friend class AppConfig;

    AString strAppId;

    // StreamMedia property
    IMS_BOOL bStreamMediaSupported;
    IMS_BOOL bStreamAudioSupported;
    IMS_BOOL bStreamVideoSupported;

    // FramedMedia property
    IMS_BOOL bFramedMediaSupported;
    AStringArray objFramedMediaMimeTypes;
    IMS_BOOL bFramedMediaTransferMaxSize;
    IMS_UINT32 nFramedMediaTransferMaxSize;

    // BasicMedia property
    IMS_BOOL bBasicMediaSupported;
    AStringArray objBasicMediaMimeTypes;

    // Supported event package property
    AStringArray objEventPackages;

    // CoreService property
    IMSList<CoreServiceConfig*> objCoreServiceConfigs;

    // Writable headers property
    AStringArray objWriteHeaders;

    // Readable headers property
    AStringArray objReadHeaders;

    // SDP fields property for Capabilities
    IMSList<CapProperty*> objCapabilities;
};

PUBLIC
AppConfigPrivate::AppConfigPrivate(IN const AString& strAppId_) :
        strAppId(strAppId_),
        bStreamMediaSupported(IMS_FALSE),
        bStreamAudioSupported(IMS_FALSE),
        bStreamVideoSupported(IMS_FALSE),
        bFramedMediaSupported(IMS_FALSE),
        bFramedMediaTransferMaxSize(IMS_FALSE),
        nFramedMediaTransferMaxSize(0),
        bBasicMediaSupported(IMS_FALSE)
{
}

PUBLIC VIRTUAL AppConfigPrivate::~AppConfigPrivate()
{
    if (!objCoreServiceConfigs.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objCoreServiceConfigs.GetSize(); ++i)
        {
            CoreServiceConfig* pServiceConfig = objCoreServiceConfigs.GetAt(i);

            if (pServiceConfig != IMS_NULL)
            {
                delete pServiceConfig;
            }
        }

        objCoreServiceConfigs.Clear();
    }

    if (!objCapabilities.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objCapabilities.GetSize(); ++i)
        {
            CapProperty* pCapProperty = objCapabilities.GetAt(i);

            if (pCapProperty != IMS_NULL)
            {
                delete pCapProperty;
            }
        }

        objCapabilities.Clear();
    }
}

PUBLIC
IMS_BOOL AppConfigPrivate::CheckMandatoryProperty() const
{
    // Check if the registry MUST contain at least one of the IMS properties:
    //  StreamMedia, FramedMedia, BasicMedia, Event, CoreService,

    if (!bStreamMediaSupported && !bFramedMediaSupported && !bBasicMediaSupported &&
            objEventPackages.IsEmpty() && objCoreServiceConfigs.IsEmpty())
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
CapProperty* AppConfigPrivate::GetCapProperty(
        IN IMS_SINT32 nSector, IN IMS_SINT32 nMessageType) const
{
    if (objCapabilities.IsEmpty())
    {
        return IMS_NULL;
    }

    AString strCapKey = CapProperty::CreateCapKey(nSector, nMessageType);

    for (IMS_UINT32 i = 0; i < objCapabilities.GetSize(); ++i)
    {
        CapProperty* pProperty = objCapabilities.GetAt(i);

        if (pProperty->Equals(strCapKey))
        {
            return pProperty;
        }
    }

    return IMS_NULL;
}

PUBLIC
CoreServiceConfig* AppConfigPrivate::GetCoreServiceConfig(IN const AString& strServiceId) const
{
    for (IMS_UINT32 i = 0; i < objCoreServiceConfigs.GetSize(); ++i)
    {
        CoreServiceConfig* pServiceConfig = objCoreServiceConfigs.GetAt(i);

        if (pServiceConfig->GetServiceId().EqualsIgnoreCase(strServiceId))
        {
            return pServiceConfig;
        }
    }

    return IMS_NULL;
}

PUBLIC
IMS_BOOL AppConfigPrivate::AddCapabilitySDP(
        IN IMS_SINT32 nSector, IN IMS_SINT32 nMessageType, IN const AString& strSDPField)
{
    CapProperty* pProperty = GetCapProperty(nSector, nMessageType);

    if (pProperty != IMS_NULL)
    {
        pProperty->AddValue(strSDPField);
    }
    else
    {
        pProperty = new CapProperty(nSector, nMessageType);

        if (pProperty == IMS_NULL)
        {
            return IMS_FALSE;
        }

        pProperty->AddValue(strSDPField);

        if (!objCapabilities.Append(pProperty))
        {
            delete pProperty;
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL AppConfigPrivate::AddStreamMediaProperty(
        IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigP)
{
    // One or both of Audio and Video
    // { "Stream", "Audio Video" }

    if (objProperty.GetCount() != 2)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    pConfigP->bStreamMediaSupported = IMS_TRUE;

    AStringArray objSupportedTypes = ImsProperty::Decode(objProperty.GetElementAt(1));

#if 1
    // v1.1
    // Empty case
    if (objSupportedTypes.IsEmpty())
    {
        return IMS_TRUE;
    }
#else
    if ((objSupportedTypes.GetCount() < 1) || (objSupportedTypes.GetCount() > 2))
    {
        // TRACE("Property is malformed, wrong number of supported stream media types: %s",
        // ImsProperty::objPropertys(objProperty).GetStr());
        return IMS_FALSE;
    }
#endif

    if (!ImsProperty::CheckDuplicate(objSupportedTypes, IMS_TRUE))
    {
        return IMS_FALSE;
    }

    for (IMS_SINT32 i = 0; i < objSupportedTypes.GetCount(); ++i)
    {
        const AString& strValue = objSupportedTypes.GetElementAt(i);

        if (strValue.EqualsIgnoreCase(ImsProperty::STREAM_MEDIA_TYPE_AUDIO))
        {
            pConfigP->bStreamAudioSupported = IMS_TRUE;
        }
        else if (strValue.EqualsIgnoreCase(ImsProperty::STREAM_MEDIA_TYPE_VIDEO))
        {
            pConfigP->bStreamVideoSupported = IMS_TRUE;
        }
        else
        {
            IMS_TRACE_E(0,
                    "Property is malformed, stream media type(%s) does not support, "
                    "property (%s)",
                    strValue.GetStr(), ImsProperty::ToString(objProperty).GetStr(), 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL AppConfigPrivate::AddFramedMediaProperty(
        IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigP)
{
    // Set of MIME content types
    // { "Framed", "text/plain image/png", "4096" }

    if (objProperty.GetCount() != 3)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    pConfigP->objFramedMediaMimeTypes = ImsProperty::Decode(objProperty.GetElementAt(1));

    if (!ImsProperty::CheckDuplicate(pConfigP->objFramedMediaMimeTypes, IMS_FALSE))
    {
        return IMS_FALSE;
    }

    pConfigP->bFramedMediaSupported = IMS_TRUE;

    const AString& strMaxSize = objProperty.GetElementAt(2);

    if (strMaxSize.IsEmpty() || strMaxSize.IsNULL())
    {
        pConfigP->nFramedMediaTransferMaxSize = 0;
        pConfigP->bFramedMediaTransferMaxSize = IMS_FALSE;
    }
    else
    {
        IMS_BOOL bOK = IMS_TRUE;

        pConfigP->nFramedMediaTransferMaxSize = strMaxSize.ToUInt32(&bOK, 10);

        if (!bOK)
        {
            IMS_TRACE_E(0, "Property value is invalid, max-size (%s)", strMaxSize.GetStr(), 0, 0);
            pConfigP->objFramedMediaMimeTypes.RemoveAllElements();
            return IMS_FALSE;
        }

        pConfigP->bFramedMediaTransferMaxSize = IMS_TRUE;
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL AppConfigPrivate::AddBasicMediaProperty(
        IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigP)
{
    // Set of MIME content types
    // { "Basic", "application/myChess" }

    if (objProperty.GetCount() != 2)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    pConfigP->objBasicMediaMimeTypes = ImsProperty::Decode(objProperty.GetElementAt(1));

    if (!ImsProperty::CheckDuplicate(pConfigP->objBasicMediaMimeTypes, IMS_FALSE))
    {
        return IMS_FALSE;
    }

    pConfigP->bBasicMediaSupported = IMS_TRUE;

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL AppConfigPrivate::AddEventProperty(
        IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigP)
{
    // Set of event package names
    // { "Event", "presence" }

    if (objProperty.GetCount() != 2)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    pConfigP->objEventPackages = ImsProperty::Decode(objProperty.GetElementAt(1));

    if (!ImsProperty::CheckDuplicate(pConfigP->objEventPackages, IMS_TRUE))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL AppConfigPrivate::AddWriteHeaderProperty(
        IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigP)
{
    // Set of SIP header names for write access
    // { "Write", "P-ImageIncluded" }

    if (objProperty.GetCount() != 2)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    pConfigP->objWriteHeaders = ImsProperty::Decode(objProperty.GetElementAt(1));

    if (!ImsProperty::CheckDuplicate(pConfigP->objReadHeaders, IMS_FALSE))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL AppConfigPrivate::AddReadHeaderProperty(
        IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigP)
{
    // Set of SIP header names for read access
    // { "Read", "P-ImageIncluded" }

    if (objProperty.GetCount() != 2)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    pConfigP->objReadHeaders = ImsProperty::Decode(objProperty.GetElementAt(1));

    if (!ImsProperty::CheckDuplicate(pConfigP->objReadHeaders, IMS_FALSE))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL AppConfigPrivate::AddCapabilityProperty(
        IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigP)
{
    // The value is multi-valued, where the first value is the "sector id" that can be "Session",
    // "Framed", "StreamAudio" or "StreamVideo". The second value is a "message type" that
    // can be "Req", "Resp" or "Req_Resp". The remaining values are one or more SDP fields.
    // { "Cap", "Session", "Req_Resp", "a=x-type:videolive" }

    if (objProperty.GetCount() < 3)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    IMS_SINT32 nSectorId = CapProperty::StringToSectorId(objProperty.GetElementAt(1));

    switch (nSectorId)
    {
        case CapProperty::SECTOR_SESSION:
        case CapProperty::SECTOR_FRAMED:
        case CapProperty::SECTOR_STREAM_AUDIO:
        case CapProperty::SECTOR_STREAM_VIDEO:
            break;

        default:
            IMS_TRACE_E(0, "Property is malformed, unknown sector ID: %s",
                    ImsProperty::ToString(objProperty).GetStr(), 0, 0);
            return IMS_FALSE;
    }

    IMS_SINT32 nMessageType = CapProperty::StringToMessageType(objProperty.GetElementAt(2));

    switch (nMessageType)
    {
        case CapProperty::MESSAGE_TYPE_REQUEST:
        case CapProperty::MESSAGE_TYPE_RESPONSE:
        case CapProperty::MESSAGE_TYPE_REQUEST_RESPONSE:
            break;

        default:
            IMS_TRACE_E(0, "Property is malformed, unknown message type: %s",
                    ImsProperty::ToString(objProperty).GetStr(), 0, 0);
            return IMS_FALSE;
    }

    AString strSDPTypes;

    if (nSectorId == CapProperty::SECTOR_SESSION)
    {
        strSDPTypes = "vosiuepcbtrzka";
    }
    else
    {
        strSDPTypes = "micbka";
    }

    for (IMS_SINT32 i = 3; i < objProperty.GetCount(); ++i)
    {
        const AString& strSDPField = objProperty.GetElementAt(i);

        if ((strSDPField.GetLength() < 2) || (strSDPField[1] != '='))
        {
            IMS_TRACE_E(0, "Property is malformed, illegal SDP property: %s",
                    ImsProperty::ToString(objProperty).GetStr(), 0, 0);
            return IMS_FALSE;
        }

        // Check if the SDP field is a valid SDP line or not
        if (!strSDPTypes.Contains(strSDPField[0]))
        {
            IMS_TRACE_E(0, "Property is malformed, illegal SDP property: %s",
                    ImsProperty::ToString(objProperty).GetStr(), 0, 0);
            return IMS_FALSE;
        }

        pConfigP->AddCapabilitySDP(nSectorId, nMessageType, strSDPField);
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL AppConfigPrivate::AddCoreServiceProperty(
        IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigP)
{
    // The CoreService property is multi-valued, where the contents of service definitions are
    // elements in the order; "service id", "IARI", "ICSIs", and "Feature Tags".
    // Individual ICSIs and Feature Tags can be appended with ";require" and/or ";explicit"
    // to flag them for "require" and "explicit" processing.
    // A white space is used as separator if more than one ICSI or Feature Tag.
    // { "CoreService", "myChess", "urn:IMSAPI:com.myCompany.chess", "", "" }
    // { "CoreService", "myChess", "", "urn:urn-3gpp:org.3gpp.icsi;require;explicit", "" }
    // { "CoreService", "myChess", "", "urn:urn-7:3gpp-service.ims.icsi.mmtel",
    //    "Audio Video;explicit Duplex;require" }

    if (objProperty.GetCount() != 5)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    if (pConfigP->GetCoreServiceConfig(objProperty.GetElementAt(1)) != IMS_NULL)
    {
        IMS_TRACE_E(0, "Property is malformed, CoreService with id (%s) defined multiple times",
                objProperty.GetElementAt(1).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    CoreServiceConfig* pCoreServiceConfig = new CoreServiceConfig(objProperty.GetElementAt(1));

    if (!pCoreServiceConfig->Create(objProperty))
    {
        delete pCoreServiceConfig;
        return IMS_FALSE;
    }

    if (!pConfigP->objCoreServiceConfigs.Append(pCoreServiceConfig))
    {
        delete pCoreServiceConfig;
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL AppConfigPrivate::AddCoreServiceRelatedProperty(
        IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigP)
{
    if (objProperty.GetCount() < 2)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    CoreServiceConfig* pCoreServiceConfig =
            pConfigP->GetCoreServiceConfig(objProperty.GetElementAt(1));

    if (pCoreServiceConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "Property is malformed, %s specified core service does not exist: %s",
                objProperty.GetElementAt(0).GetStr(), ImsProperty::ToString(objProperty).GetStr(),
                0);
        return IMS_FALSE;
    }

    return pCoreServiceConfig->AddProperty(objProperty);
}

PUBLIC
AppConfig::AppConfig(IN const AString& strAppId_) :
        pConfigP(new AppConfigPrivate(strAppId_))
{
    if (pConfigP != IMS_NULL)
    {
        pConfigP->AddReference();
    }
}

PUBLIC
AppConfig::AppConfig(IN const AppConfig& objRHS) :
        pConfigP(objRHS.pConfigP)
{
    if (pConfigP != IMS_NULL)
    {
        pConfigP->AddReference();
    }
}

PUBLIC VIRTUAL AppConfig::~AppConfig()
{
    if (pConfigP != IMS_NULL)
    {
        pConfigP->RemoveReference();
    }
}

PUBLIC
AppConfig& AppConfig::operator=(IN const AppConfig& objRHS)
{
    if (this != &objRHS)
    {
        AppConfigPrivate* pOldConfigP = pConfigP;

        pConfigP = objRHS.pConfigP;

        if (pConfigP != IMS_NULL)
        {
            pConfigP->AddReference();
        }

        if (pOldConfigP != IMS_NULL)
        {
            pOldConfigP->RemoveReference();
        }
    }

    return (*this);
}

PUBLIC VIRTUAL const AString& AppConfig::GetAppId() const
{
    return pConfigP->strAppId;
}

PUBLIC VIRTUAL const ICoreServiceConfig* AppConfig::GetCoreServiceConfig(
        IN const AString& strServiceId) const
{
    return pConfigP->GetCoreServiceConfig(strServiceId);
}

PUBLIC VIRTUAL ImsRegistry* AppConfig::ToRegistry() const
{
    AStringArray objProperty;
    ImsRegistry* pRegistry = new ImsRegistry();

    // Stream media property
    if (IsStreamMediaAudioSupported() || IsStreamMediaVideoSupported())
    {
        AStringArray objValues;

        if (IsStreamMediaAudioSupported())
        {
            objValues.AddElement(ImsProperty::STREAM_MEDIA_TYPE_AUDIO);
        }
        if (IsStreamMediaVideoSupported())
        {
            objValues.AddElement(ImsProperty::STREAM_MEDIA_TYPE_VIDEO);
        }

        objProperty.AddElement(ImsProperty::PKEY_STRING[ImsProperty::PKEY_STREAM]);
        objProperty.AddElement(ImsProperty::Encode(objValues));

        pRegistry->Add(objProperty);
    }

    // Framed media property
    if (IsFramedMediaSupported())
    {
        objProperty.RemoveAllElements();

        objProperty.AddElement(ImsProperty::PKEY_STRING[ImsProperty::PKEY_FRAMED]);
        objProperty.AddElement(ImsProperty::Encode(pConfigP->objFramedMediaMimeTypes));

        if (pConfigP->bFramedMediaTransferMaxSize)
        {
            AString strTransferSize;

            strTransferSize.SetNumber(pConfigP->nFramedMediaTransferMaxSize);
            objProperty.AddElement(strTransferSize);
        }
        else
        {
            objProperty.AddElement(AString::ConstEmpty());
        }

        pRegistry->Add(objProperty);
    }

    // Basic media property
    if (IsBasicMediaSupported())
    {
        objProperty.RemoveAllElements();

        objProperty.AddElement(ImsProperty::PKEY_STRING[ImsProperty::PKEY_BASIC]);
        objProperty.AddElement(ImsProperty::Encode(pConfigP->objBasicMediaMimeTypes));

        pRegistry->Add(objProperty);
    }

    // Event package property
    if (pConfigP->objEventPackages.GetCount() > 0)
    {
        objProperty.AddElement(ImsProperty::PKEY_STRING[ImsProperty::PKEY_EVENT]);

        for (IMS_SINT32 i = 0; i < pConfigP->objEventPackages.GetCount(); ++i)
        {
            objProperty.AddElement(pConfigP->objEventPackages.GetElementAt(i));
        }

        pRegistry->Add(objProperty);
    }

    // Read property
    if (pConfigP->objReadHeaders.GetCount() > 0)
    {
        objProperty.AddElement(ImsProperty::PKEY_STRING[ImsProperty::PKEY_READ]);

        for (IMS_SINT32 i = 0; i < pConfigP->objReadHeaders.GetCount(); ++i)
        {
            objProperty.AddElement(pConfigP->objReadHeaders.GetElementAt(i));
        }

        pRegistry->Add(objProperty);
    }

    // Write property
    if (pConfigP->objWriteHeaders.GetCount() > 0)
    {
        objProperty.AddElement(ImsProperty::PKEY_STRING[ImsProperty::PKEY_WRITE]);

        for (IMS_SINT32 i = 0; i < pConfigP->objWriteHeaders.GetCount(); ++i)
        {
            objProperty.AddElement(pConfigP->objWriteHeaders.GetElementAt(i));
        }

        pRegistry->Add(objProperty);
    }

    // Cap property
    if (pConfigP->objCapabilities.GetSize() > 0)
    {
        for (IMS_SINT32 i = (CapProperty::SECTOR_INVALID + 1); i < CapProperty::SECTOR_MAX; ++i)
        {
            for (IMS_SINT32 j = (CapProperty::MESSAGE_TYPE_INVALID + 1);
                    j < CapProperty::MESSAGE_TYPE_MAX; ++j)
            {
                AStringArray objSDPFields = GetCapabilitySDPs(i, j);

                if (objSDPFields.GetCount() > 0)
                {
                    objProperty.RemoveAllElements();

                    objProperty.AddElement(ImsProperty::PKEY_STRING[ImsProperty::PKEY_CAP]);
                    objProperty.AddElement(CapProperty::SectorIdToString(i));
                    objProperty.AddElement(CapProperty::MessageTypeToString(j));

                    for (IMS_SINT32 k = 0; k < objSDPFields.GetCount(); ++k)
                    {
                        objProperty.AddElement(objSDPFields.GetElementAt(k));
                    }

                    pRegistry->Add(objProperty);
                }
            }
        }
    }

    // CoreService property
    if (pConfigP->objCoreServiceConfigs.GetSize() > 0)
    {
        for (IMS_UINT32 i = 0; i < pConfigP->objCoreServiceConfigs.GetSize(); ++i)
        {
            CoreServiceConfig* pCoreServiceConfig = pConfigP->objCoreServiceConfigs.GetAt(i);

            if (pCoreServiceConfig != IMS_NULL)
            {
                pCoreServiceConfig->ToRegistry(pRegistry);
            }
        }
    }

    return pRegistry;
}

PUBLIC
IMS_BOOL AppConfig::Create(
        IN const ImsRegistry& objRegistry, IN IMS_SINT32 /*nSlotId = IMS_SLOT_0*/)
{
    ImsRegistry objTrimmedRegistry;
    IMS_BOOL bResult = ImsProperty::TrimAndCheckProperties(objRegistry, objTrimmedRegistry);

    if (!bResult)
    {
        IMS_TRACE_E(0, "Property is malformed, validation check failed", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_SINT32 i = 0;

    // Add CoreService property first
    for (i = 0; i < objTrimmedRegistry.GetCount(); ++i)
    {
        const AStringArray& objProperty = objTrimmedRegistry.GetAt(i);

        if (ImsProperty::PKEY_CORE_SERVICE == ImsProperty::StringToKey(objProperty.GetElementAt(0)))
        {
            if (!AppConfigPrivate::AddCoreServiceProperty(objProperty, pConfigP))
                return IMS_FALSE;
        }
    }

    // Add Application registry properties & CoreService related properties
    for (i = 0; i < objTrimmedRegistry.GetCount(); ++i)
    {
        const AStringArray& objProperty = objTrimmedRegistry.GetAt(i);
        IMS_SINT32 nKey = ImsProperty::StringToKey(objProperty.GetElementAt(0));

        bResult = IMS_FALSE;

        switch (nKey)
        {
            case ImsProperty::PKEY_STREAM:
                bResult = AppConfigPrivate::AddStreamMediaProperty(objProperty, pConfigP);
                break;

            case ImsProperty::PKEY_FRAMED:
                bResult = AppConfigPrivate::AddFramedMediaProperty(objProperty, pConfigP);
                break;

            case ImsProperty::PKEY_BASIC:
                bResult = AppConfigPrivate::AddBasicMediaProperty(objProperty, pConfigP);
                break;

            case ImsProperty::PKEY_EVENT:
                bResult = AppConfigPrivate::AddEventProperty(objProperty, pConfigP);
                break;

            case ImsProperty::PKEY_READ:
                bResult = AppConfigPrivate::AddReadHeaderProperty(objProperty, pConfigP);
                break;

            case ImsProperty::PKEY_WRITE:
                bResult = AppConfigPrivate::AddWriteHeaderProperty(objProperty, pConfigP);
                break;

            case ImsProperty::PKEY_CAP:
                bResult = AppConfigPrivate::AddCapabilityProperty(objProperty, pConfigP);
                break;

                // CoreService related properties
            case ImsProperty::PKEY_CORE_SERVICE:
                // It does not need to be added; Already processed.
                bResult = IMS_TRUE;
                break;

            case ImsProperty::PKEY_QOS:
            case ImsProperty::PKEY_REG:
            case ImsProperty::PKEY_MPROF:
            case ImsProperty::PKEY_CONNECTION:
                bResult = AppConfigPrivate::AddCoreServiceRelatedProperty(objProperty, pConfigP);
                break;
        }

        if (!bResult)
        {
            IMS_TRACE_E(0, "Adding a property (%s) failed",
                    ImsProperty::ToString(objProperty).GetStr(), 0, 0);
            return IMS_FALSE;
        }
    }

    // Check if the registry MUST contain at least one of the IMS properties:
    //  StreamMedia, FramedMedia, BasicMedia, Event, CoreService,
    if (!pConfigP->CheckMandatoryProperty())
    {
        IMS_TRACE_E(0,
                "The registry does not support at least one of the properties:"
                "Stream, Framed, Basic, Event, CoreService",
                0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AppConfig::Equals(IN const AString& strAppId) const
{
    return pConfigP->strAppId.EqualsIgnoreCase(strAppId);
}

PUBLIC
IMS_BOOL AppConfig::IsStreamMediaSupported() const
{
    return pConfigP->bStreamMediaSupported;
}

PUBLIC
IMS_BOOL AppConfig::IsStreamMediaAudioSupported() const
{
    return pConfigP->bStreamAudioSupported;
}

PUBLIC
IMS_BOOL AppConfig::IsStreamMediaVideoSupported() const
{
    return pConfigP->bStreamVideoSupported;
}

PUBLIC
IMS_BOOL AppConfig::IsFramedMediaSupported() const
{
    return pConfigP->bFramedMediaSupported;
}

PUBLIC
const AStringArray& AppConfig::GetFramedMediaMimeTypes() const
{
    return pConfigP->objFramedMediaMimeTypes;
}

PUBLIC
IMS_BOOL AppConfig::IsFramedMediaMaxSizePresent() const
{
    return pConfigP->bFramedMediaTransferMaxSize;
}

PUBLIC
IMS_UINT32 AppConfig::GetFramedMediaMaxSize() const
{
    return pConfigP->nFramedMediaTransferMaxSize;
}

PUBLIC
IMS_BOOL AppConfig::IsBasicMediaSupported() const
{
    return pConfigP->bBasicMediaSupported;
}

PUBLIC
const AStringArray& AppConfig::GetBasicMediaMimeTypes() const
{
    return pConfigP->objBasicMediaMimeTypes;
}

PUBLIC
IMS_BOOL AppConfig::IsEventPackageSupported(IN const AString& strEvent) const
{
    // Compares byte-by-byte
    return pConfigP->objEventPackages.Contains(strEvent);
}

PUBLIC
const AStringArray& AppConfig::GetSupportedEventPackages() const
{
    return pConfigP->objEventPackages;
}

PUBLIC
IMS_BOOL AppConfig::IsHeaderReadable(IN const AString& strHeader) const
{
    return pConfigP->objReadHeaders.Contains(strHeader, IMS_FALSE);
}

PUBLIC
IMS_BOOL AppConfig::IsHeaderWritable(IN const AString& strHeader) const
{
    return pConfigP->objWriteHeaders.Contains(strHeader, IMS_FALSE);
}

PUBLIC
AStringArray AppConfig::GetCapabilitySDPs(IN IMS_SINT32 nSector, IN IMS_SINT32 nMessageType) const
{
    CapProperty* pProperty = pConfigP->GetCapProperty(nSector, nMessageType);

    if (pProperty != IMS_NULL)
    {
        return pProperty->GetValues();
    }

    return AStringArray();
}

PUBLIC
const CoreServiceConfig* AppConfig::GetCoreServiceConfigEx(IN const AString& strServiceId) const
{
    return pConfigP->GetCoreServiceConfig(strServiceId);
}

PUBLIC
const IMSList<CoreServiceConfig*>& AppConfig::GetCoreServiceConfigs() const
{
    return pConfigP->objCoreServiceConfigs;
}
