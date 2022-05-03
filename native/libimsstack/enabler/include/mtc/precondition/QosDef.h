#ifndef QOS_DEF_H_
#define QOS_DEF_H_

enum class QosLossPolicy
{
    MAINTAIN = 0,
    MODIFY = 1,
    RELEASE = 2,
};

enum class QosCheckType
{
    ALL_STATUS = 0,
    LOCAL_STATUS = 1,
    REMOTE_STATUS = 2,
};

enum class QosStatus
{
    IDLE = 0,
    AVAILABLE = 1,
    LOST = 2,
};

enum class QosTimerType
{
    WAIT_AVAILABLE = 0,
    GUARD_INACTIVE = 1,
    FORCE_AVAILABLE = 2,
};

#endif
