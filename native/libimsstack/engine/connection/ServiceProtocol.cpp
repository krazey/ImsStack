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

#include "IService.h"
#include "ServiceProtocol.h"

__IMS_TRACE_TAG_USER_DECL__("GCF");

PUBLIC GLOBAL const IMS_CHAR ServiceProtocol::CONNECTION_PARAM_USER_ID[] = "userId";
PUBLIC GLOBAL const IMS_CHAR ServiceProtocol::CONNECTION_PARAM_SERVICE_ID[] = "serviceId";

PUBLIC VIRTUAL IConnection* ServiceProtocol::OpenPrim(IN const AString& strName)
{
    AString strServiceType;
    AString strAppId;
    AString strParams;

    Protocol::ParseName(strName, strServiceType, strAppId, strParams);

    if (!strAppId.StartsWith("//"))
    {
        IMS_TRACE_E(0, "Open string is malformed : %s", strName.GetStr(), 0, 0);
        return IMS_NULL;
    }

    strAppId = strAppId.GetSubStr(2);

    if (strAppId.GetLength() == 0)
    {
        IMS_TRACE_E(0, "Invalid application Id: %s", strName.GetStr(), 0, 0);
        return IMS_NULL;
    }

    return OpenPrim(strServiceType, strAppId, strParams);
}

PUBLIC VIRTUAL IConnection* ServiceProtocol::OpenPrim(
        IN const AString& strScheme, IN const AString& strTarget, IN const AString& strParams)
{
    AString strUserId;
    AString strServiceId(AString::ConstEmpty());

    // Check if the scheme is valid or not
    if (!strScheme.EqualsIgnoreCase(GetConnectionScheme()))
    {
        IMS_TRACE_E(0, "Scheme is not supported; name=%s://%s;%s", strScheme.GetStr(),
                strTarget.GetStr(), strParams.GetStr());
        return IMS_NULL;
    }

    ImsList<AString> objTokens = strParams.Split(TextParser::CHAR_SEMICOLON);

    for (IMS_UINT32 i = 0; i < objTokens.GetSize(); ++i)
    {
        const AString& strToken = objTokens.GetAt(i);
        IMS_SINT32 nPos = strToken.GetIndexOf(TextParser::CHAR_EQUAL);

        if (nPos != AString::NPOS)
        {
            AString strName = strToken.GetSubStr(0, nPos);

            if (strName.EqualsIgnoreCase(CONNECTION_PARAM_USER_ID))
            {
                strUserId = strToken.GetSubStr(nPos + 1);
            }
            else if (strName.EqualsIgnoreCase(CONNECTION_PARAM_SERVICE_ID))
            {
                strServiceId = strToken.GetSubStr(nPos + 1);
            }
        }
    }

    return CreateService(strTarget, strServiceId, strUserId);
}

PROTECTED VIRTUAL IService* ServiceProtocol::CreateService(IN const AString& /*strAppId*/,
        IN const AString& /*strServiceId*/, IN const AString& /*strUserId*/)
{
    // The subclass MUST implement this method
    return IMS_NULL;
}

PROTECTED VIRTUAL const IMS_CHAR* ServiceProtocol::GetConnectionScheme() const
{
    // The subclass MUST implement this method
    return IMS_NULL;
}
