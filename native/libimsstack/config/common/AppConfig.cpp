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
#include "RcObject.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "private/AppConfig.h"
#include "private/CapProperty.h"
#include "private/CoreServiceConfig.h"

__IMS_TRACE_TAG_CONF__;

class AppConfigPrivate : public RcObject
{
public:
    explicit AppConfigPrivate(IN const AString& strAppId);
    ~AppConfigPrivate();

    AppConfigPrivate(IN const AppConfigPrivate&) = delete;
    AppConfigPrivate& operator=(IN const AppConfigPrivate&) = delete;

public:
    IMS_BOOL CheckMandatoryProperty() const;
    CapProperty* GetCapProperty(IN IMS_SINT32 nSector, IN IMS_SINT32 nMessageType) const;
    CoreServiceConfig* GetCoreServiceConfig(IN const AString& strServiceId) const;

    IMS_BOOL AddCapabilitySdp(
            IN IMS_SINT32 nSector, IN IMS_SINT32 nMessageType, IN const AString& strSdpField);

    static IMS_BOOL AddStreamMediaProperty(
            IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigPrivate);
    static IMS_BOOL AddFramedMediaProperty(
            IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigPrivate);
    static IMS_BOOL AddBasicMediaProperty(
            IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigPrivate);
    static IMS_BOOL AddEventProperty(
            IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigPrivate);
    static IMS_BOOL AddWriteHeaderProperty(
            IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigPrivate);
    static IMS_BOOL AddReadHeaderProperty(
            IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigPrivate);
    static IMS_BOOL AddCapabilityProperty(
            IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigPrivate);
    static IMS_BOOL AddCoreServiceProperty(
            IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigPrivate);
    static IMS_BOOL AddCoreServiceRelatedProperty(
            IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigPrivate);

private:
    friend class AppConfig;

    AString m_strAppId;

    // StreamMedia property
    IMS_BOOL m_bStreamMediaSupported;
    IMS_BOOL m_bStreamAudioSupported;
    IMS_BOOL m_bStreamVideoSupported;
    IMS_BOOL m_bStreamTextSupported;

    // FramedMedia property
    IMS_BOOL m_bFramedMediaSupported;
    AStringArray m_objFramedMediaMimeTypes;
    IMS_BOOL m_bFramedMediaTransferMaxSize;
    IMS_UINT32 m_nFramedMediaTransferMaxSize;

    // BasicMedia property
    IMS_BOOL m_bBasicMediaSupported;
    AStringArray m_objBasicMediaMimeTypes;

    // Supported event package property
    AStringArray m_objEventPackages;

    // CoreService property
    ImsList<CoreServiceConfig*> m_objCoreServiceConfigs;

    // Writable headers property
    AStringArray m_objWriteHeaders;

    // Readable headers property
    AStringArray m_objReadHeaders;

    // SDP fields property for Capabilities
    ImsList<CapProperty*> m_objCapabilities;
};

PUBLIC
AppConfigPrivate::AppConfigPrivate(IN const AString& strAppId) :
        m_strAppId(strAppId),
        m_bStreamMediaSupported(IMS_FALSE),
        m_bStreamAudioSupported(IMS_FALSE),
        m_bStreamVideoSupported(IMS_FALSE),
        m_bStreamTextSupported(IMS_FALSE),
        m_bFramedMediaSupported(IMS_FALSE),
        m_bFramedMediaTransferMaxSize(IMS_FALSE),
        m_nFramedMediaTransferMaxSize(0),
        m_bBasicMediaSupported(IMS_FALSE)
{
}

PUBLIC VIRTUAL AppConfigPrivate::~AppConfigPrivate()
{
    if (!m_objCoreServiceConfigs.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objCoreServiceConfigs.GetSize(); ++i)
        {
            CoreServiceConfig* pServiceConfig = m_objCoreServiceConfigs.GetAt(i);

            if (pServiceConfig != IMS_NULL)
            {
                delete pServiceConfig;
            }
        }

        m_objCoreServiceConfigs.Clear();
    }

    if (!m_objCapabilities.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objCapabilities.GetSize(); ++i)
        {
            CapProperty* pCapProperty = m_objCapabilities.GetAt(i);

            if (pCapProperty != IMS_NULL)
            {
                delete pCapProperty;
            }
        }

        m_objCapabilities.Clear();
    }
}

PUBLIC
IMS_BOOL AppConfigPrivate::CheckMandatoryProperty() const
{
    // Check if the registry MUST contain at least one of the IMS properties:
    //  StreamMedia, FramedMedia, BasicMedia, Event, CoreService,

    if (!m_bStreamMediaSupported && !m_bFramedMediaSupported && !m_bBasicMediaSupported &&
            m_objEventPackages.IsEmpty() && m_objCoreServiceConfigs.IsEmpty())
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
CapProperty* AppConfigPrivate::GetCapProperty(
        IN IMS_SINT32 nSector, IN IMS_SINT32 nMessageType) const
{
    if (m_objCapabilities.IsEmpty())
    {
        return IMS_NULL;
    }

    AString strCapKey = CapProperty::CreateCapKey(nSector, nMessageType);

    for (IMS_UINT32 i = 0; i < m_objCapabilities.GetSize(); ++i)
    {
        CapProperty* pProperty = m_objCapabilities.GetAt(i);

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
    for (IMS_UINT32 i = 0; i < m_objCoreServiceConfigs.GetSize(); ++i)
    {
        CoreServiceConfig* pServiceConfig = m_objCoreServiceConfigs.GetAt(i);

        if (pServiceConfig->GetServiceId().EqualsIgnoreCase(strServiceId))
        {
            return pServiceConfig;
        }
    }

    return IMS_NULL;
}

PUBLIC
IMS_BOOL AppConfigPrivate::AddCapabilitySdp(
        IN IMS_SINT32 nSector, IN IMS_SINT32 nMessageType, IN const AString& strSdpField)
{
    CapProperty* pProperty = GetCapProperty(nSector, nMessageType);

    if (pProperty != IMS_NULL)
    {
        pProperty->AddValue(strSdpField);
    }
    else
    {
        pProperty = new CapProperty(nSector, nMessageType);

        if (pProperty == IMS_NULL)
        {
            return IMS_FALSE;
        }

        pProperty->AddValue(strSdpField);

        if (!m_objCapabilities.Append(pProperty))
        {
            delete pProperty;
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL AppConfigPrivate::AddStreamMediaProperty(
        IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigPrivate)
{
    // One or both of Audio and Video
    // { "Stream", "Audio Video" }

    if (objProperty.GetCount() != 2)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    pConfigPrivate->m_bStreamMediaSupported = IMS_TRUE;

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
            pConfigPrivate->m_bStreamAudioSupported = IMS_TRUE;
        }
        else if (strValue.EqualsIgnoreCase(ImsProperty::STREAM_MEDIA_TYPE_VIDEO))
        {
            pConfigPrivate->m_bStreamVideoSupported = IMS_TRUE;
        }
        else if (strValue.EqualsIgnoreCase(ImsProperty::STREAM_MEDIA_TYPE_TEXT))
        {
            pConfigPrivate->m_bStreamTextSupported = IMS_TRUE;
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
        IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigPrivate)
{
    // Set of MIME content types
    // { "Framed", "text/plain image/png", "4096" }

    if (objProperty.GetCount() != 3)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    pConfigPrivate->m_objFramedMediaMimeTypes = ImsProperty::Decode(objProperty.GetElementAt(1));

    if (!ImsProperty::CheckDuplicate(pConfigPrivate->m_objFramedMediaMimeTypes, IMS_FALSE))
    {
        return IMS_FALSE;
    }

    pConfigPrivate->m_bFramedMediaSupported = IMS_TRUE;

    const AString& strMaxSize = objProperty.GetElementAt(2);

    if (strMaxSize.IsEmpty() || strMaxSize.IsNULL())
    {
        pConfigPrivate->m_nFramedMediaTransferMaxSize = 0;
        pConfigPrivate->m_bFramedMediaTransferMaxSize = IMS_FALSE;
    }
    else
    {
        IMS_BOOL bOk = IMS_TRUE;

        pConfigPrivate->m_nFramedMediaTransferMaxSize = strMaxSize.ToUInt32(&bOk, 10);

        if (!bOk)
        {
            IMS_TRACE_E(0, "Property value is invalid, max-size (%s)", strMaxSize.GetStr(), 0, 0);
            pConfigPrivate->m_objFramedMediaMimeTypes.RemoveAllElements();
            return IMS_FALSE;
        }

        pConfigPrivate->m_bFramedMediaTransferMaxSize = IMS_TRUE;
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL AppConfigPrivate::AddBasicMediaProperty(
        IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigPrivate)
{
    // Set of MIME content types
    // { "Basic", "application/myChess" }

    if (objProperty.GetCount() != 2)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    pConfigPrivate->m_objBasicMediaMimeTypes = ImsProperty::Decode(objProperty.GetElementAt(1));

    if (!ImsProperty::CheckDuplicate(pConfigPrivate->m_objBasicMediaMimeTypes, IMS_FALSE))
    {
        return IMS_FALSE;
    }

    pConfigPrivate->m_bBasicMediaSupported = IMS_TRUE;

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL AppConfigPrivate::AddEventProperty(
        IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigPrivate)
{
    // Set of event package names
    // { "Event", "presence" }

    if (objProperty.GetCount() != 2)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    pConfigPrivate->m_objEventPackages = ImsProperty::Decode(objProperty.GetElementAt(1));

    if (!ImsProperty::CheckDuplicate(pConfigPrivate->m_objEventPackages, IMS_TRUE))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL AppConfigPrivate::AddWriteHeaderProperty(
        IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigPrivate)
{
    // Set of SIP header names for write access
    // { "Write", "P-ImageIncluded" }

    if (objProperty.GetCount() != 2)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    pConfigPrivate->m_objWriteHeaders = ImsProperty::Decode(objProperty.GetElementAt(1));

    if (!ImsProperty::CheckDuplicate(pConfigPrivate->m_objReadHeaders, IMS_FALSE))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL AppConfigPrivate::AddReadHeaderProperty(
        IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigPrivate)
{
    // Set of SIP header names for read access
    // { "Read", "P-ImageIncluded" }

    if (objProperty.GetCount() != 2)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    pConfigPrivate->m_objReadHeaders = ImsProperty::Decode(objProperty.GetElementAt(1));

    if (!ImsProperty::CheckDuplicate(pConfigPrivate->m_objReadHeaders, IMS_FALSE))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL AppConfigPrivate::AddCapabilityProperty(
        IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigPrivate)
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
        case CapProperty::SECTOR_SESSION:       // FALL-THROUGH
        case CapProperty::SECTOR_FRAMED:        // FALL-THROUGH
        case CapProperty::SECTOR_STREAM_AUDIO:  // FALL-THROUGH
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
        case CapProperty::MESSAGE_TYPE_REQUEST:   // FALL-THROUGH
        case CapProperty::MESSAGE_TYPE_RESPONSE:  // FALL-THROUGH
        case CapProperty::MESSAGE_TYPE_REQUEST_RESPONSE:
            break;

        default:
            IMS_TRACE_E(0, "Property is malformed, unknown message type: %s",
                    ImsProperty::ToString(objProperty).GetStr(), 0, 0);
            return IMS_FALSE;
    }

    AString strSdpTypes;

    if (nSectorId == CapProperty::SECTOR_SESSION)
    {
        strSdpTypes = "vosiuepcbtrzka";
    }
    else
    {
        strSdpTypes = "micbka";
    }

    for (IMS_SINT32 i = 3; i < objProperty.GetCount(); ++i)
    {
        const AString& strSdpField = objProperty.GetElementAt(i);

        if ((strSdpField.GetLength() < 2) || (strSdpField[1] != '='))
        {
            IMS_TRACE_E(0, "Property is malformed, illegal SDP property: %s",
                    ImsProperty::ToString(objProperty).GetStr(), 0, 0);
            return IMS_FALSE;
        }

        // Check if the SDP field is a valid SDP line or not
        if (!strSdpTypes.Contains(strSdpField[0]))
        {
            IMS_TRACE_E(0, "Property is malformed, illegal SDP property: %s",
                    ImsProperty::ToString(objProperty).GetStr(), 0, 0);
            return IMS_FALSE;
        }

        pConfigPrivate->AddCapabilitySdp(nSectorId, nMessageType, strSdpField);
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL AppConfigPrivate::AddCoreServiceProperty(
        IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigPrivate)
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

    if (pConfigPrivate->GetCoreServiceConfig(objProperty.GetElementAt(1)) != IMS_NULL)
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

    if (!pConfigPrivate->m_objCoreServiceConfigs.Append(pCoreServiceConfig))
    {
        delete pCoreServiceConfig;
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL AppConfigPrivate::AddCoreServiceRelatedProperty(
        IN const AStringArray& objProperty, IN_OUT AppConfigPrivate* pConfigPrivate)
{
    if (objProperty.GetCount() < 2)
    {
        IMS_TRACE_E(0, "Property is malformed, wrong number of elements: %s",
                ImsProperty::ToString(objProperty).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    CoreServiceConfig* pCoreServiceConfig =
            pConfigPrivate->GetCoreServiceConfig(objProperty.GetElementAt(1));

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
AppConfig::AppConfig(IN const AString& strAppId) :
        m_pConfigPrivate(new AppConfigPrivate(strAppId))
{
    if (m_pConfigPrivate != IMS_NULL)
    {
        m_pConfigPrivate->AddReference();
    }
}

PUBLIC
AppConfig::AppConfig(IN const AppConfig& other) :
        m_pConfigPrivate(other.m_pConfigPrivate)
{
    if (m_pConfigPrivate != IMS_NULL)
    {
        m_pConfigPrivate->AddReference();
    }
}

PUBLIC VIRTUAL AppConfig::~AppConfig()
{
    if (m_pConfigPrivate != IMS_NULL)
    {
        m_pConfigPrivate->RemoveReference();
    }
}

PUBLIC
AppConfig& AppConfig::operator=(IN const AppConfig& other)
{
    if (this != &other)
    {
        AppConfigPrivate* pOldConfigPrivate = m_pConfigPrivate;

        m_pConfigPrivate = other.m_pConfigPrivate;

        if (m_pConfigPrivate != IMS_NULL)
        {
            m_pConfigPrivate->AddReference();
        }

        if (pOldConfigPrivate != IMS_NULL)
        {
            pOldConfigPrivate->RemoveReference();
        }
    }

    return (*this);
}

PUBLIC VIRTUAL const AString& AppConfig::GetAppId() const
{
    return m_pConfigPrivate->m_strAppId;
}

PUBLIC VIRTUAL const ICoreServiceConfig* AppConfig::GetCoreServiceConfig(
        IN const AString& strServiceId) const
{
    return m_pConfigPrivate->GetCoreServiceConfig(strServiceId);
}

PUBLIC VIRTUAL ImsRegistry* AppConfig::ToRegistry() const
{
    AStringArray objProperty;
    ImsRegistry* pRegistry = new ImsRegistry();

    // Stream media property
    if (IsStreamMediaAudioSupported() || IsStreamMediaVideoSupported() ||
            IsStreamMediaTextSupported())
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
        if (IsStreamMediaTextSupported())
        {
            objValues.AddElement(ImsProperty::STREAM_MEDIA_TYPE_TEXT);
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
        objProperty.AddElement(ImsProperty::Encode(m_pConfigPrivate->m_objFramedMediaMimeTypes));

        if (m_pConfigPrivate->m_bFramedMediaTransferMaxSize)
        {
            AString strTransferSize;

            strTransferSize.SetNumber(m_pConfigPrivate->m_nFramedMediaTransferMaxSize);
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
        objProperty.AddElement(ImsProperty::Encode(m_pConfigPrivate->m_objBasicMediaMimeTypes));

        pRegistry->Add(objProperty);
    }

    // Event package property
    if (m_pConfigPrivate->m_objEventPackages.GetCount() > 0)
    {
        objProperty.AddElement(ImsProperty::PKEY_STRING[ImsProperty::PKEY_EVENT]);

        for (IMS_SINT32 i = 0; i < m_pConfigPrivate->m_objEventPackages.GetCount(); ++i)
        {
            objProperty.AddElement(m_pConfigPrivate->m_objEventPackages.GetElementAt(i));
        }

        pRegistry->Add(objProperty);
    }

    // Read property
    if (m_pConfigPrivate->m_objReadHeaders.GetCount() > 0)
    {
        objProperty.AddElement(ImsProperty::PKEY_STRING[ImsProperty::PKEY_READ]);

        for (IMS_SINT32 i = 0; i < m_pConfigPrivate->m_objReadHeaders.GetCount(); ++i)
        {
            objProperty.AddElement(m_pConfigPrivate->m_objReadHeaders.GetElementAt(i));
        }

        pRegistry->Add(objProperty);
    }

    // Write property
    if (m_pConfigPrivate->m_objWriteHeaders.GetCount() > 0)
    {
        objProperty.AddElement(ImsProperty::PKEY_STRING[ImsProperty::PKEY_WRITE]);

        for (IMS_SINT32 i = 0; i < m_pConfigPrivate->m_objWriteHeaders.GetCount(); ++i)
        {
            objProperty.AddElement(m_pConfigPrivate->m_objWriteHeaders.GetElementAt(i));
        }

        pRegistry->Add(objProperty);
    }

    // Cap property
    if (m_pConfigPrivate->m_objCapabilities.GetSize() > 0)
    {
        for (IMS_SINT32 i = (CapProperty::SECTOR_INVALID + 1); i < CapProperty::SECTOR_MAX; ++i)
        {
            for (IMS_SINT32 j = (CapProperty::MESSAGE_TYPE_INVALID + 1);
                    j < CapProperty::MESSAGE_TYPE_MAX; ++j)
            {
                AStringArray objSdpFields = GetCapabilitySdps(i, j);

                if (objSdpFields.GetCount() > 0)
                {
                    objProperty.RemoveAllElements();

                    objProperty.AddElement(ImsProperty::PKEY_STRING[ImsProperty::PKEY_CAP]);
                    objProperty.AddElement(CapProperty::SectorIdToString(i));
                    objProperty.AddElement(CapProperty::MessageTypeToString(j));

                    for (IMS_SINT32 k = 0; k < objSdpFields.GetCount(); ++k)
                    {
                        objProperty.AddElement(objSdpFields.GetElementAt(k));
                    }

                    pRegistry->Add(objProperty);
                }
            }
        }
    }

    // CoreService property
    if (m_pConfigPrivate->m_objCoreServiceConfigs.GetSize() > 0)
    {
        for (IMS_UINT32 i = 0; i < m_pConfigPrivate->m_objCoreServiceConfigs.GetSize(); ++i)
        {
            CoreServiceConfig* pCoreServiceConfig =
                    m_pConfigPrivate->m_objCoreServiceConfigs.GetAt(i);

            if (pCoreServiceConfig != IMS_NULL)
            {
                pCoreServiceConfig->ToRegistry(pRegistry);
            }
        }
    }

    return pRegistry;
}

PUBLIC
IMS_BOOL AppConfig::Create(IN const ImsRegistry& objRegistry, IN IMS_SINT32 /*nSlotId*/)
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
            if (!AppConfigPrivate::AddCoreServiceProperty(objProperty, m_pConfigPrivate))
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
                bResult = AppConfigPrivate::AddStreamMediaProperty(objProperty, m_pConfigPrivate);
                break;
            case ImsProperty::PKEY_FRAMED:
                bResult = AppConfigPrivate::AddFramedMediaProperty(objProperty, m_pConfigPrivate);
                break;
            case ImsProperty::PKEY_BASIC:
                bResult = AppConfigPrivate::AddBasicMediaProperty(objProperty, m_pConfigPrivate);
                break;
            case ImsProperty::PKEY_EVENT:
                bResult = AppConfigPrivate::AddEventProperty(objProperty, m_pConfigPrivate);
                break;
            case ImsProperty::PKEY_READ:
                bResult = AppConfigPrivate::AddReadHeaderProperty(objProperty, m_pConfigPrivate);
                break;
            case ImsProperty::PKEY_WRITE:
                bResult = AppConfigPrivate::AddWriteHeaderProperty(objProperty, m_pConfigPrivate);
                break;
            case ImsProperty::PKEY_CAP:
                bResult = AppConfigPrivate::AddCapabilityProperty(objProperty, m_pConfigPrivate);
                break;
                // CoreService related properties
            case ImsProperty::PKEY_CORE_SERVICE:
                // It does not need to be added; Already processed.
                bResult = IMS_TRUE;
                break;
            case ImsProperty::PKEY_QOS:    // FALL-THROUGH
            case ImsProperty::PKEY_REG:    // FALL-THROUGH
            case ImsProperty::PKEY_MPROF:  // FALL-THROUGH
            case ImsProperty::PKEY_CONNECTION:
                bResult = AppConfigPrivate::AddCoreServiceRelatedProperty(
                        objProperty, m_pConfigPrivate);
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
    if (!m_pConfigPrivate->CheckMandatoryProperty())
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
    return m_pConfigPrivate->m_strAppId.EqualsIgnoreCase(strAppId);
}

PUBLIC
IMS_BOOL AppConfig::IsStreamMediaSupported() const
{
    return m_pConfigPrivate->m_bStreamMediaSupported;
}

PUBLIC
IMS_BOOL AppConfig::IsStreamMediaAudioSupported() const
{
    return m_pConfigPrivate->m_bStreamAudioSupported;
}

PUBLIC
IMS_BOOL AppConfig::IsStreamMediaVideoSupported() const
{
    return m_pConfigPrivate->m_bStreamVideoSupported;
}

PUBLIC
IMS_BOOL AppConfig::IsStreamMediaTextSupported() const
{
    return m_pConfigPrivate->m_bStreamTextSupported;
}

PUBLIC
IMS_BOOL AppConfig::IsFramedMediaSupported() const
{
    return m_pConfigPrivate->m_bFramedMediaSupported;
}

PUBLIC
const AStringArray& AppConfig::GetFramedMediaMimeTypes() const
{
    return m_pConfigPrivate->m_objFramedMediaMimeTypes;
}

PUBLIC
IMS_BOOL AppConfig::IsFramedMediaMaxSizePresent() const
{
    return m_pConfigPrivate->m_bFramedMediaTransferMaxSize;
}

PUBLIC
IMS_UINT32 AppConfig::GetFramedMediaMaxSize() const
{
    return m_pConfigPrivate->m_nFramedMediaTransferMaxSize;
}

PUBLIC
IMS_BOOL AppConfig::IsBasicMediaSupported() const
{
    return m_pConfigPrivate->m_bBasicMediaSupported;
}

PUBLIC
const AStringArray& AppConfig::GetBasicMediaMimeTypes() const
{
    return m_pConfigPrivate->m_objBasicMediaMimeTypes;
}

PUBLIC
IMS_BOOL AppConfig::IsEventPackageSupported(IN const AString& strEvent) const
{
    // Compares byte-by-byte
    return m_pConfigPrivate->m_objEventPackages.Contains(strEvent);
}

PUBLIC
const AStringArray& AppConfig::GetSupportedEventPackages() const
{
    return m_pConfigPrivate->m_objEventPackages;
}

PUBLIC
IMS_BOOL AppConfig::IsHeaderReadable(IN const AString& strHeader) const
{
    return m_pConfigPrivate->m_objReadHeaders.Contains(strHeader, IMS_FALSE);
}

PUBLIC
IMS_BOOL AppConfig::IsHeaderWritable(IN const AString& strHeader) const
{
    return m_pConfigPrivate->m_objWriteHeaders.Contains(strHeader, IMS_FALSE);
}

PUBLIC
AStringArray AppConfig::GetCapabilitySdps(IN IMS_SINT32 nSector, IN IMS_SINT32 nMessageType) const
{
    CapProperty* pProperty = m_pConfigPrivate->GetCapProperty(nSector, nMessageType);

    if (pProperty != IMS_NULL)
    {
        return pProperty->GetValues();
    }

    return AStringArray();
}

PUBLIC
const CoreServiceConfig* AppConfig::GetCoreServiceConfigEx(IN const AString& strServiceId) const
{
    return m_pConfigPrivate->GetCoreServiceConfig(strServiceId);
}

PUBLIC
const ImsList<CoreServiceConfig*>& AppConfig::GetCoreServiceConfigs() const
{
    return m_pConfigPrivate->m_objCoreServiceConfigs;
}
