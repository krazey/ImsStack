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
#ifndef SERVICE_UTIL_H_
#define SERVICE_UTIL_H_

#include "PlatformService.h"

class AString;
class IImsPrivateProperty;
class ISystemProperty;
class ISystemUtil;
class IZLib;

class UtilService : public PlatformService
{
public:
    UtilService();
    UtilService(IN const UtilService&) = delete;
    UtilService& operator=(IN const UtilService&) = delete;

protected:
    ~UtilService() override;

public:
    virtual IImsPrivateProperty* GetPrivateProperty();
    virtual ISystemUtil* GetSystemUtil();
    virtual ISystemProperty* GetSystemProperty();
    virtual IZLib* GetZLib();

    static UtilService* GetUtilService();
    // Sets the debugging flag
    static void SetDebugOn(IN IMS_BOOL bDebugOn);
    // Return value: strOutput (user mode & config-debug-off), strInput (non-user mode)
    static const AString& GetLogString(IN const AString& strInput, IN_OUT AString& strOutput,
            IN IMS_SINT32 nOutSize, IN IMS_CHAR cDelimiter = 0 /* no delimiter */);

    ////
    // Logging
    ////
    static void LogSipMessage(IN const IMS_CHAR* pszMessage, IN IMS_SINT32 nLength,
        IN IMS_SINT32 nSlotId = 0, IN IMS_BOOL bOutgoing = IMS_FALSE);
};

#define IMS_UTIL_SYS_PROP_IS_SERVER_INFO_HIDDEN_IN_LOG() \
    UtilService::GetUtilService()->GetSystemProperty()->IsServerInfoHiddenInLog()

#define IMS_UTIL_SYS_PROP_IS_DEBUG_MODE() \
    UtilService::GetUtilService()->GetSystemProperty()->IsDebugMode()

#define IMS_UTIL_SYS_PROP_IS_USER_MODE() \
    UtilService::GetUtilService()->GetSystemProperty()->IsUserMode()

#define IMS_UTIL_SYS_PROP_SET_DEBUG_ON(bDebugOn) UtilService::GetUtilService()->SetDebugOn(bDebugOn)

#define IMS_UTIL_ZLIB_Compress(DATA, COMP_DATA) \
    UtilService::GetUtilService()->GetZLib()->Compress(DATA, COMP_DATA)

#define IMS_UTIL_ZLIB_Uncompress(COMP_DATA, DATA) \
    UtilService::GetUtilService()->GetZLib()->Uncompress(COMP_DATA, DATA)

#endif
