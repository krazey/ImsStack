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

import java.util.Hashtable;
import java.util.Iterator;
import java.util.Map.Entry;
import java.util.Set;

public final class MapIntLong {
    public final static int INVALID_KEY = (-1);
    public final static long INVALID_VALUE = (-1);

    private final static boolean DBG = false;
    private static int sNextKey = 1;
    private final Object mLock = new Object();
    private Hashtable<Integer, Long> mIntLong = new Hashtable<Integer, Long>();

    public MapIntLong() {
    }

    public void add(int key, long value) {
        synchronized(mLock) {
            Long v = mIntLong.put(Integer.valueOf(key), Long.valueOf(value));

            if (v != null) {
                ImsLog.d("add :: k=" + key + ", v(prev)=" + v + ", size=" + mIntLong.size());
            }

            if (DBG) {
                ImsLog.d("add :: k=" + key + ", v=" + value + ", size=" + mIntLong.size());
            }
        }
    }

    public boolean contains(int key) {
        synchronized(mLock) {
            return mIntLong.containsKey(Integer.valueOf(key));
        }
    }

    public int getNewKey() {
        synchronized(mLock) {
            do {
                Integer key = Integer.valueOf(sNextKey);

                sNextKey++;

                if (sNextKey == Integer.MAX_VALUE) {
                    sNextKey = 1;
                }

                if (mIntLong.get(key) == null) {
                    return key.intValue();
                }
            } while (true);
        }
    }

    public int getKey(long value) {
        synchronized(mLock) {
            if (!mIntLong.contains(Long.valueOf(value))) {
                // no specified object
                ImsLog.d("getKey :: no value; v=" + value);
                return INVALID_KEY;
            }

            Set<Entry<Integer, Long>> entries = mIntLong.entrySet();

            if (entries != null) {
                Iterator<Entry<Integer, Long>> iterator = entries.iterator();

                while (iterator.hasNext()) {
                    Entry<Integer, Long> entry = iterator.next();

                    if (entry.getValue().longValue() == value) {
                        return entry.getKey().intValue();
                    }
                }
            }

            return INVALID_KEY;
        }
    }

    public long getValue(int key) {
        synchronized(mLock) {
            Long v = mIntLong.get(Integer.valueOf(key));
            return (v == null) ? INVALID_VALUE : v.longValue();
        }
    }

    public void remove(int key) {
        synchronized(mLock) {
            if (DBG) {
                ImsLog.d("remove :: k=" + key + ", size(b)=" + mIntLong.size());
            }

            Long v = mIntLong.remove(Integer.valueOf(key));

            if (DBG) {
                ImsLog.d("remove :: k=" + key + ", v=" + v + ", size(a)=" + mIntLong.size());
            }
        }
    }

    public void removeAll() {
        synchronized(mLock) {
            if (DBG) {
                ImsLog.d("removeAll :: size(b)=" + mIntLong.size());
            }

            mIntLong.clear();
        }
    }

    public void removeValue(long value) {
        synchronized(mLock) {
            if (!mIntLong.contains(Long.valueOf(value))) {
                // no specified object
                ImsLog.d("removeValue :: no value; v=" + value);
                return;
            }

            Set<Entry<Integer, Long>> entries = mIntLong.entrySet();

            if (entries != null) {
                Iterator<Entry<Integer, Long>> iterator = entries.iterator();

                while (iterator.hasNext()) {
                    Entry<Integer, Long> entry = iterator.next();

                    if (entry.getValue().longValue() == value) {
                        if (DBG) {
                            ImsLog.d("removeValue :: k=" + entry.getKey()
                                    + ", v=" + value + ", size(b)=" + mIntLong.size());
                        }

                        mIntLong.remove(entry.getKey());

                        if (DBG) {
                            ImsLog.d("removeValue :: size(a)=" + mIntLong.size());
                        }
                        break;
                    }
                }
            }
        }
    }
}
