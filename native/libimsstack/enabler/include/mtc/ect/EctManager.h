#ifndef ECT_MANAGER_H_
#define ECT_MANAGER_H_

#include "call/IMtcCallManager.h"
#include "ect/IEctControllerListener.h"
#include "ect/EctController.h"
#include "helper/ObjectAsyncDestroyer.h"

class IMtcContext;

class EctManager final : public IEctControllerListener
{
public:
    explicit EctManager(IN IMtcContext& objContext);
    ~EctManager();
    EctManager(IN const EctManager&) = delete;
    EctManager& operator=(IN const EctManager&) = delete;

    void OnEctCompleted() override;

    void Transfer(IN CallKey nCallKey, IN const AString& strNumber);

private:
    IMtcContext& m_objContext;
    EctController* m_pController;
    ObjectAsyncDestroyer<EctController> m_objDestroyer;
};

#endif
