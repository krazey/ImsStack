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
#include "TextParser.h"

#include "ConfigLoader.h"
#include "IConfigBuffer.h"
#include "StaticConfig.h"
#include "private/MediaConfig.h"

__IMS_TRACE_TAG_CONF__;

class MediaProfileParameter
{
public:
    inline MediaProfileParameter() :
            m_nType(IMediaConfig::MEDIA_INVALID)
    {
    }
    inline explicit MediaProfileParameter(IN IMS_SINT32 nType) :
            m_nType(nType)
    {
    }
    inline MediaProfileParameter(IN const MediaProfileParameter& other) :
            m_nType(other.m_nType),
            m_objValues(other.m_objValues)
    {
    }
    ~MediaProfileParameter() = default;

    inline MediaProfileParameter& operator=(IN const MediaProfileParameter& other)
    {
        if (this != &other)
        {
            m_nType = other.m_nType;
            m_objValues = other.m_objValues;
        }

        return (*this);
    }

public:
    inline void AddValue(IN const AString& strValue) { m_objValues.AddElement(strValue); }
    inline IMS_SINT32 GetType() const { return m_nType; }
    inline const AStringArray& GetValues() const { return m_objValues; }

private:
    IMS_SINT32 m_nType;
    AStringArray m_objValues;
};

class MediaProfile
{
public:
    inline MediaProfile(IN const AString& strName, IN const AString& strConfName) :
            m_strName(strName),
            m_strConfName(strConfName)
    {
    }
    ~MediaProfile() = default;

public:
    IMS_BOOL AddValue(IN IMS_SINT32 nType, IN const AString& strValue);
    inline const AString& GetName() const { return m_strName; }
    MediaProfileParameter* GetParameter(IN IMS_SINT32 nType) const;

private:
    AString m_strName;
    AString m_strConfName;

    ImsList<MediaProfileParameter> m_objParameters;
};

PUBLIC
IMS_BOOL MediaProfile::AddValue(IN IMS_SINT32 nType, IN const AString& strValue)
{
    MediaProfileParameter* pParameter = GetParameter(nType);

    if (pParameter == IMS_NULL)
    {
        // Add a new parameter
        MediaProfileParameter objParameter(nType);
        objParameter.AddValue(strValue);
        return m_objParameters.Append(objParameter);
    }

    pParameter->AddValue(strValue);
    return IMS_TRUE;
}

PUBLIC
MediaProfileParameter* MediaProfile::GetParameter(IN IMS_SINT32 nType) const
{
    if (m_objParameters.IsEmpty())
    {
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < m_objParameters.GetSize(); ++i)
    {
        const MediaProfileParameter& objParameter = m_objParameters.GetAt(i);

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
    MediaConfigPrivate() = default;
    inline ~MediaConfigPrivate() { Clear(); }

    MediaConfigPrivate(IN const MediaConfigPrivate&) = delete;
    MediaConfigPrivate& operator=(IN const MediaConfigPrivate&) = delete;

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
    ImsList<MediaProfile*> m_objProfiles;
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
    for (IMS_UINT32 i = 0; i < m_objProfiles.GetSize(); ++i)
    {
        MediaProfile* pProfile = m_objProfiles.GetAt(i);

        if (pProfile != IMS_NULL)
        {
            delete pProfile;
        }
    }

    m_objProfiles.Clear();
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

    if (!m_objProfiles.Append(pProfile))
    {
        delete pProfile;

        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
void MediaConfigPrivate::DestroyProfile(IN const AString& strName)
{
    if (m_objProfiles.IsEmpty())
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objProfiles.GetSize(); ++i)
    {
        MediaProfile* pProfile = m_objProfiles.GetAt(i);

        if (strName.EqualsIgnoreCase(pProfile->GetName()))
        {
            m_objProfiles.RemoveAt(i);
            delete pProfile;
            return;
        }
    }
}

PUBLIC
MediaProfile* MediaConfigPrivate::FindProfile(IN const AString& strName) const
{
    if (m_objProfiles.IsEmpty())
    {
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < m_objProfiles.GetSize(); ++i)
    {
        MediaProfile* pProfile = m_objProfiles.GetAt(i);

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
        m_pConfigPrivate(new MediaConfigPrivate())
{
}

PUBLIC VIRTUAL MediaConfig::~MediaConfig()
{
    if (m_pConfigPrivate != IMS_NULL)
    {
        delete m_pConfigPrivate;
    }
}

PUBLIC VIRTUAL const AStringArray& MediaConfig::GetMediaCapabilities(IN IMS_SINT32 nMediaType) const
{
    const MediaProfile* pProfile = m_pConfigPrivate->GetCapabilityProfile();
    const MediaProfileParameter* pParameter = pProfile->GetParameter(nMediaType);

    if (pParameter == IMS_NULL)
    {
        return AStringArray::ConstNull();
    }

    return pParameter->GetValues();
}

PUBLIC VIRTUAL const AStringArray& MediaConfig::GetMediaProfile(
        IN const AString& strName, IN IMS_SINT32 nMediaType) const
{
    const MediaProfile* pProfile = m_pConfigPrivate->FindProfile(strName);

    if (pProfile == IMS_NULL)
    {
        return AStringArray::ConstNull();
    }

    const MediaProfileParameter* pParameter = pProfile->GetParameter(nMediaType);

    if (pParameter == IMS_NULL)
    {
        return AStringArray::ConstNull();
    }

    return pParameter->GetValues();
}

PUBLIC
void MediaConfig::Refresh()
{
    m_pConfigPrivate->Clear();

    ReadFrom();
}

PUBLIC
IMS_BOOL MediaConfig::CreateMediaProfile(IN const AString& strName)
{
    const MediaProfile* pProfile = m_pConfigPrivate->FindProfile(strName);

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

    if (!m_pConfigPrivate->CreateProfile(strName, strProfConfName))
    {
        IMS_TRACE_E(0, "Creating a media profile(%s) failed", strName.GetStr(), 0, 0);
        piBuffer->Destroy();
        return IMS_FALSE;
    }

    if (!ReadMediaProfile(strName, strProfConfName))
    {
        IMS_TRACE_E(0, "Reading a media profile(%s) failed", strName.GetStr(), 0, 0);

        m_pConfigPrivate->DestroyProfile(strName);
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
    m_pConfigPrivate->DestroyProfile(strName);
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
    ImsList<AString> objIds = strIds.Split(TextParser::CHAR_COMMA);

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

        if (!m_pConfigPrivate->CreateProfile(strMProfName, strConfName))
        {
            piBuffer->Destroy();
            return IMS_FALSE;
        }

        if (!ReadMediaProfile(strMProfName, strConfName))
        {
            m_pConfigPrivate->DestroyProfile(strMProfName);

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
    MediaProfile* pProfile = m_pConfigPrivate->FindProfile(strMProfName);

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
