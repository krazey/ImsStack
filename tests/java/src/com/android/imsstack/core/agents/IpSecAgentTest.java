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

import static org.junit.Assert.assertNull;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.atLeastOnce;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.util.SparseArray;

import androidx.test.filters.SmallTest;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.system.IpSecSaParameter;
import com.android.imsstack.util.IndentingPrintWriter;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mockito;

import java.lang.reflect.Field;
import java.util.concurrent.ExecutorService;

@RunWith(JUnit4.class)
public class IpSecAgentTest {
    private static final int IPSEC_ID = 1;

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

    @Test
    @SmallTest
    public void removeIpSecSaParameter_dispatchesCloseAfterRemovingConnector() throws Exception {
        ExecutorService mockExecutor = Mockito.mock(ExecutorService.class);
        IpSecConnector mockConnector = Mockito.mock(IpSecConnector.class);
        IpSecSaParameter mockParameter = Mockito.mock(IpSecSaParameter.class);
        ArgumentCaptor<Runnable> cleanupCaptor = ArgumentCaptor.forClass(Runnable.class);
        SparseArray<IpSecConnector> connectors = getConnectors();

        when(mockConnector.getSaParameter()).thenReturn(mockParameter);
        connectors.put(IPSEC_ID, mockConnector);
        replaceInstance("mCleanupExecutor", mockExecutor);

        mIpSecAgent.removeIpSecSaParameter(IPSEC_ID);

        assertNull(connectors.get(IPSEC_ID));
        verify(mockConnector).markAsRemoved();
        verify(mockExecutor).execute(cleanupCaptor.capture());
        verify(mockConnector, never()).close();

        cleanupCaptor.getValue().run();

        verify(mockConnector).close();
    }

    @Test
    @SmallTest
    public void cleanup_shutsDownCleanupExecutor() throws Exception {
        ExecutorService mockExecutor = Mockito.mock(ExecutorService.class);
        replaceInstance("mCleanupExecutor", mockExecutor);

        mIpSecAgent.cleanup();

        verify(mockExecutor).shutdown();
    }

    @SuppressWarnings("unchecked")
    private SparseArray<IpSecConnector> getConnectors() throws Exception {
        Field field = IpSecAgent.class.getDeclaredField("mConnectors");
        field.setAccessible(true);
        return (SparseArray<IpSecConnector>) field.get(mIpSecAgent);
    }

    private void replaceInstance(String fieldName, Object value) throws Exception {
        Field field = IpSecAgent.class.getDeclaredField(fieldName);
        field.setAccessible(true);
        field.set(mIpSecAgent, value);
    }
}
