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

#ifndef INTERFACE_ECT_CONTROLLER_LISTENER_H_
#define INTERFACE_ECT_CONTROLLER_LISTENER_H_

/**
 * @brief Listener interface for events from an EctController.
 *
 * This interface is used by classes that need to be notified about the completion
 * of an Explicit Call Transfer (ECT) operation.
 */
class IEctControllerListener
{
public:
    virtual ~IEctControllerListener() = default;

    /** Notifies the listener that the ECT operation has been completed. */
    virtual void OnEctCompleted() = 0;
};

#endif
