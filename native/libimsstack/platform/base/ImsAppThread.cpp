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
#include "ImsAppThread.h"
#include "ImsMessageDef.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_BASE__;

PUBLIC
ImsAppThread::ImsAppThread() :
        BaseThread(),
        m_objApps(ImsList<ImsApp*>())
{
}

PUBLIC
void ImsAppThread::AddApp(IN ImsApp_Creator pfnCreator, IN const AString& strName)
{
    ControlAppAsync(PARAM_APP_CONTROL_ADD, strName, pfnCreator);
}

PUBLIC
void ImsAppThread::RemoveApp(IN const AString& strName)
{
    ControlAppAsync(PARAM_APP_CONTROL_REMOVE, strName);
}

PUBLIC
void ImsAppThread::RemoveAndDestroyApp(IN const AString& strName)
{
    ControlAppAsync(PARAM_APP_CONTROL_REMOVE_N_DESTROY, strName);
}

PROTECTED VIRTUAL void ImsAppThread::OnAppControl(IN IMS_SINT32 nParam, IN const AppInfo* pAppInfo)
{
    if (pAppInfo == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("OnAppControl :: param=%d, name=%s", nParam, pAppInfo->m_strName.GetStr(), 0);

    switch (nParam)
    {
        case PARAM_APP_CONTROL_ADD:
        {
            ImsApp* pApp = pAppInfo->m_pfnCreator(pAppInfo->m_strName);
            AttachApp(pApp);
            break;
        }
        case PARAM_APP_CONTROL_REMOVE:
            DetachApp(pAppInfo->m_strName);
            break;
        case PARAM_APP_CONTROL_REMOVE_N_DESTROY:
            DetachApp(pAppInfo->m_strName, IMS_TRUE);
            break;
        default:
            // no-op
            break;
    }

    delete pAppInfo;
}

PROTECTED
IMS_BOOL ImsAppThread::AttachApp(IN ImsApp* pApp)
{
    if (pApp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    m_objApps.Append(pApp);

    return IMS_TRUE;
}

PROTECTED
void ImsAppThread::DetachApp(IN const AString& strName, IN IMS_BOOL bDestroy /*= IMS_FALSE*/)
{
    for (IMS_UINT32 i = 0; i < m_objApps.GetSize(); ++i)
    {
        ImsApp* pApp = m_objApps.GetAt(i);

        if (strName.Equals(pApp->GetName()))
        {
            m_objApps.RemoveAt(i);

            if (bDestroy)
            {
                delete pApp;
            }
            break;
        }
    }
}

PROTECTED
void ImsAppThread::ControlAppAsync(IN IMS_SINT32 nParam, IN const AString& strName,
        IN ImsApp_Creator pfnCreator /*= IMS_NULL*/) const
{
    IThread* piThread = GetThread();

    if (piThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "No target thread - App(%s)", strName.GetStr(), 0, 0);
        return;
    }

    AppInfo* pAppInfo = new AppInfo(strName, pfnCreator);

    piThread->PostMessageI(IMS_MSG_APP_CONTROL, nParam, reinterpret_cast<IMS_UINTP>(pAppInfo));
}

PRIVATE
void ImsAppThread::UnloadAllApp()
{
    IMS_TRACE_D("UnloadAllApp", 0, 0, 0);

    m_objApps.Clear();
}

PRIVATE VIRTUAL IMS_BOOL ImsAppThread::Runnable_Run(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case IMS_MSG_START:
            if (!Initialize())
            {
                return IMS_FALSE;
            }

            return OnStart(objMsg);

        case IMS_MSG_TERMINATE:
            OnTerminate(objMsg);

            UnloadAllApp();
            Uninitialize();

            return IMS_TRUE;

        case IMS_MSG_APP_CONTROL:
            OnAppControl(LONG_TO_INT(objMsg.nWparam), reinterpret_cast<AppInfo*>(objMsg.nLparam));
            return IMS_TRUE;

        default:
            break;
    }

    if (!IsThreadMessage(objMsg))
    {
        return m_objActivityManager.HandleMessage(objMsg);
    }

    return OnMessage(objMsg);
}
