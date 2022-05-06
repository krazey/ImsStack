/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091026  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "TextParser.h"
#include "IConfigBuffer.h"
#include "ConfigLoader.h"
#include "StaticConfig.h"
#include "private/MediaConfig.h"

__IMS_TRACE_TAG_CONF__;

class MediaProfileParameter
{
public:
    inline MediaProfileParameter() :
            nType(IMediaConfig::MEDIA_INVALID)
    {
    }
    inline MediaProfileParameter(IN IMS_SINT32 nType_) :
            nType(nType_)
    {
    }
    inline ~MediaProfileParameter() {}

public:
    inline IMS_BOOL AddValue(IN const AString& strValue)
    {
        objValues.AddElement(strValue);
        return IMS_TRUE;
    }

    inline IMS_SINT32 GetType() const { return nType; }
    inline const AStringArray& GetValues() const { return objValues; }

private:
    IMS_SINT32 nType;
    AStringArray objValues;
};

class MediaProfile
{
public:
    inline MediaProfile(IN const AString& strName_, IN const AString& strConfName_) :
            strName(strName_),
            strConfName(strConfName_)
    {
    }
    inline ~MediaProfile() {}

public:
    IMS_BOOL AddValue(IN IMS_SINT32 nType, IN const AString& strValue);
    const AString& GetName() const;
    MediaProfileParameter* GetParameter(IN IMS_SINT32 nType) const;

private:
    AString strName;
    AString strConfName;

    IMSList<MediaProfileParameter> objParameters;
};

PUBLIC
IMS_BOOL MediaProfile::AddValue(IN IMS_SINT32 nType, IN const AString& strValue)
{
    MediaProfileParameter* pParameter = GetParameter(nType);

    if (pParameter == IMS_NULL)
    {
        // Add a new parameter
        MediaProfileParameter objParameter(nType);

        if (!objParameter.AddValue(strValue))
        {
            return IMS_FALSE;
        }

        return objParameters.Append(objParameter);
    }
    else
    {
        return pParameter->AddValue(strValue);
    }
}

PUBLIC
const AString& MediaProfile::GetName() const
{
    return strName;
}

PUBLIC
MediaProfileParameter* MediaProfile::GetParameter(IN IMS_SINT32 nType) const
{
    if (objParameters.IsEmpty())
    {
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < objParameters.GetSize(); ++i)
    {
        const MediaProfileParameter& objParameter = objParameters.GetAt(i);

        if (nType == objParameter.GetType())
        {
            return const_cast<MediaProfileParameter*>(&objParameter);
        }
    }

    // Not found
    return IMS_NULL;
}

class MediaConfigPrivate
{
public:
    inline MediaConfigPrivate() {}
    inline ~MediaConfigPrivate() { Clear(); }

public:
    void Clear();
    IMS_BOOL CreateProfile(IN const AString& strName, IN const AString& strConfName);
    void DestroyProfile(IN const AString& strName);
    MediaProfile* FindProfile(IN const AString& strName) const;
    MediaProfile* GetCapabilityProfile() const;

public:
    static const IMS_CHAR MPROF_CAPABILITIES[];
    static const IMS_CHAR KEY_STREAM_AUDIO[];
    static const IMS_CHAR KEY_STREAM_VIDEO[];
    static const IMS_CHAR KEY_FRAMED[];
    static const IMS_CHAR KEY_BASIC_UNRELIABLE[];
    static const IMS_CHAR KEY_BASIC_RELIABLE[];

private:
    IMSList<MediaProfile*> objProfiles;
};

PUBLIC GLOBAL const IMS_CHAR MediaConfigPrivate::MPROF_CAPABILITIES[] = "capabilities";
PUBLIC GLOBAL const IMS_CHAR MediaConfigPrivate::KEY_STREAM_AUDIO[] = "stream_audio";
PUBLIC GLOBAL const IMS_CHAR MediaConfigPrivate::KEY_STREAM_VIDEO[] = "stream_video";
PUBLIC GLOBAL const IMS_CHAR MediaConfigPrivate::KEY_FRAMED[] = "framed";
PUBLIC GLOBAL const IMS_CHAR MediaConfigPrivate::KEY_BASIC_UNRELIABLE[] = "basic_unreliable";
PUBLIC GLOBAL const IMS_CHAR MediaConfigPrivate::KEY_BASIC_RELIABLE[] = "basic_reliable";

PUBLIC
void MediaConfigPrivate::Clear()
{
    for (IMS_UINT32 i = 0; i < objProfiles.GetSize(); ++i)
    {
        MediaProfile* pProfile = objProfiles.GetAt(i);

        if (pProfile != IMS_NULL)
        {
            delete pProfile;
        }
    }

    objProfiles.Clear();
}

PUBLIC
IMS_BOOL MediaConfigPrivate::CreateProfile(IN const AString& strName, IN const AString& strConfName)
{
    MediaProfile* pProfile = FindProfile(strName);

    if (pProfile != IMS_NULL)
    {
        // Already exists
        return IMS_TRUE;
    }

    pProfile = new MediaProfile(strName, strConfName);

    if (pProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!objProfiles.Append(pProfile))
    {
        delete pProfile;

        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
void MediaConfigPrivate::DestroyProfile(IN const AString& strName)
{
    if (objProfiles.IsEmpty())
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objProfiles.GetSize(); ++i)
    {
        MediaProfile* pProfile = objProfiles.GetAt(i);

        if (strName.EqualsIgnoreCase(pProfile->GetName()))
        {
            objProfiles.RemoveAt(i);
            delete pProfile;
            return;
        }
    }
}

PUBLIC
MediaProfile* MediaConfigPrivate::FindProfile(IN const AString& strName) const
{
    if (objProfiles.IsEmpty())
    {
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < objProfiles.GetSize(); ++i)
    {
        MediaProfile* pProfile = objProfiles.GetAt(i);

        if (strName.EqualsIgnoreCase(pProfile->GetName()))
        {
            return pProfile;
        }
    }

    // Not found
    return IMS_NULL;
}

PUBLIC
MediaProfile* MediaConfigPrivate::GetCapabilityProfile() const
{
    return FindProfile(MediaConfigPrivate::MPROF_CAPABILITIES);
}

PUBLIC GLOBAL const IMS_CHAR MediaConfig::SECTION_PROFILES[] = "profiles";
PUBLIC GLOBAL const IMS_CHAR MediaConfig::KEY_IDS[] = "ids";

PUBLIC
MediaConfig::MediaConfig(IN IMS_SINT32 nSlotId) :
        ConfigBase(nSlotId),
        pMediaConfigP(new MediaConfigPrivate())
{
}

PUBLIC VIRTUAL MediaConfig::~MediaConfig()
{
    if (pMediaConfigP != IMS_NULL)
    {
        delete pMediaConfigP;
    }
}

PUBLIC VIRTUAL const AStringArray& MediaConfig::GetMediaCapabilities(IN IMS_SINT32 nMediaType) const
{
    MediaProfile* pProfile = pMediaConfigP->GetCapabilityProfile();
    MediaProfileParameter* pParameter = pProfile->GetParameter(nMediaType);

    if (pParameter == IMS_NULL)
    {
        return AStringArray::ConstNull();
    }

    return pParameter->GetValues();
}

PUBLIC VIRTUAL const AStringArray& MediaConfig::GetMediaProfile(
        IN const AString& strName, IN IMS_SINT32 nMediaType) const
{
    MediaProfile* pProfile = pMediaConfigP->FindProfile(strName);

    if (pProfile == IMS_NULL)
    {
        return AStringArray::ConstNull();
    }

    MediaProfileParameter* pParameter = pProfile->GetParameter(nMediaType);

    if (pParameter == IMS_NULL)
    {
        return AStringArray::ConstNull();
    }

    return pParameter->GetValues();
}

PUBLIC
void MediaConfig::Refresh()
{
    pMediaConfigP->Clear();

    ReadFrom();
}

PUBLIC
IMS_BOOL MediaConfig::CreateMediaProfile(IN const AString& strName)
{
    MediaProfile* pProfile = pMediaConfigP->FindProfile(strName);

    if (pProfile != IMS_NULL)
    {
        // The profile is already loaded by other service
        IMS_TRACE_D("Media profile(%s) is already loaded", strName.GetStr(), 0, 0);
        return IMS_TRUE;
    }

    IConfigBuffer* piBuffer = IMS_NULL;
    const AString strContent = StaticConfig::GetMediaConfig();

    if (strContent.GetLength() != 0)
    {
        piBuffer = GetConfigBufferFromContent(strContent);
    }
    else
    {
        IMS_TRACE_D("No matched static configurtion: fallback to legacy is failed.", 0, 0, 0);
    }

    if (piBuffer == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!piBuffer->CaptureSection(strName.GetStr()))
    {
        IMS_TRACE_E(0, "Media profile(%s) does not exist", strName.GetStr(), 0, 0);
        piBuffer->Destroy();
        return IMS_FALSE;
    }

    // Read the value: "section_name"
    const AString& strProfConfName = piBuffer->ReadValue(strName.GetStr());

    if (!pMediaConfigP->CreateProfile(strName, strProfConfName))
    {
        IMS_TRACE_E(0, "Creating a media profile(%s) failed", strName.GetStr(), 0, 0);
        piBuffer->Destroy();
        return IMS_FALSE;
    }

    if (!ReadMediaProfile(strName, strProfConfName))
    {
        IMS_TRACE_E(0, "Reading a media profile(%s) failed", strName.GetStr(), 0, 0);

        pMediaConfigP->DestroyProfile(strName);
        piBuffer->Destroy();
        return IMS_FALSE;
    }

    // Release the current section
    piBuffer->ReleaseSection();

    piBuffer->Destroy();

    return IMS_TRUE;
}

PUBLIC
void MediaConfig::DestroyMediaProfile(IN const AString& strName)
{
    pMediaConfigP->DestroyProfile(strName);
}

PROTECTED VIRTUAL IMS_BOOL MediaConfig::ReadFrom()
{
    IConfigBuffer* piBuffer = IMS_NULL;
    const AString strContent = StaticConfig::GetMediaConfig();

    if (strContent.GetLength() != 0)
    {
        piBuffer = GetConfigBufferFromContent(strContent);
    }
    else
    {
        IMS_TRACE_D("No matched static configurtion: fallback to legacy is failed.", 0, 0, 0);
    }

    if (piBuffer == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Set the currect section: "profiles"
    if (!piBuffer->CaptureSection(SECTION_PROFILES))
    {
        piBuffer->Destroy();
        return IMS_FALSE;
    }

    // Read the value: "ids"
    const AString& strIds = piBuffer->ReadValue(KEY_IDS);
    IMSList<AString> objIds = strIds.Split(TextParser::CHAR_COMMA);

    if (objIds.IsEmpty())
    {
        // Release the current section
        piBuffer->ReleaseSection();

        // Destroy the config buffer
        piBuffer->Destroy();
        return IMS_TRUE;
    }

    for (IMS_UINT32 i = 0; i < objIds.GetSize(); ++i)
    {
        const AString& strMProfName = objIds.GetAt(i);

        // Read the value: "section_name"
        const AString& strConfName = piBuffer->ReadValue(strMProfName.GetStr());

        if (!pMediaConfigP->CreateProfile(strMProfName, strConfName))
        {
            piBuffer->Destroy();
            return IMS_FALSE;
        }

        if (!ReadMediaProfile(strMProfName, strConfName))
        {
            pMediaConfigP->DestroyProfile(strMProfName);

            piBuffer->Destroy();
            return IMS_FALSE;
        }
    }

    // Release the current section
    piBuffer->ReleaseSection();

    // Destroy the config buffer
    piBuffer->Destroy();

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL MediaConfig::WriteTo()
{
    return IMS_FALSE;
}

PROTECTED
IMS_BOOL MediaConfig::ReadMediaProfile(
        IN const AString& strMProfName, IN const AString& strConfName)
{
    IConfigBuffer* piBuffer = IMS_NULL;
    const AString strContent = StaticConfig::GetConfig(strConfName);

    if (strContent.GetLength() != 0)
    {
        piBuffer = GetConfigBufferFromContent(strContent);
    }
    else
    {
        IMS_TRACE_D("No matched static configurtion: fallback to legacy is failed.", 0, 0, 0);
    }

    if (piBuffer == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Read the specific section: strMProfName
    MediaProfile* pProfile = pMediaConfigP->FindProfile(strMProfName);

    if (pProfile == IMS_NULL)
    {
        piBuffer->Destroy();
        return IMS_FALSE;
    }

    // Set the currect section
    if (!piBuffer->CaptureSection(strMProfName.GetStr()))
    {
        piBuffer->Destroy();
        return IMS_FALSE;
    }

    // Read the value:
    // "stream_audio" / "stream_video" / "framed" / "basic_unreliable" / "basic_reliable"

    // "stream_audio"
    IMS_SINT32 nKeyCount = piBuffer->ReadKeyCount(MediaConfigPrivate::KEY_STREAM_AUDIO);

    if (nKeyCount > 0)
    {
        for (IMS_SINT32 i = 0; i < nKeyCount; ++i)
        {
            const AString& strTmpVal = piBuffer->ReadValue(MediaConfigPrivate::KEY_STREAM_AUDIO, i);

            if (!pProfile->AddValue(IMediaConfig::STREAM_AUDIO, strTmpVal))
            {
                piBuffer->Destroy();
                return IMS_FALSE;
            }
        }
    }

    // "stream_video"
    nKeyCount = piBuffer->ReadKeyCount(MediaConfigPrivate::KEY_STREAM_VIDEO);

    if (nKeyCount > 0)
    {
        for (IMS_SINT32 i = 0; i < nKeyCount; ++i)
        {
            const AString& strTmpVal = piBuffer->ReadValue(MediaConfigPrivate::KEY_STREAM_VIDEO, i);

            if (!pProfile->AddValue(IMediaConfig::STREAM_VIDEO, strTmpVal))
            {
                piBuffer->Destroy();
                return IMS_FALSE;
            }
        }
    }

    // "framed"
    nKeyCount = piBuffer->ReadKeyCount(MediaConfigPrivate::KEY_FRAMED);

    if (nKeyCount > 0)
    {
        for (IMS_SINT32 i = 0; i < nKeyCount; ++i)
        {
            const AString& strTmpVal = piBuffer->ReadValue(MediaConfigPrivate::KEY_FRAMED, i);

            if (!pProfile->AddValue(IMediaConfig::FRAMED, strTmpVal))
            {
                piBuffer->Destroy();
                return IMS_FALSE;
            }
        }
    }

    // "basic_unreliable"
    nKeyCount = piBuffer->ReadKeyCount(MediaConfigPrivate::KEY_BASIC_UNRELIABLE);

    if (nKeyCount > 0)
    {
        for (IMS_SINT32 i = 0; i < nKeyCount; ++i)
        {
            const AString& strTmpVal =
                    piBuffer->ReadValue(MediaConfigPrivate::KEY_BASIC_UNRELIABLE, i);

            if (!pProfile->AddValue(IMediaConfig::BASIC_UNRELIABLE, strTmpVal))
            {
                piBuffer->Destroy();
                return IMS_FALSE;
            }
        }
    }

    // "basic_reliable"
    nKeyCount = piBuffer->ReadKeyCount(MediaConfigPrivate::KEY_BASIC_RELIABLE);

    if (nKeyCount > 0)
    {
        for (IMS_SINT32 i = 0; i < nKeyCount; ++i)
        {
            const AString& strTmpVal =
                    piBuffer->ReadValue(MediaConfigPrivate::KEY_BASIC_RELIABLE, i);

            if (!pProfile->AddValue(IMediaConfig::BASIC_RELIABLE, strTmpVal))
            {
                piBuffer->Destroy();
                return IMS_FALSE;
            }
        }
    }

    // Release the current section
    piBuffer->ReleaseSection();

    // Destroy the config buffer
    piBuffer->Destroy();

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaConfig::WriteMediaProfile(
        IN const AString& strMProfName, IN const AString& strConfName)
{
    (void)strMProfName;
    (void)strConfName;

    return IMS_FALSE;
}
