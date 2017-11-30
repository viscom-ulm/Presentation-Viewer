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
        SlaveNodeInternal{ appNode }
    {
    }

    void SlaveNode::InitOpenGL()
    {
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

    void SlaveNode::addTexture(int index, TextureInfo info, std::vector<float> data)
    {
        if (data.size() <= 0) return;
        if(textures_.find(index) == textures_.end())
        {
            auto tex = std::make_shared<Texture>(info, data, GetApplication());
            textures_[index] = std::move(tex);
        }
    }

    SlaveNode::~SlaveNode() = default;
#ifdef VISCOM_USE_SGCT
    void SlaveNode::DecodeData()
    {
        SlaveNodeInternal::DecodeData();
        sgct::SharedData::instance()->readInt32(&sharedIndex_);
        sgct::SharedData::instance()->readObj(&sharedData_);
        sgct::SharedData::instance()->readVector(&sharedVector_);
    }

    void SlaveNode::UpdateSyncedInfo()
    {
        SlaveNodeInternal::UpdateSyncedInfo();
        const auto index = sharedIndex_.getVal();
        addTexture(index, sharedData_.getVal(), sharedVector_.getVal());
        if(textures_.find(index) != textures_.end())
        {
            setCurrentTexture(textures_[index]);
        }
    }
#endif
}
