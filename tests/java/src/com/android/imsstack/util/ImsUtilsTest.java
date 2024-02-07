/*
 * Copyright (C) 2024 The Android Open Source Project
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
package com.android.imsstack.util;

import static org.junit.Assert.assertThrows;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;

import androidx.test.filters.SmallTest;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.MockitoAnnotations;

import java.io.FileOutputStream;
import java.io.IOException;

@RunWith(JUnit4.class)
public class ImsUtilsTest {
    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
    }

    @After
    public void tearDown() throws Exception {
        // no-op
    }

    @Test
    @SmallTest
    public void testCloseQuietly() throws Exception {
        FileOutputStream output = mock(FileOutputStream.class);
        ImsUtils.closeQuietly(null);
        ImsUtils.closeQuietly(output);

        verify(output).close();
    }

    @Test
    @SmallTest
    public void testCloseQuietlyWithRuntimeException() throws Exception {
        FileOutputStream output = mock(FileOutputStream.class);
        doThrow(new RuntimeException("close failed.")).when(output).close();

        assertThrows(RuntimeException.class, () -> {
            ImsUtils.closeQuietly(output);
        });
    }

    @Test
    @SmallTest
    public void testCloseQuietlyWithIOException() throws Exception {
        FileOutputStream output = mock(FileOutputStream.class);
        doThrow(new IOException("close failed.")).when(output).close();
        ImsUtils.closeQuietly(output);

        // Expected: exception is ignored.
        verify(output).close();
    }
}
