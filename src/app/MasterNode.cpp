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
        ApplicationNodeImplementation{ appNode }, current_slide_(-1), numberOfSlides_(0)
    {
    }

    void MasterNode::InitOpenGL()
    {
        ApplicationNodeImplementation::InitOpenGL();
        loadSlides();
        MasterMessage mm;
        mm.numberOfSlide = numberOfSlides_;
        for(auto clientId = 0; clientId < 6; ++clientId)
        {
            sgct::Engine::instance()->transferDataToNode(&mm, sizeof(MasterMessage), 0, clientId);
        }
    }

    MasterNode::~MasterNode() = default;

    void MasterNode::Draw2D(FrameBuffer& fbo)
    {
        ApplicationNodeImplementation::Draw2D(fbo);
    }

    void MasterNode::loadSlides() {
        auto slideNumber = 1;
        while (exists_test3("resources/slides/Folie" + std::to_string(slideNumber) + ".PNG"))
        {
            const auto texture = GetTextureManager().GetResource("/slides/Folie" + std::to_string(slideNumber) + ".PNG");
            if (!texture) break;
            texture_slides_.push_back(texture);
            slideNumber++;
        }
        numberOfSlides_ = texture_slides_.size();
    }



    bool MasterNode::KeyboardCallback(int key, int scancode, int action, int mods)
    {
        if (ApplicationNodeBase::KeyboardCallback(key, scancode, action, mods)) return true;

        switch (key)
        {
        case GLFW_KEY_LEFT:
            if (action == GLFW_REPEAT || action == GLFW_PRESS) {
                if (current_slide_ - 1 >= 0) {
                    current_slide_--;
                }
                setCurrentTexture(texture_slides_[current_slide_]);
                return true;
            }

        case GLFW_KEY_RIGHT:
            if (action == GLFW_REPEAT || action == GLFW_PRESS) {
                if (current_slide_ + 1 < numberOfSlides_) {
                    current_slide_++;
                }
                setCurrentTexture(texture_slides_[current_slide_]);
                return true;
            }
        default: return false;
        }
    }

    bool MasterNode::DataTransferCallback(void* receivedData, int receivedLength, int packageID, int clientID)
    {
        return true;
    }


#ifdef VISCOM_USE_SGCT
    void MasterNode::EncodeData()
    {
        ApplicationNodeImplementation::EncodeData();
        sgct::SharedData::instance()->writeInt32(&sharedIndex_);
    }

    void MasterNode::PreSync()
    {
        ApplicationNodeImplementation::PreSync();
        sharedIndex_.setVal(current_slide_);
    }
#endif
}
