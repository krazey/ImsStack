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

#include "ConfigAppFactory.h"
#include "base/ConfigApp.h"

static AString configAppFactory_CreateName(IN IMS_SINT32 nSlotId)
{
    AString strName;

    strName.Sprintf("ConfigApp%02d", nSlotId);

    return strName;
}

ConfigApp* ConfigAppFactory::Create(IN IMS_SINT32 nSlotId)
{
    ConfigApp* pConfigApp = new ConfigApp(configAppFactory_CreateName(nSlotId));

    if (pConfigApp != IMS_NULL)
    {
        pConfigApp->Start();
    }

    return pConfigApp;
}

void ConfigAppFactory::Destroy(IN_OUT ConfigApp*& pConfigApp)
{
    if (pConfigApp != IMS_NULL)
    {
        delete pConfigApp;
        pConfigApp = IMS_NULL;
    }
}
