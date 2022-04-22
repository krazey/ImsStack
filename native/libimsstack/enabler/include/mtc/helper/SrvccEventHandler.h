#ifndef SRVCC_EVENT_HANDLER_H_
#define SRVCC_EVENT_HANDLER_H_

#include "IMSTypeDef.h"
#include "IMSList.h"
#include "helper/ISrvccStateListener.h"

class IMtcContext;

class SrvccEventHandler final
{
public:
    SrvccEventHandler(IN IMtcContext& objContext);
    ~SrvccEventHandler();
    SrvccEventHandler(IN const SrvccEventHandler&) = delete;
    SrvccEventHandler& operator=(IN const SrvccEventHandler&) = delete;

    void AddListener(IN ISrvccStateListener* piListener);
    void RemoveListener(IN ISrvccStateListener* piListener);

    void UpdateSrvccState(IN SrvccState eState);

private:
    void NotifyListeners();
    void HandleCalls();

private:
    IMtcContext& m_objContext;
    SrvccState m_eState;
    IMSList<ISrvccStateListener*> m_objListeners;
};

#endif
