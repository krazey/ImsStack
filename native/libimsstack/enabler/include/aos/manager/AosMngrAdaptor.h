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
#ifndef AOS_MNGR_ADAPTOR_H_
#define AOS_MNGR_ADAPTOR_H_

#include "IMSTypeDef.h"
#include "IMSList.h"

#include "manager/ImsAosManager.h"

class AString;
class AosMngr;

class AosMngrAdaptor
    : public ImsAosManager
{
public:
    AosMngrAdaptor(IN const AString& strAppName, IN IMS_SINT32 nSlotId);
    virtual ~AosMngrAdaptor();

private:
    AosMngrAdaptor(IN const AosMngrAdaptor& objRhs);
    AosMngrAdaptor& operator=(IN const AosMngrAdaptor& objRhs);

public:
    virtual IImsAos* GetImsAos(IN const AString& strAppId, IN const AString& strServiceId);
    virtual IMSList<IImsAos*> GetImsAosList(IN const AString& strAppId,
            IN const AString& strServiceId);
    virtual IMSList<IImsAos*> GetImsAosList(IN const AString& strAppId);

private:
    IMS_SINT32 m_nSlotId;
    AosMngr* m_pAdaptee;
};
#endif // AOS_MNGR_ADAPTOR_H_
