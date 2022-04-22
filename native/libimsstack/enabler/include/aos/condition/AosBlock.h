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

#include "IMSHashMap.h"
#include "IMSList.h"
#include "interface/IAosBlock.h"

class IAosAppContext;
class IAosBlockListener;

class AosBlock
    : public IAosBlock
{
public:
    AosBlock(IN IAosAppContext* piAppContext);
    virtual ~AosBlock();

    virtual void SetListener(IN IAosBlockListener* piListener);
    virtual void RemoveListener(IN IAosBlockListener* piListener);

    virtual void SetBlockReason(IN BLOCK_REASON eReason, IN IMS_BOOL bNotify = IMS_TRUE);
    virtual void ResetBlockReason(IN BLOCK_REASON eReason, IN IMS_BOOL bNotify = IMS_TRUE);
    virtual void ClearAllBlockReasons();
    virtual void PrintBlockReasons();

    virtual void GetBlockReasons(OUT IMSList<IMS_UINT32> &objReasons,
            IN SERVICE_TYPE eType = SERVICE_CELLULAR);

    virtual IMS_BOOL IsReasonBlocked(IN BLOCK_REASON eReason,
            IN IMS_BOOL bOnlyEnabled = IMS_FALSE, IN SERVICE_TYPE eType = SERVICE_CELLULAR);

    virtual IMS_BOOL IsCleared(IN SERVICE_TYPE eType = SERVICE_CELLULAR);

    static const IMS_CHAR* BlockReasonToString(IN IMS_UINT32 nReason);

private :
    void Notify(IN BLOCK_REASON eReason, IN IMS_BOOL bIsEnable);
    static IMS_UINT32 GetBlockType(IN BLOCK_REASON eReason);
    static const IMS_CHAR* ServiceTypeToString(IN SERVICE_TYPE eType);

private:
    enum
    {
        BLOCK_COMMON = 0,
        BLOCK_CELLULAR,
        BLOCK_WIFI
    };

    IAosAppContext* m_piAppContext;

    IMS_UINT32 BLOCK_ENABLED;
    BLOCK_REASON REASON[BLOCK_MAX];

    IMSHashMap m_objBlock;
    IMSHashMap m_objBlockCellular;
    IMSHashMap m_objBlockWifi;

    IMSList<IAosBlockListener*> m_objListeners;

    AString m_strTag;
    IMSList<IMS_UINT32> objServiceBlockReasons;
};
#endif // AOS_BLOCK_H_
