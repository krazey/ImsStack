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
#ifndef RETRY_CMD_H_
#define RETRY_CMD_H_

#include "ImsTypeDef.h"

class IRetryCmdListener;

class RetryCmd
{
public:
    explicit RetryCmd(IN IMS_UINT32 nCmdId = 0);
    virtual ~RetryCmd();

    RetryCmd(IN const RetryCmd&) = delete;
    RetryCmd& operator=(IN const RetryCmd&) = delete;

public:
    // Executes the command
    virtual IMS_RESULT ExecuteCmd() = 0;

    inline IMS_UINT32 GetCmdId() const { return m_nCmdId; }
    inline void SetCmdListener(IN IRetryCmdListener* piListener) { m_piListener = piListener; }

protected:
    void OnCmdCompleted(IN IMS_SINT32 nResultCode, IN IMS_SINT32 nRetryAfter = 0);

private:
    IMS_UINT32 m_nCmdId;
    IRetryCmdListener* m_piListener;
};

#endif
