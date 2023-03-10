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
#ifndef UCE_ASSET_ITEMS_H_
#define UCE_ASSET_ITEMS_H_

class UceAssetItems
{
public:
    explicit UceAssetItems() :
            m_nExpireValuePublish(0),
            m_nExtendedExpireValuePublish(0),
            m_nPublishRefreshRatio(80),
            m_nExpireValueListSubscribe(60),
            m_strRlsUri("pres-list=dummy-rfc5367"),
            m_bSubscribeIndepedentOfPublish(IMS_FALSE),
            m_nAnonymousFetchMethod(2),
            m_bEncodePublishBody(IMS_FALSE),
            m_bEncodeSubscribeBody(IMS_FALSE),
            m_bSupportOptions(IMS_FALSE),
            m_bUseContactHeaderInPublish(IMS_FALSE),
            m_bUseContactHeaderInSubscribe(IMS_FALSE),
            m_bAddVideoTagContactHeaderInPublish(IMS_FALSE),
            m_objImmediatelyRetryPublishResponse(ImsVector<IMS_SINT32>()),
            m_nImmediatelyRetryPublishResponseMaxCount(0),
            m_objRetryPublishResponse(ImsVector<IMS_SINT32>()),
            m_nRetryPublishResponseMaxCount(0),
            m_nRetryPublishResponseTimeSec(0),
            m_objVariableRetryPublishResponse(ImsVector<IMS_SINT32>()),
            m_nVariableRetryPublishResponseMaxCount(0),
            m_objVariableRetryPublishResponseTimeSec(ImsVector<IMS_SINT32>()),
            m_objReAttemptRegistrationPublishResponse(ImsVector<IMS_SINT32>()),
            m_objReAttemptRegistrationSubscribeResponse(ImsVector<IMS_SINT32>())
    {
        m_objImmediatelyRetryPublishResponse.Push(412);

        m_objRetryPublishResponse.Push(0);

        m_objVariableRetryPublishResponse.Push(0);

        m_objVariableRetryPublishResponseTimeSec.Push(0);

        m_objReAttemptRegistrationPublishResponse.Push(403);

        m_objReAttemptRegistrationSubscribeResponse.Push(403);
    }

public:
    IMS_UINT32 m_nExpireValuePublish;
    IMS_UINT32 m_nExtendedExpireValuePublish;
    IMS_UINT32 m_nPublishRefreshRatio;
    IMS_UINT32 m_nExpireValueListSubscribe;
    AString m_strRlsUri;
    IMS_BOOL m_bSubscribeIndepedentOfPublish;
    IMS_UINT32 m_nAnonymousFetchMethod;
    IMS_BOOL m_bEncodePublishBody;
    IMS_BOOL m_bEncodeSubscribeBody;
    IMS_BOOL m_bSupportOptions;
    IMS_BOOL m_bUseContactHeaderInPublish;
    IMS_BOOL m_bUseContactHeaderInSubscribe;
    IMS_BOOL m_bAddVideoTagContactHeaderInPublish;
    ImsVector<IMS_SINT32> m_objImmediatelyRetryPublishResponse;
    IMS_UINT32 m_nImmediatelyRetryPublishResponseMaxCount;
    ImsVector<IMS_SINT32> m_objRetryPublishResponse;
    IMS_UINT32 m_nRetryPublishResponseMaxCount;
    IMS_UINT32 m_nRetryPublishResponseTimeSec;
    ImsVector<IMS_SINT32> m_objVariableRetryPublishResponse;
    IMS_UINT32 m_nVariableRetryPublishResponseMaxCount;
    ImsVector<IMS_SINT32> m_objVariableRetryPublishResponseTimeSec;
    ImsVector<IMS_SINT32> m_objReAttemptRegistrationPublishResponse;
    ImsVector<IMS_SINT32> m_objReAttemptRegistrationSubscribeResponse;
};
#endif
