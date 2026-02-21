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
package com.android.imsstack.enabler.uce.impl;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Parcel;
import android.telecom.PhoneAccount;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.NativeStateInterface;
import com.android.imsstack.enabler.uce.impl.define.UceConstant;
import com.android.imsstack.enabler.uce.impl.define.UceFeatureTags;
import com.android.imsstack.enabler.uce.impl.define.UceMessage;
import com.android.imsstack.enabler.uce.impl.jni.IUceJNIListener;
import com.android.imsstack.enabler.uce.impl.jni.UceJNI;
import com.android.imsstack.enabler.uce.impl.options.UceOptionsResponseCallback;
import com.android.imsstack.enabler.uce.impl.publish.UcePublishRequestController;
import com.android.imsstack.enabler.uce.impl.subscribe.UceSubscribeRequestController;
import com.android.imsstack.enabler.uce.interf.PublishResponse;
import com.android.imsstack.enabler.uce.interf.SubscribeResponse;
import com.android.imsstack.enabler.uce.interf.UceApiConstant;
import com.android.imsstack.enabler.uce.interf.UceEventListener;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.stubbing.Answer;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Set;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class UceAgentTest extends ImsStackTest {
    private static final int MAX_WAIT_TIME = 1000;
    private static final int SLOT_ID = 0;
    private static final int testKey = 100;
    private static final String number = "123456";
    @Mock UceEventListener listener;
    @Mock PublishResponse publihsCb;
    @Mock SubscribeResponse subscribeCb;
    @Mock UcePublishRequestController publishController;
    @Mock UceSubscribeRequestController subscribeController;
    @Mock private NativeStateInterface mNativeStateInterface;

    private UceAgent mAgent;

    private TestUceJni mUceJni;

    private static class TestUceJni extends UceJNI {
        public IUceJNIListener mUceJniListener;
        private CountDownLatch mLatch;
        public TestUceJni() {
            super();
            mUceJniListener = null;
        }

        @Override
        public void init(int nSimSlot) {}

        @Override
        public void release(int nSimSlot) {}

        @Override
        public void addListener(int nSimSlot, IUceJNIListener mListener, int nMsgType) {
            mUceJniListener = mListener;
            if (mLatch != null) {
                mLatch.countDown();
            }
        }

        @Override
        public void sendMessage(int nSimSlot, Parcel parcel) {
            parcel.recycle();
            if (mLatch != null) {
                mLatch.countDown();
            }
        }

        public void setCountDownLatch(CountDownLatch cl) {
            mLatch = cl;
        }
    }

    @Before
    public void setUp(){
        MockitoAnnotations.initMocks(this);

        AgentFactory.getInstance()
                .setAgent(NativeStateInterface.class, mNativeStateInterface, SLOT_ID);

        mUceJni = new TestUceJni();
    }

    @After
    public void tearDown() throws Exception {
        AgentFactory.getInstance().setAgent(NativeStateInterface.class, null, SLOT_ID);

        mUceJni = null;
        if (mAgent != null) {
            mAgent.interrupt();
            mAgent = null;
        }

        super.tearDown();
    }

    @Test
    public void test_initialize() throws Exception {
        mAgent = createUceAgent();
        mAgent.mLoop = Looper.getMainLooper();

        mAgent.initialize();

        verify(mNativeStateInterface).addListener(any(NativeStateInterface.Listener.class));
    }

    @Test
    public void test_deInitialize() throws Exception {
        mAgent = createUceAgent();

        mAgent.deInitialize();

        verify(mNativeStateInterface).removeListener(any(NativeStateInterface.Listener.class));
    }

    @Test
    public void test_onNativeServiceReady() throws Exception {
        mAgent = createUceAgent();

        CountDownLatch lock = new CountDownLatch(1);
        mUceJni.setCountDownLatch(lock);

        mAgent.mNativeStateListener.onNativeServiceReady();

        assertTrue(lock.await(10, TimeUnit.SECONDS));
    }

    @Test
    public void test_setListener() throws Exception {
        mAgent = createUceAgent();
        mAgent.setImsRegistered(true);
        mAgent.setListener(listener);

        verify(listener).onRequestPublishCapabilities(anyInt());
        verifyNoMoreInteractions(listener);
    }

    @Test
    public void test_publishCapabilities() throws Exception {
        String pidfXml = "pidfXmlForTest";
        mAgent = createUceAgent();

        mAgent.setPublishController(null);
        mAgent.publishCapabilities(pidfXml, publihsCb);
        verify(publihsCb).onCommandError(eq(UceApiConstant.COMMAND_CODE_SERVICE_UNKNOWN));
        verifyNoMoreInteractions(publihsCb);

        mAgent.setPublishController(publishController);
        mAgent.publishCapabilities(pidfXml, publihsCb);
        verify(publishController).publishCapabilities(eq(pidfXml), eq(publihsCb));
        verifyNoMoreInteractions(publishController);
    }

    @Test
    public void test_subscribeCapabilities() throws Exception {
        Uri contact1 = Uri.fromParts(PhoneAccount.SCHEME_TEL, "123456789", null);
        Collection<Uri> uris = new ArrayList<>(1);
        uris.add(contact1);

        mAgent = createUceAgent();

        mAgent.setSubscribeController(null);
        mAgent.subscribeCapabilities(uris, subscribeCb);
        verify(subscribeCb).onCommandError(eq(UceApiConstant.COMMAND_CODE_SERVICE_UNKNOWN));
        verifyNoMoreInteractions(subscribeCb);

        mAgent.setSubscribeController(subscribeController);
        mAgent.subscribeCapabilities(uris, subscribeCb);
        verify(subscribeController).subscribeCapabilities(any(), eq(subscribeCb));
        verifyNoMoreInteractions(subscribeController);
    }

    @Test
    public void test_registered() throws Exception {
        mAgent = createUceAgent();
        mAgent.setListener(listener);
        mAgent.start();

        CountDownLatch lock = new CountDownLatch(1);
        mUceJni.setCountDownLatch(lock);
        lock.await(10, TimeUnit.SECONDS);

        mAgent.setPublishController(publishController);
        mAgent.setSubscribeController(subscribeController);

        Parcel parcel = Parcel.obtain();
        try {
            parcel.writeInt(UceMessage.UCE_IMS_AGENT_CONNECTED_IND);
            parcel.writeInt(UceConstant.RADIO_TECHNOLOGY_TYPE_LTE);
            parcel.writeLong(0);
            parcel.setDataPosition(0);
            mUceJni.mUceJniListener.onServiceStatusMessage(parcel);
        } finally {
            parcel.recycle();
        }
        waitForHandlerActionDelayed(mAgent.getHandler(), MAX_WAIT_TIME, 0);

        verify(publishController, timeout(MAX_WAIT_TIME)).setImsRegistrationStatus(eq(true));
        verify(publishController, timeout(MAX_WAIT_TIME)).setUseExpiredEtag(anyBoolean());
        verifyNoMoreInteractions(publishController);

        verify(subscribeController, timeout(MAX_WAIT_TIME)).setImsRegistrationStatus(eq(true));
        verifyNoMoreInteractions(subscribeController);

        verify(listener, timeout(MAX_WAIT_TIME)).onRequestPublishCapabilities(eq(
                UceApiConstant.CAPABILITY_UPDATE_TRIGGER_MOVE_TO_LTE_VOPS_ENABLED));
        verifyNoMoreInteractions(listener);
    }

    @Test
    public void test_deRegistered() throws Exception {
        mAgent = createUceAgent();
        mAgent.setListener(listener);
        mAgent.start();

        CountDownLatch lock = new CountDownLatch(1);
        mUceJni.setCountDownLatch(lock);
        lock.await(10, TimeUnit.SECONDS);

        mAgent.setPublishController(publishController);
        mAgent.setSubscribeController(subscribeController);

        Parcel parcel = Parcel.obtain();
        try {
            parcel.writeInt(UceMessage.UCE_IMS_AGENT_DISCONNECTED_IND);
            parcel.setDataPosition(0);
            mUceJni.mUceJniListener.onServiceStatusMessage(parcel);
        } finally {
            parcel.recycle();
        }
        waitForHandlerActionDelayed(mAgent.getHandler(), MAX_WAIT_TIME, 0);

        verify(publishController, timeout(MAX_WAIT_TIME)).setImsRegistrationStatus(eq(false));
        verify(subscribeController, timeout(MAX_WAIT_TIME)).setImsRegistrationStatus(eq(false));
        verifyNoMoreInteractions(publishController);
        verifyNoMoreInteractions(subscribeController);
    }

    @Test
    public void test_getCapabilityUpdateTriggerType() throws Exception {
        mAgent = createUceAgent();

        assertEquals(UceApiConstant.CAPABILITY_UPDATE_TRIGGER_MOVE_TO_2G,
                mAgent.getCapabilityUpdateTriggerType(UceConstant.RADIO_TECHNOLOGY_TYPE_GERAN));
        assertEquals(UceApiConstant.CAPABILITY_UPDATE_TRIGGER_MOVE_TO_3G,
                mAgent.getCapabilityUpdateTriggerType(UceConstant.RADIO_TECHNOLOGY_TYPE_HRPD));
        assertEquals(UceApiConstant.CAPABILITY_UPDATE_TRIGGER_MOVE_TO_3G,
                mAgent.getCapabilityUpdateTriggerType(UceConstant.RADIO_TECHNOLOGY_TYPE_UTRAN));
        assertEquals(UceApiConstant.CAPABILITY_UPDATE_TRIGGER_MOVE_TO_EHRPD,
                mAgent.getCapabilityUpdateTriggerType(UceConstant.RADIO_TECHNOLOGY_TYPE_EHRPD));
        assertEquals(UceApiConstant.CAPABILITY_UPDATE_TRIGGER_MOVE_TO_LTE_VOPS_ENABLED,
                mAgent.getCapabilityUpdateTriggerType(UceConstant.RADIO_TECHNOLOGY_TYPE_LTE));
        assertEquals(UceApiConstant.CAPABILITY_UPDATE_TRIGGER_MOVE_TO_LTE_VOPS_DISABLED,
                mAgent.getCapabilityUpdateTriggerType(
                        UceConstant.RADIO_TECHNOLOGY_TYPE_LTE_NO_VOPS));
        assertEquals(UceApiConstant.CAPABILITY_UPDATE_TRIGGER_MOVE_TO_IWLAN,
                mAgent.getCapabilityUpdateTriggerType(UceConstant.RADIO_TECHNOLOGY_TYPE_WIFI));
        assertEquals(UceApiConstant.CAPABILITY_UPDATE_TRIGGER_MOVE_TO_NR5G_VOPS_ENABLED,
                mAgent.getCapabilityUpdateTriggerType(UceConstant.RADIO_TECHNOLOGY_TYPE_NR));
        assertEquals(UceApiConstant.CAPABILITY_UPDATE_TRIGGER_MOVE_TO_NR5G_VOPS_DISABLED,
                mAgent.getCapabilityUpdateTriggerType(
                        UceConstant.RADIO_TECHNOLOGY_TYPE_NR_NO_VOPS));
    }

    @Test
    public void test_connectivityChangedFromImsDeregistered() throws Exception {
        mAgent = createUceAgent();
        mAgent.setListener(listener);
        mAgent.start();

        CountDownLatch lock = new CountDownLatch(1);
        mUceJni.setCountDownLatch(lock);
        lock.await(10, TimeUnit.SECONDS);

        mAgent.setImsRegistered(false);

        Parcel parcel = Parcel.obtain();
        try {
            parcel.writeInt(UceMessage.UCE_NETWORK_CHANGED);
            parcel.writeInt(UceConstant.RADIO_TECHNOLOGY_TYPE_LTE); // network type
            parcel.setDataPosition(0);
            mUceJni.mUceJniListener.onNetworkStatusMessage(parcel);
        } finally {
            parcel.recycle();
        }
        waitForHandlerActionDelayed(mAgent.getHandler(), MAX_WAIT_TIME, 0);

        verifyNoMoreInteractions(listener);
    }

    @Test
    public void test_connectivityNotChanged() throws Exception {
        mAgent = createUceAgent();
        mAgent.setListener(listener);
        mAgent.start();

        CountDownLatch lock = new CountDownLatch(1);
        mUceJni.setCountDownLatch(lock);
        lock.await(10, TimeUnit.SECONDS);

        mAgent.setImsRegistered(true);

        Parcel parcel = Parcel.obtain();
        try {
            parcel.writeInt(UceMessage.UCE_NETWORK_CHANGED);
            parcel.writeInt(UceConstant.RADIO_TECHNOLOGY_TYPE_UNKNOWN); // network type
            parcel.setDataPosition(0);
            mUceJni.mUceJniListener.onNetworkStatusMessage(parcel);
        } finally {
            parcel.recycle();
        }
        waitForHandlerActionDelayed(mAgent.getHandler(), MAX_WAIT_TIME, 0);

        verifyNoMoreInteractions(listener);
    }

    @Test
    public void test_connectivityChanged() throws Exception {
        mAgent = createUceAgent();
        mAgent.setListener(listener);
        mAgent.start();

        CountDownLatch lock = new CountDownLatch(7);
        mUceJni.setCountDownLatch(lock);
        lock.await(10, TimeUnit.SECONDS);
        Handler handler = mAgent.getHandler();
        waitForHandlerActionDelayed(handler, 100, 150);

        mAgent.setImsRegistered(true);

        Parcel parcel = Parcel.obtain();
        try {
            parcel.writeInt(UceMessage.UCE_NETWORK_CHANGED);
            parcel.writeInt(UceConstant.RADIO_TECHNOLOGY_TYPE_LTE); // network type
            parcel.setDataPosition(0);
            mUceJni.mUceJniListener.onNetworkStatusMessage(parcel);
        } finally {
            parcel.recycle();
        }
        waitForHandlerActionDelayed(handler, MAX_WAIT_TIME, 50);

        verify(listener).onRequestPublishCapabilities(eq(
                UceApiConstant.CAPABILITY_UPDATE_TRIGGER_MOVE_TO_LTE_VOPS_ENABLED));
    }

    @Test
    public void test_publishUpdated() throws Exception {
        long capability = 10;
        int responseCode = 200;
        String reason = "OK";
        int reasonHeaderCause = 100;
        String reasonHeaderText = "testWarning";
        int needToRetry = 0;

        mAgent = createUceAgent();
        mAgent.setListener(listener);
        mAgent.start();

        CountDownLatch lock = new CountDownLatch(1);
        mUceJni.setCountDownLatch(lock);
        lock.await(10, TimeUnit.SECONDS);

        mAgent.setPublishController(publishController);
        mAgent.setSubscribeController(subscribeController);

        Parcel parcel = Parcel.obtain();
        try {
            parcel.writeInt(UceMessage.UCE_PUBLISH_UPDATED_IND);
            parcel.writeLong(capability); // capability
            parcel.writeInt(responseCode); // responseCode
            parcel.writeString(reason); // reason
            parcel.writeInt(reasonHeaderCause); // reasonHeaderCause
            parcel.writeString(reasonHeaderText); // reasonHeaderText
            parcel.writeString(""); // etag
            parcel.writeInt(needToRetry); // needToRetry
            parcel.setDataPosition(0);
            mUceJni.mUceJniListener.onPublishStatusMessage(parcel);
        } finally {
            parcel.recycle();
        }
        waitForHandlerActionDelayed(mAgent.getHandler(), MAX_WAIT_TIME, 0);

        verify(publishController, timeout(MAX_WAIT_TIME)).setCapability(eq(capability));
        verify(publishController, timeout(MAX_WAIT_TIME)).handlePendingRequest();
        verifyNoMoreInteractions(publishController);
/*
        ArgumentCaptor<Integer> codeCaptor = ArgumentCaptor.forClass(Integer.class);
        ArgumentCaptor<String> reasonCaptor = ArgumentCaptor.forClass(String.class);
        ArgumentCaptor<Integer> reasonHeaderCauseCaptor = ArgumentCaptor.forClass(Integer.class);
        ArgumentCaptor<String> reasonHeaderTextCaptor = ArgumentCaptor.forClass(String.class);

        verify(listener, times(1)).onPublishUpdated(codeCaptor.capture(), reasonCaptor.capture(),
                reasonHeaderCauseCaptor.capture(), reasonHeaderTextCaptor.capture());

        assertTrue(codeCaptor.getValue().equals(responseCode));
        assertEquals(reasonCaptor.getValue(), reason);
        assertTrue(reasonHeaderCauseCaptor.getValue().equals(reasonHeaderCause));
        assertEquals(reasonHeaderTextCaptor.getValue(), reasonHeaderText);
 */
       verify(listener, times(1)).onPublishUpdated(eq(responseCode), eq(reason),
                eq(reasonHeaderCause), eq(reasonHeaderText));
    }

    @Test
    public void test_unpublished() throws Exception {
        mAgent = createUceAgent();
        mAgent.setListener(listener);
        mAgent.start();

        CountDownLatch lock = new CountDownLatch(1);
        mUceJni.setCountDownLatch(lock);
        lock.await(10, TimeUnit.SECONDS);

        mAgent.setPublishController(publishController);
        mAgent.setSubscribeController(subscribeController);

        Parcel parcel = Parcel.obtain();
        try {
            parcel.writeInt(UceMessage.UCE_UNPUBLISHED_IND);
            parcel.setDataPosition(0);
            mUceJni.mUceJniListener.onPublishStatusMessage(parcel);
        } finally {
            parcel.recycle();
        }
        waitForHandlerActionDelayed(mAgent.getHandler(), MAX_WAIT_TIME, 0);

        verify(publishController, timeout(MAX_WAIT_TIME)).deletePendingRequest();
        verify(listener, timeout(MAX_WAIT_TIME)).onUnPublish();
        verifyNoMoreInteractions(publishController);
        verifyNoMoreInteractions(listener);
    }

    @Test
    public void test_receivedOptions() throws Exception {
        mAgent = createUceAgent();
        mAgent.setListener(listener);
        mAgent.start();

        CountDownLatch lock = new CountDownLatch(1);
        mUceJni.setCountDownLatch(lock);
        lock.await(10, TimeUnit.SECONDS);

        mAgent.setPublishController(publishController);
        mAgent.setSubscribeController(subscribeController);

        Parcel parcel = Parcel.obtain();
        try {
            parcel.writeInt(UceMessage.UCE_OPTIONS_RECEIVED_IND);
            parcel.writeInt(testKey);
            parcel.writeString(number);
            parcel.writeLong(UceFeatureTags.Tags.FEATURE_TAG_PRESENCE.getCapa());
            parcel.setDataPosition(0);
            mUceJni.mUceJniListener.onReceivedRemoteOptionsMessage(parcel);
        } finally {
            parcel.recycle();
        }
        waitForHandlerActionDelayed(mAgent.getHandler(), MAX_WAIT_TIME, 0);

        doAnswer(new Answer<Void>() {
            @Override
            public Void answer(InvocationOnMock invocation) throws Throwable {
                //Object[] args = invocation.getArguments();
                Uri testUri = (Uri)invocation.getArguments()[0];
                //Uri testUri = (Uri)args[0];
                Set<String> remoteCapabilities = (Set<String>)invocation.getArguments()[1];
                UceOptionsResponseCallback callback =
                        (UceOptionsResponseCallback)invocation.getArguments()[2];
                assertEquals(testUri, Uri.parse(number));
                assertTrue(remoteCapabilities.contains(
                        UceFeatureTags.Tags.FEATURE_TAG_PRESENCE.getTag()));
                assertEquals(callback.mKey, testKey);
                return null;
            }
        }).when(listener).onRemoteCapabilityRequest(any(), any(), any());
    }

    private UceAgent createUceAgent() {
        return new UceAgent("UceTest", SLOT_ID, mUceJni);
    }

    private void waitForHandlerActionDelayed(Handler h, long timeoutMillis, long delayMs) {
        final CountDownLatch lock = new CountDownLatch(1);
        h.postDelayed(lock::countDown, delayMs);
        while (lock.getCount() > 0) {
            try {
                lock.await(timeoutMillis, TimeUnit.MILLISECONDS);
            } catch (InterruptedException e) {
                // do nothing
            }
        }
    }
}
