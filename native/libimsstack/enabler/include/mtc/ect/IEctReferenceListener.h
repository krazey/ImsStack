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

#ifndef INTERFACE_ECT_REFERENCE_LISTENER_H_
#define INTERFACE_ECT_REFERENCE_LISTENER_H_

class IEctReferenceListener
{
public:
    virtual ~IEctReferenceListener() = default;

    /**
     * @brief Notifies
     *
     */
    virtual void OnReferenceStarted() = 0;

    /**
     * @brief Notifies
     *
     */
    virtual void OnReferenceStartFailed() = 0;

    /**
     * @brief Notifies
     *
     * @param nSipFragCode
     */
    virtual void OnReferenceUpdated(IN IMS_SINT32 nSipFragCode) = 0;
};

#endif
