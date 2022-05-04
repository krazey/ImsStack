#ifndef OBJECT_ASYNC_DESTROYER_H_
#define OBJECT_ASYNC_DESTROYER_H_

#include "IMSTypeDef.h"
#include "IMSActivity.h"
#include "ImsMessage.h"

template <typename MtcObject>
class ObjectAsyncDestroyer final : public IMSActivity
{
public:
    inline explicit ObjectAsyncDestroyer() {}
    inline virtual ~ObjectAsyncDestroyer() {}
    ObjectAsyncDestroyer(IN const ObjectAsyncDestroyer&) = delete;
    ObjectAsyncDestroyer& operator=(IN const ObjectAsyncDestroyer&) = delete;

    // IMSActivity implementation
    inline IIMSActivityControl* GetController() override { return IMS_NULL; }
    inline IMS_BOOL DispatchMessage(IN IMSMSG& objMsg) override;

    inline void Destroy(IN MtcObject* pObject);

private:
    static const IMS_UINT32 MSG_DESTROY_OBJECT = 0;
};

PUBLIC VIRTUAL template <typename MtcObject>
inline IMS_BOOL ObjectAsyncDestroyer<MtcObject>::DispatchMessage(IN IMSMSG& objMsg)
{
    switch (objMsg.GetName())
    {
        case MSG_DESTROY_OBJECT:
            delete reinterpret_cast<MtcObject*>(objMsg.nLparam);
            return IMS_TRUE;
        default:
            return IMS_FALSE;
    }
}

PUBLIC
template <typename MtcObject>
inline void ObjectAsyncDestroyer<MtcObject>::Destroy(IN MtcObject* pObject)
{
    PostMessage(MSG_DESTROY_OBJECT, 0, reinterpret_cast<IMS_UINTP>(pObject));
}

#endif
