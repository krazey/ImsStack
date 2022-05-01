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

#include "ConfigLoader.h"
#include "conf/ConfigFileBuffer.h"

__IMS_TRACE_TAG_CONF__;

PUBLIC GLOBAL
IConfigBuffer* ConfigLoader::GetConfig(IN const AString& strContent)
{
    if (strContent.GetLength() == 0)
    {
        IMS_TRACE_E(0, "ConfigLoader: No configuration data.", 0, 0, 0);
        return IMS_NULL;
    }

    return ConfigFileBuffer::CreateFileBuffer(strContent);
}
