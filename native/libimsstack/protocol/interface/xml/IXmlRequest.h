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
#ifndef INTERFACE_XML_REQUEST_H_
#define INTERFACE_XML_REQUEST_H_

#include "AString.h"

class IXmlRequest
{
protected:
    virtual ~IXmlRequest() = default;

public:
    /**
     * @brief Sets a raw XML string.
     *
     * @param pszRawXml a raw XML string
     */
    virtual void SetRawXml(IN const IMS_CHAR* pszRawXml) = 0;

    /**
     * @brief Sets a raw XML string.
     *
     * @param strRawXml a raw XML string
     */
    virtual void SetRawXml(IN const AString& strRawXml) = 0;

    /**
     * @brief Gets a raw XML string.
     *
     * @return A raw XML string.
     */
    virtual const AString& GetRawXml() const = 0;
};

#endif
