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
#ifndef JNI_CONNECTOR_H_
#define JNI_CONNECTOR_H_

#include "ImsTypeDef.h"

/*
    EnablerService : (mandatory) the class which receives message from Jni
        (ex. AosService / EabApp / IMtcService / IMtsService)
    JniService : (mandatory) the JniService
        (ex. JniMtcService / JniAosService/ JniEabService / JniMtsService)
*/
template <typename EnablerService, typename JniService>
class JniConnector
{
public:
    inline JniConnector();
    inline virtual ~JniConnector();

    inline void SetEnablerService(IN EnablerService* pEnablerService)
    { m_pEnablerService = pEnablerService; }

    inline void SetJniService(IN JniService* pJniService)
    { m_pJniService = pJniService; }

    inline EnablerService* GetEnablerService();

    //EnablerService calls this to get JniService.
    inline JniService* GetJniService();

private:
    EnablerService* m_pEnablerService;
    JniService* m_pJniService;
};

PUBLIC
template <typename EnablerService, typename JniService> inline
JniConnector<EnablerService, JniService>::JniConnector() :
        m_pEnablerService(IMS_NULL),
        m_pJniService(IMS_NULL)
{
}

PUBLIC VIRTUAL
template <typename EnablerService, typename JniService> inline
JniConnector<EnablerService, JniService>::~JniConnector()
{
}

PUBLIC
template <typename EnablerService, typename JniService> inline
EnablerService* JniConnector<EnablerService, JniService>::GetEnablerService()
{
    return m_pEnablerService;
}

PUBLIC VIRTUAL
template <typename EnablerService, typename JniService> inline
JniService* JniConnector<EnablerService, JniService>::
        GetJniService()
{
    return m_pJniService;
}

#endif
