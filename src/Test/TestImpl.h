#ifndef VO_CMPE460_QUEUEDRATEGROUPIMPL_H
#define VO_CMPE460_QUEUEDRATEGROUPIMPL_H

#include <src/Test/TestComponentAc.hpp>

namespace Svc
{
    class TestImpl : public TestComponentBase
    {
    public:
        TestImpl();
        void init(
                NATIVE_INT_TYPE queueDepth, /*!< The queue depth*/
                NATIVE_INT_TYPE instance = 0 /*!< The instance number*/
        );
    PRIVATE:
        void schedIn_handler(NATIVE_INT_TYPE portNum, NATIVE_UINT_TYPE context) override;
    };
}

#endif //VO_CMPE460_QUEUEDRATEGROUPIMPL_H
