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
#include "TextParser.h"

#include "Protocol.h"

PUBLIC VIRTUAL Protocol::~Protocol() {}

/**
 * @brief Parses the connection string which consists of three parts: scheme, target, parameters.
 */
PUBLIC GLOBAL void Protocol::ParseName(IN const AString& strName, OUT AString& strScheme,
        OUT AString& strTarget, OUT AString& strParams)
{
    IMS_SINT32 nPosColon;
    IMS_SINT32 nPosSemiColon;
    IMS_SINT32 nPosAt;

    AString strTmp = strName.Trim();

    // Get scheme information
    nPosColon = strTmp.GetIndexOf(TextParser::CHAR_COLON);
    strScheme = strTmp.GetSubStr(0, nPosColon);

    // Get target information
    nPosAt = strTmp.GetIndexOf(TextParser::CHAR_AT, nPosColon + 1);

    // Get the position of semi-colon to identify if the parameter is present or not.
    if (nPosAt == AString::NPOS)
    {
        nPosSemiColon = strTmp.GetIndexOf(TextParser::CHAR_SEMICOLON, nPosColon + 1);
    }
    else
    {
        nPosSemiColon = strTmp.GetIndexOf(TextParser::CHAR_SEMICOLON, nPosAt + 1);
    }

    // Check if the parameter exists or not
    if (nPosSemiColon == AString::NPOS)
    {
        // Get target
        strTarget = strTmp.GetSubStr(nPosColon + 1);
        strParams = AString::ConstNull();
    }
    else
    {
        // Get target
        strTarget = strTmp.GetSubStr(nPosColon + 1, nPosSemiColon - nPosColon - 1);
        // Get parameters information
        strParams = strTmp.GetSubStr(nPosSemiColon + 1);
    }
}
