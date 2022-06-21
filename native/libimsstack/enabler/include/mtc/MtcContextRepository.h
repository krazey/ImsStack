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

#ifndef MTC_CONTEXT_REPOSITORY_H_
#define MTC_CONTEXT_REPOSITORY_H_

#include "IMSMap.h"
#include "IMSTypeDef.h"

class IMtcContext;

class MtcContextRepository
{
private:
    MtcContextRepository();
    ~MtcContextRepository();
    MtcContextRepository(IN const MtcContextRepository&);
    MtcContextRepository& operator=(IN const MtcContextRepository&);

public:
    static MtcContextRepository* GetInstance();

public:
    static IMtcContext* GetContext(IN IMS_SINT32 nSlotId = INVALID_SLOT_ID);
    IMtcContext* GetContextBySlot(IN IMS_SINT32 nSlotId);
    void AddContext(IN IMS_SINT32 nSlotId, IN IMtcContext* piContext);
    void RemoveContext(IN IMS_SINT32 nSlotId);

private:
    static MtcContextRepository* s_pThis;
    static const IMS_SINT32 INVALID_SLOT_ID = -1;
    IMSMap<IMS_SINT32, IMtcContext*> m_objContexts;
};

#endif
