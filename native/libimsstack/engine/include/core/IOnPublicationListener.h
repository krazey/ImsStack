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
#ifndef INTERFACE_ON_PUBLICATION_LISTENER_H_
#define INTERFACE_ON_PUBLICATION_LISTENER_H_

#include "ImsTypeDef.h"

class Publication;

/**
 * @brief This listener type is used to notify the application of the status
 *        of requested publications.
 *
 * @see Publication
 */
class IOnPublicationListener
{
public:
    /**
     * @brief Notifies the application that the publication request was successfully delivered.
     *
     * @param pPublication The Publication object
     */
    virtual void OnPublication_Delivered(IN Publication* pPublication) = 0;

    /**
     * @brief Notifies the application that the publication request was not successfully delivered.
     *
     * @param pPublication The Publication object
     */
    virtual void OnPublication_DeliveryFailed(IN Publication* pPublication) = 0;

    /**
     * @brief Notifies the application that the publication was terminated.
     *
     * @param pPublication The Publication object
     */
    virtual void OnPublication_Terminated(IN Publication* pPublication) = 0;

    /**
     * @brief Notifies the application that the publication was terminated.
     *
     * @param pPublication The Publication object
     */
    virtual void OnPublication_RefreshStarted(IN Publication* pPublication) = 0;

    /**
     * @brief Notifies the application that the publication was terminated.
     *
     * @param pPublication The Publication object
     */
    virtual void OnPublication_RefreshCompleted(IN Publication* pPublication) = 0;
};

#endif
