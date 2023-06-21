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

import android.util.SparseArray;

/** A class for providing the integer key and long value pair. */
public final class MapIntLong {
    public static final int INVALID_KEY = (-1);
    public static final long INVALID_VALUE = (-1);

    private static final boolean DBG = false;
    private final SparseArray<Long> mIntLong = new SparseArray<>();
    private int mNextKey = 1;

    public MapIntLong() {
    }

    /**
     * Adds a key and value.
     *
     * @param key The integer type key.
     * @param value The long type value.
     */
    public void add(int key, long value) {
        synchronized (mIntLong) {
            mIntLong.put(key, Long.valueOf(value));

            if (DBG) {
                ImsLog.d("add :: k=" + key + ", v=" + value + ", size=" + mIntLong.size());
            }
        }
    }

    /**
     * Checks whether the given key is contained or not.
     *
     * @param key The integer type key.
     * @return {@code true} if the given key contains, {@code false} otherwise.
     */
    public boolean contains(int key) {
        synchronized (mIntLong) {
            return mIntLong.contains(key);
        }
    }

    /**
     * Returns a new key.
     *
     * @return A new key.
     */
    public int getNewKey() {
        synchronized (mIntLong) {
            do {
                int key = mNextKey;

                mNextKey++;

                if (mNextKey == Integer.MAX_VALUE) {
                    mNextKey = 1;
                }

                if (!mIntLong.contains(key)) {
                    return key;
                }
            } while (true);
        }
    }

    /**
     * Returns the integer type key for the given long type value.
     *
     * @param value The long type value.
     * @return The integer type key.
     */
    public int getKey(long value) {
        synchronized (mIntLong) {
            for (int i = 0; i < mIntLong.size(); ++i) {
                if (value == mIntLong.valueAt(i)) {
                    return mIntLong.keyAt(i);
                }
            }
        }

        // no specified object
        ImsLog.d("getKey :: no value; v=" + value);
        return INVALID_KEY;
    }

    /**
     * Returns the long type value for the given integer type key.
     *
     * @param key The integer type key.
     * @return The long type value.
     */
    public long getValue(int key) {
        synchronized (mIntLong) {
            Long v = mIntLong.get(key);
            return (v == null) ? INVALID_VALUE : v;
        }
    }

    /**
     * Removes the value for the given key.
     *
     * @param key The integer type key.
     */
    public void remove(int key) {
        synchronized (mIntLong) {
            mIntLong.remove(key);

            if (DBG) {
                ImsLog.d("remove :: k=" + key + ", size(a)=" + mIntLong.size());
            }
        }
    }

    /**
     * Removes all the values.
     */
    public void removeAll() {
        synchronized (mIntLong) {
            if (DBG) {
                ImsLog.d("removeAll :: size(b)=" + mIntLong.size());
            }

            mIntLong.clear();
        }
    }

    /**
     * Removes the entry for the given value.
     *
     * @param value The long type value to be removed.
     */
    public void removeValue(long value) {
        synchronized (mIntLong) {
            for (int i = 0; i < mIntLong.size(); ++i) {
                if (value == mIntLong.valueAt(i)) {
                    if (DBG) {
                        ImsLog.d("removeValue :: k=" + mIntLong.keyAt(i)
                                + ", v=" + value + ", size(b)=" + mIntLong.size());
                    }

                    mIntLong.removeAt(i);

                    if (DBG) {
                        ImsLog.d("removeValue :: size(a)=" + mIntLong.size());
                    }
                    return;
                }
            }
        }

        // no specified object
        ImsLog.d("removeValue :: no value; v=" + value);
    }
}
