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
#ifndef ENGINE_LOADER_H_
#define ENGINE_LOADER_H_

#include "ImsTypeDef.h"

/**
 * @brief This class provides an interface to initialize/uninitialize the IMS engine.
 *
 * It will be called by EnablerThread to load a proper component for each slot.
 */
class EngineLoader
{
public:
    /**
     * @brief Initializes the engine with the given slot id.
     *
     * @param nSlotId The slot id to be initialized
     */
    static void Initialize(IN IMS_SINT32 nSlotId);

    /**
     * @brief Uninitializes the engine with the given slot id.
     *
     * @param nSlotId The slot id to be uninitialized
     */
    static void Uninitialize(IN IMS_SINT32 nSlotId);
};

#endif
