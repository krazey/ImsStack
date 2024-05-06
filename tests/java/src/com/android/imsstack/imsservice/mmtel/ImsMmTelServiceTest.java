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

package com.android.imsstack.imsservice.mmtel;

import static android.telephony.PreciseCallState.PRECISE_CALL_STATE_INCOMING_SETUP;

import static com.android.imsstack.base.TestAppContext.SLOT0;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.clearInvocations;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.os.Handler;
import android.os.Looper;
import android.telecom.TelecomManager;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsExternalCallState;
import android.telephony.ims.MediaQualityStatus;
import android.telephony.ims.MediaThreshold;
import android.telephony.ims.SrvccCall;
import android.telephony.ims.feature.CapabilityChangeRequest;
import android.telephony.ims.feature.ImsFeature;
import android.telephony.ims.feature.MmTelFeature.MmTelCapabilities;
import android.telephony.ims.stub.ImsCallSessionImplBase;
import android.telephony.ims.stub.ImsEcbmImplBase;
import android.telephony.ims.stub.ImsMultiEndpointImplBase;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.telephony.ims.stub.ImsSmsImplBase;
import android.telephony.ims.stub.ImsUtImplBase;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.base.ContentProviderProxy.SettingsProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.imsservice.base.ImsContext;
import com.android.imsstack.imsservice.mmtel.sms.SmsTransferLayer;
import com.android.imsstack.imsservice.mmtel.ut.UtFactory;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.internal.imsservice.MmTelFeatureRegistry;
import com.android.imsstack.internal.imsservice.MmTelMediaRegistry;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.function.Consumer;

@RunWith(JUnit4.class)
public class ImsMmTelServiceTest extends ImsStackTest {
    @Mock private ImsContext mMockImsContext;
    @Mock private IBaseContext mMockBaseContext;
    @Mock private ImsCallContext mMockCallContext;
    @Mock private ImsServiceRegistry mMockServiceRegistry;
    @Mock private ImsServiceRecord mMockServiceRecord;
    @Mock private ImsRegistrationTracker mMockRegTracker;
    @Mock private IUtInterface mMockUtInterface;
    @Mock private ImsCallApp mMockImsCallApp;
    @Mock private SettingsProxy mGlobalSettings;

    private TestAppContext mTestAppContext;
    private ImsServiceManager mServiceManager;
    private MmTelFeatureRegistry mMmTelFeatureRegistry;
    private TestImsMmTelService mMmTelFeature;

    @Before
    public void setUp() throws Exception {
        super.setUp(ImsMmTelServiceTest.class.getSimpleName());
        MockitoAnnotations.initMocks(this);
        mTestAppContext = new TestAppContext(mContext);
        mTestAppContext.setUp();

        when(mMockImsContext.getContext()).thenReturn(mContext);
        when(mMockImsContext.getSlotId()).thenReturn(SLOT0);
        when(mMockImsContext.getExecutor()).thenReturn(Runnable::run);
        when(mMockBaseContext.getSlotId()).thenReturn(SLOT0);

        when(mTestAppContext.getContentProviderProxy().getGlobalSettings())
                .thenReturn(mGlobalSettings);
        UtFactory.getInstance().setUtInterfaceForSlot(SLOT0, mMockUtInterface);
        when(mMockServiceRecord.getRegistrationTracker()).thenReturn(mMockRegTracker);
        mServiceManager = new ImsServiceManager(mContext, null);
        mMmTelFeatureRegistry = ImsServiceRegistry.getInstance(SLOT0).getMmTelFeatureRegistry();
        mMmTelFeature = createMmTelService(mMockServiceRecord);
        mMmTelFeature.setDefaultExecutor(Runnable::run);
    }

    @After
    public void tearDown() throws Exception {
        super.tearDown();
        mMmTelFeature = null;
        ImsServiceManager.setDefault(null);
        if (mServiceManager != null) {
            mServiceManager.dispose();
            mServiceManager = null;
        }
        UtFactory.getInstance().setUtInterfaceForSlot(SLOT0, null);
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    public void testStart() {
        when(mMockServiceRecord.isServiceUp()).thenReturn(false);
        mMmTelFeature.start();
        verify(mMockServiceRecord, times(1)).setListener(any(ImsMmTelService.class));
        verify(mMockServiceRegistry, never()).setMmTelFeature(any(ImsMmTelService.class));
        verify(mMockRegTracker, times(1)).setCapabilityUpdateListener(any(ImsMmTelService.class));

        when(mMockServiceRecord.isServiceUp()).thenReturn(true);
        mMmTelFeature.start();
        verify(mMockServiceRecord, times(2)).setListener(any(ImsMmTelService.class));
        verify(mMockServiceRegistry).setMmTelFeature(any(ImsMmTelService.class));
        verify(mMockRegTracker, times(2)).setCapabilityUpdateListener(any(ImsMmTelService.class));
    }

    @Test
    public void testBinderDied() {
        mMockImsCallApp = null;
        mMmTelFeature = createMmTelService(null);
        mMmTelFeature.start();
        mMmTelFeature.binderDied();
        verify(mMockServiceRegistry, times(1)).setMmTelFeature(null);

        mMockImsCallApp = Mockito.mock(ImsCallApp.class);
        mMmTelFeature.binderDied();
        verify(mMockServiceRegistry, times(2)).setMmTelFeature(null);
        verify(mMockImsCallApp).onBinderDied();
    }

    @Test
    public void testOnServiceRecordStateChanged() {
        ImsServiceManager.setDefault(null);
        mMmTelFeature = createMmTelService(null);
        mMmTelFeature.onServiceRecordStateChanged();
        verify(mMockServiceRegistry, never()).setMmTelFeature(eq(null));

        mMockServiceRecord = Mockito.mock(ImsServiceRecord.class);
        mMmTelFeature = createMmTelService(mMockServiceRecord);
        when(mMockServiceRecord.isServiceUp()).thenReturn(true);
        mMmTelFeature.onServiceRecordStateChanged();
        verify(mMockServiceRegistry).setMmTelFeature(any(ImsMmTelService.class));

        when(mMockServiceRecord.isServiceUp()).thenReturn(false);
        mMmTelFeature.onServiceRecordStateChanged();
        verify(mMockServiceRegistry).setMmTelFeature(null);
    }

    @Test
    public void testQueryCapabilityConfiguration() {
        boolean result = mMmTelFeature.queryCapabilityConfiguration(
                MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                ImsRegistrationImplBase.REGISTRATION_TECH_NONE);
        assertFalse(result);

        result = mMmTelFeature.queryCapabilityConfiguration(
                MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE);
        assertTrue(result);

        result = mMmTelFeature.queryCapabilityConfiguration(
                MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE);
        assertTrue(result);

        result = mMmTelFeature.queryCapabilityConfiguration(
                MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                ImsRegistrationImplBase.REGISTRATION_TECH_NONE);
        assertFalse(result);

        result = mMmTelFeature.queryCapabilityConfiguration(
                MmTelCapabilities.CAPABILITY_TYPE_SMS,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE);
        assertFalse(result);

        result = mMmTelFeature.queryCapabilityConfiguration(
                MmTelCapabilities.CAPABILITY_TYPE_CALL_COMPOSER,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE);
        assertFalse(result);
    }

    @Test
    public void testChangeEnabledCapabilities() throws Exception {
        mMmTelFeature.changeEnabledCapabilities(null, null);
        verify(mMockRegTracker, never()).changeCapabilities(any(), any());
        verify(mMockUtInterface, never()).changeCapabilities(any(), any());

        CapabilityChangeRequest capabilityRequest = new CapabilityChangeRequest();
        capabilityRequest.addCapabilitiesToEnableForTech(MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE);

        capabilityRequest.addCapabilitiesToEnableForTech(MmTelCapabilities.CAPABILITY_TYPE_UT,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE);

        capabilityRequest.addCapabilitiesToDisableForTech(MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE);

        when(mMockServiceRecord.isServiceUp()).thenReturn(false);
        mMmTelFeature.start();
        mMmTelFeature.changeEnabledCapabilities(capabilityRequest, null);
        verify(mMockRegTracker).changeCapabilities(any(), any());
        verify(mMockUtInterface).changeCapabilities(any(), any());

        mMmTelFeature.onCapabilitiesUpdateFailed(MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                ImsRegistrationImplBase.REGISTRATION_TECH_NONE,
                ImsFeature.CAPABILITY_ERROR_GENERIC);
    }

    @Test
    public void testCreateCallProfile() {
        ImsCallProfile imsCallProfile = new ImsCallProfile();
        ImsCallProfile callProfile = mMmTelFeature.createCallProfile(
                ImsCallProfile.SERVICE_TYPE_NORMAL, ImsCallProfile.CALL_TYPE_VOICE);
        assertNull(callProfile);

        mMmTelFeature = createMmTelService(null);
        mMmTelFeature.start();
        when(mMockImsCallApp.createCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE)).thenReturn(imsCallProfile);
        callProfile = mMmTelFeature.createCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE);
        verify(mMockImsCallApp).createCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE);
        assertNotNull(callProfile);

        mMockImsCallApp = null;
        callProfile = mMmTelFeature.createCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE);
        assertNull(callProfile);
    }

    @Test
    public void testCreateCallSession() {
        ImsCallSessionImpl callSesison = Mockito.mock(ImsCallSessionImpl.class);
        ImsCallProfile imsCallProfile = new ImsCallProfile();
        ImsCallSessionImplBase session = mMmTelFeature.createCallSession(imsCallProfile);
        assertNull(session);

        mMmTelFeature = createMmTelService(null);
        mMmTelFeature.start();
        when(mMockImsCallApp.createCallSession(imsCallProfile)).thenReturn(callSesison);
        session = mMmTelFeature.createCallSession(imsCallProfile);
        assertNotNull(session);

        mMockImsCallApp = null;
        session = mMmTelFeature.createCallSession(imsCallProfile);
        assertNull(session);
    }

    @Test
    public void testGetUt() {
        ImsUtImplBase utImplBase = mMmTelFeature.getUt();
        ImsUtImpl utImpl = new ImsUtImpl(mMockBaseContext);
        assertNull(utImplBase);

        mMmTelFeature = createMmTelService(null);
        mMmTelFeature.start();
        when(mMockImsCallApp.getUtInterface()).thenReturn(utImpl);
        utImplBase = mMmTelFeature.getUt();
        assertNotNull(utImplBase);

        mMockImsCallApp = null;
        assertNull(mMmTelFeature.getUt());
    }

    @Test
    public void testGetEcbm() {
        ImsEcbmImplBase ecbmImplBase = mMmTelFeature.getEcbm();
        ImsEcbmImpl ecbmImpl = new ImsEcbmImpl(mMockCallContext);
        assertNull(ecbmImplBase);

        mMmTelFeature = createMmTelService(null);
        mMmTelFeature.start();
        when(mMockImsCallApp.getEcbmInterface()).thenReturn(ecbmImpl);
        ecbmImplBase = mMmTelFeature.getEcbm();
        assertNotNull(ecbmImplBase);

        mMockImsCallApp = null;
        assertNull(mMmTelFeature.getEcbm());
    }

    @Test
    public void testSetUiTtyMode() {
        mMmTelFeature.setUiTtyMode(TelecomManager.TTY_MODE_OFF, null);
        verify(mMockImsCallApp, never()).setTtyMode(TelecomManager.TTY_MODE_OFF);

        mMmTelFeature = createMmTelService(null);
        mMmTelFeature.start();
        mMmTelFeature.setUiTtyMode(TelecomManager.TTY_MODE_OFF, null);
        verify(mMockImsCallApp).setTtyMode(TelecomManager.TTY_MODE_OFF);
    }

    @Test
    public void testGetSmsImplementation() {
        ImsSmsImplBase smsImplBase = mMmTelFeature.getSmsImplementation();
        SmsTransferLayer smsTransFerLayer = Mockito.mock(SmsTransferLayer.class);
        String smsc = "+91987654321";
        ImsSmsImpl smsImpl = new ImsSmsImpl(mMockCallContext, smsTransFerLayer, smsc);
        assertNull(smsImplBase);

        mMmTelFeature = createMmTelService(null);
        mMmTelFeature.start();
        when(mMockImsCallApp.getSmsInterface()).thenReturn(smsImpl);
        smsImplBase = mMmTelFeature.getSmsImplementation();
        assertNotNull(smsImplBase);

        mMockImsCallApp = null;
        assertNull(mMmTelFeature.getSmsImplementation());
    }

    @Test
    public void testGetMultiEndpoint() {
        ImsMultiEndpointImpl multiEndpointImpl = new ImsMultiEndpointImpl(mMockCallContext);
        ImsMultiEndpointImplBase multiEndpointImplBase = mMmTelFeature.getMultiEndpoint();
        assertNotNull(multiEndpointImplBase);

        mMmTelFeature = createMmTelService(null);
        mMmTelFeature.start();
        when(mMockImsCallApp.getMultiEndpointInterface()).thenReturn(multiEndpointImpl);
        multiEndpointImplBase = mMmTelFeature.getMultiEndpoint();
        assertNotNull(multiEndpointImplBase);

        mMockImsCallApp = null;
        assertNull(mMmTelFeature.getMultiEndpoint());
    }

    @Test
    public void testOnFeatureRemoved() {
        mMmTelFeature.onFeatureRemoved();
        verify(mMockServiceRegistry, never()).setMmTelFeature(null);

        ConcurrentHashMap<Integer, ImsCallApp> callApps;
        callApps = mServiceManager.getCallAppMap();
        callApps.put(SLOT0, mMockImsCallApp);
        mMmTelFeature = createMmTelService(null);
        mMmTelFeature.start();
        ImsServiceManager.setDefault(mServiceManager);
        when(mMockImsContext.getPhoneId()).thenReturn(SLOT0);
        mMmTelFeature.onFeatureRemoved();
        verify(mMockServiceRegistry).setMmTelFeature(null);
        callApps.clear();
    }

    @Test
    public void testOnFeatureReady() {
        ImsServiceManager serviceManager = new ImsServiceManager(mContext, null);
        ImsServiceManager.setDefault(serviceManager);
        try {
            when(mMockImsContext.getPhoneId()).thenReturn(SLOT0);
            mMockImsCallApp = null;
            mMmTelFeature.onFeatureReady();
            assertNotNull(mMockImsCallApp);
        } finally {
            serviceManager.dispose();
            ImsServiceManager.setDefault(null);
        }
    }

    @Test
    public void testOnIncomingCallReceived() {
        assertThrows(IllegalArgumentException.class,
                () -> mMmTelFeature.makeIncomingCall(null));

        final ImsCallSessionImpl callSession = Mockito.mock(ImsCallSessionImpl.class);
        mMockImsCallApp =  null;
        assertThrows(IllegalArgumentException.class,
                () -> mMmTelFeature.makeIncomingCall(callSession));

        String callId = "1";
        mMockImsCallApp = Mockito.mock(ImsCallApp.class);
        when(mMockImsContext.getDefaultHandler()).thenReturn(new Handler(Looper.getMainLooper()));
        when(callSession.getProperty(eq(ImsCallProfile.EXTRA_USSD))).thenReturn(null, "true");
        when(callSession.getCallId()).thenReturn(callId);
        mMmTelFeature.makeIncomingCall(callSession);
        verify(mMockImsCallApp, times(1)).takeCallSession(callSession);
        verify(callSession, times(1)).alertUser();

        mMmTelFeature.makeIncomingCall(callSession);
        verify(mMockImsCallApp, times(2)).takeCallSession(callSession);
        verify(callSession, times(1)).alertUser();
    }

    @Test
    public void testOnImsExternalCallStateChanged() {
        ImsMultiEndpointImpl mockMultiEndpointImpl = Mockito.mock(ImsMultiEndpointImpl.class);
        when(mMockImsCallApp.getMultiEndpointInterface()).thenReturn(mockMultiEndpointImpl);
        List<ImsExternalCallState> states = new ArrayList<ImsExternalCallState>();
        mMmTelFeature.onStateChange(states);
        verify(mockMultiEndpointImpl).updateDialogState(states);
    }

    @Test
    public void testShouldProcessCall() {
        String[] number = {"IMS"};
        assertEquals(0, mMmTelFeature.shouldProcessCall(number));
    }

    @Test
    public void testNotifySrvccStates() {
        MmTelFeatureRegistry.Listener mMmtelFeatureListener =
                Mockito.mock(MmTelFeatureRegistry.Listener.class);
        mMmTelFeatureRegistry.addListener(mMmtelFeatureListener);

        ImsCallManager mockImsCallManager = Mockito.mock(ImsCallManager.class);
        when(mMockImsCallApp.getCallManager()).thenReturn(mockImsCallManager);

        SrvccCall srvccProfile = new SrvccCall("", PRECISE_CALL_STATE_INCOMING_SETUP,
                new ImsCallProfile());

        Consumer<List<SrvccCall>> consumer = value -> List.of(srvccProfile);
        mMmTelFeature.notifySrvccStarted(consumer);
        verify(mMmtelFeatureListener).onSrvccStateChanged(MmTelFeatureRegistry.SRVCC_STATE_STARTED);
        clearInvocations(mMmtelFeatureListener);

        mMmTelFeature.notifySrvccCompleted();
        verify(mMmtelFeatureListener)
                .onSrvccStateChanged(MmTelFeatureRegistry.SRVCC_STATE_COMPLETED);
        clearInvocations(mMmtelFeatureListener);

        mMmTelFeature.notifySrvccFailed();
        verify(mMmtelFeatureListener).onSrvccStateChanged(MmTelFeatureRegistry.SRVCC_STATE_FAILED);
        clearInvocations(mMmtelFeatureListener);

        mMmTelFeature.notifySrvccCanceled();
        verify(mMmtelFeatureListener)
                .onSrvccStateChanged(MmTelFeatureRegistry.SRVCC_STATE_CANCELED);

        mMmTelFeatureRegistry.removeListener(mMmtelFeatureListener);
    }

    @Test
    public void testSetMediaThreshold() {
        int sessionType = MediaQualityStatus.MEDIA_SESSION_TYPE_AUDIO;
        int[] packetLossRate = {1, 20};
        int[] jitterMillies = {20};
        long[] inactivityTimeMillies = {100, 7000};

        MediaThreshold mediaThreshold = new MediaThreshold.Builder()
                .setThresholdsRtpPacketLossRate(packetLossRate)
                .setThresholdsRtpJitterMillis(jitterMillies)
                .setThresholdsRtpInactivityTimeMillis(inactivityTimeMillies)
                .build();
        MmTelMediaRegistry mockMediaRegistry = Mockito.mock(MmTelMediaRegistry.class);
        when(mMockServiceRegistry.getMmTelMediaRegistry()).thenReturn(mockMediaRegistry);

        mMmTelFeature.setMediaThreshold(sessionType, mediaThreshold);
        verify(mockMediaRegistry).setMediaThreshold(sessionType, mediaThreshold);

        mMmTelFeature.setMediaThreshold(sessionType, null);
        verify(mockMediaRegistry).setMediaThreshold(sessionType, null);

        mMmTelFeature.clearMediaThreshold(sessionType);
        verify(mockMediaRegistry, times(2)).setMediaThreshold(sessionType, null);
    }

    private TestImsMmTelService createMmTelService(ImsServiceRecord sr) {
        TestImsMmTelService imsMmTelFeature = new TestImsMmTelService();

        if (sr != null) {
            ConcurrentHashMap<Integer, ImsServiceRecord> serviceRecords =
                    mServiceManager.getServiceRecordMap();
            ImsServiceManager.setDefault(mServiceManager);
            serviceRecords.put(SLOT0, sr);
        }

        return imsMmTelFeature;
    }

    private class TestImsMmTelService extends ImsMmTelService {
        TestImsMmTelService() {
            super(mMockImsContext, mMockServiceRegistry);
        }

        @Override
        public void changeEnabledCapabilities(CapabilityChangeRequest request,
                CapabilityCallbackProxy c) {
            c = new CapabilityCallbackProxy(null);
            ImsUtImpl utImpl = new ImsUtImpl(mMockBaseContext);
            when(mMockImsCallApp.getUtInterface()).thenReturn(utImpl);

            super.changeEnabledCapabilities(request, c);
        }

        public void makeIncomingCall(ImsCallSessionImpl callSesison) {
            mCallListener.onIncomingCallReceived(callSesison);
        }

        public void onStateChange(List<ImsExternalCallState> imsExternalCallStates) {
            mCallListener.onImsExternalCallStateChanged(imsExternalCallStates);
        }

        protected ImsCallApp createCallApp() {
            if (mMockImsCallApp != null) {
                return mMockImsCallApp;
            } else {
                mMockImsCallApp = Mockito.mock(ImsCallApp.class);
                return mMockImsCallApp;
            }
        }

        protected ImsCallApp getCallApp() {
            return mMockImsCallApp;
        }
    }
}
