/**
 * @file   SlaveNode.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Implementation of the slave application node.
 */

#include "SlaveNode.h"

#include <imgui.h>
#include <iostream>

namespace viscom {

    SlaveNode::SlaveNode(ApplicationNodeInternal* appNode)
        : SlaveNodeInternal{appNode}
    {
        sharedIndex_.setVal(-1);
    }

    void SlaveNode::InitOpenGL()
    {
        SlaveNodeInternal::InitOpenGL();
#ifdef VISCOM_USE_SGCT
        LOG(INFO) << "This is client: " << sgct_core::ClusterManager::instance()->getThisNodeId();
#endif
        LOG(INFO) << "InitOpenGL::glGetError(): " << glGetError();
    }

    void SlaveNode::Draw2D(FrameBuffer& fbo)
    {
        if constexpr (SHOW_CLIENT_GUI) ImGui::ShowTestWindow();

        // always do this call last!
        SlaveNodeInternal::Draw2D(fbo);
    }

    SlaveNode::~SlaveNode() = default;

    void SlaveNode::DecodeData()
    {
#ifdef VISCOM_USE_SGCT
        SlaveNodeInternal::DecodeData();
        sgct::SharedData::instance()->readInt32(&sharedIndex_);
#endif
    }

    bool SlaveNode::DataTransferCallback(void* receivedData, int receivedLength, std::uint16_t packageID, int clientID)
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

    void SlaveNode::RequestSlides()
    {
        int tmp = 0;
        GetApplication()->TransferDataToNode(&tmp,sizeof(int), static_cast<std::uint16_t>(SlideMessages::RequestSlideNames), 0);
        waitForSlides_ = true;
    }

    inline void CheckSlideNameLength(std::size_t receivedLength, std::size_t sizeToRead) {
        if (receivedLength < sizeToRead) {
            LOG(FATAL) << "Encoded slide names are corrupt.";
            throw std::runtime_error("Encoded slide names are corrupt.");
        }
    }
    std::vector<std::string> SlaveNode::DecodeSlideNames(const void* receivedData, int receivedLength)
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

    void SlaveNode::UpdateSyncedInfo()
    {
        SlaveNodeInternal::UpdateSyncedInfo();

        if (waitForSlides_) return;
        if (!requestSlideNames_.empty()) {
            LoadTextures(requestSlideNames_);
            requestSlideNames_.clear();
        }

#ifdef VISCOM_USE_SGCT
        int currentSlideFromMaster = sharedIndex_.getVal();
        std::cout << currentSlideFromMaster << std::endl;
        SetCurrentSlide(currentSlideFromMaster);

        if (currentSlideFromMaster != GetCurrentSlide()) RequestSlides();
#endif
    }
}
