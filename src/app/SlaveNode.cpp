/**
 * @file   SlaveNode.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Implementation of the slave application node.
 */

#include "SlaveNode.h"

#include <imgui.h>

namespace viscom {

    SlaveNode::SlaveNode(ApplicationNodeInternal* appNode)
        : SlaveNodeInternal{appNode}, current_slide_(0)
    {
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

    SlaveNode::~SlaveNode()
    {
        glDeleteTextures(static_cast<GLsizei>(textureIds_.size()), textureIds_.data());
    }

    void SlaveNode::addTexture(int index)
    {
        if (buffered_image_data_[index].second.empty()) return;
        if (!isSynced(index))
        {
            auto& descriptor = buffered_image_data_[index].first;
            auto& data = buffered_image_data_[index].second;
            glBindTexture(GL_TEXTURE_2D, textureIds_[index]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, descriptor.desc_.internalFormat_, static_cast<GLsizei>(descriptor.width_), static_cast<GLsizei>(descriptor.height_), 0, descriptor.desc_.format_, descriptor.desc_.type_, data.data());

            hasTextures_[index] = true;
        }
    }

    void SlaveNode::DecodeData()
    {
#ifdef VISCOM_USE_SGCT
        SlaveNodeInternal::DecodeData();
        sgct::SharedData::instance()->readInt32(&sharedIndex_);
#endif
    }

    bool SlaveNode::DataTransferCallback(void* receivedData, int receivedLength, std::uint16_t packageID, int clientID)
    {
        std::lock_guard<std::mutex> slideTransferLock{ slideTransferMutex_ };
        switch (PackageID(packageID)) 
        {
        case PresentationData:
        {
            buffered_image_data_.resize(*reinterpret_cast<std::size_t*>(receivedData));
            resetPresentation_ = true;
        } break;
        case TextureData:
        {
            if (hasTextures_.empty()) {
                // send command to resend all data.
#ifdef VISCOM_USE_SGCT
                ClientState clientState(static_cast<int>(sgct_core::ClusterManager::instance()->getThisNodeId()));
                sgct::Engine::instance()->transferDataToNode(&clientState, sizeof(ClientState), 1, 0);
#endif
                break;
            }

            auto header = reinterpret_cast<TextureHeaderMessage*>(receivedData);
            if (std::find(resetTextures_.begin(), resetTextures_.end(), header->index) != resetTextures_.end()) break;

            buffered_image_data_[header->index].first = header->descriptor;
            buffered_image_data_[header->index].second.resize(header->descriptor.width_ * header->descriptor.height_ * header->descriptor.desc_.bytesPP_);

            assert(receivedLength == sizeof(TextureHeaderMessage) + buffered_image_data_[header->index].second.size());
            memcpy(buffered_image_data_[header->index].second.data(),
                reinterpret_cast<std::uint8_t*>(receivedData) + sizeof(TextureHeaderMessage),
                buffered_image_data_[header->index].second.size());
            resetTextures_.push_back(header->index);
        } break;
        }
        return true;
    }

    void SlaveNode::HandleSlideTransfer()
    {
        if (resetPresentation_) {
            glDeleteTextures(static_cast<GLsizei>(textureIds_.size()), textureIds_.data());
            textureIds_.resize(0);
            hasTextures_.resize(0);
            textureIds_.resize(buffered_image_data_.size(), 0);
            hasTextures_.resize(textureIds_.size(), false);
            glGenTextures(static_cast<GLsizei>(textureIds_.size()), textureIds_.data());

#ifdef VISCOM_USE_SGCT
            ClientState clientState(static_cast<int>(sgct_core::ClusterManager::instance()->getThisNodeId()));
            clientState.numberOfSlides = static_cast<int>(textureIds_.size());
            sgct::Engine::instance()->transferDataToNode(&clientState, sizeof(ClientState), 0, 0);
#endif
            resetPresentation_ = false;
            return;
        }

        while (!resetTextures_.empty()) {
            auto resetTexture = resetTextures_.back();
            resetTextures_.pop_back();

            addTexture(static_cast<int>(resetTexture));

#ifdef VISCOM_USE_SGCT
            ClientState clientState(static_cast<int>(sgct_core::ClusterManager::instance()->getThisNodeId()));
            clientState.textureIndex = static_cast<int>(resetTexture);
            sgct::Engine::instance()->transferDataToNode(&clientState, sizeof(ClientState), 0, 0);
#endif
        }
    }

    void SlaveNode::UpdateSyncedInfo()
    {
        SlaveNodeInternal::UpdateSyncedInfo();

        std::unique_lock<std::mutex> slideTransferLock{ slideTransferMutex_ };
        if (slideTransferLock.owns_lock()) HandleSlideTransfer();

#ifdef VISCOM_USE_SGCT
        current_slide_ = sharedIndex_.getVal();
        if (hasTextures_.size() <= current_slide_) {
            if (hasTextures_.empty()) {
                // send command to resend all data.
                ClientState clientState(static_cast<int>(sgct_core::ClusterManager::instance()->getThisNodeId()));
                sgct::Engine::instance()->transferDataToNode(&clientState, sizeof(ClientState), 1, 0);
            }
        } else if (hasTextures_[current_slide_]) {
            setCurrentTexture(textureIds_[current_slide_]);
        }
#endif
    }
}
