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

#ifndef INTERFACE_MTC_APP_H_
#define INTERFACE_MTC_APP_H_

/**
 * @brief Interface for the MTC application.
 *
 * This interface defines the lifecycle methods for the MTC application.
 * It serves as the main entry point for the MTC enabler, managing services and components
 * related to IMS telephony features such as voice/video calls and supplementary services.
 */
class IMtcApp
{
public:
    virtual ~IMtcApp(){};

    /**
     * @brief Starts the MTC application.
     *
     * This method initializes the necessary resources and starts the application logic.
     * It creates {@code IMtcService} instances (Normal and Emergency), initializes the
     * {@code IMtcCallManager}, and sets up other components such as {@code IMtcRadioChecker}
     * and {@code MultiEndpointManager} required for the MTC enabler.
     */
    virtual void Start() = 0;

    /**
     * @brief Stops the MTC application.
     *
     * This method stops the application logic and releases allocated resources.
     * It de-initializes the {@code IMtcCallManager} and destroys the {@code IMtcService}
     * instances and other components created during start.
     */
    virtual void Stop() = 0;
};

#endif
