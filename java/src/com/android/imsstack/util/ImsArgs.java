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

/**
 * This class is a helper class for passing more arguments though a message
 * and avoiding allocation of a custom class for wrapping the arguments.
 * And, it also maintains a pool of instances and it is responsibility of
 * the user of this class to recycle and instance once it is no longer used.
 */
public final class ImsArgs {
    public Object mArg1;
    public Object mArg2;
    public Object mArg3;
    public int mIntArg1;
    public int mIntArg2;
    public int mIntArg3;
    public long mLongArg;

    private static final int MAX_POOL_SIZE = 5;
    private static ImsArgs sPool;
    private static int sPoolSize;
    private static Object sPoolLock = new Object();

    private ImsArgs mNext;
    private boolean mInPool;

    private ImsArgs() {
    }

    /**
     * Instantiates ImsArgs object.
     *
     * @return A new ImsArgs object.
     */
    public static ImsArgs obtain() {
        synchronized (sPoolLock) {
            if (sPoolSize > 0) {
                ImsArgs args = sPool;
                sPool = sPool.mNext;
                args.mNext = null;
                args.mInPool = false;
                sPoolSize--;
                return args;
            } else {
                return new ImsArgs();
            }
        }
    }

    /**
     * Instantiates ImsArgs object with the specified arguments.
     *
     * @param arg1 Any integer argument.
     * @param arg2 Any integer argument.
     * @param arg3 Any integer argument.
     * @return A new ImsArgs object.
     */
    public static ImsArgs obtain(int arg1, int arg2, int arg3) {
        ImsArgs args = obtain();

        args.mIntArg1 = arg1;
        args.mIntArg2 = arg2;
        args.mIntArg3 = arg3;

        return args;
    }

    /**
     * Instantiates ImsArgs object with the specified arguments.
     *
     * @param arg1 Any object argument.
     * @param arg2 Any object argument.
     * @param arg3 Any object argument.
     * @return A new ImsArgs object.
     */
    public static ImsArgs obtain(Object arg1, Object arg2, Object arg3) {
        ImsArgs args = obtain();

        args.mArg1 = arg1;
        args.mArg2 = arg2;
        args.mArg3 = arg3;

        return args;
    }

    /**
     * Instantiates ImsArgs object with the specified arguments.
     *
     * @param arg1 Any object argument.
     * @param arg2 Any object argument.
     * @param arg3 Any object argument.
     * @param intArg1 Any integer argument.
     * @param intArg2 Any integer argument.
     * @param intArg3 Any integer argument.
     * @return A new ImsArgs object.
     */
    public static ImsArgs obtain(Object arg1, Object arg2, Object arg3,
            int intArg1, int intArg2, int intArg3) {
        ImsArgs args = obtain();

        args.mArg1 = arg1;
        args.mArg2 = arg2;
        args.mArg3 = arg3;

        args.mIntArg1 = intArg1;
        args.mIntArg2 = intArg2;
        args.mIntArg3 = intArg3;

        return args;
    }

    /**
     * Adds this object to the pool for a future use if it's no longer used.
     * This object is added to the pool and re-used when a new ImsArgs needs to be instantiated.
     */
    public void recycle() {
        if (mInPool) {
            return;
        }

        synchronized (sPoolLock) {
            clear();

            if (sPoolSize < MAX_POOL_SIZE) {
                mNext = sPool;
                mInPool = true;
                sPool = this;
                sPoolSize++;
            }
        }
    }

    /**
     * Initializes all the member variables of this object.
     */
    public void clear() {
        mArg1 = null;
        mArg2 = null;
        mArg3 = null;
        mIntArg1 = 0;
        mIntArg2 = 0;
        mIntArg3 = 0;
        mLongArg = 0;
    }
}
