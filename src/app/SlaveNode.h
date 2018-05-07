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
        

        void RequestSlides();
        std::vector<std::string> DecodeSlideNames(const void* receivedData, int receivedLength);

        virtual bool DataTransferCallback(void* receivedData, int receivedLength, std::uint16_t packageID, int clientID) override;
        virtual void UpdateSyncedInfo() override;
        virtual void DecodeData() override;

    private:
#ifdef VISCOM_USE_SGCT
        /** Holds the data shared by the master. */
        sgct::SharedInt32 sharedIndex_;
#endif

        /** The names of slides to be requested. */
        std::vector<std::string> requestSlideNames_;
        /** Wait for requested slide names to return from master. */
        bool waitForSlides_ = false;
    };
}
