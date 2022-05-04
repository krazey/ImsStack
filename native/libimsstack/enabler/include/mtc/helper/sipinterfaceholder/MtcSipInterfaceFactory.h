#ifndef MTC_SIP_INTERFACE_FACTORY_H_
#define MTC_SIP_INTERFACE_FACTORY_H_

#include "helper/sipinterfaceholder/IInterfaceHolderListener.h"

class SessionInterfaceHolder;
class ReferenceInterfaceHolder;
class SubscriptionInterfaceHolder;

class MtcSipInterfaceFactory final : public IInterfaceHolderListener
{
public:
    explicit MtcSipInterfaceFactory();
    ~MtcSipInterfaceFactory();
    MtcSipInterfaceFactory(IN const MtcSipInterfaceFactory&) = delete;
    MtcSipInterfaceFactory& operator=(IN const MtcSipInterfaceFactory&) = delete;

public:
    // IInterfaceHolderListener implementation
    void OnSessionInterfaceCleared() override;
    void OnReferenceInterfaceCleared() override;
    void OnSubscriptionInterfaceCleared() override;

    SessionInterfaceHolder* GetISessionHolder();
    ReferenceInterfaceHolder* GetIReferenceHolder();
    SubscriptionInterfaceHolder* GetISubscriptionHolder();

private:
    SessionInterfaceHolder* m_piSessionHolder;
    ReferenceInterfaceHolder* m_piReferenceHolder;
    SubscriptionInterfaceHolder* m_piSubscriptionHolder;
};

#endif
