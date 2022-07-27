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
#ifndef PLATFORM_PROPERTY_H_
#define PLATFORM_PROPERTY_H_

#include "ImsTypeDef.h"

class PlatformProperty
{
public:
    static IMS_BOOL Initialize();
    static void InitializeOnImsThread();
    static void Start();
    static void Stop();
    static void Uninitialize();
};

#endif
