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
        : SlaveNodeInternal{appNode}
#ifdef VISCOM_USE_SGCT
          , current_slide_(0)
          , number_of_slides_(0), hasData_(false)
          , hasDescriptor_(false)
#endif
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

    SlaveNode::~SlaveNode() = default;
#ifdef VISCOM_USE_SGCT
    void SlaveNode::addTexture(int index, TextureDescriptor descriptor, std::vector<cType> data)
    {
        if (data.size() <= 0) return;
        if (!isSynced(index))
        {
            buffered_image_data_.push_back(std::make_pair(descriptor, data));
        }
    }

    void SlaveNode::DecodeData()
    {
        SlaveNodeInternal::DecodeData();
        sgct::SharedData::instance()->readInt32(&sharedIndex_);
    }

    bool SlaveNode::DataTransferCallback(void* receivedData, int receivedLength, int packageID, int clientID)
    {
        std::vector<cType> data ;
        switch (PackageID(packageID)) 
        { 
        case Descriptor: 
            if(!hasDescriptor_)
            {
                masterMessage_ = *reinterpret_cast<MasterMessage*>(receivedData);
                if (number_of_slides_ <= 0)
                    number_of_slides_ = masterMessage_.numberOfSlide;
                hasDescriptor_ = true;
            }
            break;
        case Data: 
           if(!hasData_)
           {
               //data = static_cast<std::vector<cType>>(receivedData);
               const auto size = receivedLength / sizeof(cType);
               std::vector<cType> vuc(static_cast<cType*>(receivedData), static_cast<cType*>(receivedData) + size);
               data_.resize(size);
               //std::copy(data, data + size, data_.begin());
               std::copy(vuc.begin(), vuc.end(), data_.begin());
               hasData_ = true;
           }
            break;
        default: return false;
        }
        if(hasData_ && hasDescriptor_)
        {
            ClientState clientState(sgct_core::ClusterManager::instance()->getThisNodeId(),masterMessage_.index);
            sgct::Engine::instance()->transferDataToNode(&clientState, sizeof(ClientState), 0, 0);
            addTexture(masterMessage_.index, masterMessage_.descriptor, data_);
            data_.resize(0);
            masterMessage_ = MasterMessage();
            hasData_ = false,
            hasDescriptor_ = false;
        }
        return true;
    }

    void SlaveNode::UpdateSyncedInfo()
    {
        SlaveNodeInternal::UpdateSyncedInfo();
        for(auto i = 0; i < buffered_image_data_.size(); ++i)
        {
            if (textures_.find(i) == textures_.end())
            {
                const auto texture = GetTextureManager().GetResource(std::string("texture").append(std::to_string(i)), buffered_image_data_[i].first, buffered_image_data_[i].second);
                textures_[i] = texture;
            }
        }
        current_slide_ = sharedIndex_.getVal();
        if (textures_.find(current_slide_) != textures_.end())
        {
            setCurrentTexture(textures_[current_slide_]);
        }
    }
#endif
}
