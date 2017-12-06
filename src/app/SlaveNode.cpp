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

    SlaveNode::SlaveNode(ApplicationNodeInternal* appNode) :
        SlaveNodeInternal{appNode}, current_slide_(0), hasData_(false), hasDescriptor_(false)
    {
    }

    void SlaveNode::InitOpenGL()
    {
        LOG(INFO) << "This is client: " << sgct_core::ClusterManager::instance()->getThisNodeId();
        SlaveNodeInternal::InitOpenGL();
    }

    void SlaveNode::Draw2D(FrameBuffer& fbo)
    {
#ifdef VISCOM_CLIENTGUI
        ImGui::ShowTestWindow();
#endif

        // always do this call last!
        SlaveNodeInternal::Draw2D(fbo);
    }

    void SlaveNode::addTexture(int index, TextureDescriptor descriptor, unsigned char* data)
    {
        if (!data) return;
        if(!isSynced(index))
        {
            const auto tex = std::make_shared<Texture>(descriptor, data, GetApplication());
            textures_[index] = tex;
            LOG(INFO) << "slide " << index << " for client " << sgct_core::ClusterManager::instance()->getThisNodeId()  << " with textureID: " << tex->getTextureId();
            setCurrentTexture(textures_[current_slide_]);
        }
    }

    SlaveNode::~SlaveNode() = default;
#ifdef VISCOM_USE_SGCT
    void SlaveNode::DecodeData()
    {
        SlaveNodeInternal::DecodeData();
        sgct::SharedData::instance()->readInt32(&sharedIndex_);
    }

    bool SlaveNode::DataTransferCallback(void* receivedData, int receivedLength, int packageID, int clientID)
    {
        unsigned char* data = nullptr;
        switch (PackageID(packageID)) 
        { 
        case Descriptor: 
            if(!hasDescriptor_)
            {
                masterMessage_ = *reinterpret_cast<MasterMessage*>(receivedData);
                hasDescriptor_ = true;
            }
            break;
        case Data: 
           if(!hasData_)
           {
               data = reinterpret_cast<unsigned char*>(receivedData);
               data_.resize(receivedLength);
               std::copy(data, data + receivedLength, data_.begin());
               hasData_ = true;
           }
            break;
        default: return false;
        }
        if(hasData_ && hasDescriptor_)
        {
            ClientState clientState(sgct_core::ClusterManager::instance()->getThisNodeId(),masterMessage_.index);
            sgct::Engine::instance()->transferDataToNode(&clientState, sizeof(ClientState), 0, 0);
            addTexture(masterMessage_.index, masterMessage_.descriptor, data_.data());
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
        current_slide_= sharedIndex_.getVal();
    }
#endif
}
