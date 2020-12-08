/**
 * @file   WorkerNode.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Implementation of the worker application node.
 */

#include "core/open_gl.h"
#include "WorkerNode.h"
#include <imgui.h>

namespace viscom {

    WorkerNode::WorkerNode(ApplicationNodeInternal* appNode) :
        ApplicationNodeImplementation{ appNode }
    {
        sharedIndex_.setVal(-1);
#ifdef VISCOM_USE_SGCT
        spdlog::info("This is client: {}", sgct_core::ClusterManager::instance()->getThisNodeId());
#endif
        spdlog::info("InitOpenGL::glGetError(): {}", glGetError());
    }

    WorkerNode::~WorkerNode() = default;

    void WorkerNode::Draw2D(FrameBuffer& fbo)
    {
        fbo.DrawToFBO([]() {
            if constexpr (SHOW_CLIENT_GUI) ImGui::ShowDemoWindow();
        });
    }

    void WorkerNode::DecodeData()
    {
#ifdef VISCOM_USE_SGCT
        sgct::SharedData::instance()->readInt32(&sharedIndex_);
#endif
    }

    bool WorkerNode::DataTransferCallback(void* receivedData, int receivedLength, std::uint16_t packageID, int clientID)
    {
        switch (static_cast<SlideMessages>(packageID))
        {
        case SlideMessages::SlideNamesTransfer:
            requestSlideNames_ = DecodeSlideNames(receivedData, receivedLength);
            waitForSlides_ = false;
            break;
        case SlideMessages::ResetPresentation:
            sharedIndex_.setVal(-1);
            waitForSlides_ = false;
            LoadTextures(std::vector<std::string>());
            break;
        }
        return true;
    }

    void WorkerNode::RequestSlides()
    {
        int tmp = 0;
        TransferDataToNode(&tmp, sizeof(int), static_cast<std::uint16_t>(SlideMessages::RequestSlideNames), 0);
        waitForSlides_ = true;
    }

    inline void CheckSlideNameLength(std::size_t receivedLength, std::size_t sizeToRead) {
        if (receivedLength < sizeToRead) {
            spdlog::error("Encoded slide names are corrupt.");
            throw std::runtime_error("Encoded slide names are corrupt.");
        }
    }
    std::vector<std::string> WorkerNode::DecodeSlideNames(const void* receivedData, int receivedLength)
    {
        std::size_t restLength = receivedLength;
        CheckSlideNameLength(restLength, sizeof(std::size_t));

        std::vector<std::string> slideNames;
        auto dataPtr = reinterpret_cast<const std::uint8_t*>(receivedData);
        slideNames.resize(reinterpret_cast<const std::size_t*>(dataPtr)[0]);
        dataPtr += sizeof(std::size_t);
        restLength -= sizeof(std::size_t);

        for (auto& slideName : slideNames) {
            CheckSlideNameLength(restLength, sizeof(std::size_t));

            slideName.resize(reinterpret_cast<const std::size_t*>(dataPtr)[0]);
            dataPtr += sizeof(std::size_t);
            restLength -= sizeof(std::size_t);

            CheckSlideNameLength(restLength, slideName.size());
            memcpy(slideName.data(), dataPtr, slideName.size());
            dataPtr += slideName.size();
            restLength -= slideName.size();
        }

        return slideNames;
    }

    void WorkerNode::UpdateSyncedInfo()
    {
        if (waitForSlides_) return;
        if (!requestSlideNames_.empty()) {
            LoadTextures(requestSlideNames_);
            requestSlideNames_.clear();
        }

#ifdef VISCOM_USE_SGCT
        int currentSlideFromMaster = sharedIndex_.getVal();
        SetCurrentSlide(currentSlideFromMaster);

        if (currentSlideFromMaster != GetCurrentSlide()) RequestSlides();
#endif
    }

}
