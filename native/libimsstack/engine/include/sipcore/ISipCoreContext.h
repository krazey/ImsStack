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
#ifndef INTERFACE_SIP_CORE_CONTEXT_H_
#define INTERFACE_SIP_CORE_CONTEXT_H_

class Protocol;

/**
 * A context interface for providing the singleton instances for sipcore layer.
 */
class ISipCoreContext
{
protected:
    virtual ~ISipCoreContext() = default;

public:
    /**
     * @brief Returns the SipProtocol instance.
     */
    virtual Protocol* GetSipProtocol() const = 0;
};

#endif
