package com.android.imsstack.core.agents.dcm;

import android.content.Context;

import com.android.imsstack.core.agents.dcmif.IDC;
import com.android.imsstack.util.MSimUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * This class provides the APIs to manage and control Multi-SIM state.
 */
public final class DCFactory {
    public static final int GOVERNOR = 1;
    public static final int UTIL = 2;
    public static final int SETTING = 3;
    public static final int NETWORK_WATCHER = 4;
    public static final int APN = 5;
    /** NOT_USED
    private static final int START = 0;
    */
    private static final int END = (APN + 1);
    private static final int MAX = END;

    private static Map<Integer, HashMap<Integer, IDC>> sDCSlots
            = new HashMap<Integer, HashMap<Integer, IDC>>(MSimUtils.getMaxSimSlot());

    private DCFactory() {
    }

    public static IDC getDC(int type, int slotId) {
        HashMap<Integer, IDC> dcs = getObjects(slotId);

        if (dcs != null) {
            IDC dc = dcs.get(type);
            if (dc != null) {
                return dc;
            }
        }

        return null;
    }

    public static void createDC(Context context, int slotId) {
        HashMap<Integer, IDC> agents = getObjects(slotId);

        if (agents != null) {
            return;
        }

        agents = new HashMap<Integer, IDC>(MAX);

        agents.put(GOVERNOR, new DCGov(slotId));
        agents.put(UTIL, new DcUtils(slotId));
        agents.put(SETTING, new DCSettings(slotId));
        agents.put(NETWORK_WATCHER, new DCNetWatcher(slotId));
        agents.put(APN, new DCApn(slotId));

        setObjects(slotId, agents);
    }

    public static void cleanUpDC(int slotId) {
        HashMap<Integer, IDC> dcs = getObjects(slotId);

        if (dcs == null) {
            return;
        }

        List<IDC> dcList = new ArrayList<IDC>();
        dcList.add(dcs.get(GOVERNOR));
        dcList.add(dcs.get(UTIL));
        dcList.add(dcs.get(SETTING));
        dcList.add(dcs.get(NETWORK_WATCHER));
        dcList.add(dcs.get(APN));

        for (int i = 0; i < dcList.size(); i++) {
            IDC dc = dcList.get(i);
            if (dc != null) {
                dc.cleanup();
            }
        }
    }

    public static void initDC(Context context, int slotId) {
        HashMap<Integer, IDC> dcs = getObjects(slotId);

        if (dcs == null) {
            return;
        }

        List<IDC> dcList = new ArrayList<IDC>();
        dcList.add(dcs.get(GOVERNOR));
        dcList.add(dcs.get(UTIL));
        dcList.add(dcs.get(SETTING));
        dcList.add(dcs.get(NETWORK_WATCHER));
        dcList.add(dcs.get(APN));

        for (int i = 0; i < dcList.size(); i++) {
            IDC dc = dcList.get(i);
            if (dc != null) {
                dc.init(context);
            }
        }
    }

    private static synchronized HashMap<Integer, IDC> getObjects(int slotId) {
        return sDCSlots.get(slotId);
    }

    private static synchronized void setObjects(int slotId, HashMap<Integer, IDC> objects) {
        sDCSlots.put(slotId, objects);
    }
}
