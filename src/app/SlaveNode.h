/**
 * @file   SlaveNode.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Declaration of the ApplicationNodeImplementation for the slave node.
 */

#pragma once

#include "core/SlaveNodeHelper.h"

namespace viscom {

    class SlaveNode final : public SlaveNodeInternal
    {
    public:
        explicit SlaveNode(ApplicationNodeInternal* appNode);
        virtual ~SlaveNode() override;

        void Draw2D(FrameBuffer& fbo) override;

#ifdef VISCOM_USE_SGCT
        virtual void UpdateSyncedInfo() override;
        virtual void EncodeData() override;
        virtual void DecodeData() override;

    private:
        /** Holds the data shared by the master. */
        sgct::SharedObject<int> sharedData_;
#endif
    };
}
