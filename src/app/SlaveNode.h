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
        virtual void InitOpenGL() override;
        void Draw2D(FrameBuffer& fbo) override;
        void addTexture(int index, TextureInfo info, std::vector<float> data);

#ifdef VISCOM_USE_SGCT
        virtual void UpdateSyncedInfo() override;
        virtual void DecodeData() override;

    private:
        /** Holds the data shared by the master. */
        sgct::SharedObject<TextureInfo> sharedData_;
        sgct::SharedVector<float> sharedVector_;
        sgct::SharedInt32 sharedIndex_;
        std::map<int, std::shared_ptr<Texture>> textures_;
#endif
    };
}
