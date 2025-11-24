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
#ifndef AOS_BLOCK_H_
#define AOS_BLOCK_H_

#include "ImsHashMap.h"
#include "ImsList.h"
#include "interface/IAosBlock.h"

class IAosAppContext;
class IAosBlockListener;

class AosBlock : public IAosBlock
{
public:
    explicit AosBlock(IN IAosAppContext* piAppContext);
    ~AosBlock() override;

    void SetListener(IN IAosBlockListener* piListener) override;
    void RemoveListener(IN IAosBlockListener* piListener) override;
    void SetSilentListener(IN IAosBlockSilentListener* piListener) override;
    void RemoveSilentListener(IN IAosBlockSilentListener* piListener) override;

    IMS_BOOL SetBlockReason(IN BLOCK_REASON eReason, IN IMS_BOOL bNotify = IMS_TRUE) override;
    IMS_BOOL ResetBlockReason(IN BLOCK_REASON eReason, IN IMS_BOOL bNotify = IMS_TRUE) override;
    void ClearAllBlockReasons() override;
    void GetBlockReasonsString(OUT AString& strOutLog) override;
    IMS_BOOL PrintBlockReasons() override;

    void GetBlockReasons(
            OUT ImsList<IMS_UINT32>& objReasons, IN SERVICE_TYPE eType = SERVICE_CELLULAR) override;

    IMS_BOOL IsReasonBlocked(IN BLOCK_REASON eReason, IN IMS_BOOL bOnlyEnabled = IMS_FALSE,
            IN SERVICE_TYPE eType = SERVICE_CELLULAR) override;

    IMS_BOOL IsCleared(IN SERVICE_TYPE eType = SERVICE_CELLULAR) override;

    static const IMS_CHAR* BlockReasonToString(IN IMS_UINT32 nReason);

protected:
    void Notify(IN BLOCK_REASON eReason, IN IMS_BOOL bIsEnable, IN IMS_BOOL bNotify = IMS_TRUE);
    static IMS_UINT32 GetBlockType(IN BLOCK_REASON eReason);
    static const IMS_CHAR* ServiceTypeToString(IN SERVICE_TYPE eType);

protected:
    enum
    {
        BLOCK_COMMON = 0,
        BLOCK_CELLULAR,
        BLOCK_WIFI
    };

    IAosAppContext* m_piAppContext;

    IMS_UINT32 BLOCK_ENABLED;
    BLOCK_REASON REASON[BLOCK_MAX];

    ImsHashMap m_objBlock;
    ImsHashMap m_objBlockCellular;
    ImsHashMap m_objBlockWifi;

    ImsList<IAosBlockListener*> m_objListeners;
    ImsList<IAosBlockSilentListener*> m_objSilentListeners;

    AString m_strTag;
    ImsList<IMS_UINT32> objServiceBlockReasons;
};

#endif  // AOS_BLOCK_H_
