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

#ifndef DIALOG_INFO_MANAGER_H_
#define DIALOG_INFO_MANAGER_H_

#include "ImsTypeDef.h"
#include "dialogevent/DialogInfo.h"
#include "dialogevent/IDialogInfoManager.h"
#include <memory>

class AString;
template <class T>
class ImsList;

/**
 * @brief Manages dialog information received from a dialog event package.
 *
 * This class is responsible for parsing an XML dialog event package and storing the
 * resulting dialog information. It provides access to the list of dialogs and
 * other metadata from the last processed package.
 */
class DialogInfoManager final : public IDialogInfoManager
{
public:
    DialogInfoManager();
    virtual ~DialogInfoManager() override;
    DialogInfoManager(IN const DialogInfoManager&) = delete;
    DialogInfoManager& operator=(IN const DialogInfoManager&) = delete;

    /** See {@link IDialogInfoManager#Update}. */
    IMS_RESULT Update(IN const AString& strEventPackage) override;

    /** See {@link IDialogInfoManager#GetDialogs}. */
    const ImsList<Dialog*>& GetDialogs() const override;

    /** See {@link IDialogInfoManager#GetState}. */
    IMS_UINT32 GetState() const override;

    /** See {@link IDialogInfoManager#GetVersion}. */
    IMS_UINT32 GetVersion() const override;

    /** See {@link IDialogInfoManager#GetEntity}. */
    const AString& GetEntity() const override;

private:
    std::unique_ptr<DialogInfo> m_pLastDialogInfo;
};

#endif
