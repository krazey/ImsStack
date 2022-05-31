package com.android.imsstack.enabler;

import android.os.Handler;
import android.os.Looper;

import com.android.imsstack.core.ICommonPackageListener;
import com.android.imsstack.core.agents.UsatInterface;
import com.android.imsstack.core.agents.agentif.ILocationAgent;
import com.android.imsstack.core.agents.agentif.ISharedState;
import com.android.imsstack.core.agents.agentif.ISubscription;
import com.android.imsstack.core.agents.dcmif.IDCApn;
import com.android.imsstack.core.agents.dcmif.IDCNetWatcher;
import com.android.imsstack.enabler.mtc.IServiceStateTracker;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.test.IImsTestMode;

public interface IBaseContext extends IContext {
    /**
     * Utilities for this context.
     */
    public Handler getCallHandler();
    public Looper getCallLooper();

    /**
     * Returns service state tracker to check or monitor VoLte/Vt registration.
     */
    public IServiceStateTracker getServiceStateTracker();

    /**
     * Platform interface's wrappers.
     */
    public IDCApn getDCApn();
    public IDCNetWatcher getDCNetWatcher();
    public ISharedState getSharedState();
    public ISubscription getSubscription();
    public ISystem getSystem();
    public IImsTestMode getTestMode();

    public ILocationAgent getLocationAgent();

    /** Returns the USAT interface. */
    UsatInterface getUsatInterface();

    public boolean isCommonPackageReady();
    public void addCommonPackageListener(ICommonPackageListener listener);
    public void removeCommonPackageListener(ICommonPackageListener listener);
}
