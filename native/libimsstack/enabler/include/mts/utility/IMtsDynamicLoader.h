/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef INTERFACE_MTS_DYNAMIC_LOADER_H_
#define INTERFACE_MTS_DYNAMIC_LOADER_H_

class MtsSipFormUtils;
class MtsSmUtils;
class MtsGeolocationUtils;
class MtsAosUtils;

class IMtsDynamicLoader
{
public:
    virtual ~IMtsDynamicLoader() {}

    /**
     * @brief Gets an instance of the MtsSipFormUtils class.
     *
     * This class provides utility functions related to SIP messages.
     *
     * @return A pointer to the MtsSipFormUtils object.
     */
    virtual MtsSipFormUtils* GetMtsSipFormUtils() const = 0;

    /**
     * @brief Gets an instance of the MtsSmUtils class.
     *
     * This class provides utility functions for retrieving information from the PDU.
     *
     * @return A pointer to the MtsSmUtils object.
     */
    virtual MtsSmUtils* GetMtsSmUtils() const = 0;

    /**
     * @brief Gets an instance of the MtsGeolocationUtils class.
     *
     * This class provides utility functions related to the geolocation.
     *
     * @return A pointer to the MtsGeolocationUtils object.
     */
    virtual MtsGeolocationUtils* GetMtsGeolocationUtils() const = 0;

    /**
     * @brief Gets an instance of the MtsAosUtils class.
     *
     * This class provides utility functions related to the Aos.
     *
     * @return A pointer to the MtsAosUtils object.
     */
    virtual MtsAosUtils* GetMtsAosUtils() const = 0;
};

#endif
