/*
 * Copyright (C) 2026 The Android Open Source Project
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

import static com.android.imsstack.base.TestAppContext.SLOT0;

import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.atLeastOnce;
import static org.mockito.Mockito.verify;

import androidx.test.filters.SmallTest;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.util.IndentingPrintWriter;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;

@RunWith(JUnit4.class)
public class IpSecAgentTest {
    private TestAppContext mTestAppContext;
    private IpSecAgent mIpSecAgent;

    @Before
    public void setUp() throws Exception {
        mTestAppContext = new TestAppContext(new ContextFixture().getTestDouble());
        mIpSecAgent = new IpSecAgent(SLOT0);
        mIpSecAgent.init(mTestAppContext.getContext());
    }

    @After
    public void tearDown() throws Exception {
        if (mIpSecAgent != null) {
            mIpSecAgent.cleanup();
            mIpSecAgent = null;
        }
    }

    @Test
    @SmallTest
    public void testDump() {
        IndentingPrintWriter mockIpw = Mockito.mock(IndentingPrintWriter.class);

        mIpSecAgent.dump(mockIpw);

        verify(mockIpw, atLeastOnce()).println(anyString());
        verify(mockIpw, atLeastOnce()).increaseIndent();
        verify(mockIpw, atLeastOnce()).decreaseIndent();
    }
}
