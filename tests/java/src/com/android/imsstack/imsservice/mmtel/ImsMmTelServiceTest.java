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

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.telecom.TelecomManager;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.aidl.IImsMmTelListener;
import android.telephony.ims.feature.CapabilityChangeRequest;
import android.telephony.ims.feature.MmTelFeature.MmTelCapabilities;
import android.telephony.ims.stub.ImsCallSessionImplBase;
import android.telephony.ims.stub.ImsEcbmImplBase;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.telephony.ims.stub.ImsSmsImplBase;
import android.telephony.ims.stub.ImsUtImplBase;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.enabler.IContext;
import com.android.imsstack.imsservice.base.ImsContext;
import com.android.imsstack.imsservice.mmtel.sms.SmsTransferLayer;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.util.MessageExecutor;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;

import java.util.concurrent.ConcurrentHashMap;

@RunWith(JUnit4.class)
public class ImsMmTelServiceTest extends ImsStackTest {
    private IContext mMockImsContext;
    private ImsCallContext mMockCallContext;
    private ImsServiceRegistry mMockServiceRegistry;
    private ImsServiceRecord mMockServiceRecord;
    private ImsRegistrationTracker mMockRegTracker;
    private ImsCallApp mMockImsCallApp;
    private IImsMmTelListener mMockMmTelListener;
    private ImsServiceManager mServiceManager;
    private TestImsMmTelService mMmTelFeatrure;
    private MessageExecutor mExecutor;

    @Before
    public void setUp() throws Exception {
        mMockImsContext = Mockito.mock(ImsContext.class);
        when(mMockImsContext.getContext()).thenReturn(mContext);
        when(mMockImsContext.getSlotId()).thenReturn(0);
        mMockServiceRegistry = Mockito.mock(ImsServiceRegistry.class);
        mMockServiceRecord = Mockito.mock(ImsServiceRecord.class);
        mMockRegTracker = Mockito.mock(ImsRegistrationTracker.class);
        mMockImsCallApp = Mockito.mock(ImsCallApp.class);
        mMockCallContext = Mockito.mock(ImsCallContext.class);
        mMockMmTelListener = Mockito.mock(IImsMmTelListener.class);

        mServiceManager = new ImsServiceManager(mContext, mExecutor);
        mMmTelFeatrure = createMmTelService(mMockServiceRecord);
        mExecutor = new MessageExecutor(ImsMmTelService.class.getSimpleName());
        when(mMockImsContext.getExecutor()).thenReturn(mExecutor);
        mMmTelFeatrure.setDefaultExecutor(mExecutor);
        mMmTelFeatrure.getBinder().setListener(mMockMmTelListener);
    }

    @After
    public void tearDown() throws Exception {
        mMmTelFeatrure = null;
        mServiceManager.getServiceRecordMap().clear();
        ImsServiceManager.setDefault(null);
        mServiceManager = null;
    }

    @Test
    public void testStart() {
        when(mMockServiceRecord.getRegistrationTracker()).thenReturn(mMockRegTracker);
        when(mMockServiceRecord.isServiceUp()).thenReturn(false);
        mMmTelFeatrure.start();
        verify(mMockServiceRecord, times(1)).setListener(any(ImsMmTelService.class));
        verify(mMockServiceRegistry, never()).setMmTelFeature(any(ImsMmTelService.class));
        verify(mMockRegTracker, times(1)).setCapabilityUpdateListener(any(ImsMmTelService.class));

        when(mMockServiceRecord.isServiceUp()).thenReturn(true);
        mMmTelFeatrure.start();
        verify(mMockServiceRecord, times(2)).setListener(any(ImsMmTelService.class));
        verify(mMockServiceRegistry).setMmTelFeature(any(ImsMmTelService.class));
        verify(mMockRegTracker, times(2)).setCapabilityUpdateListener(any(ImsMmTelService.class));
    }

    @Test
    public void testBinderDied() {
        mMockImsCallApp = null;
        mMmTelFeatrure = createMmTelService(null);
        mMmTelFeatrure.start();
        mMmTelFeatrure.binderDied();
        verify(mMockServiceRegistry, times(1)).setMmTelFeature(null);

        mMockImsCallApp = Mockito.mock(ImsCallApp.class);
        mMmTelFeatrure.binderDied();
        verify(mMockServiceRegistry, times(2)).setMmTelFeature(null);
        verify(mMockImsCallApp).onBinderDied();
    }

    @Test
    public void testOnServiceRecordStateChanged() {
        mMmTelFeatrure = createMmTelService(null);
        mMmTelFeatrure.onServiceRecordStateChanged();
        verify(mMockServiceRegistry, never()).setMmTelFeature(null);

        mMockServiceRecord = Mockito.mock(ImsServiceRecord.class);
        mMmTelFeatrure = createMmTelService(mMockServiceRecord);
        when(mMockServiceRecord.isServiceUp()).thenReturn(true);
        mMmTelFeatrure.onServiceRecordStateChanged();
        verify(mMockServiceRegistry).setMmTelFeature(any(ImsMmTelService.class));

        when(mMockServiceRecord.isServiceUp()).thenReturn(false);
        mMmTelFeatrure.onServiceRecordStateChanged();
        verify(mMockServiceRegistry).setMmTelFeature(null);
    }

    @Test
    public void testQueryCapabilityConfiguration() {
        boolean result = mMmTelFeatrure.queryCapabilityConfiguration(
                MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                ImsRegistrationImplBase.REGISTRATION_TECH_NONE);
        assertFalse(result);

        result = mMmTelFeatrure.queryCapabilityConfiguration(
                MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE);
        assertTrue(result);

        result = mMmTelFeatrure.queryCapabilityConfiguration(
                MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE);
        assertTrue(result);

        result = mMmTelFeatrure.queryCapabilityConfiguration(
                MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                ImsRegistrationImplBase.REGISTRATION_TECH_NONE);
        assertFalse(result);

        result = mMmTelFeatrure.queryCapabilityConfiguration(
                MmTelCapabilities.CAPABILITY_TYPE_SMS,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE);
        assertFalse(result);
    }

    @Test
    public void testChangeEnabledCapabilities() {
        mMmTelFeatrure.changeEnabledCapabilities(null, null);
        verify(mMockRegTracker, never()).changeCapabilities(any(), any());

        CapabilityChangeRequest capabilityRequest = new CapabilityChangeRequest();
        capabilityRequest.addCapabilitiesToEnableForTech(MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE);

        capabilityRequest.addCapabilitiesToDisableForTech(MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE);

        when(mMockServiceRecord.getRegistrationTracker()).thenReturn(mMockRegTracker);
        when(mMockServiceRecord.isServiceUp()).thenReturn(false);
        mMmTelFeatrure.start();
        mMmTelFeatrure.changeEnabledCapabilities(capabilityRequest, null);
        verify(mMockRegTracker).changeCapabilities(any(), any());
    }

    @Test
    public void testCreateCallProfile() {
        ImsCallProfile imsCallProfile = new ImsCallProfile();
        ImsCallProfile callProfile = mMmTelFeatrure.createCallProfile(
                ImsCallProfile.SERVICE_TYPE_NORMAL, ImsCallProfile.CALL_TYPE_VOICE);
        assertNull(callProfile);

        mMmTelFeatrure = createMmTelService(null);
        mMmTelFeatrure.start();
        when(mMockImsCallApp.createCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE)).thenReturn(imsCallProfile);
        callProfile = mMmTelFeatrure.createCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE);
        verify(mMockImsCallApp).createCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE);
        assertNotNull(callProfile);

        mMockImsCallApp = null;
        callProfile = mMmTelFeatrure.createCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE);
        assertNull(callProfile);
    }

    @Test
    public void testCreateCallSession() {
        ImsCallSessionImpl callSesison = Mockito.mock(ImsCallSessionImpl.class);
        ImsCallProfile imsCallProfile = new ImsCallProfile();
        ImsCallSessionImplBase session = mMmTelFeatrure.createCallSession(imsCallProfile);
        assertNull(session);

        mMmTelFeatrure = createMmTelService(null);
        mMmTelFeatrure.start();
        when(mMockImsCallApp.createCallSession(imsCallProfile)).thenReturn(callSesison);
        session = mMmTelFeatrure.createCallSession(imsCallProfile);
        assertNotNull(session);

        mMockImsCallApp = null;
        session = mMmTelFeatrure.createCallSession(imsCallProfile);
        assertNull(session);
    }

    @Test
    public void testGetUt() {
        ImsUtImplBase utImplBase = null;
        ImsUtImpl utImpl = new ImsUtImpl(mMockCallContext);
        assertNull(mMmTelFeatrure.getUt());

        mMmTelFeatrure = createMmTelService(null);
        mMmTelFeatrure.start();
        when(mMockImsCallApp.getUtInterface()).thenReturn(utImpl);
        utImplBase = mMmTelFeatrure.getUt();
        assertNotNull(utImplBase);

        mMockImsCallApp = null;
        assertNull(mMmTelFeatrure.getUt());
    }

    @Test
    public void testGetEcbm() {
        ImsEcbmImplBase ecbmImplBase = null;
        ImsEcbmImpl ecbmImpl = new ImsEcbmImpl(mMockCallContext);
        assertNull(mMmTelFeatrure.getEcbm());

        mMmTelFeatrure = createMmTelService(null);
        mMmTelFeatrure.start();
        when(mMockImsCallApp.getEcbmInterface()).thenReturn(ecbmImpl);
        ecbmImplBase = mMmTelFeatrure.getEcbm();
        assertNotNull(ecbmImplBase);

        mMockImsCallApp = null;
        assertNull(mMmTelFeatrure.getEcbm());
    }

    @Test
    public void testSetUiTtyMode() {
        mMmTelFeatrure.setUiTtyMode(TelecomManager.TTY_MODE_OFF, null);
        verify(mMockImsCallApp, never()).setTtyMode(TelecomManager.TTY_MODE_OFF);

        mMmTelFeatrure = createMmTelService(null);
        mMmTelFeatrure.start();
        mMmTelFeatrure.setUiTtyMode(TelecomManager.TTY_MODE_OFF, null);
        verify(mMockImsCallApp).setTtyMode(TelecomManager.TTY_MODE_OFF);
    }

    @Test
    public void testGetSmsImplementation() {
        ImsSmsImplBase smsImplBase = null;
        SmsTransferLayer smsTransFerLayer = Mockito.mock(SmsTransferLayer.class);
        ImsSmsImpl smsImpl = new ImsSmsImpl(mMockCallContext, smsTransFerLayer);
        smsImplBase = mMmTelFeatrure.getSmsImplementation();
        assertNull(smsImplBase);

        mMmTelFeatrure = createMmTelService(null);
        mMmTelFeatrure.start();
        when(mMockImsCallApp.getSmsInterface()).thenReturn(smsImpl);
        smsImplBase = mMmTelFeatrure.getSmsImplementation();
        assertNotNull(smsImplBase);

        mMockImsCallApp = null;
        assertNull(mMmTelFeatrure.getSmsImplementation());
    }

    @Test
    public void testOnFeatureRemoved() {
        mMmTelFeatrure.onFeatureRemoved();
        verify(mMockServiceRegistry, never()).setMmTelFeature(null);

        ConcurrentHashMap<Integer, ImsCallApp> callApps;
        callApps = mServiceManager.getCallAppMap();
        callApps.put(0, mMockImsCallApp);
        mMmTelFeatrure = createMmTelService(null);
        mMmTelFeatrure.start();
        ImsServiceManager.setDefault(mServiceManager);
        when(mMockImsContext.getPhoneId()).thenReturn(0);
        mMmTelFeatrure.onFeatureRemoved();
        verify(mMockServiceRegistry).setMmTelFeature(null);
        callApps.clear();
    }

    @Test
    public void testOnFeatureReady() {
        ImsServiceManager serviceManager = new ImsServiceManager(mContext, mExecutor);
        ImsServiceManager.setDefault(serviceManager);
        when(mMockImsContext.getPhoneId()).thenReturn(0);
        mMockImsCallApp = null;
        mMmTelFeatrure.onFeatureReady();
        assertNotNull(mMockImsCallApp);
        ImsServiceManager.setDefault(null);
    }

    @Test
    public void testOnIncomingCallReceived() {
        final ImsCallSessionImpl callSesison;
        assertThrows(IllegalArgumentException.class,
                () ->  mMmTelFeatrure.makeIncomingCall(null));

        callSesison = Mockito.mock(ImsCallSessionImpl.class);
        mMockImsCallApp =  null;
        assertThrows(IllegalArgumentException.class,
                () ->  mMmTelFeatrure.makeIncomingCall(callSesison));

        mMockImsCallApp = Mockito.mock(ImsCallApp.class);
        when(callSesison.getProperty(ImsCallProfile.EXTRA_USSD)).thenReturn(null);
        mMmTelFeatrure.makeIncomingCall(callSesison);
        verify(mMockImsCallApp, times(1)).takeCallSession(callSesison);
        verify(callSesison, times(1)).alertUser();

        when(callSesison.getProperty(ImsCallProfile.EXTRA_USSD)).thenReturn("true");
        mMmTelFeatrure.makeIncomingCall(callSesison);
        verify(mMockImsCallApp, times(2)).takeCallSession(callSesison);
        verify(callSesison, times(1)).alertUser();
    }

    private TestImsMmTelService createMmTelService(ImsServiceRecord sr) {
        TestImsMmTelService imsMmTelFeature = new TestImsMmTelService();

        if (sr != null) {
            ConcurrentHashMap<Integer, ImsServiceRecord> serviceRecords = null;
            ImsServiceManager.setDefault(mServiceManager);
            serviceRecords = mServiceManager.getServiceRecordMap();
            serviceRecords.put(0, sr);
        } else {
            ImsServiceManager.setDefault(null);
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
            super.changeEnabledCapabilities(request, c);
        }

        public void makeIncomingCall(ImsCallSessionImpl callSesison) {
            mCallListener.onIncomingCallReceived(callSesison);
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
