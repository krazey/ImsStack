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
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "OsDll.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_ADAPT__;

PUBLIC GLOBAL void* OsDll::LoadLibrary(IN const IMS_CHAR* pszName)
{
    if (pszName == IMS_NULL)
    {
        IMS_TRACE_E(0, "DLL :: Library name is null", 0, 0, 0);
        return IMS_NULL;
    }

    void* pvHandle = dlopen(pszName, RTLD_LAZY);

    if (pvHandle == IMS_NULL)
    {
        IMS_TRACE_E(0, "DLL :: Loading %s failed - %s", pszName, dlerror(), 0);
        return IMS_NULL;
    }

    return pvHandle;
}

PUBLIC GLOBAL void OsDll::FreeLibrary(IN void* pvHandle)
{
    if (pvHandle == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nResult = dlclose(pvHandle);

    if (nResult != 0)
    {
        IMS_TRACE_E(0, "DLL :: Freeing the library failed - %s", dlerror(), 0, 0);
    }
}

PUBLIC GLOBAL void* OsDll::GetProcAddress(IN void* pvHandle, IN const IMS_CHAR* pszName)
{
    IMS_CHAR acProcName[512];

    if (pszName == IMS_NULL)
    {
        IMS_TRACE_E(0, "DLL :: Name is null", 0, 0, 0);
        return IMS_NULL;
    }

    // Some platforms might need a leading underscore for the function symbol
    (void)snprintf(acProcName, sizeof(acProcName), "_%s", pszName);

    void* pvProcAddress = dlsym(pvHandle, acProcName);

    if (pvProcAddress == IMS_NULL)
    {
        // Find the symbol without the leading underscore
        pvProcAddress = dlsym(pvHandle, pszName);
    }

    if (pvProcAddress == IMS_NULL)
    {
        IMS_TRACE_E(0, "DLL :: Loading the symbol(%s) failed - %s", pszName, dlerror(), 0);
        return IMS_NULL;
    }

    return pvProcAddress;
}
