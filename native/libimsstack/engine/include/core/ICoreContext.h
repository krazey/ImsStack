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
#ifndef INTERFACE_CORE_CONTEXT_H_
#define INTERFACE_CORE_CONTEXT_H_

class CallControlHelper;
class CallerPreferenceManager;
class ServiceProtocol;

/**
 * A context interface for providing the singleton instances for core (imscore) layer.
 */
class ICoreContext
{
protected:
    virtual ~ICoreContext() = default;

public:
    /**
     * @brief Returns the ImsCoreProtocol instance.
     */
    virtual ServiceProtocol* GetImsCoreProtocol() const = 0;

    /**
     * @brief Returns the CallControlHelper instance.
     */
    virtual CallControlHelper* GetCallControlHelper() = 0;

    /**
     * @brief Returns the CallerPreferenceManager instance.
     */
    virtual CallerPreferenceManager* GetCallerPreferenceManager() = 0;
};

#endif
