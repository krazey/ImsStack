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
#include <algorithm>
#include <vector>

class IMessage;
class IMtcCallContext;

/**
 * This class provides basic methods for general extensions without any extension-specific logic.
 */
class MtcExtension : public IMtcExtension
{
public:
    MtcExtension(IN IMtcCallContext& objContext, IN const AString& strOptionTag,
            IN const std::vector<RequestType>& lstSupportedRequestType,
            IN const std::vector<ResponseType>& lstSupportedResponseType);
    MtcExtension(IN const MtcExtension& objRhs);
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

protected:
    IMtcCallContext& m_objContext;

private:
    const AString m_strOptionTag;
    const std::vector<RequestType> m_lstSupportedRequestType;
    const std::vector<ResponseType> m_lstSupportedResponseType;

    IMS_BOOL m_bRequiredOnRemote;
    IMS_BOOL m_bSupportedOnRemote;

    void UpdateFromRequireAndSupportedHeader(IN const IMessage& objMessage);

    inline IMS_BOOL IsSupportedType(IN RequestType eType) const
    {
        return std::find(m_lstSupportedRequestType.begin(), m_lstSupportedRequestType.end(),
                       eType) != m_lstSupportedRequestType.end();
    }

    inline IMS_BOOL IsSupportedType(IN ResponseType eType) const
    {
        return std::find(m_lstSupportedResponseType.begin(), m_lstSupportedResponseType.end(),
                       eType) != m_lstSupportedResponseType.end();
    }
};

#endif
