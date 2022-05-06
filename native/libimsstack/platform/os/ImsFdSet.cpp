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
#include "ImsFdSet.h"
#include "ServiceMemory.h"

PUBLIC
ImsFdSet::ImsFdSet() {}

PUBLIC
ImsFdSet::ImsFdSet(IN const ImsFdSet& /*other*/) {}

PUBLIC VIRTUAL ImsFdSet::~ImsFdSet() {}

PUBLIC
ImsFdSet& ImsFdSet::operator=(IN const ImsFdSet& other)
{
    if (this != &other)
    {
        // no-op
    }

    return (*this);
}

/**
 * @brief Clears the specified events (mask of EVENT_XXX).
 *
 * It returns the cleared event or zero if there is nothing to be cleared.
 */
PUBLIC VIRTUAL IMS_SINT32 ImsFdSet::ClearEvent(IN IMS_SINT32 /*nFd*/, IN IMS_SINT32 /*nEvent*/)
{
    return 0;
}

/**
 * @brief Copies the content from the specified ImsFdSet instance.
 */
PUBLIC VIRTUAL void ImsFdSet::CopyFrom(IN const ImsFdSet* /*pFdSet*/)
{
    // no-op
}

/**
 * @brief Gets the signaled events for the specified descriptor.
 *
 * It is invoked when the events which returned from the system need to be checked.
 */
PUBLIC VIRTUAL IMS_SINT32 ImsFdSet::GetSignaledEvents(
        IN IMS_SINT32 /*nFd*/, IN_OUT IMS_SINT32& /*nSignaledCount*/)
{
    return 0;
}

/**
 * @brief Checks if the specified event (one of EVENT_XXX) is set or not.
 */
PUBLIC VIRTUAL IMS_BOOL ImsFdSet::IsEventSet(IN IMS_SINT32 /*nFd*/, IN IMS_SINT32 /*nEvent*/)
{
    return IMS_FALSE;
}

/**
 * @brief Checks if the highest file descriptor is required or not.
 */
PUBLIC VIRTUAL IMS_BOOL ImsFdSet::IsHighestFdRequired() const
{
    return IMS_FALSE;
}

/**
 * @brief Sets the specified events (mask of EVENT_XXX).
 *
 * It returns the set event or zero if there is nothing to be set.
 */
PUBLIC VIRTUAL IMS_SINT32 ImsFdSet::SetEvent(IN IMS_SINT32 /*nFd*/, IN IMS_SINT32 /*nEvent*/)
{
    return 0;
}

/**
 * @brief Sets the highest file descriptor.
 */
PUBLIC VIRTUAL void ImsFdSet::SetHighestFd(IN IMS_SINT32 /*nFd*/)
{
    // no-op
}

/**
 * @brief It returns a value when either any one of the specified descriptors is
 *        in the signaled state, or the time-out interval elapses.
 *
 * On success, a positive number is returned; this is the number of events.
 * (in other words, those descriptors with events or errors reported).
 * A value of 0 indicates that the call timed out and no file descriptors were ready.
 * On error, -1 is returned, and errno is set appropriately.
 */
PUBLIC VIRTUAL IMS_SINT32 ImsFdSet::WaitForEvents(IN IMS_SINT32 /*nMilliseconds = NO_TIMEOUT*/)
{
    return 0;
}
