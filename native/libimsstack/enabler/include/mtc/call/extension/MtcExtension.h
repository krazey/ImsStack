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

#ifndef MTC_EXTENSION_H_
#define MTC_EXTENSION_H_

#include "AString.h"
#include "ImsTypeDef.h"
#include "call/extension/IMtcExtension.h"

class IMessage;

/**
 * This class provides basic methods for general extensions without any extension-specific logic.
 */
class MtcExtension : public IMtcExtension
{
public:
    explicit MtcExtension(IN const AString& strOptionTag);
    explicit MtcExtension(IN const MtcExtension& objRhs);
    virtual ~MtcExtension();
    MtcExtension& operator=(IN const MtcExtension&) = delete;

    IMtcExtension* Clone() const override;
    IMS_BOOL IsAvailableOnRemote() const override;
    IMS_BOOL IsRequiredOnRemote() const override;
    const AString& GetOptionTag() const override;

    void FormatRequest(IN RequestType eType, IN_OUT IMessage& objRequest) override;
    void FormatResponse(IN ResponseType eType, IN_OUT IMessage& objResponse) override;
    void HandleRequest(IN RequestType eType, IN const IMessage& objRequest) override;
    void HandleResponse(IN ResponseType eType, IN const IMessage& objResponse) override;

private:
    void UpdateFromRequireAndSupportedHeader(IN const IMessage& objMessage);

    AString m_strOptionTag;

    IMS_BOOL m_bRequiredOnRemote;
    IMS_BOOL m_bSupportedOnRemote;
};

#endif
