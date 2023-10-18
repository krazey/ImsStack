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
#ifndef CONFIG_BUFFER_H_
#define CONFIG_BUFFER_H_

#include "IConfigBuffer.h"

class ConfigBuffer : public IConfigBuffer
{
public:
    inline ConfigBuffer(IN const AString& strLocator, IN const AString& strName) :
            m_strLocator(strLocator),
            m_strName(strName)
    {
    }
    inline virtual ~ConfigBuffer() {}

    ConfigBuffer(IN const ConfigBuffer&) = delete;
    ConfigBuffer& operator=(IN const ConfigBuffer&) = delete;

public:
    virtual IMS_BOOL Create(IN IMS_SINT32 nId) = 0;

protected:
    inline const AString& GetLocator() const { return m_strLocator; }
    inline const AString& GetName() const { return m_strName; }

private:
    AString m_strLocator;
    AString m_strName;
};

#endif
