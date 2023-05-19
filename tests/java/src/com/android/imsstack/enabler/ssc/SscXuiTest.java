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

package com.android.imsstack.enabler.ssc;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.when;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.PreferenceInterface;
import com.android.imsstack.core.agents.SimInterface;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;

@RunWith(JUnit4.class)
public class SscXuiTest {
    private static final int SLOT_0 = 0;

    @Mock private PreferenceInterface mMockPreference;
    @Mock private SimInterface mMockSimInterface;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        AgentFactory.getInstance().setAgent(PreferenceInterface.class, mMockPreference);
        AgentFactory.getInstance().setAgent(SimInterface.class, mMockSimInterface, SLOT_0);
    }

    @After
    public void cleanup() {
        AgentFactory.getInstance().setAgent(PreferenceInterface.class, null);
        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT_0);
    }

    @Test
    public void getXui_noPaidAndNoImpu() {
        when(mMockSimInterface.getIsimImpu()).thenReturn(null);
        when(mMockPreference.getString(SscXui.IMPU_FILE_NAME, SscXui.IMPU_LIST_SIZE,
                SLOT_0)).thenReturn("0");

        String xui = SscXui.getInstance().getXui(SLOT_0, null);

        assertNull(xui);
    }

    @Test
    public void getXui_preferenceIsNullWhileGetPAssociatedUriSize() {
        String impu = "001010123456789@test.3gpp.com";
        ArrayList<String> impuList = new ArrayList<>();
        impuList.add(impu);
        when(mMockSimInterface.getIsimImpu()).thenReturn(impuList);
        AgentFactory.getInstance().setAgent(PreferenceInterface.class, null);

        String xui = SscXui.getInstance().getXui(SLOT_0, null);

        assertEquals(impu, xui);
    }

    @Test
    public void getXui_preferenceIsNullWhileGetPAssociatedUriValue() {
        String impu = "001010123456789@test.3gpp.com";
        ArrayList<String> impuList = new ArrayList<>();
        impuList.add(impu);
        when(mMockSimInterface.getIsimImpu()).thenReturn(impuList);
        doAnswer((invocation) -> {
            AgentFactory.getInstance().setAgent(PreferenceInterface.class, null);
            return "1";
        }).when(mMockPreference).getString(SscXui.IMPU_FILE_NAME, SscXui.IMPU_LIST_SIZE, SLOT_0);

        String xui = SscXui.getInstance().getXui(SLOT_0, null);

        assertEquals(impu, xui);
    }

    @Test
    public void getXui_noPaidAndGettingImpuFromSim() {
        String impu = "001010123456789@test.3gpp.com";
        ArrayList<String> impuList = new ArrayList<>();
        impuList.add(impu);
        when(mMockPreference.getString(SscXui.IMPU_FILE_NAME, SscXui.IMPU_LIST_SIZE, SLOT_0))
                .thenReturn("0");
        when(mMockSimInterface.getIsimImpu()).thenReturn(impuList);

        String xui = SscXui.getInstance().getXui(SLOT_0, null);

        assertEquals(impu, xui);
    }

    @Test
    public void getXui_impuListNumberFormatException() {
        String telPaid = "tel:1234567890";
        String impu = "001010123456789@test.3gpp.com";
        ArrayList<String> impuList = new ArrayList<>();
        impuList.add(impu);
        when(mMockPreference.getString(SscXui.IMPU_FILE_NAME, SscXui.IMPU_LIST_SIZE, SLOT_0))
                .thenReturn("!@$%@$@#");
        when(mMockPreference.getString(SscXui.IMPU_FILE_NAME, "0", SLOT_0))
                .thenReturn(telPaid);
        when(mMockSimInterface.getIsimImpu()).thenReturn(impuList);

        String xui = SscXui.getInstance().getXui(SLOT_0, null);

        assertEquals(impu, xui);
    }

    @Test
    public void getXui_firstPaid() {
        String telPaid = "tel:1234567890";
        String sipPaid = "sip:+11234567890@test.3gpp.com";
        when(mMockPreference.getString(SscXui.IMPU_FILE_NAME, SscXui.IMPU_LIST_SIZE, SLOT_0))
                .thenReturn("2");
        when(mMockPreference.getString(SscXui.IMPU_FILE_NAME, "0", SLOT_0))
                .thenReturn(telPaid);
        when(mMockPreference.getString(SscXui.IMPU_FILE_NAME, "1", SLOT_0))
                .thenReturn(sipPaid);

        String xui = SscXui.getInstance().getXui(SLOT_0, null);

        assertEquals(telPaid, xui);
    }

    @Test
    public void getXui_includingPasswordButNoSipUri() {
        String telPaid = "tel:1234567890";
        String password = "0000";
        when(mMockPreference.getString(SscXui.IMPU_FILE_NAME, SscXui.IMPU_LIST_SIZE, SLOT_0))
                .thenReturn("1");
        when(mMockPreference.getString(SscXui.IMPU_FILE_NAME, "0", SLOT_0))
                .thenReturn(telPaid);

        String xui = SscXui.getInstance().getXui(SLOT_0, password);

        assertEquals(telPaid, xui);
    }

    @Test
    public void getXui_includingPasswordButWrongSipUri() {
        String sipPaid = "sip:+11234567890";
        String password = "0000";
        when(mMockPreference.getString(SscXui.IMPU_FILE_NAME, SscXui.IMPU_LIST_SIZE, SLOT_0))
                .thenReturn("1");
        when(mMockPreference.getString(SscXui.IMPU_FILE_NAME, "0", SLOT_0))
                .thenReturn(sipPaid);

        String xui = SscXui.getInstance().getXui(SLOT_0, password);

        assertEquals("sip:+11234567890", xui);
    }

    @Test
    public void getXui_includingPassword() {
        String telPaid = "tel:1234567890";
        String sipPaid = "sip:+11234567890@test.3gpp.com";
        String password = "0000";
        when(mMockPreference.getString(SscXui.IMPU_FILE_NAME, SscXui.IMPU_LIST_SIZE, SLOT_0))
                .thenReturn("2");
        when(mMockPreference.getString(SscXui.IMPU_FILE_NAME, "0", SLOT_0))
                .thenReturn(telPaid);
        when(mMockPreference.getString(SscXui.IMPU_FILE_NAME, "1", SLOT_0))
                .thenReturn(sipPaid);

        String xui = SscXui.getInstance().getXui(SLOT_0, password);

        assertEquals("sip:+11234567890:0000@test.3gpp.com", xui);
    }
}
