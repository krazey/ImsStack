#ifndef INTERFACE_MTC_APP_H_
#define INTERFACE_MTC_APP_H_

class IMtcApp
{
public:
    virtual ~IMtcApp(){};
    virtual void Start() = 0;
    virtual void Stop() = 0;
};

#endif
