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

#ifndef SUPP_SERVICE_UTILS_H_
#define SUPP_SERVICE_UTILS_H_

#include "ImsList.h"
#include "ImsTypeDef.h"

class AString;
class SuppService;

/**
 * This class contains utility functions for handle a list of SuppService.
 */
class SuppServiceUtils final
{
public:
    SuppServiceUtils() = delete;
    ~SuppServiceUtils() = delete;
    SuppServiceUtils(IN const SuppServiceUtils&) = delete;
    SuppServiceUtils& operator=(IN const SuppServiceUtils&) = delete;

    static void Add(IN ImsList<SuppService*>& objSuppServices, IN IMS_SINT32 nSuppType,
            IN const AString& strValue);
    static void Add(IN ImsList<SuppService*>& objSuppServices, IN IMS_SINT32 nSuppType,
            IN IMS_SINT32 nValue);
    static void Add(
            IN ImsList<SuppService*>& objSuppServices, IN IMS_SINT32 nSuppType, IN IMS_BOOL bValue);
    static void Delete(IN ImsList<SuppService*>& objSuppServices, IN IMS_SINT32 nType);
    static void DeleteServices(IN ImsList<SuppService*>& objSuppServices);
    static SuppService* Get(IN const ImsList<SuppService*>& objSuppServices, IN IMS_SINT32 nType);
    static IMS_BOOL IsSameSuppServices(IN const ImsList<SuppService*>& objSuppServicesA,
            IN const ImsList<SuppService*>& objSuppServicesB);
    static ImsList<SuppService*> Clone(IN const ImsList<SuppService*>& objSuppServices);
};

#endif
