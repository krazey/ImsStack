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
#ifndef IMS_AOS_MANAGER_H_
#define IMS_AOS_MANAGER_H_

#include "ImsTypeDef.h"
#include "ImsApp.h"
#include "AString.h"

class IImsAos;

/**
 * @brief This class provides the base class based on IMS Application for AoS interfaces.
 */

class ImsAosManager : public ImsApp
{
public:
    explicit ImsAosManager(IN const AString& strAppName);
    virtual ~ImsAosManager();

    /// Implement child class
    virtual IImsAos* GetImsAos(IN const AString& strAppId, IN const AString& strServiceId);
    virtual ImsList<IImsAos*> GetImsAosList(
            IN const AString& strAppId, IN const AString& strServiceId);
    virtual ImsList<IImsAos*> GetImsAosList(IN const AString& strAppId);

    /// ImsApp Class
    virtual IMS_BOOL OnPreprocess(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnPostprocess(IN IMSMSG& objMSG);
};

#endif  // IMS_AOS_MANAGER_H_
