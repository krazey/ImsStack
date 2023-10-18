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

#include "IMtcContext.h"
#include "ServiceTrace.h"
#include "ect/BlindTransferController.h"
#include "ect/ConsultativeTransferController.h"
#include "ect/EctController.h"
#include "ect/EctFactory.h"
#include "ect/EctManager.h"
#include "ect/IEctManager.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
EctManager::EctManager(IN IMtcContext& objContext) :
        m_objEctFactory(),
        m_objContext(objContext),
        m_eState(IEctManager::State::IDLE),
        m_pController(nullptr)
{
    IMS_TRACE_D("+EctManager", 0, 0, 0);
}

PUBLIC
EctManager::~EctManager()
{
    IMS_TRACE_D("~EctManager", 0, 0, 0);
}

PUBLIC VIRTUAL void EctManager::OnEctCompleted()
{
    IMS_TRACE_D("OnEctCompleted", 0, 0, 0);
    m_pController = nullptr;
    m_eState = IEctManager::State::IDLE;
}

PUBLIC
IMS_RESULT EctManager::Transfer(IN CallKey nCallKey, IN const AString& strNumber)
{
    if (m_eState != IEctManager::State::IDLE)
    {
        IMS_TRACE_E(0, "no multiple ECT is supported.", 0, 0, 0);
        // TODO: send error to UI.
        return IMS_FAILURE;
    }

    if (strNumber.GetLength() > 0)
    {
        m_pController = m_objEctFactory.CreateBlindController(m_objContext, nCallKey, *this);
        m_pController->Transfer(strNumber);
        m_eState = IEctManager::State::BLIND_TRANSFERRING;
    }
    else
    {
        m_pController = m_objEctFactory.CreateConsultativeController(m_objContext, nCallKey, *this);
        m_pController->Transfer();
        m_eState = IEctManager::State::CONSULTATIVE_TRANSFERRING;
    }

    return IMS_SUCCESS;
}
