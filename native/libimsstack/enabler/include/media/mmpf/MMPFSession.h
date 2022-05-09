/**
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

/*!
 *    @brief        new interface of the MMPF
 */

#ifndef MMPF_MMPFSESSION_H_INCLUDED
#define MMPF_MMPFSESSION_H_INCLUDED

#include <mmpf/MMPFDefinition.h>
#include <mmpf/IMMPF.h>

/**
 *    @class        IMMPFSessionListener
 *
 *    @brief        abstract class for receiving notify from MMPF
 *                MMPF client should inherit this class and give it to the MMPF
 *                for receiving notify from MMPF
 */
class IMMPFSessionListener
{
public:
    virtual ~IMMPFSessionListener() {}

public:
    // response per request
    virtual void OnResponse(const HMMPFSession hMMPFSession, const eMMPFRequest nRequest,
            const eMMPFResponse eResponse) = 0;
    // notify from mmpf
    virtual void OnNotify(const HMMPFSession hMMPFSession, const eMMPFNotify nNotify,
            const tMMPFNotificationData* pNotifyData = NULL) = 0;
};

/**
 *    @class        MMPFSession
 *
 *    @brief        Interface class of MMPFSession
 *                MMPF client can access the MMPF through this interface
 */
class MMPFSession
{
public:
    static void Clear();
    static MMPFSession* create(MMPF_IN eMMPFInterfaceID eID = MMPF_INTERFACEID_MANAGER,
            MMPF_IN eMMPFSessionType eSessionType = MMPF_SESSION_CORE);
    static void destroy(MMPF_IN MMPFSession* pMMPFSession);
    eMMPFResult setListener(MMPF_IN IMMPFSessionListener* pListener);
    eMMPFResult setProperty(MMPF_IN const tMMPFProperty* pstProperty);
    eMMPFResult request(MMPF_IN const tMMPFRequest* pstRequest);

private:
    MMPFSession();
    ~MMPFSession();
    mmpf_bool Initialize(eMMPFSessionType eSessionType);

private:
    friend class MMPFSessionListener;
};

#endif  // MMPF_MMPFSESSION_H_INCLUDED
