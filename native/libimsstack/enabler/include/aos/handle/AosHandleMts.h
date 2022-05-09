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
#ifndef AOS_HANDLE_MTS_H_
#define AOS_HANDLE_MTS_H_

#include "handle/AosHandle.h"

class AosHandleMts : public AosHandle
{
    // Operation
public:
    AosHandleMts(IN IAosAppContext* piAppContext, IN const AString& strAppId,
            IN const AString& strServiceId, IN const IMS_SINT32 nServiceType);
    virtual ~AosHandleMts();

protected:
    virtual void Init();
    virtual void CleanUp();

    virtual void InitializeServiceBlock();
    virtual void InitializeServiceFeature();

    virtual void ProcessCapabilitiesChanged(
            IN const IMSMap<IMS_UINT32, IMS_UINT32>& objCapabilities);

private:
    IMS_BOOL m_bSmsOverIp;
};
#endif  // AOS_HANDLE_MTS_H_
