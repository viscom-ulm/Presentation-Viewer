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
    void SlaveNode::DecodeData()
    {
        SlaveNodeInternal::DecodeData();
        sgct::SharedData::instance()->readObj(&sharedData_);
    }

    void SlaveNode::UpdateSyncedInfo()
    {
        SlaveNodeInternal::UpdateSyncedInfo();
		setCurrentTexture(static_cast<std::shared_ptr<Texture>>(sharedData_.getVal()));
    }
#endif
}
