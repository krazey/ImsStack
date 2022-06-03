#ifndef USSI_EVENT_NOTIFIER_H_
#define USSI_EVENT_NOTIFIER_H_

#include "MtcDef.h"

#include "call/IMtcCall.h"
#include "ussi/UssiDef.h"

class IMtcCallContext;

class UssiEventNotifier final
{
public:
    explicit UssiEventNotifier(IN IMtcCallContext& objContext);
    ~UssiEventNotifier();
    UssiEventNotifier(IN const UssiEventNotifier&) = delete;
    UssiEventNotifier& operator=(IN const UssiEventNotifier&) = delete;

public:
    void NotifyUssiError(IN AString strUssdString);
    void NotifyUssiResult(IN AString strUssdString, IN UssiModeType eType);

private:
    IMtcCallContext& m_objContext;
    static const IMS_UINT32 INFO_TYPE_USSI = 11;
};

#endif
