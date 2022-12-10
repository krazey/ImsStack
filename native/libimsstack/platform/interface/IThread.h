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
#ifndef INTERFACE_THREAD_H_
#define INTERFACE_THREAD_H_

#include "AString.h"
#include "ImsList.h"
#include "ImsMessage.h"
#include "IRunnable.h"

class IThread
{
protected:
    virtual ~IThread() = default;

public:
    virtual IMS_BOOL Activate() = 0;

    virtual void Deactivate() = 0;

    virtual IMS_BOOL Equals(IN const IThread* piThread) const = 0;

    virtual const AString& GetName() const = 0;

    virtual IMS_SINT32 GetSlotId() const = 0;

    virtual IMS_BOOL IsRunning() const = 0;

    virtual IMS_BOOL PostMessageI(IN ImsMessage& objMsg) = 0;

    virtual IMS_BOOL PostMessageI(
            IN IMS_UINT32 nMsg, IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) = 0;

    virtual void SetRunnable(IN IRunnable* piRunnable) = 0;

    /**
     * @brief Removes the messages from this thread's message queue
     *        that match the specified callback.
     *
     * @param piCallback The callback instance to be compared
     * @param pImsMsgs A list of matched messages or empty list if this is a non-null
     * @return The count of the removed ImsMessage.
     */
    virtual IMS_SINT32 RemoveMessages(IN ImsMessage::IMessageCallback* piCallback,
            OUT ImsList<ImsMessage>* pImsMsgs = IMS_NULL) = 0;
};

#endif
