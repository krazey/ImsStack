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

#ifndef ECT_FACTORY_H_
#define ECT_FACTORY_H_

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "ect/BlindTransferController.h"
#include "ect/ConsultativeTransferController.h"
#include "ect/EctReference.h"
#include <memory>

class EctController;
class IMtcContext;
class IEctControllerListener;
class IEctReferenceListener;

class EctFactory final
{
public:
    inline explicit EctFactory() :
            m_pBlindController(nullptr),
            m_pConsultativeController(nullptr),
            m_pReference(nullptr)
    {
    }
    inline ~EctFactory() {}
    EctFactory(IN const EctFactory&) = delete;
    EctFactory& operator=(IN const EctFactory&) = delete;

    inline std::unique_ptr<EctController> CreateBlindController(IN IMtcContext& objContext,
            IN CallKey nTransfereeKey, IN IEctControllerListener& objListener)
    {
        if (m_pBlindController != nullptr)
        {
            return std::move(m_pBlindController);
        }
        return std::make_unique<BlindTransferController>(
                objContext, nTransfereeKey, objListener, *this);
    }
    inline void SetBlindController(IN std::unique_ptr<BlindTransferController> pController)
    {
        m_pBlindController = std::move(pController);
    }

    inline std::unique_ptr<EctController> CreateConsultativeController(IN IMtcContext& objContext,
            IN CallKey nTransfereeKey, IN IEctControllerListener& objListener)
    {
        if (m_pConsultativeController != nullptr)
        {
            return std::move(m_pConsultativeController);
        }
        return std::make_unique<ConsultativeTransferController>(
                objContext, nTransfereeKey, objListener, *this);
    }
    inline void SetConsultativeController(
            IN std::unique_ptr<ConsultativeTransferController> pController)
    {
        m_pConsultativeController = std::move(pController);
    }

    inline std::unique_ptr<EctReference> CreateReference(IN IMtcContext& objContext,
            IN CallKey nTransfereeKey, IN IEctReferenceListener& objListener)
    {
        if (m_pReference != nullptr)
        {
            return std::move(m_pReference);
        }
        return std::make_unique<EctReference>(objContext, nTransfereeKey, objListener);
    }
    inline void SetReference(IN std::unique_ptr<EctReference> pReference)
    {
        m_pReference = std::move(pReference);
    }

private:
    std::unique_ptr<BlindTransferController> m_pBlindController;
    std::unique_ptr<ConsultativeTransferController> m_pConsultativeController;
    std::unique_ptr<EctReference> m_pReference;
};

#endif
