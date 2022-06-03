#ifndef USSI_DEF_H_
#define USSI_DEF_H_

enum class UssiModeType
{
    ERROR = -1,
    NOTIFY = 0,
    REQUEST = 1,
    NONE = 2
};

enum class UssiError
{
    CODE_NONE = 0,  // no error-code specified
    CODE_1 = 1,     // unspecified. default error-code
    CODE_2 = 2,     // language/alphabet not supported
    CODE_3 = 3,     // unexpected data value
    CODE_4 = 4      // USSD-busy
};

enum class UssiNextAction
{
    NOTHING = 0,
    SEND_INFO_WITH_NOTIFY_ELEMENT = 1,
    SEND_INFO_WITH_ERROR_CODE = 2,
    SEND_INFO_WITH_ERROR_CODE_AND_TERMINATE = 3
};

struct UssiResult
{
public:
    UssiResult(UssiNextAction _eAction, UssiError _eErrorCode) :
            eAction(_eAction),
            eErrorCode(_eErrorCode)
    {
    }

    UssiResult& operator=(const UssiResult& objRhs)
    {
        if (this != &objRhs)
        {
            eAction = objRhs.eAction;
            eErrorCode = objRhs.eErrorCode;
        }

        return *this;
    }

    UssiNextAction eAction;
    UssiError eErrorCode;
};

#endif
