/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef INTERFACE_MULTI_ENDPOINT_MANAGER_H_
#define INTERFACE_MULTI_ENDPOINT_MANAGER_H_

#include "AString.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"

struct MediaInfo;

/**
 * @brief An interface for managing multi-endpoint (MEP) functionality.
 *
 * This manager handles dialog event subscriptions and provides information about external calls
 * on other devices, which can be used for features like call pull.
 */
class IMultiEndpointManager
{
public:
    /** A struct that holds information about a dialog that can be pulled. */
    struct PullingDialogInfo final
    {
    public:
        PullingDialogInfo() :
                strCallId(AString::ConstNull()),
                strLocalTag(AString::ConstNull()),
                strRemoteTag(AString::ConstNull()),
                bHeld(IMS_FALSE),
                bPullable(IMS_FALSE),
                eCallType(CallType::UNKNOWN),
                pMediaInfo(IMS_NULL)
        {
        }

        PullingDialogInfo(IN const PullingDialogInfo& objRhs) :
                strCallId(objRhs.strCallId),
                strLocalTag(objRhs.strLocalTag),
                strRemoteTag(objRhs.strRemoteTag),
                bHeld(objRhs.bHeld),
                bPullable(objRhs.bPullable),
                eCallType(objRhs.eCallType),
                pMediaInfo(objRhs.pMediaInfo)
        {
        }

        ~PullingDialogInfo(){};
        PullingDialogInfo& operator=(IN const PullingDialogInfo&) = delete;

        AString strCallId;
        AString strLocalTag;
        AString strRemoteTag;
        IMS_BOOL bHeld;
        IMS_BOOL bPullable;
        CallType eCallType;
        const MediaInfo* pMediaInfo;
    };

    virtual ~IMultiEndpointManager() = default;

    /**
     * @brief Returns PullingDialogInfo if a dialog exists with the {@link Dialog#strId}.
     *
     * When an external call (DialogInfo) occurs, the IMS stack shares the corresponding ID with
     * Telephony. When Telephony attempts a call pull, it passes the hash code of that ID to the
     * IMS Stack. This function then finds the corresponding DialogInfo by comparing this hash
     * code.
     *
     * @param nId The hash code of a Dialog ID to find.
     * @return The PullingDialogInfo of the dialog ID.
     *         If no matching dialog exists, a PullingDialogInfo with empty value will be returned.
     */
    virtual PullingDialogInfo GetDialogInfo(IN IMS_UINT32 nId) const = 0;
};

#endif
