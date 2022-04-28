#ifndef INTERFACE_MTS_APP_H_
#define INTERFACE_MTS_APP_H_

class IMtsApp
{
public:
    virtual void Start() = 0;
    virtual void Stop() = 0;
};

#endif
