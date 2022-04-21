package com.android.imsstack.core.service.serviceif;

import android.content.Context;

public interface IVoLteService extends IService {

    public static final int TYPE_DEFAULT = 0;
    public static final int TYPE_NOTIFICATION = TYPE_DEFAULT + 1;
    public static final int TYPE_DEBUG = TYPE_DEFAULT + 2;
    public static final int TYPE_CALLSETTING = TYPE_DEFAULT + 3;
    public static final int TYPE_REGIPROCESS = TYPE_DEFAULT + 4;
    public static final int TYPE_SRVCCSTATE = TYPE_DEFAULT + 5;
    public static final int TYPE_DEBUGMEDIAINFO = TYPE_DEFAULT + 6;
    public static final int TYPE_ACBSKIP = TYPE_DEFAULT + 7;
    public static final int TYPE_EDPGCALL = TYPE_DEFAULT + 8;
    public static final int TYPE_ECALLSTATE = TYPE_DEFAULT + 9;
    public static final int TYPE_USAT = TYPE_DEFAULT + 10;
    public static final int TYPE_DCN = TYPE_DEFAULT + 11;
    public static final int TYPE_CALLINFO = TYPE_DEFAULT + 12;
    public static final int TYPE_MSIMCALLSTATE = TYPE_DEFAULT + 13;
    public static final int TYPE_CALLSTATENOTIFICATION = TYPE_DEFAULT + 14;
    public static final int TYPE_MAX = TYPE_DEFAULT + 15;

    void start(int slotID);

    Context getContext();
    int getSlotID();
    IService getService(int type);
}
