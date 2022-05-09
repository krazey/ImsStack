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
 *  @brief        interface of the MMPF ( Android Interface / MMPF Manager Interface)
 */

#ifndef MMPF_IMMPF_H_INCLUDED
#define MMPF_IMMPF_H_INCLUDED

#include <mmpf/MMPFDefinition.h>

/*!
 *  @class        IMMPFListener
 *  @brief        abstract class for receiving notify from MMPF
 *                MMPF client should inherit this class and give it to the MMPF\
 *                for receiving notify from MMPF
 */

class IMMPFListener
{
public:
    virtual ~IMMPFListener() {}

public:
    // response per request
    virtual void OnResponse(const HMMPFSession hMMPFSession, const eMMPFRequest nRequest,
            const eMMPFResponse eResponse) = 0;
    // notify from mmpf
    virtual void OnNotify(const HMMPFSession hMMPFSession, const eMMPFNotify nNotify,
            const tMMPFNotificationData* pNotifyData = NULL) = 0;
};

/**
 *    @class        IMMPF
 *    @brief        Interface class of MMPF
 *                MMPF client can access the MMPF through this interface
 */
class IMMPF
{
public:
    virtual ~IMMPF() {}

public:
    // Get / Release interface
    static IMMPF* getInterface(MMPF_IN eMMPFInterfaceID eIfaceID = MMPF_INTERFACEID_AUTO);
    static eMMPFResult releaseInterface(MMPF_IN IMMPF* pMMPF);
    // Set / Request interfaces
    // If nSession is NULL, default session will be selected and used.
    virtual eMMPFResult setListener(MMPF_IN IMMPFListener* pListener) = 0;
    virtual eMMPFResult setProperty(MMPF_IN const HMMPFSession m_hMMPFSession,
            MMPF_IN const tMMPFProperty* pstProperty) = 0;
    virtual eMMPFResult request(
            MMPF_IN const HMMPFSession m_hMMPFSession, MMPF_IN const tMMPFRequest* pstRequest) = 0;
    // Media Session control methods
    // Single session application (ex. VT, VSC) doesn't need to call session control methods below.
    virtual HMMPFSession createMediaSession(eMMPFSessionType eSessionType) = 0;
    virtual eMMPFResult destroyMediaSession(MMPF_IN const HMMPFSession m_hMMPFSession) = 0;

public:
    static void CPInitialize();
};

#endif  // MMPF_IMMPF_H_INCLUDED
