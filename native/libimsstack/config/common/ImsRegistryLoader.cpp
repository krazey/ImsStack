/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091106  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ConfigLoader.h"
#include "StaticConfig.h"
#include "private/ConfigBase.h"
#include "private/ImsProperty.h"
#include "private/ImsRegistryLoader.h"

__IMS_TRACE_TAG_CONF__;

class RegistryLoader
{
private:
    RegistryLoader();

public:
    static IMS_BOOL AddStreamProperty(
            IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry);
    static IMS_BOOL AddFramedProperty(
            IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry);
    static IMS_BOOL AddBasicProperty(
            IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry);
    static IMS_BOOL AddEventProperty(
            IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry);
    static IMS_BOOL AddCoreServiceProperty(
            IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry);
    static IMS_BOOL AddQosProperty(IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry);
    static IMS_BOOL AddRegProperty(IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry);
    static IMS_BOOL AddWriteProperty(
            IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry);
    static IMS_BOOL AddReadProperty(IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry);
    static IMS_BOOL AddCapProperty(IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry);
    static IMS_BOOL AddMprofProperty(
            IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry);
    static IMS_BOOL AddConnectionProperty(
            IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry);
    static IMS_BOOL ValidateUniqueness(IN IMS_SINT32 nPropertyCount[ImsProperty::PKEY_MAX]);

public:
    // IMSRegistry is used for section name & parameter name
    // It has the application identity;
    // So, if this field is not matched with the specified application identity,
    // we can assume that the registry is corrupted.
    static const IMS_CHAR SECTION_IMS_REGISTRY[];

    static const IMS_CHAR PARAM_MEDIA_TYPES[];
    static const IMS_CHAR PARAM_MAX_SIZE[];
    static const IMS_CHAR PARAM_PACKAGE_NAMES[];
    static const IMS_CHAR PARAM_SERVICE_ID[];
    static const IMS_CHAR PARAM_IARI[];
    static const IMS_CHAR PARAM_ICSI[];
    static const IMS_CHAR PARAM_FEATURE_TAG[];
    static const IMS_CHAR PARAM_SEND_FLOW_SPEC[];
    static const IMS_CHAR PARAM_RECEIVE_FLOW_SPEC[];
    static const IMS_CHAR PARAM_HEADER[];
    static const IMS_CHAR PARAM_HEADER_NAMES[];
    static const IMS_CHAR PARAM_SECTOR_ID[];
    static const IMS_CHAR PARAM_MESSAGE_TYPE[];
    static const IMS_CHAR PARAM_SDP[];
    static const IMS_CHAR PARAM_PROFILE[];
};

PUBLIC GLOBAL const IMS_CHAR RegistryLoader::SECTION_IMS_REGISTRY[] = "IMSRegistry";
PUBLIC GLOBAL const IMS_CHAR RegistryLoader::PARAM_MEDIA_TYPES[] = "media_types";
PUBLIC GLOBAL const IMS_CHAR RegistryLoader::PARAM_MAX_SIZE[] = "max_size";
PUBLIC GLOBAL const IMS_CHAR RegistryLoader::PARAM_PACKAGE_NAMES[] = "package_names";
PUBLIC GLOBAL const IMS_CHAR RegistryLoader::PARAM_SERVICE_ID[] = "service_id";
PUBLIC GLOBAL const IMS_CHAR RegistryLoader::PARAM_IARI[] = "iari";
PUBLIC GLOBAL const IMS_CHAR RegistryLoader::PARAM_ICSI[] = "icsi";
PUBLIC GLOBAL const IMS_CHAR RegistryLoader::PARAM_FEATURE_TAG[] = "feature_tag";
PUBLIC GLOBAL const IMS_CHAR RegistryLoader::PARAM_SEND_FLOW_SPEC[] = "send_flow_spec";
PUBLIC GLOBAL const IMS_CHAR RegistryLoader::PARAM_RECEIVE_FLOW_SPEC[] = "receive_flow_spec";
PUBLIC GLOBAL const IMS_CHAR RegistryLoader::PARAM_HEADER[] = "header";
PUBLIC GLOBAL const IMS_CHAR RegistryLoader::PARAM_HEADER_NAMES[] = "header_names";
PUBLIC GLOBAL const IMS_CHAR RegistryLoader::PARAM_SECTOR_ID[] = "sector_id";
PUBLIC GLOBAL const IMS_CHAR RegistryLoader::PARAM_MESSAGE_TYPE[] = "message_type";
PUBLIC GLOBAL const IMS_CHAR RegistryLoader::PARAM_SDP[] = "sdp";
PUBLIC GLOBAL const IMS_CHAR RegistryLoader::PARAM_PROFILE[] = "profile";

PUBLIC GLOBAL IMS_BOOL RegistryLoader::AddStreamProperty(
        IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry)
{
    AStringArray objProperty;

    // "Stream" property
    objProperty.AddElement(AString(ImsProperty::PKEY_STRING[ImsProperty::PKEY_STREAM]));
    // "media_types" parameter
    objProperty.AddElement(piBuffer->ReadValue(PARAM_MEDIA_TYPES));

    objRegistry.Add(objProperty);

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL RegistryLoader::AddFramedProperty(
        IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry)
{
    AStringArray objProperty;

    // "Framed" property
    objProperty.AddElement(AString(ImsProperty::PKEY_STRING[ImsProperty::PKEY_FRAMED]));
    // "media_types" parameter
    objProperty.AddElement(piBuffer->ReadValue(PARAM_MEDIA_TYPES));

    // "max-size" parameter if present
    const AString& strMaxSize = piBuffer->ReadValue(PARAM_MAX_SIZE);

    if (strMaxSize.IsNULL())
    {
        objProperty.AddElement(AString::ConstEmpty());
    }
    else
    {
        objProperty.AddElement(strMaxSize);
    }

    objRegistry.Add(objProperty);

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL RegistryLoader::AddBasicProperty(
        IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry)
{
    AStringArray objProperty;

    // "Basic" property
    objProperty.AddElement(AString(ImsProperty::PKEY_STRING[ImsProperty::PKEY_BASIC]));
    // "media_types" parameter
    objProperty.AddElement(piBuffer->ReadValue(PARAM_MEDIA_TYPES));

    objRegistry.Add(objProperty);

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL RegistryLoader::AddEventProperty(
        IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry)
{
    AStringArray objProperty;

    // "Event" property
    objProperty.AddElement(AString(ImsProperty::PKEY_STRING[ImsProperty::PKEY_EVENT]));
    // "package_names" parameter
    objProperty.AddElement(piBuffer->ReadValue(PARAM_PACKAGE_NAMES));

    objRegistry.Add(objProperty);

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL RegistryLoader::AddCoreServiceProperty(
        IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry)
{
    AStringArray objProperty;

    // "CoreService" property
    objProperty.AddElement(AString(ImsProperty::PKEY_STRING[ImsProperty::PKEY_CORE_SERVICE]));
    // "service_id" parameter
    objProperty.AddElement(piBuffer->ReadValue(PARAM_SERVICE_ID));
    // "iari" parameter
    objProperty.AddElement(piBuffer->ReadValue(PARAM_IARI));
    // "icsi" parameter
    objProperty.AddElement(piBuffer->ReadValue(PARAM_ICSI));
    // "feature_tag" parameter
    objProperty.AddElement(piBuffer->ReadValue(PARAM_FEATURE_TAG));

    objRegistry.Add(objProperty);

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL RegistryLoader::AddQosProperty(
        IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry)
{
    AStringArray objProperty;

    // "Qos" property
    objProperty.AddElement(AString(ImsProperty::PKEY_STRING[ImsProperty::PKEY_QOS]));
    // "service_id" parameter
    objProperty.AddElement(piBuffer->ReadValue(PARAM_SERVICE_ID));
    // "media_types" parameter
    objProperty.AddElement(piBuffer->ReadValue(PARAM_MEDIA_TYPES));
    // "send_flow_spec" parameter
    objProperty.AddElement(piBuffer->ReadValue(PARAM_SEND_FLOW_SPEC));
    // "receive_flow_spec" parameter
    objProperty.AddElement(piBuffer->ReadValue(PARAM_RECEIVE_FLOW_SPEC));

    objRegistry.Add(objProperty);

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL RegistryLoader::AddRegProperty(
        IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry)
{
    AStringArray objProperty;

    // "Reg" property
    objProperty.AddElement(AString(ImsProperty::PKEY_STRING[ImsProperty::PKEY_REG]));
    // "service_id" parameter
    objProperty.AddElement(piBuffer->ReadValue(PARAM_SERVICE_ID));

    // "header_count"
    IMS_SINT32 nCount_Header = piBuffer->ReadKeyCount(PARAM_HEADER);

    for (IMS_SINT32 i = 0; i < nCount_Header; ++i)
    {
        // "header" parameter
        objProperty.AddElement(piBuffer->ReadValue(PARAM_HEADER, i));
    }

    objRegistry.Add(objProperty);

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL RegistryLoader::AddWriteProperty(
        IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry)
{
    AStringArray objProperty;

    // "Write" property
    objProperty.AddElement(AString(ImsProperty::PKEY_STRING[ImsProperty::PKEY_WRITE]));
    // "header_names" parameter
    objProperty.AddElement(piBuffer->ReadValue(PARAM_HEADER_NAMES));

    objRegistry.Add(objProperty);

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL RegistryLoader::AddReadProperty(
        IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry)
{
    AStringArray objProperty;

    // "Read" property
    objProperty.AddElement(AString(ImsProperty::PKEY_STRING[ImsProperty::PKEY_READ]));
    // "header_names" parameter
    objProperty.AddElement(piBuffer->ReadValue(PARAM_HEADER_NAMES));

    objRegistry.Add(objProperty);

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL RegistryLoader::AddCapProperty(
        IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry)
{
    AStringArray objProperty;

    // "Cap" property
    objProperty.AddElement(AString(ImsProperty::PKEY_STRING[ImsProperty::PKEY_CAP]));

    // "sector_id" parameter
    objProperty.AddElement(piBuffer->ReadValue(PARAM_SECTOR_ID));
    // "message_type" parameter
    objProperty.AddElement(piBuffer->ReadValue(PARAM_MESSAGE_TYPE));

    IMS_SINT32 nCount_SDP = piBuffer->ReadKeyCount(PARAM_SDP);

    for (IMS_SINT32 i = 0; i < nCount_SDP; ++i)
    {
        objProperty.AddElement(piBuffer->ReadValue(PARAM_SDP, i));
    }

    objRegistry.Add(objProperty);

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL RegistryLoader::AddMprofProperty(
        IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry)
{
    AStringArray objProperty;

    // "Mprof" property
    objProperty.AddElement(AString(ImsProperty::PKEY_STRING[ImsProperty::PKEY_MPROF]));

    // "service_id" parameter
    objProperty.AddElement(piBuffer->ReadValue(PARAM_SERVICE_ID));
    // "profile" parameter
    objProperty.AddElement(piBuffer->ReadValue(PARAM_PROFILE));

    objRegistry.Add(objProperty);

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL RegistryLoader::AddConnectionProperty(
        IN CONST IConfigBuffer* piBuffer, OUT ImsRegistry& objRegistry)
{
    AStringArray objProperty;

    // "Connection" property
    objProperty.AddElement(AString(ImsProperty::PKEY_STRING[ImsProperty::PKEY_CONNECTION]));

    // "service_id" parameter
    objProperty.AddElement(piBuffer->ReadValue(PARAM_SERVICE_ID));

    objRegistry.Add(objProperty);

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL RegistryLoader::ValidateUniqueness(
        IN IMS_SINT32 nPropertyCount[ImsProperty::PKEY_MAX])
{
    if (nPropertyCount[ImsProperty::PKEY_STREAM] > 1)
    {
        return IMS_FALSE;
    }

    if (nPropertyCount[ImsProperty::PKEY_FRAMED] > 1)
    {
        return IMS_FALSE;
    }

    if (nPropertyCount[ImsProperty::PKEY_BASIC] > 1)
    {
        return IMS_FALSE;
    }

    if (nPropertyCount[ImsProperty::PKEY_EVENT] > 1)
    {
        return IMS_FALSE;
    }

    if (nPropertyCount[ImsProperty::PKEY_CORE_SERVICE] < 1)
    {
        return IMS_FALSE;
    }

    // Skip the property: Qos

    // Skip the property: Reg

    if (nPropertyCount[ImsProperty::PKEY_WRITE] > 1)
    {
        return IMS_FALSE;
    }

    if (nPropertyCount[ImsProperty::PKEY_READ] > 1)
    {
        return IMS_FALSE;
    }

    // Skip the property: Cap, Mprof, Connection

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL ImsRegistryLoader::GetRegistry(
        IN const AString& strAppId, OUT ImsRegistry& objRegistry)
{
    IConfigBuffer* piBuffer = IMS_NULL;
    const AString strContent = StaticConfig::GetConfig(strAppId);

    if (strContent.GetLength() != 0)
    {
        piBuffer = ConfigLoader::GetConfig(strContent);
    }
    else
    {
        IMS_TRACE_D("No matched static configurtion: fallback to legacy is failed.", 0, 0, 0);
    }

    if (piBuffer == IMS_NULL)
    {
        return IMS_FALSE;
    }

    //// Uniqueness property -- starts
    IMS_SINT32 nPropertyCount[ImsProperty::PKEY_MAX] = {
            0,
    };

    if (!piBuffer->CaptureSection(ConfigBase::SECTION_UNIQUENESS))
    {
        piBuffer->Destroy();
        return IMS_FALSE;
    }

    for (IMS_SINT32 i = ImsProperty::PKEY_STREAM; i < ImsProperty::PKEY_MAX; ++i)
    {
        const AString& strTmpVal = piBuffer->ReadValue(ImsProperty::PKEY_STRING[i]);
        nPropertyCount[i] = strTmpVal.ToInt32();
    }

    piBuffer->ReleaseSection();
    //// Uniqueness property -- ends

    //// IMSRegistry property -- starts
    if (!piBuffer->CaptureSection(RegistryLoader::SECTION_IMS_REGISTRY))
    {
        piBuffer->Destroy();
        return IMS_FALSE;
    }

    const AString& strIMSRegistry = piBuffer->ReadValue(RegistryLoader::SECTION_IMS_REGISTRY);

    if (!strIMSRegistry.Equals(strAppId))
    {
        IMS_TRACE_E(0, "IMS Registry is not consistent (IMSRegistry: %s, AppId: %s)",
                strIMSRegistry.GetStr(), strAppId.GetStr(), 0);

        piBuffer->Destroy();
        return IMS_FALSE;
    }

    piBuffer->ReleaseSection();
    //// IMSRegistry property -- ends

    // Check if the configuration satisfies the uniqueness.
    if (!RegistryLoader::ValidateUniqueness(nPropertyCount))
    {
        IMS_TRACE_E(0, "Validation failed - Uniqueness", 0, 0, 0);

        piBuffer->Destroy();
        return IMS_FALSE;
    }

    //// Stream property -- starts
    if (nPropertyCount[ImsProperty::PKEY_STREAM] > 0)
    {
        if (!piBuffer->CaptureSection(ImsProperty::PKEY_STRING[ImsProperty::PKEY_STREAM]))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        if (!RegistryLoader::AddStreamProperty(piBuffer, objRegistry))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        piBuffer->ReleaseSection();
    }
    //// Stream property -- ends

    //// Framed property -- starts
    if (nPropertyCount[ImsProperty::PKEY_FRAMED] > 0)
    {
        if (!piBuffer->CaptureSection(ImsProperty::PKEY_STRING[ImsProperty::PKEY_FRAMED]))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        if (!RegistryLoader::AddFramedProperty(piBuffer, objRegistry))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        piBuffer->ReleaseSection();
    }
    //// Framed property -- ends

    //// Basic property -- starts
    if (nPropertyCount[ImsProperty::PKEY_BASIC] > 0)
    {
        if (!piBuffer->CaptureSection(ImsProperty::PKEY_STRING[ImsProperty::PKEY_BASIC]))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        if (!RegistryLoader::AddBasicProperty(piBuffer, objRegistry))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        piBuffer->ReleaseSection();
    }
    //// Basic property -- ends

    //// Event property -- starts
    if (nPropertyCount[ImsProperty::PKEY_EVENT] > 0)
    {
        if (!piBuffer->CaptureSection(ImsProperty::PKEY_STRING[ImsProperty::PKEY_EVENT]))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        if (!RegistryLoader::AddEventProperty(piBuffer, objRegistry))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        piBuffer->ReleaseSection();
    }
    //// Event property -- ends

    //// CoreService property -- starts
    for (IMS_SINT32 i = 0; i < nPropertyCount[ImsProperty::PKEY_CORE_SERVICE]; ++i)
    {
        if (!piBuffer->CaptureSection(ImsProperty::PKEY_STRING[ImsProperty::PKEY_CORE_SERVICE], i))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        if (!RegistryLoader::AddCoreServiceProperty(piBuffer, objRegistry))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        piBuffer->ReleaseSection();
    }
    //// CoreService property -- ends

    //// Qos property -- starts
    for (IMS_SINT32 i = 0; i < nPropertyCount[ImsProperty::PKEY_QOS]; ++i)
    {
        if (!piBuffer->CaptureSection(ImsProperty::PKEY_STRING[ImsProperty::PKEY_QOS], i))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        if (!RegistryLoader::AddQosProperty(piBuffer, objRegistry))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        piBuffer->ReleaseSection();
    }
    //// Qos property -- ends

    //// Reg property -- starts
    if (nPropertyCount[ImsProperty::PKEY_REG] > 0)
    {
        for (IMS_SINT32 i = 0; i < nPropertyCount[ImsProperty::PKEY_REG]; ++i)
        {
            if (!piBuffer->CaptureSection(ImsProperty::PKEY_STRING[ImsProperty::PKEY_REG], i))
            {
                piBuffer->Destroy();

                return IMS_FALSE;
            }

            if (!RegistryLoader::AddRegProperty(piBuffer, objRegistry))
            {
                piBuffer->Destroy();

                return IMS_FALSE;
            }

            piBuffer->ReleaseSection();
        }
    }
    //// Reg property -- ends

    //// Write property -- starts
    if (nPropertyCount[ImsProperty::PKEY_WRITE] > 0)
    {
        if (!piBuffer->CaptureSection(ImsProperty::PKEY_STRING[ImsProperty::PKEY_WRITE]))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        if (!RegistryLoader::AddWriteProperty(piBuffer, objRegistry))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        piBuffer->ReleaseSection();
    }
    //// Write property -- ends

    //// Read property -- starts
    if (nPropertyCount[ImsProperty::PKEY_READ] > 0)
    {
        if (!piBuffer->CaptureSection(ImsProperty::PKEY_STRING[ImsProperty::PKEY_READ]))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        if (!RegistryLoader::AddReadProperty(piBuffer, objRegistry))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        piBuffer->ReleaseSection();
    }
    //// Read property -- ends

    //// Cap property -- starts
    for (IMS_SINT32 i = 0; i < nPropertyCount[ImsProperty::PKEY_CAP]; ++i)
    {
        if (!piBuffer->CaptureSection(ImsProperty::PKEY_STRING[ImsProperty::PKEY_CAP], i))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        if (!RegistryLoader::AddCapProperty(piBuffer, objRegistry))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        piBuffer->ReleaseSection();
    }
    //// Cap property -- ends

    //// Mprof property -- starts
    for (IMS_SINT32 i = 0; i < nPropertyCount[ImsProperty::PKEY_MPROF]; ++i)
    {
        if (!piBuffer->CaptureSection(ImsProperty::PKEY_STRING[ImsProperty::PKEY_MPROF], i))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        if (!RegistryLoader::AddMprofProperty(piBuffer, objRegistry))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        piBuffer->ReleaseSection();
    }
    //// Mprof property -- ends

    //// Connection property -- starts
    for (IMS_SINT32 i = 0; i < nPropertyCount[ImsProperty::PKEY_CONNECTION]; ++i)
    {
        if (!piBuffer->CaptureSection(ImsProperty::PKEY_STRING[ImsProperty::PKEY_CONNECTION], i))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        if (!RegistryLoader::AddConnectionProperty(piBuffer, objRegistry))
        {
            piBuffer->Destroy();

            return IMS_FALSE;
        }

        piBuffer->ReleaseSection();
    }
    //// Connection property -- ends

    piBuffer->Destroy();

    return IMS_TRUE;
}
