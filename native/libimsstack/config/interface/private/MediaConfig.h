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
#ifndef MEDIA_CONFIG_H_
#define MEDIA_CONFIG_H_

#include "IMediaConfig.h"
#include "private/ConfigBase.h"

class MediaConfigPrivate;

class MediaConfig : public ConfigBase, public IMediaConfig
{
public:
    explicit MediaConfig(IN IMS_SINT32 nSlotId);
    virtual ~MediaConfig();

    MediaConfig(IN const MediaConfig&) = delete;
    MediaConfig& operator=(IN const MediaConfig&) = delete;

public:
    // IMediaConfig interface
    const AStringArray& GetMediaCapabilities(IN IMS_SINT32 nMediaType) const override;
    const AStringArray& GetMediaProfile(
            IN const AString& strName, IN IMS_SINT32 nMediaType) const override;

    // ConfigBase class
    void Refresh() override;

    IMS_BOOL CreateMediaProfile(IN const AString& strName);
    void DestroyMediaProfile(IN const AString& strName);

protected:
    // ConfigBase class
    IMS_BOOL ReadFrom() override;
    inline IMS_BOOL WriteTo() override { return IMS_FALSE; }

    IMS_BOOL ReadMediaProfile(IN const AString& strMProfName, IN const AString& strConfName);
    inline IMS_BOOL WriteMediaProfile(
            IN const AString& /*strMProfName*/, IN const AString& /*strConfName*/)
    {
        return IMS_FALSE;
    }

private:
    static const IMS_CHAR SECTION_PROFILES[];
    static const IMS_CHAR KEY_IDS[];

    MediaConfigPrivate* m_pConfigPrivate;
};

#endif
