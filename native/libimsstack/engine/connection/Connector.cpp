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

#include "Connector.h"
#include "Protocol.h"
#include "ProtocolPermission.h"

__IMS_TRACE_TAG_USER_DECL__("GCF");

PUBLIC
IConnection* Connector::Open(IN const AString& strName)
{
    IMS_SINT32 nIndexOfColon = strName.GetIndexOf(TextParser::CHAR_COLON);

    if (nIndexOfColon == AString::NPOS)
    {
        IMS_TRACE_E(0, "Connection name is malformed: %s", strName.GetStr(), 0, 0);
        return IMS_NULL;
    }

    AString strUri;

    // Remove the display name and AQUOT if present
    IMS_SINT32 nIndexOfLaquot = strName.GetIndexOf(TextParser::CHAR_LAQUOT);

    if (nIndexOfLaquot != AString::NPOS)
    {
        IMS_SINT32 nIndexOfRaquot = strName.GetIndexOf(TextParser::CHAR_RAQUOT);

        strUri = strName.GetSubStr(nIndexOfLaquot + 1, nIndexOfRaquot - nIndexOfLaquot - 1);
    }
    else
    {
        strUri = strName;
    }

    strUri = strUri.Trim();

    // Tokenize the scheme field in the given name
    nIndexOfColon = strUri.GetIndexOf(TextParser::CHAR_COLON);

    AString strScheme = strUri.Left(nIndexOfColon).Trim();

    // Look up the protocol to determine if this URI scheme supports or not
    Protocol* pProtocol = ProtocolPermission::Lookup(strScheme);

    if (pProtocol == IMS_NULL)
    {
        // ConnectionNotFoundException
        IMS_TRACE_E(0, "Protocol permissioni is not allowed: %s, uri: %s", strName.GetStr(),
                strUri.GetStr(), 0);
        return IMS_NULL;
    }

    return pProtocol->OpenPrim(strUri);
}

PUBLIC
IConnection* Connector::Open(
        IN const AString& strScheme, IN const AString& strTarget, IN const AString& strParams)
{
    AString strTmpScheme = strScheme.Trim();

    // Look up the protocol to determine if this URI scheme supports or not
    Protocol* pProtocol = ProtocolPermission::Lookup(strTmpScheme);

    if (pProtocol == IMS_NULL)
    {
        IMS_TRACE_E(0, "Protocol permissioni is not allowed: %s", strScheme.GetStr(), 0, 0);
        return IMS_NULL;
    }

    return pProtocol->OpenPrim(strTmpScheme, strTarget, strParams);
}
