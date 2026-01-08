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
#ifndef NATIVE_COMMANDS_H_
#define NATIVE_COMMANDS_H_

#include "ImsTypeDef.h"

class NativeCommands
{
public:
    NativeCommands() = delete;

public:
    static inline const IMS_SINT32 CMD_SET_DEVICE_CONFIG = 1;
    static inline const IMS_SINT32 CMD_START_ENABLER = 2;
    static inline const IMS_SINT32 CMD_STOP_ENABLER = 3;
};

#endif
