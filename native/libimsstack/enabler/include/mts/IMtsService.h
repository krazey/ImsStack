#ifndef INTERFACE_MTS_SERVICE_H_
#define INTERFACE_MTS_SERVICE_H_

class ICoreService;

class IMtsService
{
public:
    virtual const AString& GetId() const;
    virtual ICoreService* GetICoreService();
};

#endif
