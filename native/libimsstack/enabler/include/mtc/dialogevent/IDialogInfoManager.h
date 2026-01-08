/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef INTERFACE_DIALOG_INFO_MANAGER_H_
#define INTERFACE_DIALOG_INFO_MANAGER_H_

#include "ImsTypeDef.h"

class AString;
class Dialog;
template <class T>
class ImsList;

class IDialogInfoManager
{
public:
    virtual ~IDialogInfoManager() = default;

    /**
     * @brief Updates internal dialog-info data by parsing the dialog package..
     *
     * @param strEventPackage The dialog package will be parsed.
     * @return The result of updating.
     */
    virtual IMS_RESULT Update(IN const AString& strEventPackage) = 0;

    /**
     * @brief Gets Dialog list.
     *
     * @return The reference of Dialog pointer list.
     */
    virtual const ImsList<Dialog*>& GetDialogs() const = 0;

    /**
     * @brief Gets the state of last created DialogInfo.
     *
     * @return The state value of dialog-info element in the last received dialog event package.
     *         Valid values are
     *         {@link DialogInfo#STATE_INVALID},
     *         {@link DialogInfo#STATE_FULL},
     *         {@link Dialog#State#STATE_PARTIAL}.
     */
    virtual IMS_UINT32 GetState() const = 0;

    /**
     * @brief Gets the version of last created DialogInfo.
     *
     * @return The version value of dialog-info element in the last received dialog event package.
     */
    virtual IMS_UINT32 GetVersion() const = 0;

    /**
     * @brief Gets the entity of last created DialogInfo.
     *
     * @return The entity value of dialog-info element in the last received dialog event package.
     */
    virtual const AString& GetEntity() const = 0;
};

#endif
