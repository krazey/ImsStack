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
#ifndef REG_PROPERTY_H_
#define REG_PROPERTY_H_

#include "private/ImsProperty.h"

class RegProperty : public ImsProperty
{
public:
    explicit RegProperty(IN const AString& strServiceId);
    RegProperty(IN const RegProperty& other);
    inline virtual ~RegProperty() {}

public:
    RegProperty& operator=(IN const RegProperty& other);

public:
    inline IMS_BOOL AddValue(IN const AString& strValue) { return m_objHeaders.Append(strValue); }
    inline const IMSList<AString>& GetValues() const { return m_objHeaders; }

protected:
    IMS_BOOL Equals(IN const ImsProperty& objOther) const override;

private:
    AString m_strServiceId;
    // "Name: Value"
    IMSList<AString> m_objHeaders;
};

#endif
