#ifndef TERMINATING_STATE_H_
#define TERMINATING_STATE_H_

#include "IMSTypeDef.h"
#include "call/state/MtcCallState.h"
#include "MtcDef.h"

class TerminatingState : public MtcCallState
{
public:
    TerminatingState(IN IMtcCallContext& objContext);
    virtual ~TerminatingState();
    TerminatingState(IN const TerminatingState&) = delete;
    TerminatingState& operator=(IN const TerminatingState&) = delete;
};

#endif
