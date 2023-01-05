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
#ifndef INTERFACE_REG_INFO_H_
#define INTERFACE_REG_INFO_H_

#include "IRegInfoRegistration.h"

/**
 * @brief This class provides an interface to access the "reginfo" XML document.
 */
class IRegInfo
{
protected:
    virtual ~IRegInfo() = default;

public:
    /**
     * @brief Returns the registration element information of reginfo.
     *
     * @param strAor the public user identity to be retrieved
     * @return Pointer to IRegInfoRegistration or null
     */
    virtual IRegInfoRegistration* GetRegistration(IN const AString& strAor) const = 0;

    /**
     * @brief Returns the registration element information of reginfo.
     *
     * @param strAor the public user identity to be retrieved
     * @return Pointer to IRegInfoRegistration or null
     */
    virtual IRegInfoRegistration* GetRegistration(IN const SipAddress& objAor) const = 0;

    /**
     * @brief Returns all the registration elements of reginfo.
     *
     * @return List of pointer to IRegInfoRegistration
     */
    virtual IMSList<IRegInfoRegistration*> GetRegistrations() const = 0;
};

#endif
