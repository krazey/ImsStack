/*
 * Copyright (C) 2024 The Android Open Source Project
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
#include "Sip.h"

#include "TestSipProtocol.h"

IConnection* TestSipProtocol::OpenPrim(IN const AString& strName)
{
    AString strScheme;
    AString strTarget;
    AString strParams;

    Protocol::ParseName(strName, strScheme, strTarget, strParams);

    return OpenPrim(strScheme, strTarget, strParams);
}

IConnection* TestSipProtocol::OpenPrim(
        IN const AString& strScheme, IN const AString& strTarget, IN const AString& strParams)
{
    (void)strScheme;
    (void)strParams;

    // Check if it is SIP connection notifier or not
    if (strTarget.IsEmpty())
    {
        // SIP transaction notifier : dedicated mode (Port will be selected by the system)
        return IMS_NULL;
    }
    else if (strTarget.Equals(TextParser::CHAR_ASTERISK))
    {
        // SIP transaction notifier : shared mode
        return IMS_NULL;
    }
    else
    {
        IMS_BOOL bOk = IMS_FALSE;
        IMS_SINT32 nPort = strTarget.ToInt32(&bOk);

        if (bOk == IMS_TRUE)
        {
            // SIP transaction notifier : dedicated mode
            return IMS_NULL;
        }
        else
        {
            return m_piScc;
        }
    }
}
