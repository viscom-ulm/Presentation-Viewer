/**
 * @file   WorkerNode.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Declaration of the ApplicationNodeImplementation for the worker node.
 */

#pragma once

#include "app/ApplicationNodeImplementation.h"

namespace viscom {

    class WorkerNode final : public ApplicationNodeImplementation
    {
    public:
        explicit WorkerNode(ApplicationNodeInternal* appNode);
        virtual ~WorkerNode() override;
        virtual void InitOpenGL() override;

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
