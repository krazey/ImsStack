package com.android.imsstack.core.service.serviceif;

import android.content.Context;

public interface IVoLteService extends IService {

    int TYPE_DEFAULT = 0;
    int TYPE_CALLSETTING = TYPE_DEFAULT + 1;
    int TYPE_SRVCCSTATE = TYPE_DEFAULT + 2;
    int TYPE_ACBSKIP = TYPE_DEFAULT + 3;
    int TYPE_CALLINFO = TYPE_DEFAULT + 4;
    int TYPE_CALLSTATENOTIFICATION = TYPE_DEFAULT + 5;
    int TYPE_MAX = TYPE_DEFAULT + 6;

    void start(int slotID);

    Context getContext();
    int getSlotID();
    IService getService(int type);
}
