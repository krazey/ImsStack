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

#include "SipClientConnection.h"
#include "SipClientConnectionImpl.h"
#include "SipConnectionNotifier.h"
#include "SipConnectionNotifierImpl.h"
#include "SipManager.h"
#include "SipPrivate.h"
#include "SipProtocol.h"

__IMS_TRACE_TAG_SIP__;

/**
 * Creates and opens a SIP Connection.
 */
PRIVATE VIRTUAL IConnection* SipProtocol::OpenPrim(IN const AString& strName)
{
    AString strScheme;
    AString strTarget;
    AString strParams;

    Protocol::ParseName(strName, strScheme, strTarget, strParams);

    return OpenPrim(strScheme, strTarget, strParams);
}

/**
 * Creates and opens a SIP Connection.
 */
PRIVATE VIRTUAL IConnection* SipProtocol::OpenPrim(
        IN const AString& strScheme, IN const AString& strTarget, IN const AString& strParams)
{
    IMS_SINT32 nScheme;

    SipPrivate::SetLastError(SipError::NO_ERROR);

    if (strScheme.EqualsIgnoreCase(Sip::STR_SIP))
    {
        nScheme = Sip::URI_SCHEME_SIP;
    }
    else
    {
        nScheme = Sip::URI_SCHEME_SIPS;
    }

    // Check if it is SIP connection notifier or not
    if (strTarget.IsEmpty())
    {
        // SIP transaction notifier : dedicated mode (Port will be selected by the system)
        return CreateConnectionNotifier(nScheme, 0, strParams);
    }
    else if (strTarget.Equals(TextParser::CHAR_ASTERISK))
    {
        // SIP transaction notifier : shared mode
        return CreateConnectionNotifier(nScheme, 0, strParams, IMS_TRUE);
    }
    else
    {
        IMS_BOOL bOk = IMS_FALSE;
        IMS_SINT32 nPort = strTarget.ToInt32(&bOk);

        if (bOk == IMS_TRUE)
        {
            // SIP transaction notifier : dedicated mode
            return CreateConnectionNotifier(nScheme, nPort, strParams);
        }
        else
        {
            AString strUri = strScheme + TextParser::CHAR_COLON + strTarget;

            if (strParams.GetLength() > 0)
            {
                strUri.Prepend(TextParser::CHAR_LAQUOT);

                strUri += TextParser::CHAR_SEMICOLON + strParams;

                strUri.Append(TextParser::CHAR_RAQUOT);
            }

            // SIP client transaction
            SipClientConnection* pScc = new SipClientConnection(strUri);

            if (pScc == IMS_NULL)
            {
                SipPrivate::SetLastError(SipError::NO_MEMORY);

                IMS_TRACE_E(0, "Allocating SCC failed", 0, 0, 0);
                return IMS_NULL;
            }

            SipClientConnectionImpl* pSccImpl = new SipClientConnectionImpl(pScc);

            if (pSccImpl == IMS_NULL)
            {
                delete pScc;
                SipPrivate::SetLastError(SipError::NO_MEMORY);

                IMS_TRACE_E(0, "Allocating SCCImpl failed", 0, 0, 0);
                return IMS_NULL;
            }

            return pSccImpl;
        }
    }
}

PRIVATE
IConnection* SipProtocol::CreateConnectionNotifier(IN IMS_SINT32 nScheme, IN IMS_SINT32 nPort,
        IN const AString& strParams, IN IMS_BOOL bSharedMode /* = IMS_FALSE */)
{
    SipConnectionNotifier* pScn = new SipConnectionNotifier(nScheme, nPort, strParams, bSharedMode);

    if (pScn == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::NO_MEMORY);

        IMS_TRACE_E(0, "Allocating SCN failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (SipPrivate::GetLastError() != SipError::NO_ERROR)
    {
        delete pScn;

        IMS_TRACE_E(0, "Parsing the parameters for SCN failed", 0, 0, 0);
        return IMS_NULL;
    }

    SipConnectionNotifierImpl* pScnImpl = new SipConnectionNotifierImpl(pScn);

    if (pScnImpl == IMS_NULL)
    {
        delete pScn;
        SipPrivate::SetLastError(SipError::NO_MEMORY);

        IMS_TRACE_E(0, "Allocating SCNImpl failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (!SipManager::GetInstance()->AttachConnectionNotifier(pScn))
    {
        delete pScnImpl;

        IMS_TRACE_E(0, "Attaching SCN failed", 0, 0, 0);
        return IMS_NULL;
    }

    return pScnImpl;
}
