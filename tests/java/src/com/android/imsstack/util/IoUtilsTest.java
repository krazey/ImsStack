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
package com.android.imsstack.util;

import static org.junit.Assert.assertThrows;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.verify;

import android.test.suitebuilder.annotation.SmallTest;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.io.FileOutputStream;
import java.io.IOException;

@RunWith(JUnit4.class)
public class IoUtilsTest {
    @Mock private FileOutputStream mFileOutputStream;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
    }

    @After
    public void tearDown() throws Exception {
        mFileOutputStream = null;
    }

    @Test
    @SmallTest
    public void testCloseQuietly() throws Exception {
        IoUtils.closeQuietly(null);
        IoUtils.closeQuietly(mFileOutputStream);

        verify(mFileOutputStream).close();
    }

    @Test
    @SmallTest
    public void testCloseQuietlyWithRuntimeException() throws Exception {
        doThrow(new RuntimeException("close failed.")).when(mFileOutputStream).close();

        assertThrows(RuntimeException.class, () -> {
            IoUtils.closeQuietly(mFileOutputStream);
        });
    }

    @Test
    @SmallTest
    public void testCloseQuietlyWithIOException() throws Exception {
        doThrow(new IOException("close failed.")).when(mFileOutputStream).close();
        IoUtils.closeQuietly(mFileOutputStream);

        // Expected: exception is ignored.
        verify(mFileOutputStream).close();
    }
}
