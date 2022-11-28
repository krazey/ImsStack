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

#ifndef USSI_DATA_CREATOR_H_
#define USSI_DATA_CREATOR_H_

#include "AStringBuffer.h"
#include "ussi/UssiData.h"

class UssiDataCreator
{
public:
    static void GetXmlBody(IN const AString& strUssdString, OUT AStringBuffer& objXml,
            IN UssiModeType eUssiModeType = UssiModeType::NONE,
            IN UssiError eErrorCode = UssiError::CODE_NONE);

private:
    static const AString CreateStartElement(IN const AString& strStartElementName);
    static const AString CreateAttribute(
            IN const AString& strAttributeName, IN const AString& strValue);
    static const AString CreateEndElement(IN const AString& strEndElementName);

    static void GetErrorCode(IN UssiError eErrorCode, OUT AString& strErrorCode);
};

#endif
