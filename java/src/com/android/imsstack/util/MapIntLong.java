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

public final class MapIntLong {
    public final static int INVALID_KEY = (-1);
    public final static long INVALID_VALUE = (-1);

    private final static boolean DBG = false;
    private final SparseArray<Long> mIntLong = new SparseArray<>();
    private int mNextKey = 1;

    public MapIntLong() {
    }

    public void add(int key, long value) {
        synchronized (mIntLong) {
            mIntLong.put(key, Long.valueOf(value));

            if (DBG) {
                ImsLog.d("add :: k=" + key + ", v=" + value + ", size=" + mIntLong.size());
            }
        }
    }

    public boolean contains(int key) {
        synchronized (mIntLong) {
            return mIntLong.contains(key);
        }
    }

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

    public long getValue(int key) {
        synchronized (mIntLong) {
            Long v = mIntLong.get(key);
            return (v == null) ? INVALID_VALUE : v;
        }
    }

    public void remove(int key) {
        synchronized (mIntLong) {
            mIntLong.remove(key);

            if (DBG) {
                ImsLog.d("remove :: k=" + key + ", size(a)=" + mIntLong.size());
            }
        }
    }

    public void removeAll() {
        synchronized (mIntLong) {
            if (DBG) {
                ImsLog.d("removeAll :: size(b)=" + mIntLong.size());
            }

            mIntLong.clear();
        }
    }

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
