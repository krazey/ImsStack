/*
 * Copyright (C) 2025 The Android Open Source Project
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
package com.android.imsstack;

import android.util.ArrayMap;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.List;

/**
 * A helper class for managing the singleton instances during unit tests.
 */
public class TestInstanceHolder {
    private final ArrayMap<InstanceKey, Object> mOldInstances = new ArrayMap<>();
    private final List<InstanceKey> mInstanceKeys = new ArrayList<>();

    private static class InstanceKey {
        public final Class mClass;
        public final String mInstName;
        public final Object mObj;

        InstanceKey(final Class c, final String instName, final Object obj) {
            mClass = c;
            mInstName = instName;
            mObj = obj;
        }

        @Override
        public int hashCode() {
            return (mClass.getName().hashCode() * 31 + mInstName.hashCode()) * 31;
        }

        @Override
        public boolean equals(Object obj) {
            if (!(obj instanceof InstanceKey)) {
                return false;
            }

            InstanceKey other = (InstanceKey) obj;
            return other.mClass == mClass
                    && other.mInstName.equals(mInstName)
                    && other.mObj == mObj;
        }
    }

    /**
     * Replaces the given instance.
     *
     * @param c A class name to be replaced.
     * @param instanceName An instance name to be replaced.
     * @param obj An object to be replaced. null for static member field.
     * @param newValue A value to be replaced.
     */
    public void replace(final Class c, final String instanceName, final Object obj,
            final Object newValue) throws Exception {
        Field field = c.getDeclaredField(instanceName);
        field.setAccessible(true);

        InstanceKey key = new InstanceKey(c, instanceName, obj);
        if (!mOldInstances.containsKey(key)) {
            mOldInstances.put(key, field.get(obj));
            mInstanceKeys.add(key);
        }
        field.set(obj, newValue);
    }

    /**
     * Restores the given instance.
     *
     * @param c A class name to be restored.
     * @param instanceName An instance name to be restored.
     * @param obj An object to be restored. null for static member field.
     */
    public void restore(final Class c, final String instanceName, final Object obj)
            throws Exception {
        InstanceKey key = new InstanceKey(c, instanceName, obj);
        if (mOldInstances.containsKey(key)) {
            Field field = c.getDeclaredField(instanceName);
            field.setAccessible(true);
            field.set(obj, mOldInstances.get(key));
            mOldInstances.remove(key);
            mInstanceKeys.remove(key);
        }
    }

    /**
     * Restores all the instances.
     */
    public void restoreAll() throws Exception {
        for (int i = mInstanceKeys.size() - 1; i >= 0; --i) {
            InstanceKey key = mInstanceKeys.get(i);
            Field field = key.mClass.getDeclaredField(key.mInstName);
            field.setAccessible(true);
            field.set(key.mObj, mOldInstances.get(key));
        }

        mInstanceKeys.clear();
        mOldInstances.clear();
    }
}
