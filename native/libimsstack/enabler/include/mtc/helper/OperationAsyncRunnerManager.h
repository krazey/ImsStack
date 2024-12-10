/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef OPERATION_ASYNC_RUNNER_MANAGER_H_
#define OPERATION_ASYNC_RUNNER_MANAGER_H_

#include "ImsTypeDef.h"
#include <memory>
#include <unordered_map>

class OperationAsyncRunner;

class OperationAsyncRunnerManager final
{
public:
    inline explicit OperationAsyncRunnerManager(IN IMS_SINT32 nSlotId) :
            m_nSlotId(nSlotId),
            m_objRunners()
    {
    }
    inline ~OperationAsyncRunnerManager() { RemoveAllRunners(); }
    OperationAsyncRunnerManager(IN const OperationAsyncRunnerManager&) = delete;
    OperationAsyncRunnerManager& operator=(IN const OperationAsyncRunnerManager&) = delete;

    inline void Run(IN void* pOwner, IN std::function<void()> objOperation)
    {
        if (pOwner == IMS_NULL || objOperation == IMS_NULL)
        {
            return;
        }

        auto pRunner = std::make_unique<OperationAsyncRunner>(m_nSlotId);
        pRunner->SetOperation(objOperation,
                [this, pOwner, pRawRunner = pRunner.get()]()
                {
                    this->RemoveRunner(pOwner, pRawRunner);
                });

        m_objRunners.insert({pOwner, std::move(pRunner)});
    }

    inline void Release(IN void* pOwner)
    {
        std::erase_if(m_objRunners,
                [pOwner](const auto& record)
                {
                    return record.first == pOwner && !record.second->IsOperationStarted();
                });
    }

private:
    inline void RemoveAllRunners()
    {
        std::erase_if(m_objRunners,
                [](const auto& record)
                {
                    return !record.second->IsOperationStarted();
                });
    }

    inline void RemoveRunner(IN void* pOwner, OperationAsyncRunner* pRunner)
    {
        std::erase_if(m_objRunners,
                [pOwner, pRunner](const auto& record)
                {
                    return record.first == pOwner && record.second.get() == pRunner;
                });
    }

    const IMS_SINT32 m_nSlotId;
    std::unordered_multimap<void*, std::unique_ptr<OperationAsyncRunner>> m_objRunners;
};

#endif
