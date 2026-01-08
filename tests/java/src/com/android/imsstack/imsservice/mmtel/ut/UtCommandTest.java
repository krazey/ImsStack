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

package com.android.imsstack.imsservice.mmtel.ut;

import static com.android.imsstack.base.TestAppContext.SLOT0;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.telephony.ims.ImsReasonInfo;

import com.android.imsstack.core.agents.Usat;
import com.android.imsstack.core.agents.UsatInterface;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.ssc.SscConstant;
import com.android.imsstack.enabler.ssc.SscServiceClassUtil;
import com.android.imsstack.imsservice.mmtel.ImsUtImpl;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtListener;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class UtCommandTest {
    private ImsUtImpl mImsUtImpl;

    @Mock private IBaseContext mMockBaseContext;
    @Mock private UsatInterface mMockUsatInterface;
    @Mock private Usat.CallControlCommandResponse mMockUsatCmdRes;
    @Mock private IUtListener mMockUtListener;
    @Mock private IUtInterface mMockUtInterface;

    @Captor ArgumentCaptor<ImsReasonInfo> mReasonInfoCaptor;
    @Captor ArgumentCaptor<String> mStringCaptor;
    @Captor ArgumentCaptor<Usat.Listener> mUsatListenerCaptor;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        when(mMockBaseContext.getSlotId()).thenReturn(SLOT0);
        when(mMockBaseContext.getUsatInterface()).thenReturn(mMockUsatInterface);
        when(mMockUsatInterface.isServiceAvailable(Usat.SERVICE_CALL_CONTROL)).thenReturn(true);
        when(mMockUsatInterface.createCallControlCommand(anyInt(), anyString(), anyInt(), anyInt(),
                any())).thenReturn(null);
        when(mMockUsatCmdRes.getResult()).thenReturn(Usat.RESULT_ALLOWED);

        UtFactory.getInstance().setUtInterfaceForSlot(SLOT0, mMockUtInterface);

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        mImsUtImpl.init();
    }

    @After
    public void tearDown() {
        mImsUtImpl.clear();
        UtFactory.getInstance().setUtInterfaceForSlot(SLOT0, null);
    }

    @Test
    public void startTransaction_invalidServiceClass() {
        int transactionId = 1;
        int condition = SscConstant.CONDITION_BAOC;
        int invalidServiceClass = SscServiceClassUtil.SERVICE_CLASS_FAX;
        when(mMockUsatInterface.isServiceAvailable(Usat.SERVICE_CALL_CONTROL)).thenReturn(false);

        UtCommand utCmd = new UtCommand.Builder(mMockBaseContext, transactionId, UtCommand.CMD_CB,
                SscConstant.ACTION_INTERROGATION, mMockUtListener)
                .setCondition(condition).setServiceClass(invalidServiceClass).build();
        utCmd.startTransaction();

        verify(mMockUtListener).utConfigurationQueryFailed(eq(transactionId),
                mReasonInfoCaptor.capture());
        ImsReasonInfo reasonInfo = mReasonInfoCaptor.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, reasonInfo.getCode());
    }

    @Test
    public void startTransaction_queryCallBarring() {
        int condition = SscConstant.CONDITION_BAIC;

        int transactionId = mImsUtImpl.queryCallBarring(condition);

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                mStringCaptor.capture(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        verify(mMockUsatInterface).sendCommand(any());

        String dialedString = mStringCaptor.getValue();
        assertEquals("*#35#", dialedString);
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtInterface).queryCallBarringForServiceClass(eq(transactionId), eq(condition),
                eq(SscServiceClassUtil.SERVICE_CLASS_NONE));
    }

    @Test
    public void startTransaction_queryCallBarringForServiceClass() {
        int condition = SscConstant.CONDITION_BAOC;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_VIDEO;

        int transactionId = mImsUtImpl.queryCallBarringForServiceClass(condition, serviceClass);

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                mStringCaptor.capture(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        verify(mMockUsatInterface).sendCommand(any());

        String dialedString = mStringCaptor.getValue();
        assertEquals("*#33#", dialedString);
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtInterface).queryCallBarringForServiceClass(eq(transactionId), eq(condition),
                eq(serviceClass));
    }

    @Test
    public void startTransaction_queryCallForward() {
        int condition = SscConstant.CONDITION_CFU;
        String number = "12345678901";

        int transactionId = mImsUtImpl.queryCallForward(condition, number);

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                mStringCaptor.capture(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        verify(mMockUsatInterface).sendCommand(any());

        String dialedString = mStringCaptor.getValue();
        assertEquals("*#21#", dialedString);
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtInterface).queryCallForward(eq(transactionId), eq(condition),
                eq(number));
    }

    @Test
    public void startTransaction_queryCallWaiting() {
        int transactionId = mImsUtImpl.queryCallWaiting();

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                mStringCaptor.capture(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        verify(mMockUsatInterface).sendCommand(any());

        String dialedString = mStringCaptor.getValue();
        assertEquals("*#43#", dialedString);
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtInterface).queryCallWaiting(eq(transactionId));
    }

    @Test
    public void startTransaction_queryClir() {
        int transactionId = mImsUtImpl.queryCLIR();

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                mStringCaptor.capture(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        verify(mMockUsatInterface).sendCommand(any());

        String dialedString = mStringCaptor.getValue();
        assertEquals("*#31#", dialedString);
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtInterface).queryCLIR(eq(transactionId));
    }

    @Test
    public void startTransaction_queryClip() {
        int transactionId = mImsUtImpl.queryCLIP();

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                mStringCaptor.capture(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        verify(mMockUsatInterface).sendCommand(any());

        String dialedString = mStringCaptor.getValue();
        assertEquals("*#30#", dialedString);
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtInterface).queryCLIP(eq(transactionId));
    }

    @Test
    public void startTransaction_queryColr() {
        int transactionId = mImsUtImpl.queryCOLR();

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                mStringCaptor.capture(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        verify(mMockUsatInterface).sendCommand(any());

        String dialedString = mStringCaptor.getValue();
        assertEquals("*#77#", dialedString);
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtInterface).queryCOLR(eq(transactionId));
    }

    @Test
    public void startTransaction_queryColp() {
        int transactionId = mImsUtImpl.queryCOLP();

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                mStringCaptor.capture(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        verify(mMockUsatInterface).sendCommand(any());

        String dialedString = mStringCaptor.getValue();
        assertEquals("*#76#", dialedString);
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtInterface).queryCOLP(eq(transactionId));
    }

    @Test
    public void startTransaction_updateCallBarring() {
        int condition = SscConstant.CONDITION_BOIC;
        int action = SscConstant.ACTION_ACTIVATION;

        int transactionId = mImsUtImpl.updateCallBarring(condition, action, null);

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                mStringCaptor.capture(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        verify(mMockUsatInterface).sendCommand(any());

        String dialedString = mStringCaptor.getValue();
        assertEquals("*331#", dialedString);
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtInterface).updateCallBarringWithPassword(eq(transactionId), eq(condition),
                eq(action), eq(null), eq(SscServiceClassUtil.SERVICE_CLASS_NONE), eq(null));
    }

    @Test
    public void startTransaction_updateCallBarringForServiceClass() {
        int condition = SscConstant.CONDITION_BOIC_EXHC;
        int action = SscConstant.ACTION_DEACTIVATION;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_VOICE;

        int transactionId = mImsUtImpl.updateCallBarringForServiceClass(condition, action, null,
                serviceClass);

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                mStringCaptor.capture(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        verify(mMockUsatInterface).sendCommand(any());

        String dialedString = mStringCaptor.getValue();
        assertEquals("#332**11#", dialedString);
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtInterface).updateCallBarringWithPassword(eq(transactionId), eq(condition),
                eq(action), eq(null), eq(serviceClass), eq(null));
    }

    @Test
    public void startTransaction_updateCallBarringWithPassword() {
        int condition = SscConstant.CONDITION_BIC_WR;
        int action = SscConstant.ACTION_ACTIVATION;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_VIDEO;
        String password = "1234";

        int transactionId = mImsUtImpl.updateCallBarringWithPassword(condition, action, null,
                serviceClass, password);

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                mStringCaptor.capture(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        verify(mMockUsatInterface).sendCommand(any());

        String dialedString = mStringCaptor.getValue();
        assertEquals("*351*1234*22#", dialedString);
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtInterface).updateCallBarringWithPassword(eq(transactionId), eq(condition),
                eq(action), eq(null), eq(serviceClass), eq(password));
    }

    @Test
    public void startTransaction_updateCallForward() {
        int condition = SscConstant.CONDITION_CFB;
        int action = SscConstant.ACTION_REGISTRATION;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_NONE;
        String number = "+12345678901";
        int cfnrTimer = 15;

        int transactionId = mImsUtImpl.updateCallForward(action, condition, number, serviceClass,
                cfnrTimer);

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                mStringCaptor.capture(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        verify(mMockUsatInterface).sendCommand(any());

        String dialedString = mStringCaptor.getValue();
        assertEquals("**67*+12345678901**15#", dialedString);
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtInterface).updateCallForward(eq(transactionId), eq(action), eq(condition),
                eq(number), eq(serviceClass), eq(cfnrTimer));
    }

    @Test
    public void startTransaction_updateCallWaiting() {
        boolean enable = true;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_VIDEO;

        int transactionId = mImsUtImpl.updateCallWaiting(enable, serviceClass);

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                mStringCaptor.capture(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        verify(mMockUsatInterface).sendCommand(any());

        String dialedString = mStringCaptor.getValue();
        assertEquals("*43*22#", dialedString);
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtInterface).updateCallWaiting(eq(transactionId), eq(enable), eq(serviceClass));
    }

    @Test
    public void startTransaction_updateClir() {
        int clirMode = SscConstant.OIR_INVOCATION;

        int transactionId = mImsUtImpl.updateCLIR(clirMode);

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                mStringCaptor.capture(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        verify(mMockUsatInterface).sendCommand(any());

        String dialedString = mStringCaptor.getValue();
        assertEquals("*31#", dialedString);
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtInterface).updateCLIR(eq(transactionId), eq(clirMode));
    }

    @Test
    public void startTransaction_updateClip() {
        boolean enable = false;

        int transactionId = mImsUtImpl.updateCLIP(enable);

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                mStringCaptor.capture(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        verify(mMockUsatInterface).sendCommand(any());

        String dialedString = mStringCaptor.getValue();
        assertEquals("#30#", dialedString);
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtInterface).updateCLIP(eq(transactionId), eq(enable));
    }

    @Test
    public void startTransaction_updateColr() {
        int presentation = SscConstant.TIR_NOT_PROVISIONED;

        int transactionId = mImsUtImpl.updateCOLR(presentation);

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                mStringCaptor.capture(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        verify(mMockUsatInterface).sendCommand(any());

        String dialedString = mStringCaptor.getValue();
        assertEquals("#77#", dialedString);
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtInterface).updateCOLR(eq(transactionId), eq(presentation));
    }

    @Test
    public void startTransaction_updateColp() {
        boolean enable = true;

        int transactionId = mImsUtImpl.updateCOLP(enable);

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                mStringCaptor.capture(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        verify(mMockUsatInterface).sendCommand(any());

        String dialedString = mStringCaptor.getValue();
        assertEquals("*76#", dialedString);
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtInterface).updateCOLP(eq(transactionId), eq(enable));
    }

    @Test
    public void startTransaction_usatNotSupported() {
        boolean enable = true;

        when(mMockUsatInterface.isServiceAvailable(Usat.SERVICE_CALL_CONTROL)).thenReturn(false);

        int transactionId = mImsUtImpl.updateCOLP(enable);

        verify(mMockUtInterface).updateCOLP(eq(transactionId), eq(enable));
    }

    @Test
    public void startTransaction_usatNotAllowedForQuery() {
        int transactionId = 1;
        int condition = SscConstant.CONDITION_BAIC;

        when(mMockUsatInterface.createCallControlCommand(anyInt(), anyString(), anyInt(), anyInt(),
                any())).thenReturn(null);
        when(mMockUsatCmdRes.getResult()).thenReturn(Usat.RESULT_NOT_ALLOWED);

        UtCommand utCmd = new UtCommand.Builder(mMockBaseContext, transactionId, UtCommand.CMD_CB,
                SscConstant.ACTION_INTERROGATION, mMockUtListener).setCondition(condition).build();
        utCmd.startTransaction();

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                anyString(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        verify(mMockUsatInterface).sendCommand(any());

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtListener).utConfigurationQueryFailed(eq(transactionId),
                mReasonInfoCaptor.capture());
        ImsReasonInfo reasonInfo = mReasonInfoCaptor.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, reasonInfo.getCode());
    }

    @Test
    public void startTransaction_usatNotAllowedForUpdate() {
        int transactionId = 1;
        int condition = SscConstant.CONDITION_BAIC;
        int action = SscConstant.ACTION_ACTIVATION;

        when(mMockUsatInterface.createCallControlCommand(anyInt(), anyString(), anyInt(), anyInt(),
                any())).thenReturn(null);
        when(mMockUsatCmdRes.getResult()).thenReturn(Usat.RESULT_NOT_ALLOWED);

        UtCommand utCmd = new UtCommand.Builder(mMockBaseContext, transactionId, UtCommand.CMD_CB,
                action, mMockUtListener).setCondition(condition).setBarringList(null).build();
        utCmd.startTransaction();

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                anyString(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        verify(mMockUsatInterface).sendCommand(any());

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtListener).utConfigurationUpdateFailed(eq(transactionId),
                mReasonInfoCaptor.capture());
        ImsReasonInfo reasonInfo = mReasonInfoCaptor.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, reasonInfo.getCode());
    }

    @Test
    public void startTransaction_usatAllowedWithModificationIncludingWildValue() {
        int transactionId = 1;
        int condition = SscConstant.CONDITION_BAIC;
        String modifiedDialString = "12345678901N";

        when(mMockUsatInterface.createCallControlCommand(anyInt(), anyString(), anyInt(), anyInt(),
                any())).thenReturn(null);
        when(mMockUsatCmdRes.getResult()).thenReturn(Usat.RESULT_ALLOWED_WITH_MODIFICATION);
        when(mMockUsatCmdRes.getCcType()).thenReturn(Usat.CALL_CONTROL_TYPE_MO_CALL);
        when(mMockUsatCmdRes.getDialedString()).thenReturn(modifiedDialString);

        UtCommand utCmd = new UtCommand.Builder(mMockBaseContext, transactionId, UtCommand.CMD_CB,
                SscConstant.ACTION_INTERROGATION, mMockUtListener).setCondition(condition).build();
        utCmd.startTransaction();

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                anyString(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        verify(mMockUsatInterface).sendCommand(any());

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtListener).utConfigurationQueryFailed(eq(transactionId),
                mReasonInfoCaptor.capture());
        ImsReasonInfo reasonInfo = mReasonInfoCaptor.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, reasonInfo.getCode());
    }

    @Test
    public void startTransaction_usatAllowedWithModificationToMoCall() {
        int transactionId = 1;
        int condition = SscConstant.CONDITION_BAIC;
        String modifiedDialString = "12345678901";

        when(mMockUsatCmdRes.getResult()).thenReturn(Usat.RESULT_ALLOWED_WITH_MODIFICATION);
        when(mMockUsatCmdRes.getCcType()).thenReturn(Usat.CALL_CONTROL_TYPE_MO_CALL);
        when(mMockUsatCmdRes.getDialedString()).thenReturn(modifiedDialString);
        when(mMockUsatCmdRes.getMediaType()).thenReturn(Usat.MEDIA_TYPE_VOICE);

        UtCommand utCmd = new UtCommand.Builder(mMockBaseContext, transactionId, UtCommand.CMD_CB,
                SscConstant.ACTION_INTERROGATION, mMockUtListener).setCondition(condition).build();
        utCmd.startTransaction();

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                anyString(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        verify(mMockUsatInterface).sendCommand(any());

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtListener).utConfigurationQueryFailed(eq(transactionId),
                mReasonInfoCaptor.capture());
        ImsReasonInfo reasonInfo = mReasonInfoCaptor.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_SS_MODIFIED_TO_DIAL, reasonInfo.getCode());
        assertEquals(modifiedDialString, reasonInfo.getExtraMessage());
    }

    @Test
    public void startTransaction_usatAllowedWithModificationToMoCallVideo() {
        int transactionId = 1;
        int condition = SscConstant.CONDITION_BAIC;
        String modifiedDialString = "12345678901";

        when(mMockUsatCmdRes.getResult()).thenReturn(Usat.RESULT_ALLOWED_WITH_MODIFICATION);
        when(mMockUsatCmdRes.getCcType()).thenReturn(Usat.CALL_CONTROL_TYPE_MO_CALL);
        when(mMockUsatCmdRes.getDialedString()).thenReturn(modifiedDialString);
        when(mMockUsatCmdRes.getMediaType()).thenReturn(Usat.MEDIA_TYPE_VIDEO);

        UtCommand utCmd = new UtCommand.Builder(mMockBaseContext, transactionId, UtCommand.CMD_CB,
                SscConstant.ACTION_INTERROGATION, mMockUtListener).setCondition(condition).build();
        utCmd.startTransaction();

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                anyString(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        verify(mMockUsatInterface).sendCommand(any());

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtListener).utConfigurationQueryFailed(eq(transactionId),
                mReasonInfoCaptor.capture());
        ImsReasonInfo reasonInfo = mReasonInfoCaptor.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_SS_MODIFIED_TO_DIAL_VIDEO, reasonInfo.getCode());
        assertEquals(modifiedDialString, reasonInfo.getExtraMessage());
    }

    @Test
    public void startTransaction_usatAllowedWithModificationToUssd() {
        int transactionId = 1;
        int condition = SscConstant.CONDITION_BAIC;
        String modifiedDialString = "*31*1234#";

        when(mMockUsatCmdRes.getResult()).thenReturn(Usat.RESULT_ALLOWED_WITH_MODIFICATION);
        when(mMockUsatCmdRes.getCcType()).thenReturn(Usat.CALL_CONTROL_TYPE_USSD);
        when(mMockUsatCmdRes.getDialedString()).thenReturn(modifiedDialString);

        UtCommand utCmd = new UtCommand.Builder(mMockBaseContext, transactionId, UtCommand.CMD_CB,
                SscConstant.ACTION_INTERROGATION, mMockUtListener).setCondition(condition).build();
        utCmd.startTransaction();

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                anyString(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        verify(mMockUsatInterface).sendCommand(any());

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtListener).utConfigurationQueryFailed(eq(transactionId),
                mReasonInfoCaptor.capture());
        ImsReasonInfo reasonInfo = mReasonInfoCaptor.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_SS_MODIFIED_TO_USSD, reasonInfo.getCode());
        assertEquals(modifiedDialString, reasonInfo.getExtraMessage());
    }

    @Test
    public void startTransaction_usatAllowedWithModificationToOtherSs() {
        int transactionId = 1;
        int condition = SscConstant.CONDITION_BAIC;
        String modifiedDialString = "*#43#";

        when(mMockUsatCmdRes.getResult()).thenReturn(Usat.RESULT_ALLOWED_WITH_MODIFICATION);
        when(mMockUsatCmdRes.getCcType()).thenReturn(Usat.CALL_CONTROL_TYPE_SS);
        when(mMockUsatCmdRes.getDialedString()).thenReturn(modifiedDialString);

        UtCommand utCmd = new UtCommand.Builder(mMockBaseContext, transactionId, UtCommand.CMD_CB,
                SscConstant.ACTION_INTERROGATION, mMockUtListener).setCondition(condition).build();
        utCmd.startTransaction();

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                anyString(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        verify(mMockUsatInterface).sendCommand(any());

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtListener).utConfigurationQueryFailed(eq(transactionId),
                mReasonInfoCaptor.capture());
        ImsReasonInfo reasonInfo = mReasonInfoCaptor.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_SS_MODIFIED_TO_SS, reasonInfo.getCode());
        assertEquals(modifiedDialString, reasonInfo.getExtraMessage());
    }


    @Test
    public void startTransaction_usatAllowedWithModificationToTypeNone() {
        int transactionId = 1;
        int condition = SscConstant.CONDITION_BAIC;

        when(mMockUsatCmdRes.getResult()).thenReturn(Usat.CALL_CONTROL_TYPE_NONE);
        when(mMockUsatCmdRes.getCcType()).thenReturn(Usat.CALL_CONTROL_TYPE_SS);

        UtCommand utCmd = new UtCommand.Builder(mMockBaseContext, transactionId, UtCommand.CMD_CB,
                SscConstant.ACTION_INTERROGATION, mMockUtListener).setCondition(condition).build();
        utCmd.startTransaction();

        verify(mMockUsatInterface).createCallControlCommand(eq(Usat.CALL_CONTROL_TYPE_SS),
                anyString(), anyInt(), anyInt(), mUsatListenerCaptor.capture());
        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        verify(mMockUsatInterface).sendCommand(any());

        usatListener.onCommandResponse(mMockUsatCmdRes);

        verify(mMockUtInterface).queryCallBarringForServiceClass(eq(transactionId), eq(condition),
                eq(SscServiceClassUtil.SERVICE_CLASS_NONE));
    }
}
