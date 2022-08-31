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
#ifndef MOCK_I_OS_FACTORY_H_
#define MOCK_I_OS_FACTORY_H_

#include <gmock/gmock.h>

#include "IEventReceiver.h"
#include "IEventSender.h"
#include "IFile.h"
#include "IIpcan.h"
#include "INetworkIpSec.h"
#include "INetworkWatcher.h"
#include "IOsFactory.h"
#include "IPhoneInfoCall.h"
#include "IPhoneInfoDevice.h"
#include "IPhoneInfoLocation.h"
#include "IPhoneInfoPower.h"
#include "IPhoneInfoSubscriber.h"
#include "ISystemProperty.h"
#include "ISystemTime.h"
#include "ISystemUtil.h"
#include "IWifiWatcher.h"
#include "IZLib.h"
#include "ImsCarrierConfig.h"
#include "ImsFdSet.h"
#include "ImsFile.h"
#include "ImsIsim.h"
#include "ImsMutex.h"
#include "ImsNetworkConnection.h"
#include "ImsSocket.h"
#include "ImsThread.h"
#include "ImsTimer.h"
#include "ImsTrace.h"
#include "ImsUsim.h"
#include "SslCertificate.h"

class MockIOsFactory : public IOsFactory
{
public:
    inline MockIOsFactory() {}
    inline virtual ~MockIOsFactory() {}

    MOCK_METHOD(void, Destroy, (), (override));
    MOCK_METHOD(ImsTrace*, CreateTrace, (), (override));
    MOCK_METHOD(ImsMutex*, CreateMutex, (IN const AString& strName), (override));

    MOCK_METHOD(ImsFile*, CreateFile, (), (override));
    MOCK_METHOD(IFileUtil*, CreateFileUtil, (), (override));
    MOCK_METHOD(void, DestroyFileUtil, (IN IFileUtil * &piFileUtil), (override));

    MOCK_METHOD(ImsThread*, CreateThread, (IN const AString& strName, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(IMS_ULONG, GetCurrentThreadId, (), (override));

    MOCK_METHOD(ImsTimer*, CreateTimer, (), (override));
    MOCK_METHOD(ISystemTime*, CreateSystemTime, (), (override));
    MOCK_METHOD(void, DestroySystemTime, (IN ISystemTime * &piSysTime), (override));

    MOCK_METHOD(IEventReceiver*, CreateEventReceiver, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(void, DestroyEventReceiver, (IN IEventReceiver * &piEventReceiver), (override));
    MOCK_METHOD(IEventSender*, CreateEventSender, (), (override));
    MOCK_METHOD(void, DestroyEventSender, (IN IEventSender * &piEventSender), (override));

    // Platform: network
    MOCK_METHOD(ImsNetworkConnection*, CreateNetworkConnection,
            (IN const AString& strProfile, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(ImsNetworkConnection*, CreateNetworkConnection,
            (IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId), (override));

    MOCK_METHOD(INetworkIpSec*, CreateNetworkIpSec, (), (override));
    MOCK_METHOD(void, DestroyNetworkIpSec, (IN INetworkIpSec * &piIpSec), (override));

    MOCK_METHOD(IIpcan*, CreateIpcan, (), (override));
    MOCK_METHOD(void, DestroyIpcan, (IN IIpcan * &piIpcan), (override));

    MOCK_METHOD(ImsFdSet*, CreateFdSet, (IN IMS_SINT32 nType), (override));
    MOCK_METHOD(ImsSocket*, CreateSocket, (), (override));
    MOCK_METHOD(ImsSocket*, CreateSslSocket, (IN SslCertificate * pCertificate), (override));

    // Platform: utilities
    MOCK_METHOD(ISystemUtil*, GetSystemUtil, (), (override));
    MOCK_METHOD(ISystemProperty*, GetSystemProperty, (), (override));
    MOCK_METHOD(IZLib*, GetZLib, (), (override));

    // Platform: device
    MOCK_METHOD(IPowerInfo*, CreatePowerInfo, (), (override));
    MOCK_METHOD(void, DestroyPowerInfo, (IN IPowerInfo * &piPowerInfo), (override));

    MOCK_METHOD(IDeviceInfo*, CreateDeviceInfo, (), (override));
    MOCK_METHOD(void, DestroyDeviceInfo, (IN IDeviceInfo * &piDeviceInfo), (override));

    MOCK_METHOD(ISubscriberInfo*, CreateSubscriberInfo, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(void, DestroySubscriberInfo, (IN ISubscriberInfo * &piSubscriberInfo), (override));

    MOCK_METHOD(INetworkWatcher*, CreateNetworkWatcher, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(void, DestroyNetworkWatcher, (IN INetworkWatcher * &piNetworkWatcher), (override));

    MOCK_METHOD(IWifiWatcher*, CreateWifiWatcher, (), (override));
    MOCK_METHOD(void, DestroyWifiWatcher, (IN IWifiWatcher * &piWifiWatcher), (override));

    MOCK_METHOD(ICallInfo*, CreateCallInfo, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(void, DestroyCallInfo, (IN ICallInfo * &piCallInfo), (override));

    MOCK_METHOD(ILocationInfo*, CreateLocationInfo, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(void, DestroyLocationInfo, (IN ILocationInfo * &piLocationInfo), (override));

    MOCK_METHOD(ImsIsim*, CreateIsim, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(ImsUsim*, CreateUsim, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(ImsCarrierConfig*, CreateCarrierConfig, (IN IMS_SINT32 nSlotId), (override));

    MOCK_METHOD(ImsRadio*, CreateImsRadio, (IN IMS_SINT32 nSlotId), (override));
};

#endif
