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

#include "AString.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "dialogevent/DialogInfo.h"
#include "dialogevent/IDialogInfoManager.h"
#include <memory>

class DialogInfoManager final : public IDialogInfoManager
{
public:
    DialogInfoManager();
    virtual ~DialogInfoManager() override;
    DialogInfoManager(IN const DialogInfoManager&) = delete;
    DialogInfoManager& operator=(IN const DialogInfoManager&) = delete;

    IMS_RESULT Update(IN const AString& strEventPackage) override;

    const ImsList<Dialog*>& GetDialogs() const override;
    IMS_UINT32 GetState() const override;
    IMS_UINT32 GetVersion() const override;
    const AString& GetEntity() const override;

private:
    std::unique_ptr<DialogInfo> m_pLastDialogInfo;
};

#endif
