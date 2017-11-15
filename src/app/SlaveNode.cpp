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
    void SlaveNode::EncodeData()
    {
        SlaveNodeInternal::EncodeData();
        sgct::SharedData::instance()->writeObj(&sharedData_);
    }

    void SlaveNode::DecodeData()
    {
        SlaveNodeInternal::DecodeData();
        sgct::SharedData::instance()->readObj(&sharedData_);
    }

    void SlaveNode::UpdateSyncedInfo()
    {
        SlaveNodeInternal::UpdateSyncedInfo();
        setCurrentSlide(static_cast<int>(sharedData_.getVal()));
    }
#endif
}
