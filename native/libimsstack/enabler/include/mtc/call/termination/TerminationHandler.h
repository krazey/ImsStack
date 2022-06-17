#ifndef TERMINATION_HANDLER_H_
#define TERMINATION_HANDLER_H_

#include "CallReasonInfo.h"
#include "IMSTypeDef.h"

class ISession;

class TerminationHandler final
{
public:
    explicit TerminationHandler();
    ~TerminationHandler();
    TerminationHandler(const TerminationHandler&) = delete;
    TerminationHandler& operator=(const TerminationHandler&) = delete;

    CallReasonInfo Handle(IN const ISession& objSession) const;

private:
    CallReasonInfo GetCallReasonInfoFromSessionTerminationReason(
            IN IMS_SINT32 nTerminationReason) const;
};

#endif
