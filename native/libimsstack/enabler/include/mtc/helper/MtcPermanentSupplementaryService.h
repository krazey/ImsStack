/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef MTC_PERMANENT_SUPPLEMENTARY_SERVICE_H_
#define MTC_PERMANENT_SUPPLEMENTARY_SERVICE_H_

#include "ImsList.h"
#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "utility/SuppServiceUtils.h"

class AString;

class MtcPermanentSupplementaryService final
{
public:
    MtcPermanentSupplementaryService();
    ~MtcPermanentSupplementaryService();
    MtcPermanentSupplementaryService(const MtcPermanentSupplementaryService&) = delete;
    MtcPermanentSupplementaryService& operator=(const MtcPermanentSupplementaryService&) = delete;

    void UpdateServices(IN const ImsList<SuppService*>& objSuppServices);
    IMS_BOOL IsEnabled(IN PermanentSuppType ePermanentSuppType) const;
    inline void Add(IN PermanentSuppType ePermanentSuppType, IN const AString& strValue)
    {
        SuppServiceUtils::Add(
                m_objPermanentSuppServices, static_cast<IMS_SINT32>(ePermanentSuppType), strValue);
    }
    inline void Add(IN PermanentSuppType ePermanentSuppType, IN IMS_SINT32 nValue)
    {
        SuppServiceUtils::Add(
                m_objPermanentSuppServices, static_cast<IMS_SINT32>(ePermanentSuppType), nValue);
    }
    inline void Add(IN PermanentSuppType ePermanentSuppType, IN IMS_BOOL bValue)
    {
        SuppServiceUtils::Add(
                m_objPermanentSuppServices, static_cast<IMS_SINT32>(ePermanentSuppType), bValue);
    }
    inline void Delete(IN PermanentSuppType ePermanentSuppType)
    {
        SuppServiceUtils::Delete(
                m_objPermanentSuppServices, static_cast<IMS_SINT32>(ePermanentSuppType));
    }
    inline void DeleteServices() { SuppServiceUtils::DeleteServices(m_objPermanentSuppServices); }
    inline const SuppService* Get(IN PermanentSuppType ePermanentSuppType) const
    {
        return SuppServiceUtils::Get(
                m_objPermanentSuppServices, static_cast<IMS_SINT32>(ePermanentSuppType));
    }
    inline const ImsList<SuppService*>& GetServices() const { return m_objPermanentSuppServices; }

private:
    ImsList<SuppService*> m_objPermanentSuppServices;
};
#endif
