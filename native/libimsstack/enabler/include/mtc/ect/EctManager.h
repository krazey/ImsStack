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

#ifndef ECT_MANAGER_H_
#define ECT_MANAGER_H_

#include "ImsTypeDef.h"
#include "ect/EctController.h"
#include "ect/EctFactory.h"
#include "ect/IEctControllerListener.h"
#include "ect/IEctManager.h"
#include <memory>

class IMtcContext;

/**
 * @brief Manages Explicit Call Transfer (ECT) operations.
 *
 * This class is the main entry point for initiating call transfers. It creates and manages
 * the appropriate EctController (Blind or Consultative) to handle the transfer process.
 */
class EctManager : public IEctControllerListener, public IEctManager
{
public:
    /**
     * @brief Constructs a new Ect Manager object.
     *
     * @param objContext The MTC context.
     */
    explicit EctManager(IN IMtcContext& objContext);
    virtual ~EctManager() override;
    EctManager(IN const EctManager&) = delete;
    EctManager& operator=(IN const EctManager&) = delete;

    /** See {@link IEctControllerListener#OnEctCompleted}. */
    void OnEctCompleted() override;

    /** See {@link IEctManager#Transfer}. */
    IMS_RESULT Transfer(IN CallKey nCallKey, IN const AString& strNumber) override;

    /**
     * @brief Gets the current state of the ECT manager.
     *
     * @return IEctManager::State The current state (IDLE, BLIND_TRANSFERRING, etc.).
     */
    inline IEctManager::State GetState() override { return m_eState; }

protected:
    EctFactory m_objEctFactory;

private:
    IMtcContext& m_objContext;
    IEctManager::State m_eState;
    std::unique_ptr<EctController> m_pController;
};

#endif
