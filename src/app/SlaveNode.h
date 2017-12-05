/**
 * @file   SlaveNode.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Declaration of the ApplicationNodeImplementation for the slave node.
 */

#pragma once

#include "core/SlaveNodeHelper.h"
#include "MasterNode.h"

namespace viscom {

    class SlaveNode final : public SlaveNodeInternal
    {
    public:
        explicit SlaveNode(ApplicationNodeInternal* appNode);
        virtual ~SlaveNode() override;
        virtual void InitOpenGL() override;
        void Draw2D(FrameBuffer& fbo) override;
        void addTexture(int index, TextureDescriptor descriptor, unsigned char* data);
        bool isSynced(int index) const { return textures_.find(index) != textures_.end(); };
        virtual bool DataTransferCallback(void* receivedData, int receivedLength, int packageID, int clientID) override;

#ifdef VISCOM_USE_SGCT
        virtual void UpdateSyncedInfo() override;
        virtual void DecodeData() override;

    private:
        /** Holds the data shared by the master. */
        sgct::SharedInt32 sharedIndex_;
        int current_slide_;
        int number_of_slides_;
        std::map<int, std::shared_ptr<Texture>> textures_;
#endif
    };
}
