#include "call/state/AlertingState.h"
#include "call/state/CallStateFactory.h"
#include "call/state/EstablishedState.h"
#include "call/state/IdleState.h"
#include "call/state/IncomingState.h"
#include "call/state/OutgoingState.h"
#include "call/state/TerminatingState.h"
#include "call/state/UpdatingState.h"

PUBLIC CallStateFactory::CallStateFactory(IN IMtcCallContext& objContext) :
        m_objContext(objContext)
{
}

PUBLIC VIRTUAL CallStateFactory::~CallStateFactory() {}

PUBLIC VIRTUAL IMtcCallState* CallStateFactory::CreateState(IN CallStateName eState)
{
    switch (eState)
    {
        case CallStateName::IDLE:
            return new IdleState(m_objContext);
        case CallStateName::OUTGOING:
            return new OutgoingState(m_objContext);
        case CallStateName::INCOMING:
            return new IncomingState(m_objContext);
        case CallStateName::ALERTING:
            return new AlertingState(m_objContext);
        case CallStateName::ESTABLISHED:
            return new EstablishedState(m_objContext);
        case CallStateName::UPDATING:
            return new UpdatingState(m_objContext);
        case CallStateName::TERMINATING:
            return new TerminatingState(m_objContext);
    }
}
