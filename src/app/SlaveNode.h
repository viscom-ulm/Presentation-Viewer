/**
 * @file   SlaveNode.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Declaration of the ApplicationNodeImplementation for the slave node.
 */

#pragma once

#include "MasterNode.h"
#include "core/SlaveNodeHelper.h"
#include <mutex>

namespace viscom {

    class SlaveNode final : public SlaveNodeInternal
    {
    public:
        explicit SlaveNode(ApplicationNodeInternal* appNode);
        virtual ~SlaveNode() override;
        virtual void InitOpenGL() override;
        void Draw2D(FrameBuffer& fbo) override;
        

        void addTexture(int index);
        bool isSynced(int index) const { return hasTextures_[index]; };
        void HandleSlideTransfer();
        virtual bool DataTransferCallback(void* receivedData, int receivedLength, int packageID, int clientID) override;
        virtual void UpdateSyncedInfo() override;
        virtual void DecodeData() override;

    private:
#ifdef VISCOM_USE_SGCT
        /** Holds the data shared by the master. */
        sgct::SharedInt32 sharedIndex_;
#endif
        std::size_t current_slide_;
        // std::size_t number_of_slides_;
        std::vector<GLuint> textureIds_;
        std::vector<bool> hasTextures_;

        std::mutex slideTransferMutex_;
        bool resetPresentation_ = false;
        std::vector<std::size_t> resetTextures_;
        std::vector<std::pair<SlideTexDescriptor, std::vector<std::uint8_t>>> buffered_image_data_;
    };
}
