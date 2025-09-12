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
#ifndef PUBLICATION_REFRESH_HELPER_H_
#define PUBLICATION_REFRESH_HELPER_H_

#include "util/RefreshHelper.h"

class PubState;

class PublicationRefreshHelper : public RefreshHelper
{
public:
    PublicationRefreshHelper(IN IRefreshable* piRefreshable, IN const PubState* pPubState);
    ~PublicationRefreshHelper() override;

    PublicationRefreshHelper(IN const PublicationRefreshHelper&) = delete;
    PublicationRefreshHelper& operator=(IN const PublicationRefreshHelper&) = delete;

public:
    inline IMS_BOOL AddSpecificHeader(IN ISipConnection* /*piSc*/) override { return IMS_TRUE; }
    IMS_RESULT SendRefreshRequest(IN ISipClientConnection* piScc) override;
    IMS_RESULT UpdateOnMessageReceived(IN const ISipConnection* piSc) override;
    inline IMS_RESULT UpdateOnMessageSent(IN const ISipConnection* /*piSc*/) override
    {
        return IMS_SUCCESS;
    }

protected:
    void RefreshCompleted(IN ISipClientConnection* piScc, IN IMS_SINT32 nCode = 0) override;
    void RefreshStarted() override;
    inline void RefreshTerminated() override { Refreshable_RefreshTerminated(); }

private:
    const PubState* m_pPubState;
};

#endif
