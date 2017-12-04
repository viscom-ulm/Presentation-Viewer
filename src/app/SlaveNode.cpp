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
        SlaveNodeInternal{appNode}, current_slide_(0), number_of_slides_(0), allTexturesLoaded_(false)
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

    void SlaveNode::addTexture(int index, TextureInfo info, std::vector<unsigned char> data)
    {
        if (data.size() <= 0) return;
        if(!isSynced(index))
        {
            const auto tex = std::make_shared<Texture>(info, data, GetApplication());
            textures_[index] = tex;
            LOG(INFO) << "slide " << index << " for client " << sgct_core::ClusterManager::instance()->getThisNodeId();
            setCurrentTexture(tex);
        }
        ClientState state;
        state.clientId = sgct_core::ClusterManager::instance()->getThisNodeId();
        state.textureIndex = index;
        state.synced = true;
        sgct::Engine::instance()->transferDataToNode(&state, sizeof(ClientState), 0, 0);
    }

    SlaveNode::~SlaveNode() = default;
#ifdef VISCOM_USE_SGCT
    void SlaveNode::DecodeData()
    {
        SlaveNodeInternal::DecodeData();
        //sgct::SharedData::instance()->readBool(&sharedInit_);
        sgct::SharedData::instance()->readInt32(&sharedIndex_);
        sgct::SharedData::instance()->readInt32(&sharedNumberOfSlides_);
        sgct::SharedData::instance()->readObj(&sharedData_);
        sgct::SharedData::instance()->readVector(&sharedVector_);
    }

    void SlaveNode::UpdateSyncedInfo()
    {
        SlaveNodeInternal::UpdateSyncedInfo();
        current_slide_= sharedIndex_.getVal();
        number_of_slides_ = sharedNumberOfSlides_.getVal();
        if (current_slide_ < number_of_slides_) {
            addTexture(sharedIndex_.getVal(), sharedData_.getVal(), sharedVector_.getVal());
        }
        else if(current_slide_ == number_of_slides_ && !allTexturesLoaded_)
        {
            allTexturesLoaded_ = true;
        }

        if(allTexturesLoaded_)
        {
            setCurrentTexture(textures_[current_slide_]);
        }
    }
#endif
}
