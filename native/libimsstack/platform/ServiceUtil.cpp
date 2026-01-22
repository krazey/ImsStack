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
#include "IOsFactory.h"
#include "ISystemProperty.h"
#include "ImsPrivateProperty.h"
#include "PlatformApi.h"
#include "PlatformContext.h"
#include "ServiceMemory.h"
#include "ServiceUtil.h"
#include "system-intf/System.h"

PRIVATE
UtilService::UtilService() {}

PRIVATE
UtilService::~UtilService() {}

PUBLIC
IImsPrivateProperty* UtilService::GetPrivateProperty()
{
    ImsPrivateProperty* pPrivateProperty = ImsPrivateProperty::GetInstance();

    IMS_ASSERT(pPrivateProperty != IMS_NULL);

    return pPrivateProperty;
}

PUBLIC
ISystemUtil* UtilService::GetSystemUtil()
{
    IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
    ISystemUtil* piSysUtil = piOsFactory->GetSystemUtil();

    IMS_ASSERT(piSysUtil != IMS_NULL);

    return piSysUtil;
}

PUBLIC
ISystemProperty* UtilService::GetSystemProperty()
{
    IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
    ISystemProperty* piSysProperty = piOsFactory->GetSystemProperty();

    IMS_ASSERT(piSysProperty != IMS_NULL);

    return piSysProperty;
}

PUBLIC
IZLib* UtilService::GetZLib()
{
    IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
    IZLib* piZLib = piOsFactory->GetZLib();

    IMS_ASSERT(piZLib != IMS_NULL);

    return piZLib;
}

PUBLIC GLOBAL UtilService* UtilService::GetUtilService()
{
    return DYNAMIC_CAST(UtilService*,
            PlatformContext::GetInstance()->GetService(PlatformContext::SERVICE_UTIL));
}

PUBLIC GLOBAL void UtilService::SetDebugOn(IN IMS_BOOL bDebugOn)
{
    PlatformApi::SetDebugOn(bDebugOn);
}

/**
 * Return value: strOutput (user mode & config-debug-off), strInput (non-user mode)
 */
PUBLIC GLOBAL const AString& UtilService::GetLogString(IN const AString& strInput,
        IN_OUT AString& strOutput, IN IMS_SINT32 nOutSize, IN IMS_CHAR cDelimiter /* = 0 */)
{
    if (IMS_UTIL_SYS_PROP_IS_DEBUG_MODE())
    {
        return strInput;
    }

    if (strInput.GetLength() == 0)
    {
        strOutput = "zzz";
        return strOutput;
    }

    IMS_SINT32 nIndex = AString::NPOS;

    if (cDelimiter > 0)
    {
        nIndex = strInput.GetIndexOf(cDelimiter);
    }

    IMS_BOOL bAddPostFix = IMS_TRUE;

    if (nIndex == AString::NPOS)
    {
        strOutput = strInput.GetSubStr(0, nOutSize);

        if (nOutSize >= strInput.GetLength())
        {
            bAddPostFix = IMS_FALSE;
        }
    }
    else
    {
        if ((nOutSize > 0) && (nOutSize < nIndex))
            strOutput = strInput.GetSubStr(0, nOutSize);
        else
            strOutput = strInput.GetSubStr(0, nIndex);
    }

    if (bAddPostFix)
    {
        strOutput.Append("xxx");
    }

    return strOutput;
}

PUBLIC GLOBAL void UtilService::LogSipMessage(IN const IMS_CHAR* pszMessage, IN IMS_SINT32 nLength,
        IN IMS_SINT32 nSlotId, IN IMS_BOOL bOutgoing)
{
    PlatformContext::GetInstance()->GetSystem()->LogSipMessage(
            pszMessage, nLength, nSlotId, bOutgoing);
}
