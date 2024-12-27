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

package com.android.imsstack.test;

import static com.android.imsstack.base.TestAppContext.SLOT0;

import static com.google.common.truth.Truth.assertThat;

import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.mock;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyObject;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import androidx.test.filters.SmallTest;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.aos.IAosRegistration.CapabilityPairs;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;
import com.android.imsstack.enabler.aos.service.AosService;
import com.android.imsstack.enabler.mtc.IUMtcCall;
import com.android.imsstack.enabler.mtc.IncomingMtcCall;
import com.android.imsstack.enabler.mtc.MtcApp;
import com.android.imsstack.enabler.mtc.MtcCall;
import com.android.imsstack.imsservice.mmtel.ImsCallApp;
import com.android.imsstack.imsservice.mmtel.ImsCallManager;
import com.android.imsstack.imsservice.mmtel.ImsCallSessionImpl;
import com.android.imsstack.imsservice.mmtel.ImsServiceManager;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.system.SystemInterface;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class ImsTestHelperTest extends ImsStackTest {

    private static final String INTENT_AOS_TEST = "com.android.imsstack.action.INTENT_AOS_TEST";
    private static final String INTENT_SRVCC_TEST = "com.android.imsstack.action.INTENT_SRVCC_TEST";
    private static final String INTENT_MTC_TEST = "com.android.imsstack.action.INTENT_MTC_TEST";
    private static final String INTENT_QOS_TEST = "com.android.imsstack.action.INTENT_QOS_TEST";

    @Mock private AosService mMockAosService;
    @Mock private ImsServiceManager mImsServiceManager;
    @Mock private ImsCallApp mCallApp;

    private TestAppContext mTestAppContext;
    private BroadcastReceiver mBroadcastReceiver;
    private ImsTestHelper mImsTestHelper;

    @Captor ArgumentCaptor<CapabilityPairs> mChangeCapabilitiesCaptor;

    @Before
    public void setUp() throws Exception {
        super.setUp(ImsTestHelperTest.class.getSimpleName());
        MockitoAnnotations.initMocks(this);

        mTestAppContext = new TestAppContext(mContext);
        mTestAppContext.setUp();

        ImsServiceManager.setDefault(mImsServiceManager);
        when(mImsServiceManager.getCallApp(SLOT0)).thenReturn(mCallApp);
        AosFactory.getInstance().replaceService(SLOT0, mMockAosService);

        mImsTestHelper = ImsTestHelper.getInstance();
        mImsTestHelper.init();

        ArgumentCaptor<BroadcastReceiver> captor = ArgumentCaptor.forClass(BroadcastReceiver.class);
        verify(mTestAppContext.getBroadcastReceiverProxy())
                .registerReceiver(captor.capture(), any(IntentFilter.class));
        mBroadcastReceiver = captor.getValue();
    }

    @After
    public void tearDown() throws Exception {
        super.tearDown();

        if (mImsTestHelper != null) {
            mImsTestHelper.cleanup();
            mImsTestHelper = null;
        }

        ImsServiceManager.setDefault(null);
        AosFactory.getInstance().replaceService(SLOT0, null);
        mBroadcastReceiver = null;
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    @SmallTest
    public void sendCapabilitiesChanged() {
        Intent intent = new Intent();
        intent.setAction(INTENT_AOS_TEST);
        intent.putExtra("slotid", 0);
        intent.putExtra("event", "capa");
        intent.putExtra("network", "LTE,NR,IWLAN,UTRAN");
        intent.putExtra("video", "1,1,1,0");
        intent.putExtra("voice", "1,1,1,0");
        intent.putExtra("sms", "1,1,1,0");
        intent.putExtra("text", "1,1,1,0");
        intent.putExtra("call_composer", "1,1,0,0");
        intent.putExtra("call_composer_business_only", "1,1,0,0");
        mBroadcastReceiver.onReceive(mContext, intent);
        verify(mMockAosService).changeCapabilities(mChangeCapabilitiesCaptor.capture());

        CapabilityPairs capturedCapabilityPairs = mChangeCapabilitiesCaptor.getValue();
        CapabilityPairs expectedCapabilityPairs = new CapabilityPairs();
        expectedCapabilityPairs.addCapability(IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.VOICE
                | IAosRegistrationListener.Capability.VIDEO
                | IAosRegistrationListener.Capability.SMS
                | IAosRegistrationListener.Capability.TEXT
                | IAosRegistrationListener.Capability.CALL_COMPOSER
                | IAosRegistrationListener.Capability.CALL_COMPOSER_BUSINESS_ONLY);
        expectedCapabilityPairs.addCapability(IAosRegistrationListener.NetworkType.NR,
                IAosRegistrationListener.Capability.VOICE
                | IAosRegistrationListener.Capability.VIDEO
                | IAosRegistrationListener.Capability.SMS
                | IAosRegistrationListener.Capability.TEXT
                | IAosRegistrationListener.Capability.CALL_COMPOSER
                | IAosRegistrationListener.Capability.CALL_COMPOSER_BUSINESS_ONLY);
        expectedCapabilityPairs.addCapability(IAosRegistrationListener.NetworkType.IWLAN,
                IAosRegistrationListener.Capability.VOICE
                | IAosRegistrationListener.Capability.VIDEO
                | IAosRegistrationListener.Capability.SMS
                | IAosRegistrationListener.Capability.TEXT);
        expectedCapabilityPairs.addCapability(IAosRegistrationListener.NetworkType.UTRAN,
                IAosRegistrationListener.Capability.NONE);

        assertThat(capturedCapabilityPairs).isEqualTo(expectedCapabilityPairs);
    }

    @Test
    @SmallTest
    public void sendVopsChanged() throws Exception {
        SystemInterface systemInterface = Mockito.mock(SystemInterface.class);
        ISystem system = Mockito.mock(ISystem.class);
        SystemInterface.setSystemInterface(systemInterface);
        when(systemInterface.getSystem(SLOT0)).thenReturn(system);

        try {
            Intent intent = new Intent();
            intent.setAction(INTENT_AOS_TEST);
            intent.putExtra("event", "vops");
            intent.putExtra("state", ImsEventDef.IMS_EVENT_IMS_VOICE_OVER_PS_STATE);
            mBroadcastReceiver.onReceive(mContext, intent);
            verify(system).notifyEvent(ImsEventDef.IMS_EVENT_IMS_VOICE_OVER_PS_STATE,
                    ImsEventDef.IMS_EVENT_IMS_VOICE_OVER_PS_STATE,
                    0);
        } finally {
            SystemInterface.setSystemInterface(null);
        }
    }

    @Test
    @SmallTest
    public void sendQosChanged() {
        ImsCallManager manager = Mockito.mock(ImsCallManager.class);
        ImsCallSessionImpl imsCallSession = Mockito.mock(ImsCallSessionImpl.class);
        when(mCallApp.getCallManager()).thenReturn(manager);
        when(manager.getHoldSession()).thenReturn(imsCallSession);

        Intent intentQos = new Intent();
        intentQos.setAction(INTENT_QOS_TEST);
        intentQos.putExtra("type", -1);

        intentQos.putExtra("call", 10);
        intentQos.putExtra("media", 21);
        intentQos.putExtra("port", 5060);
        intentQos.putExtra("result", "OK");
        intentQos.putExtra("ipaddress", "10.20.10.30");
        intentQos.putExtra("type", 10);
        mBroadcastReceiver.onReceive(mContext, intentQos);
        verify(mImsServiceManager).getCallApp(SLOT0);
    }

    @Test
    @SmallTest
    public void sendSrvccEvent() {
        Intent intentSrv = new Intent();
        intentSrv.setAction(INTENT_SRVCC_TEST);
        intentSrv.putExtra("type", 2);
        mBroadcastReceiver.onReceive(mContext, intentSrv);
        verify(mImsServiceManager).getCallApp(SLOT0);
    }

    @Test
    @SmallTest
    public void sendMtcTestCommand_openEmergencyService() {
        MtcApp mtcApp = Mockito.mock(MtcApp.class);
        MtcCall mtcCall = Mockito.mock(MtcCall.class);
        ImsCallManager manager = Mockito.mock(ImsCallManager.class);
        when(mtcApp.createMtcCallAndAttach(MtcCall.FLAG_MO)).thenReturn(mtcCall);
        when(manager.getMtcApp()).thenReturn(mtcApp);
        when(mCallApp.getCallManager()).thenReturn(manager);

        int[] extra = new int[]{17, 1};
        Intent intentSrv = new Intent();
        intentSrv.setAction(INTENT_MTC_TEST);
        intentSrv.putExtra("slotid", SLOT0);
        intentSrv.putExtra("command", 100);
        intentSrv.putExtra("extras", extra);

        mBroadcastReceiver.onReceive(mContext, intentSrv);
        verify(mImsServiceManager).getCallApp(SLOT0);
        verify(mtcApp).openEmergencyService(anyObject(), anyInt());
    }

    @Test
    @SmallTest
    public void sendMtcTestCommand_openCall() {
        MtcApp mtcApp = Mockito.mock(MtcApp.class);
        MtcCall mtcCall = Mockito.mock(MtcCall.class);
        ImsCallManager manager = Mockito.mock(ImsCallManager.class);
        when(mtcApp.createMtcCallAndAttach(MtcCall.FLAG_MO)).thenReturn(mtcCall);
        when(manager.getMtcApp()).thenReturn(mtcApp);
        when(mCallApp.getCallManager()).thenReturn(manager);

        int[] extra = new int[]{16, 1, 1, 1, 1, 1};
        Intent intentSrv = new Intent();
        intentSrv.setAction(INTENT_MTC_TEST);
        intentSrv.putExtra("slotid", SLOT0);
        intentSrv.putExtra("command", 101);
        intentSrv.putExtra("extras", extra);

        //Mockito.reset(mImsServiceManager);
        mBroadcastReceiver.onReceive(mContext, intentSrv);
        verify(mImsServiceManager, times(1)).getCallApp(SLOT0);
        verify(mtcApp, times(1)).createMtcCallAndAttach(anyInt());
    }

    @Test
    @SmallTest
    public void sendMtcTestCommand_startCall() {
        MtcApp mtcApp = Mockito.mock(MtcApp.class);
        MtcCall mtcCall = Mockito.mock(MtcCall.class);
        ImsCallManager manager = Mockito.mock(ImsCallManager.class);
        when(mtcApp.createMtcCallAndAttach(MtcCall.FLAG_MO)).thenReturn(mtcCall);
        when(manager.getMtcApp()).thenReturn(mtcApp);
        when(mCallApp.getCallManager()).thenReturn(manager);

        openCall(MtcCall.FLAG_MO, IUMtcCall.SERVICETYPE_NORMAL, false, false, false);

        // start call
        int[] extra = new int[]{1, 3, -1, -1};
        Intent intentSrv = new Intent();
        intentSrv.setAction(INTENT_MTC_TEST);
        intentSrv.putExtra("slotid", SLOT0);
        intentSrv.putExtra("command", 102);
        intentSrv.putExtra("callee", "+1234567890");
        intentSrv.putExtra("extras", extra);

        mBroadcastReceiver.onReceive(mContext, intentSrv);
        verify(mtcCall).start(anyInt(), anyString(), anyString(), anyObject(), anyObject());
    }

    @Test
    @SmallTest
    public void sendMtcTestCommand_terminateCall() {
        MtcApp mtcApp = Mockito.mock(MtcApp.class);
        MtcCall mtcCall = Mockito.mock(MtcCall.class);
        ImsCallManager manager = Mockito.mock(ImsCallManager.class);
        when(mtcApp.createMtcCallAndAttach(MtcCall.FLAG_MO)).thenReturn(mtcCall);
        when(manager.getMtcApp()).thenReturn(mtcApp);
        when(mCallApp.getCallManager()).thenReturn(manager);

        openCall(MtcCall.FLAG_MO, IUMtcCall.SERVICETYPE_NORMAL, false, false, false);

        // terminate call
        int[] extra = new int[]{1};
        Intent intentSrv = new Intent();
        intentSrv.setAction(INTENT_MTC_TEST);
        intentSrv.putExtra("slotid", SLOT0);
        intentSrv.putExtra("command", 103);
        intentSrv.putExtra("extras", extra);
        mBroadcastReceiver.onReceive(mContext, intentSrv);
        verify(mtcCall).terminate(extra[0], true);
    }

    @Test
    @SmallTest
    public void sendMtcTestCommand_closeCall() {
        MtcApp mtcApp = Mockito.mock(MtcApp.class);
        MtcCall mtcCall = Mockito.mock(MtcCall.class);
        ImsCallManager manager = Mockito.mock(ImsCallManager.class);
        when(mtcApp.createMtcCallAndAttach(MtcCall.FLAG_MO)).thenReturn(mtcCall);
        when(manager.getMtcApp()).thenReturn(mtcApp);
        when(mCallApp.getCallManager()).thenReturn(manager);

        openCall(MtcCall.FLAG_MO, IUMtcCall.SERVICETYPE_NORMAL, false, false, false);

        // close call
        int[] extra = new int[]{0};
        Intent intentSrv = new Intent();
        intentSrv.setAction(INTENT_MTC_TEST);
        intentSrv.putExtra("slotid", SLOT0);
        intentSrv.putExtra("command", 104);
        intentSrv.putExtra("extras", extra);
        mBroadcastReceiver.onReceive(mContext, intentSrv);
        verify(mtcCall).close();
    }

    @Test
    @SmallTest
    public void sendMtcTestCommand_addListener() {
        MtcApp mtcApp = Mockito.mock(MtcApp.class);
        MtcCall mtcCall = Mockito.mock(MtcCall.class);
        ImsCallManager manager = Mockito.mock(ImsCallManager.class);
        when(mtcApp.createMtcCallAndAttach(MtcCall.FLAG_MO)).thenReturn(mtcCall);
        when(manager.getMtcApp()).thenReturn(mtcApp);
        when(mCallApp.getCallManager()).thenReturn(manager);

        Intent intentSrv = new Intent();
        intentSrv.setAction(INTENT_MTC_TEST);
        intentSrv.putExtra("slotid", SLOT0);
        intentSrv.putExtra("command", 106);
        mBroadcastReceiver.onReceive(mContext, intentSrv);
        verify(mtcApp).setCallListener(anyObject());
    }

    @Test
    @SmallTest
    public void sendMtcTestCommand_acceptCall() {
        MtcApp mtcApp = Mockito.mock(MtcApp.class);
        MtcCall mtcCall = Mockito.mock(MtcCall.class);
        ImsCallManager manager = Mockito.mock(ImsCallManager.class);
        when(mtcApp.getPendingCall(anyInt())).thenReturn(mtcCall);
        when(manager.getMtcApp()).thenReturn(mtcApp);
        when(mCallApp.getCallManager()).thenReturn(manager);

        // Captor MtcApp.CallListener
        long testNativeCallObject = 12345L;
        ArgumentCaptor<MtcApp.CallListener> captor =
                ArgumentCaptor.forClass(MtcApp.CallListener.class);
        doNothing().when(mtcApp).setCallListener(captor.capture());

        // addListener
        Intent intentSrv = new Intent();
        intentSrv.setAction(INTENT_MTC_TEST);
        intentSrv.putExtra("slotid", SLOT0);
        intentSrv.putExtra("command", 106);
        mBroadcastReceiver.onReceive(mContext, intentSrv);

        MtcApp.CallListener appCallListener = captor.getValue();

        // Call the onPreIncomingCallReceived to simulate the event.
        when(mtcApp.getPendingCall(testNativeCallObject)).thenReturn(mtcCall);
        appCallListener.onPreIncomingCallReceived(mtcApp, testNativeCallObject);
        verify(mtcApp).getPendingCall(testNativeCallObject);

        // Captor MtcCall.Listener
        ArgumentCaptor<MtcCall.Listener> callListenerCaptor =
                ArgumentCaptor.forClass(MtcCall.Listener.class);
        verify(mtcCall).setListener(callListenerCaptor.capture());
        MtcCall.Listener mtcCallListener = callListenerCaptor.getValue();

        // Create a mock for incoming call and call onCallIncomingReceived to simulate the event.
        IncomingMtcCall incomingCall = mock(IncomingMtcCall.class);
        mtcCallListener.onCallIncomingReceived(mtcCall, incomingCall, 0);

        // accept call
        int[] extra = new int[]{1, 3, -1, -1};
        intentSrv = new Intent();
        intentSrv.setAction(INTENT_MTC_TEST);
        intentSrv.putExtra("slotid", SLOT0);
        intentSrv.putExtra("command", 105);
        intentSrv.putExtra("extras", extra);
        mBroadcastReceiver.onReceive(mContext, intentSrv);
        verify(mtcCall).accept(anyInt(), anyObject());
    }

    @Test
    @SmallTest
    public void sendMtcTestCommand_setTerminalBasedTir() {
        MtcApp mtcApp = Mockito.mock(MtcApp.class);
        ImsCallManager manager = Mockito.mock(ImsCallManager.class);
        when(manager.getMtcApp()).thenReturn(mtcApp);
        when(mCallApp.getCallManager()).thenReturn(manager);

        int[] extra = new int[]{1};
        Intent intentSrv = new Intent();
        intentSrv.setAction(INTENT_MTC_TEST);
        intentSrv.putExtra("slotid", SLOT0);
        intentSrv.putExtra("command", 107);
        intentSrv.putExtra("extras", extra);
        mBroadcastReceiver.onReceive(mContext, intentSrv);
        verify(mtcApp).setTerminalBasedTir(true);
    }

    private void openCall(int attributes, int serviceType, boolean emergency, boolean offline,
            boolean ussi) {
        int[] extra = new int[]
                {attributes,
                serviceType,
                emergency ? 1 : 0,
                offline ? 1 : 0,
                ussi ? 1 : 0};
        Intent intentSrv = new Intent();
        intentSrv.setAction(INTENT_MTC_TEST);
        intentSrv.putExtra("slotid", SLOT0);
        intentSrv.putExtra("command", 101);
        intentSrv.putExtra("extras", extra);

        mBroadcastReceiver.onReceive(mContext, intentSrv);
    }
}
