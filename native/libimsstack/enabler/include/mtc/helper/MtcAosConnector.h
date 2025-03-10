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

#ifndef MTC_AOS_CONNECTOR_H_
#define MTC_AOS_CONNECTOR_H_

#include "AString.h"
#include "IImsAos.h"
#include "IImsAosInfo.h"
#include "ImsTypeDef.h"
#include "helper/IMtcAosConnector.h"

class MtcAosConnector final : public IMtcAosConnector
{
public:
    inline MtcAosConnector(IN IImsAos& objImsAos, IN IImsAosInfo& objImsAosInfo) :
            m_objImsAos(objImsAos),
            m_objImsAosInfo(objImsAosInfo)
    {
    }
    virtual inline ~MtcAosConnector() {}
    MtcAosConnector(IN const MtcAosConnector&) = delete;
    MtcAosConnector& operator=(IN const MtcAosConnector&) = delete;

    // IImsAos interface wrappers.
    inline IMS_UINT32 GetFeatures() const override { return m_objImsAos.GetFeatures(); }
    inline IMS_UINT32 GetSuspendedReason() const override
    {
        return m_objImsAos.GetSuspendedReason();
    }
    inline IMS_BOOL IsFeatureConnected(IN IMS_UINT32 nFeature) const
    {
        return m_objImsAos.IsFeatureConnected(nFeature);
    }
    inline IMS_BOOL IsImsConnected() const override { return m_objImsAos.IsImsConnected(); }
    inline IMS_BOOL IsImsSuspended() const override { return m_objImsAos.IsImsSuspended(); }
    inline void SetReady(IN IMS_BOOL bReady, IN IMS_UINT32 nService) const override
    {
        m_objImsAos.SetReady(bReady, nService);
    }
    inline void UpdateFeature(IN IMS_UINT32 nFeatures) const override
    {
        m_objImsAos.UpdateFeature(nFeatures);
    }
    inline IMS_BOOL Control(IN IMS_UINT32 nType) const override
    {
        return m_objImsAos.Control(nType);
    }

    // IImsAosInfo interface wrappers.
    inline AString GetAssociatedUri() const override { return m_objImsAosInfo.GetAssociatedUri(); }
    inline IMS_UINT32 GetConnectionType() const override
    {
        return m_objImsAosInfo.GetConnectionType();
    }
    inline IMS_UINT32 GetImsState() const override { return m_objImsAosInfo.GetImsState(); }
    inline IMS_UINT32 GetIpcanType() const override { return m_objImsAosInfo.GetIpcanType(); }
    inline AString GetLastPathHeaderValue() const override
    {
        return m_objImsAosInfo.GetLastPathHeaderValue();
    }
    inline AString GetLocalAddress() const override { return m_objImsAosInfo.GetLocalAddress(); }
    inline IMS_UINT32 GetLocalPort() const override { return m_objImsAosInfo.GetLocalPort(); }
    inline IMS_UINT32 GetRegisteredNetworkType() const override
    {
        return m_objImsAosInfo.GetRegisteredNetworkType();
    }
    inline AString GetPathHeaderValue() const override
    {
        return m_objImsAosInfo.GetPathHeaderValue();
    }
    inline AString GetPcscfAddress() const override { return m_objImsAosInfo.GetPcscfAddress(); }
    inline IMS_UINT32 GetPcscfPort() const override { return m_objImsAosInfo.GetPcscfPort(); }
    inline IMS_UINT32 GetRegistrationMode() const override
    {
        return m_objImsAosInfo.GetRegistrationMode();
    }
    inline AString GetSupportedHeaderValue() const override
    {
        return m_objImsAosInfo.GetSupportedHeaderValue();
    }
    inline AString GetServiceRouteHeaderValue() const override
    {
        return m_objImsAosInfo.GetServiceRouteHeaderValue();
    }
    inline IMS_BOOL IsCrossSimConnected() const override
    {
        return m_objImsAosInfo.IsCrossSimConnected();
    }
    inline void NotifyEmergencyCallState(IN IMS_BOOL bIsInitialized) const override
    {
        return m_objImsAosInfo.NotifyEmergencyCallState(bIsInitialized);
    }

    inline void NotifyEpsfbCallState(IN IMS_UINT32 nState) const override
    {
        return m_objImsAosInfo.NotifyEpsfbCallState(nState);
    }

    inline void RegisterWithNextPcscf(IN IMS_UINT32 nUnavailableTimeForCurrentPcscf) const override
    {
        return m_objImsAos.RegisterWithNextPcscf(nUnavailableTimeForCurrentPcscf);
    }

private:
    IImsAos& m_objImsAos;
    IImsAosInfo& m_objImsAosInfo;
};

#endif
