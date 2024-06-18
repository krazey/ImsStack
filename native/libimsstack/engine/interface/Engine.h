/*
 * Copyright (C) 2024 The Android Open Source Project
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
#ifndef ENGINE_H_
#define ENGINE_H_

class IConfiguration;

/**
 * @brief This class provides an interface to manage and control the state of Engine.
 */
class Engine
{
public:
    /**
     * @brief Returns the Configuration interface.
     */
    static IConfiguration* GetConfiguration();
};

#endif
