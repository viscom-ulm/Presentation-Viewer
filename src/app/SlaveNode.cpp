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
#ifdef VISCOM_CLIENTGUI
        ImGui::ShowTestWindow();
#endif

        // always do this call last!
        SlaveNodeInternal::Draw2D(fbo);
    }

    SlaveNode::~SlaveNode()
    {
        glDeleteTextures(static_cast<GLsizei>(textureIds_.size()), textureIds_.data());
    }

    void SlaveNode::addTexture(int index, const SlideTexDescriptor& descriptor, const std::vector<std::uint8_t>& data)
    {
        if (data.empty()) return;
        if (!isSynced(index))
        {
            glBindTexture(GL_TEXTURE_2D, textureIds_[index]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, descriptor.desc_.internalFormat_, static_cast<GLsizei>(descriptor.width_), static_cast<GLsizei>(descriptor.height_), 0, descriptor.desc_.format_, descriptor.desc_.type_, data.data());

            // buffered_image_data_.push_back(std::make_pair(descriptor, data));

        }
    }

    void SlaveNode::DecodeData()
    {
#ifdef VISCOM_USE_SGCT
        SlaveNodeInternal::DecodeData();
        sgct::SharedData::instance()->readInt32(&sharedIndex_);
#endif
    }

    bool SlaveNode::DataTransferCallback(void* receivedData, int receivedLength, int packageID, int clientID)
    {
        switch (PackageID(packageID)) 
        { 
        // case Descriptor: 
        //     if(!hasDescriptor_)
        //     {
        //         masterMessage_ = *reinterpret_cast<MasterMessage*>(receivedData);
        //         if (number_of_slides_ <= 0)
        //             number_of_slides_ = static_cast<int>(masterMessage_.numberOfSlide);
        //         hasDescriptor_ = true;
        //     }
        //     break;
        case PresentationData:
        {
            // number_of_slides_ = *reinterpret_cast<std::size_t*>(receivedData);
            glDeleteTextures(static_cast<GLsizei>(textureIds_.size()), textureIds_.data());
            textureIds_.resize(0);
            hasTextures_.resize(0);
            textureIds_.resize(*reinterpret_cast<std::size_t*>(receivedData), 0);
            hasTextures_.resize(textureIds_.size(), false);
            glGenTextures(static_cast<GLsizei>(textureIds_.size()), textureIds_.data());

#ifdef VISCOM_USE_SGCT
            ClientState clientState(static_cast<int>(sgct_core::ClusterManager::instance()->getThisNodeId()));
            clientState.numberOfSlides = static_cast<int>(textureIds_.size());
            sgct::Engine::instance()->transferDataToNode(&clientState, sizeof(ClientState), 0, 0);
#endif
        } break;
        case TextureData:
        {
            TextureHeaderMessage header;
            memcpy(&header, receivedData, sizeof(TextureHeaderMessage));
            std::vector<std::uint8_t> textureData;
            textureData.resize(header.descriptor.width_ * header.descriptor.height_ * header.descriptor.desc_.bytesPP_);
            assert(receivedLength == sizeof(TextureHeaderMessage) + textureData.size());
            memcpy(textureData.data(), reinterpret_cast<std::uint8_t*>(receivedData) + sizeof(TextureHeaderMessage), textureData.size());

#ifdef VISCOM_USE_SGCT
            ClientState clientState(static_cast<int>(sgct_core::ClusterManager::instance()->getThisNodeId()));
            clientState.textureIndex = static_cast<int>(header.index);
            sgct::Engine::instance()->transferDataToNode(&clientState, sizeof(ClientState), 0, 0);
#endif
            addTexture(static_cast<int>(header.index), header.descriptor, textureData);
        } break;


        //    if(!hasData_)
        //    {
        //        const auto size = receivedLength / sizeof(unsigned char);
        //        std::vector<std::uint8_t> vuc(static_cast<std::uint8_t*>(receivedData), static_cast<std::uint8_t*>(receivedData) + size);
        //        data_.resize(size);
        //        std::copy(vuc.begin(), vuc.end(), data_.begin());
        //        hasData_ = true;
        //    }
        //     break;
        // default: return false;
        // }
        // if(hasData_ && hasDescriptor_)
        // {
        //     ClientState clientState(static_cast<int>(sgct_core::ClusterManager::instance()->getThisNodeId()), static_cast<int>(masterMessage_.index));
        //     sgct::Engine::instance()->transferDataToNode(&clientState, sizeof(ClientState), 0, 0);
        //     addTexture(static_cast<int>(masterMessage_.index), masterMessage_.descriptor, data_);
        // 
        //     data_.resize(0);
        //     masterMessage_ = MasterMessage();
        //     hasData_ = false,
        //     hasDescriptor_ = false;
        }
        return true;
    }

    void SlaveNode::UpdateSyncedInfo()
    {
        SlaveNodeInternal::UpdateSyncedInfo();
        // for(auto i = 0; i < buffered_image_data_.size(); ++i)
        // {
        //     if (textures_.find(i) == textures_.end())
        //     {
        //         const auto texture = GetTextureManager().GetResource(std::string("texture").append(std::to_string(i)), buffered_image_data_[i].first, buffered_image_data_[i].second);
        //         textures_[i] = texture;
        //     }
        // }
        current_slide_ = sharedIndex_.getVal();
        if (hasTextures_[current_slide_])
        {
            setCurrentTexture(textureIds_[current_slide_]);
        }
    }
}
