package com.android.imsstack.enabler.uce.interf;

import android.content.Context;

import com.android.imsstack.enabler.uce.impl.UceImpl;
import com.android.imsstack.util.ImsLog;

import java.util.HashMap;
import java.util.Map;

public class UceManager {

    private static Map<Integer, IUceApi> mSimSlotListMap = new HashMap<Integer, IUceApi>();
    private static Object mLockObj = new Object();

    public static IUceApi create(Context context, int nSimSlot) {
        synchronized (mLockObj) {
            IUceApi mUceApi = null;
            if (context == null) {
                ImsLog.i("context is null");
                return mUceApi;
            }
            if (mSimSlotListMap.get(nSimSlot) == null) {
                mUceApi = new UceImpl(nSimSlot);
                mSimSlotListMap.put(nSimSlot, mUceApi);
                ImsLog.i("create UceImpl for simSlot : " + nSimSlot);
                return mUceApi;
            } else {
                ImsLog.i("there is already UceImpl for simSlot : " + nSimSlot);
                return mSimSlotListMap.get(nSimSlot);
            }
        }
    }

    public static void delete(int nSimSlot) {
         synchronized (mLockObj) {
             IUceApi mUceApi = mSimSlotListMap.get(nSimSlot);
            if (mUceApi != null) {
                ((UceImpl)mUceApi).release();
                mSimSlotListMap.remove(nSimSlot);
                ImsLog.i("delete UceImpl for simSlot : " + nSimSlot);
            }
            ImsLog.i("there is not already UceImpl for simSlot : " + nSimSlot);
        }
    }
}
