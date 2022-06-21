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

#include "ServiceTrace.h"
#include "ect/EctManager.h"
#include "ect/EctController.h"
#include "ect/BlindTransferController.h"
#include "ect/ConsultativeTransferController.h"
#include "helper/ObjectAsyncDestroyer.h"
#include "IMtcContext.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
EctManager::EctManager(IN IMtcContext& objContext) :
        m_objContext(objContext),
        m_pController(IMS_NULL),
        m_objDestroyer(ObjectAsyncDestroyer<EctController>())
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
    m_objDestroyer.Destroy(m_pController);
    m_pController = IMS_NULL;
}

PUBLIC
void EctManager::Transfer(IN CallKey nCallKey, IN const AString& strNumber)
{
    if (m_pController)
    {
        IMS_TRACE_E(0, "no multiple ECT is supported.", 0, 0, 0);
        // TODO: send error to UI.
        return;
    }

    if (strNumber.GetLength() > 0)
    {
        m_pController = new BlindTransferController(m_objContext, nCallKey, *this);
        m_pController->Transfer(strNumber);
    }
    else
    {
        m_pController = new ConsultativeTransferController(m_objContext, nCallKey, *this);
        m_pController->Transfer();
    }
}
