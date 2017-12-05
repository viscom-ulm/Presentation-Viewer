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
        SlaveNodeInternal{appNode}, current_slide_(0), number_of_slides_(-1)
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
            LOG(INFO) << "slide " << index << " for client " << sgct_core::ClusterManager::instance()->getThisNodeId();
            setCurrentTexture(tex);
        }
        ClientState state;
        state.clientId = sgct_core::ClusterManager::instance()->getThisNodeId();
        state.textureIndex = index;
        sgct::Engine::instance()->transferDataToNode(&state, sizeof(ClientState), 0, 0);
    }

    SlaveNode::~SlaveNode() = default;
#ifdef VISCOM_USE_SGCT
    void SlaveNode::DecodeData()
    {
        SlaveNodeInternal::DecodeData();
        sgct::SharedData::instance()->readInt32(&sharedIndex_);;
    }

    bool SlaveNode::DataTransferCallback(void* receivedData, int receivedLength, int packageID, int clientID)
    {
        const auto mm = *reinterpret_cast<MasterMessage*>(receivedData);
        if(!mm.hasData)
        {
            number_of_slides_ = mm.numberOfSlide;
        } else
        {
            addTexture(mm.index, mm.descriptor, mm.data);
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
