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

package com.android.imsstack.core.agents;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.net.Uri;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.testing.TestableLooper;

import androidx.test.filters.SmallTest;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.util.SimUtils;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.Arrays;
import java.util.Set;

@RunWith(JUnit4.class)
public class UsatAgentTest {
    private static final int MAX_SIM_SLOT = 1;
    private static final int SLOT0 = 0;
    private static final int[] SUB_ID = { 1 };
    private static final byte[] USIM_SERVICE_TABLE =
            SimUtils.hexStringToBytes("000000FF00000000000000F0FF");
    private static final String SEND_ENVELOPE_OK = "9000";
    private static final String SEND_ENVELOPE_ERROR = "9300";
    /** MO SMS control */
    private static final byte[] SMS_TPDU = new byte[] {0, 10, 50, 15};
    private static final String TARGET_NUMBER = "9902219632";
    private static final String SMSC_DEST_ADDRESS = "+9876543";
    private static final String SMSC_ORIGIN_ADDRESS = "942563084";
    private static final String ORIGIN_ADDRESS = "995588443322";
    /** Call control */
    private static final String DIALED_STRING = "1234567890";

    @Mock private SimInterface mSimInterface;
    @Mock private Usat.Listener mListener;
    private ContextFixture mContextFixture;
    private TestableLooper mTestableLooper;
    private TelephonyManager mTelephonyManager;
    private UsatAgent mUsatAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mContextFixture = new ContextFixture();
        AppContext.init(mContextFixture.getTestDouble());
        SubscriptionManager sm =
                mContextFixture.getTestDouble().getSystemService(SubscriptionManager.class);
        when(sm.getSubscriptionIds(anyInt())).thenReturn(SUB_ID);

        mTelephonyManager =
                mContextFixture.getTestDouble().getSystemService(TelephonyManager.class);
        when(mTelephonyManager.createForSubscriptionId(anyInt())).thenReturn(mTelephonyManager);
        when(mTelephonyManager.getActiveModemCount()).thenReturn(MAX_SIM_SLOT);
        when(mTelephonyManager.getSupportedModemCount()).thenReturn(MAX_SIM_SLOT);

        when(mSimInterface.getSlotId()).thenReturn(SLOT0);

        mUsatAgent = new UsatAgent(mSimInterface);
        mTestableLooper = new TestableLooper(mUsatAgent.getLooper());
    }

    @After
    public void tearDown() throws Exception {
        if (mUsatAgent != null) {
            mUsatAgent.removeCallbacksAndMessages(null);
        }

        if (mTestableLooper != null) {
            mTestableLooper.destroy();
            mTestableLooper = null;
        }

        mUsatAgent = null;
        mTelephonyManager = null;
        mListener = null;
        mSimInterface = null;
        mContextFixture = null;
        AppContext.deinit();
    }

    @Test
    @SmallTest
    public void testCreateCallControlCommand() {
        Usat.CallControlCommand cmd = mUsatAgent.createCallControlCommand(
                Usat.CALL_CONTROL_TYPE_MO_CALL, DIALED_STRING,
                TelephonyManager.NETWORK_TYPE_LTE, Usat.MEDIA_TYPE_VIDEO, mListener);

        assertNotEquals(0, cmd.getCid());
        assertEquals(Usat.SERVICE_CALL_CONTROL, cmd.getServiceType());
        assertFalse(cmd.isAborted());
        assertEquals(mListener, cmd.getListener());
        assertEquals(Usat.CALL_CONTROL_TYPE_MO_CALL, cmd.getCcType());
        assertEquals(DIALED_STRING, cmd.getDialedString());
        assertEquals(TelephonyManager.NETWORK_TYPE_LTE, cmd.getNetworkType());
        assertEquals(Usat.MEDIA_TYPE_VIDEO, cmd.getMediaType());
    }

    @Test
    @SmallTest
    public void testCreateMoSmsControlCommand() {
        Usat.MoSmsControlCommand cmd = mUsatAgent.createMoSmsControlCommand(TARGET_NUMBER,
                SMSC_DEST_ADDRESS, TelephonyManager.NETWORK_TYPE_LTE, mListener);

        assertNotEquals(0, cmd.getCid());
        assertEquals(Usat.SERVICE_MO_SMS_CONTROL, cmd.getServiceType());
        assertFalse(cmd.isAborted());
        assertEquals(mListener, cmd.getListener());
        assertEquals(TARGET_NUMBER, cmd.getRpDestinationAddress());
        assertEquals(SMSC_DEST_ADDRESS, cmd.getTpDestinationAddress());
        assertEquals(TelephonyManager.NETWORK_TYPE_LTE, cmd.getNetworkType());
    }

    @Test
    @SmallTest
    public void testCreateSmsPpDownloadCommand() {
        Usat.SmsPpDownloadCommand cmd = mUsatAgent.createSmsPpDownloadCommand(SMSC_ORIGIN_ADDRESS,
                true, SMS_TPDU, ORIGIN_ADDRESS, mListener);

        assertNotEquals(0, cmd.getCid());
        assertEquals(Usat.SERVICE_DATA_DOWNLOAD_VIA_SMS_PP, cmd.getServiceType());
        assertFalse(cmd.isAborted());
        assertEquals(mListener, cmd.getListener());
        assertEquals(SMSC_ORIGIN_ADDRESS, cmd.getRpOriginatingAddress());
        assertEquals(ORIGIN_ADDRESS, cmd.getOriginatingAddress());
        assertTrue(Arrays.equals(SMS_TPDU, cmd.getTpdu()));
    }

    @Test
    @SmallTest
    public void testCreateRegEventDownloadCommand() {
        Set<Uri> impus = Set.of(Uri.parse("sip:test1@ims.com"), Uri.parse("sip:test2@ims.com"));

        Usat.RegEventDownloadCommand cmd = mUsatAgent.createRegEventDownloadCommand(
                200, impus, mListener);

        assertNotEquals(0, cmd.getCid());
        assertEquals(Usat.SERVICE_REGISTRATION_EVENT_DOWNLOAD, cmd.getServiceType());
        assertEquals(200, cmd.getStatusCode());
        assertTrue(impus.equals(cmd.getImpus()));
    }

    @Test
    @SmallTest
    public void testIsServiceAvailable_noUsimServiceTable() {
        when(mSimInterface.getUsimServiceTable()).thenReturn(null);

        assertFalse(mUsatAgent.isServiceAvailable(Usat.SERVICE_CALL_CONTROL));
        assertFalse(mUsatAgent.isServiceAvailable(Usat.SERVICE_MO_SMS_CONTROL));
        assertFalse(mUsatAgent.isServiceAvailable(Usat.SERVICE_DATA_DOWNLOAD_VIA_SMS_PP));
        assertFalse(mUsatAgent.isServiceAvailable(Usat.SERVICE_MEDIA_TYPE_SUPPORT));
        assertFalse(mUsatAgent.isServiceAvailable(Usat.SERVICE_REGISTRATION_EVENT_DOWNLOAD));
    }

    @Test
    @SmallTest
    public void testIsServiceAvailable_notSupported() {
        when(mSimInterface.getUsimServiceTable()).thenReturn(new byte[] { (byte) 0 });

        assertFalse(mUsatAgent.isServiceAvailable(Usat.SERVICE_CALL_CONTROL));
        assertFalse(mUsatAgent.isServiceAvailable(Usat.SERVICE_MO_SMS_CONTROL));
        assertFalse(mUsatAgent.isServiceAvailable(Usat.SERVICE_DATA_DOWNLOAD_VIA_SMS_PP));
        assertFalse(mUsatAgent.isServiceAvailable(Usat.SERVICE_MEDIA_TYPE_SUPPORT));
        assertFalse(mUsatAgent.isServiceAvailable(Usat.SERVICE_SUPPORT_OF_UICC_ACCESS_TO_IMS));
    }

    @Test
    @SmallTest
    public void testIsServiceAvailable_supported() {
        when(mSimInterface.getUsimServiceTable()).thenReturn(USIM_SERVICE_TABLE);

        assertTrue(mUsatAgent.isServiceAvailable(Usat.SERVICE_CALL_CONTROL));
        assertTrue(mUsatAgent.isServiceAvailable(Usat.SERVICE_MO_SMS_CONTROL));
        assertTrue(mUsatAgent.isServiceAvailable(Usat.SERVICE_DATA_DOWNLOAD_VIA_SMS_PP));
        assertTrue(mUsatAgent.isServiceAvailable(Usat.SERVICE_MEDIA_TYPE_SUPPORT));
        assertTrue(mUsatAgent.isServiceAvailable(Usat.SERVICE_SUPPORT_OF_UICC_ACCESS_TO_IMS));
    }

    @Test
    @SmallTest
    public void testCancelCommand() {
        Usat.CallControlCommand cmd = mUsatAgent.createCallControlCommand(
                Usat.CALL_CONTROL_TYPE_MO_CALL, DIALED_STRING,
                TelephonyManager.NETWORK_TYPE_LTE, Usat.MEDIA_TYPE_VIDEO, mListener);

        mUsatAgent.cancelCommand(cmd);

        assertFalse(cmd.isAborted());

        mUsatAgent.sendCommand(cmd);
        mUsatAgent.cancelCommand(cmd);

        assertTrue(cmd.isAborted());
    }

    @Test
    @SmallTest
    public void testSendCommand_callControlAllowed() {
        doReturn(SEND_ENVELOPE_OK).when(mTelephonyManager).sendEnvelopeWithStatus(any());

        Usat.CallControlCommand cmd = mUsatAgent.createCallControlCommand(
                Usat.CALL_CONTROL_TYPE_MO_CALL, DIALED_STRING,
                TelephonyManager.NETWORK_TYPE_LTE, Usat.MEDIA_TYPE_VIDEO, mListener);
        ArgumentCaptor<Usat.CommandResponse> cmdResponseCaptor =
                ArgumentCaptor.forClass(Usat.CommandResponse.class);

        mUsatAgent.sendCommand(cmd);
        processAllMessages();

        verify(mListener).onCommandResponse(cmdResponseCaptor.capture());

        Usat.CommandResponse cmdResponse = cmdResponseCaptor.getValue();
        assertEquals(Usat.RESULT_ALLOWED, cmdResponse.getResult());
        assertEquals(cmd, cmdResponse.getCommand());
    }

    @Test
    @SmallTest
    public void testSendCommand_callControlNotAllowed() {
        doReturn(SEND_ENVELOPE_ERROR).when(mTelephonyManager).sendEnvelopeWithStatus(any());

        Usat.CallControlCommand cmd = mUsatAgent.createCallControlCommand(
                Usat.CALL_CONTROL_TYPE_MO_CALL, DIALED_STRING,
                TelephonyManager.NETWORK_TYPE_LTE, Usat.MEDIA_TYPE_VIDEO, mListener);
        ArgumentCaptor<Usat.CommandResponse> cmdResponseCaptor =
                ArgumentCaptor.forClass(Usat.CommandResponse.class);

        mUsatAgent.sendCommand(cmd);
        processAllMessages();

        verify(mListener).onCommandResponse(cmdResponseCaptor.capture());

        Usat.CommandResponse cmdResponse = cmdResponseCaptor.getValue();
        assertEquals(Usat.RESULT_NOT_ALLOWED, cmdResponse.getResult());
        assertEquals(cmd, cmdResponse.getCommand());
    }

    @Test
    @SmallTest
    public void testSendCommand_smsPpDownloadOk() {
        doReturn(SEND_ENVELOPE_OK).when(mTelephonyManager).sendEnvelopeWithStatus(any());

        Usat.SmsPpDownloadCommand cmd = mUsatAgent.createSmsPpDownloadCommand(SMSC_ORIGIN_ADDRESS,
                true, SMS_TPDU, ORIGIN_ADDRESS, mListener);
        ArgumentCaptor<Usat.CommandResponse> cmdResponseCaptor =
                ArgumentCaptor.forClass(Usat.CommandResponse.class);

        mUsatAgent.sendCommand(cmd);
        processAllMessages();

        verify(mListener).onCommandResponse(cmdResponseCaptor.capture());

        Usat.CommandResponse cmdResponse = cmdResponseCaptor.getValue();
        assertEquals(Usat.RESULT_DATA_DOWNLOAD_OK, cmdResponse.getResult());
        assertEquals(cmd, cmdResponse.getCommand());
    }

    @Test
    @SmallTest
    public void testSendCommand_smsPpDownloadError() {
        doReturn(SEND_ENVELOPE_ERROR).when(mTelephonyManager).sendEnvelopeWithStatus(any());

        Usat.SmsPpDownloadCommand cmd = mUsatAgent.createSmsPpDownloadCommand(SMSC_ORIGIN_ADDRESS,
                true, SMS_TPDU, ORIGIN_ADDRESS, mListener);
        ArgumentCaptor<Usat.CommandResponse> cmdResponseCaptor =
                ArgumentCaptor.forClass(Usat.CommandResponse.class);

        mUsatAgent.sendCommand(cmd);
        processAllMessages();

        verify(mListener).onCommandResponse(cmdResponseCaptor.capture());

        Usat.CommandResponse cmdResponse = cmdResponseCaptor.getValue();
        assertEquals(Usat.RESULT_DATA_DOWNLOAD_ERROR, cmdResponse.getResult());
        assertEquals(cmd, cmdResponse.getCommand());
    }

    @Test
    @SmallTest
    public void testSendCommand_moSmsControlAllowed() {
        doReturn(SEND_ENVELOPE_OK).when(mTelephonyManager).sendEnvelopeWithStatus(any());

        Usat.MoSmsControlCommand cmd = mUsatAgent.createMoSmsControlCommand(TARGET_NUMBER,
                SMSC_DEST_ADDRESS, TelephonyManager.NETWORK_TYPE_LTE, mListener);
        ArgumentCaptor<Usat.CommandResponse> cmdResponseCaptor =
                ArgumentCaptor.forClass(Usat.CommandResponse.class);

        mUsatAgent.sendCommand(cmd);
        processAllMessages();

        verify(mListener).onCommandResponse(cmdResponseCaptor.capture());

        Usat.CommandResponse cmdResponse = cmdResponseCaptor.getValue();
        assertEquals(Usat.RESULT_ALLOWED, cmdResponse.getResult());
        assertEquals(cmd, cmdResponse.getCommand());
    }

    @Test
    @SmallTest
    public void testSendCommand_moSmsControlNotAllowed() {
        doReturn(SEND_ENVELOPE_ERROR).when(mTelephonyManager).sendEnvelopeWithStatus(any());

        Usat.MoSmsControlCommand cmd = mUsatAgent.createMoSmsControlCommand(TARGET_NUMBER,
                SMSC_DEST_ADDRESS, TelephonyManager.NETWORK_TYPE_LTE, mListener);
        ArgumentCaptor<Usat.CommandResponse> cmdResponseCaptor =
                ArgumentCaptor.forClass(Usat.CommandResponse.class);

        mUsatAgent.sendCommand(cmd);
        processAllMessages();

        verify(mListener).onCommandResponse(cmdResponseCaptor.capture());

        Usat.CommandResponse cmdResponse = cmdResponseCaptor.getValue();
        assertEquals(Usat.RESULT_NOT_ALLOWED, cmdResponse.getResult());
        assertEquals(cmd, cmdResponse.getCommand());
    }

    @Test
    @SmallTest
    public void testSendCommand_regEventDownloadAllowed() {
        doReturn(SEND_ENVELOPE_OK).when(mTelephonyManager).sendEnvelopeWithStatus(any());

        Set<Uri> impus = Set.of(Uri.parse("sip:test1@ims.com"), Uri.parse("sip:test2@ims.com"));

        Usat.RegEventDownloadCommand cmd = mUsatAgent.createRegEventDownloadCommand(
                200, impus, mListener);
        ArgumentCaptor<Usat.CommandResponse> cmdResponseCaptor =
                ArgumentCaptor.forClass(Usat.CommandResponse.class);

        mUsatAgent.sendCommand(cmd);
        processAllMessages();

        verify(mListener).onCommandResponse(cmdResponseCaptor.capture());

        Usat.CommandResponse cmdResponse = cmdResponseCaptor.getValue();
        assertEquals(Usat.RESULT_REGISTRATION_EVENT_OK, cmdResponse.getResult());
        assertEquals(cmd, cmdResponse.getCommand());
    }

    @Test
    @SmallTest
    public void testSendCommand_regEventDownloadNotAllowed() {
        doReturn(SEND_ENVELOPE_ERROR).when(mTelephonyManager).sendEnvelopeWithStatus(any());

        Set<Uri> impus = Set.of(Uri.parse("sip:test1@ims.com"), Uri.parse("sip:test2@ims.com"));

        Usat.RegEventDownloadCommand cmd = mUsatAgent.createRegEventDownloadCommand(
                200, impus, mListener);
        ArgumentCaptor<Usat.CommandResponse> cmdResponseCaptor =
                ArgumentCaptor.forClass(Usat.CommandResponse.class);

        mUsatAgent.sendCommand(cmd);
        processAllMessages();

        verify(mListener).onCommandResponse(cmdResponseCaptor.capture());

        Usat.CommandResponse cmdResponse = cmdResponseCaptor.getValue();
        assertEquals(Usat.RESULT_REGISTRATION_EVENT_ERROR, cmdResponse.getResult());
        assertEquals(cmd, cmdResponse.getCommand());
    }

    private void processAllMessages() {
        while (!mTestableLooper.getLooper().getQueue().isIdle()) {
            mTestableLooper.processAllMessages();
        }
    }
}
