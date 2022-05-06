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
#include "ImsPrivateProperty.h"
#include "PlatformApi.h"
#include "PlatformFactory.h"
#include "ServiceMemory.h"
#include "ServiceUtil.h"

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
    ISystemUtil* piSysUtil = PlatformFactory::GetSystemUtil();

    IMS_ASSERT(piSysUtil != IMS_NULL);

    return piSysUtil;
}

PUBLIC
ISystemProperty* UtilService::GetSystemProperty()
{
    ISystemProperty* piSysProperty = PlatformFactory::GetSystemProperty();

    IMS_ASSERT(piSysProperty != IMS_NULL);

    return piSysProperty;
}

PUBLIC
IZLib* UtilService::GetZLib()
{
    IZLib* piZLib = PlatformFactory::GetZLib();

    IMS_ASSERT(piZLib != IMS_NULL);

    return piZLib;
}

PUBLIC
void UtilService::SetDebugOn(IN IMS_BOOL bDebugOn)
{
    PlatformApi::SetDebugOn(bDebugOn);
}

PUBLIC GLOBAL UtilService* UtilService::GetUtilService()
{
    static UtilService* s_pUtilService = IMS_NULL;

    if (s_pUtilService == IMS_NULL)
    {
        s_pUtilService = new UtilService();
    }

    return s_pUtilService;
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
