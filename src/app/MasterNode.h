/**
 * @file   MasterNode.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Declaration of the ApplicationNodeImplementation for the master node.
 */

#pragma once

#include "../app/ApplicationNodeImplementation.h"
#ifdef WITH_TUIO
#include "core/TuioInputWrapper.h"
#endif

namespace viscom {

    class MasterNode final : public ApplicationNodeImplementation
    {
    public:
        explicit MasterNode(ApplicationNodeInternal* appNode);
        virtual ~MasterNode() override;

#ifdef VISCOM_USE_SGCT
        virtual void EncodeData() override;
        virtual void DecodeData() override;
        virtual void PreSync() override;
#endif
        void Draw2D(FrameBuffer& fbo) override;
    private:
#ifdef VISCOM_USE_SGCT
        /** Holds the data the master shares. */
        sgct::SharedObject<int> sharedData_;
#endif
    };
}
