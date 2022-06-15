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
#ifndef INTERFACE_PUBLICATION_LISTENER_H_
#define INTERFACE_PUBLICATION_LISTENER_H_

#include "ImsTypeDef.h"

class IPublication;

/**
 * @brief This class provides a listener interface to notify the application of
 *        the status of requested publications.
 *
 * @see IPublication
 */
class IPublicationListener
{
public:
    /**
     * @brief Notifies the application that the publication request was successfully delivered.
     *
     * @param piPublication Pointer to IPublication
     */
    virtual void PublicationDelivered(IN IPublication* piPublication) = 0;

    /**
     * @brief Notifies the application that the publication request was not successfully delivered.
     *
     * @param piPublication Pointer to IPublication
     */
    virtual void PublicationDeliveryFailed(IN IPublication* piPublication) = 0;

    /**
     * @brief Notifies the application that the publication was terminated.
     *
     * @param piPublication Pointer to IPublication
     */
    virtual void PublicationTerminated(IN IPublication* piPublication) = 0;

    /**
     * @brief Notifies the application that the publication was refreshed.
     *
     * @param piPublication Pointer to IPublication
     */
    virtual void PublicationRefreshStarted(IN IPublication* piPublication) = 0;

    /**
     * @brief Notifies the application that the publication was refresh done.
     *
     * @param piPublication Pointer to IPublication
     */
    virtual void PublicationRefreshCompleted(IN IPublication* piPublication) = 0;
};

#endif
