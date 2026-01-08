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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

import androidx.test.filters.SmallTest;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class ImsArgsTest {
    @Test
    @SmallTest
    public void obtain_default() throws Exception {
        ImsArgs args = ImsArgs.obtain();

        assertNotNull(args);

        assertNull(args.mArg1);
        assertNull(args.mArg2);
        assertNull(args.mArg3);

        assertEquals(0, args.mIntArg1);
        assertEquals(0, args.mIntArg2);
        assertEquals(0, args.mIntArg3);
    }

    @Test
    @SmallTest
    public void obtain_intArgs() throws Exception {
        int intArg1 = 1;
        int intArg2 = 2;
        int intArg3 = 3;
        ImsArgs args = ImsArgs.obtain(intArg1, intArg2, intArg3);

        assertNotNull(args);

        assertNull(args.mArg1);
        assertNull(args.mArg2);
        assertNull(args.mArg3);

        assertEquals(intArg1, args.mIntArg1);
        assertEquals(intArg2, args.mIntArg2);
        assertEquals(intArg3, args.mIntArg3);
    }

    @Test
    @SmallTest
    public void obtain_objectArgs() throws Exception {
        Object arg1 = new Object();
        Object arg2 = new Object();
        Object arg3 = new Object();
        ImsArgs args = ImsArgs.obtain(arg1, arg2, arg3);

        assertNotNull(args);

        assertEquals(arg1, args.mArg1);
        assertEquals(arg2, args.mArg2);
        assertEquals(arg3, args.mArg3);

        assertEquals(0, args.mIntArg1);
        assertEquals(0, args.mIntArg2);
        assertEquals(0, args.mIntArg3);
    }

    @Test
    @SmallTest
    public void obtain_allArgs() throws Exception {
        Object arg1 = new Object();
        Object arg2 = new Object();
        Object arg3 = new Object();
        int intArg1 = 1;
        int intArg2 = 2;
        int intArg3 = 3;
        ImsArgs args = ImsArgs.obtain(arg1, arg2, arg3, intArg1, intArg2, intArg3);

        assertNotNull(args);

        assertEquals(arg1, args.mArg1);
        assertEquals(arg2, args.mArg2);
        assertEquals(arg3, args.mArg3);

        assertEquals(intArg1, args.mIntArg1);
        assertEquals(intArg2, args.mIntArg2);
        assertEquals(intArg3, args.mIntArg3);
    }

    @Test
    @SmallTest
    public void recycle() throws Exception {
        ImsArgs args = ImsArgs.obtain();

        assertNotNull(args);

        args.recycle();

        ImsArgs recycledArgs = ImsArgs.obtain();

        assertNotNull(recycledArgs);

        assertEquals(args, recycledArgs);
    }
}
