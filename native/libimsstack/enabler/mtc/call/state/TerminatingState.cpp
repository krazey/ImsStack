#include "call/IMtcCallContext.h"
#include "call/MtcUiNotifier.h"
#include "call/state/TerminatingState.h"
#include "helper/MtcTimerWrapper.h"
#include "define/MtcStringDef.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
TerminatingState::TerminatingState(IN IMtcCallContext& objContext) :
        MtcCallState(CallStateName::TERMINATING, objContext)
{
}

PUBLIC VIRTUAL TerminatingState::~TerminatingState() {}
