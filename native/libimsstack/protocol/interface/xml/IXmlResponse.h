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
#ifndef INTERFACE_XML_RESPONSE_H_
#define INTERFACE_XML_RESPONSE_H_

#include "ImsTypeDef.h"

class IDocument;

class IXmlResponse
{
protected:
    virtual ~IXmlResponse() = default;

public:
    /**
     * @brief Returns a parsed XML document.
     *
     * @return Pointer to a new IDocument.
     */
    virtual IDocument* GetDocument() const = 0;

    /**
     * @brief Returns a response code.
     *
     * @return The response code of this response.\n
     *         #RESPONSE_CODE_FAILURE\n
     *         #RESPONSE_CODE_SUCCESS\n
     */
    virtual IMS_SINT32 GetResponseCode() const = 0;

public:
    enum
    {
        RESPONSE_CODE_FAILURE = 0,
        RESPONSE_CODE_SUCCESS,
    };
};

#endif
