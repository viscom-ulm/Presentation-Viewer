/**
 * @file   MasterNode.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Implementation of the master application node.
 */

#include "MasterNode.h"
#include <imgui.h>

namespace viscom {

    MasterNode::MasterNode(ApplicationNodeInternal* appNode) :
        ApplicationNodeImplementation{ appNode }
    {
    }

    MasterNode::~MasterNode() = default;

    void MasterNode::Draw2D(FrameBuffer& fbo)
    {
        ApplicationNodeImplementation::Draw2D(fbo);
    }

#ifdef VISCOM_USE_SGCT
    void MasterNode::EncodeData()
    {
        ApplicationNodeImplementation::EncodeData();
        sgct::SharedData::instance()->writeObj(&sharedData_);
    }

    void MasterNode::DecodeData()
    {
        ApplicationNodeImplementation::DecodeData();
        sgct::SharedData::instance()->readObj(&sharedData_);
    }

    void MasterNode::PreSync()
    {
        ApplicationNodeImplementation::PreSync();
        sharedData_.setVal(getCurrentSlide());
    }
#endif
}
