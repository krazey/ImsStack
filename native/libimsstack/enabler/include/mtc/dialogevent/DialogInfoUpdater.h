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

#ifndef DIALOG_INFO_UPDATER_H_
#define DIALOG_INFO_UPDATER_H_

#include "AString.h"

class DialogInfoUpdater
{
public:
    explicit DialogInfoUpdater();
    ~DialogInfoUpdater();
    DialogInfoUpdater(IN const DialogInfoUpdater&) = delete;
    DialogInfoUpdater& operator=(IN const DialogInfoUpdater&) = delete;

    const AString& Update(IN const AString& strEventPackage);

private:
    void Clear();
    IMS_SLONG GetIndexOfKeyHasSameId(IN const AString& strDialogInfoEntity);

private:
    ImsList<DialogInfo*> m_objDialogInfos;
};

#endif
