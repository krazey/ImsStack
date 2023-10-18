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
#ifndef SERVICE_RESOLVER_H_
#define SERVICE_RESOLVER_H_

#include "IRegBinding.h"

/**
 * @brief This class is an interface to get/set IMS registration binding with IMS service.
 */
class ServiceResolver
{
public:
    ServiceResolver() = delete;

public:
    /**
     * @brief Returns the IRegBinding which matches the specified value, strAppId & strServiceId.
     *
     * @param nSlotId slot id to be retrieved
     * @param strAppId an IMS application identifier
     * @param strServiceId an IMS service identifier
     * @return Pointer to IRegBinding
     */
    static IRegBinding* GetRegBinding(
            IN IMS_SINT32 nSlotId, IN const AString& strAppId, IN const AString& strServiceId);

    /**
     * @brief Returns all the registered IRegBinding.
     *
     * @param nSlotId slot id to be retrieved
     * @param strAppId an IMS application identifier
     * @param strServiceId an IMS service identifier
     * @return List of pointer to IRegBinding
     */
    static ImsList<IRegBinding*> GetRegBindings(IN IMS_SINT32 nSlotId);

    /**
     * @brief Returns all the registered IRegBinding.
     *
     * @param nSlotId slot id to be set
     * @param strAppId an IMS application identifier
     * @param strServiceId an IMS service identifier
     * @param piRegBinding Pointer to IRegBinding to be set
     */
    static void SetRegBinding(IN IMS_SINT32 nSlotId, IN const AString& strAppId,
            IN const AString& strServiceId, IN IRegBinding* piRegBinding);
};

#endif
