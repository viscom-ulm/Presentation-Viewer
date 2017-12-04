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
        ApplicationNodeImplementation{appNode}, current_slide_(-1), numberOfSlides_(0), init_(false)
    {
    }

	void MasterNode::InitOpenGL()
	{
		ApplicationNodeImplementation::InitOpenGL();
		loadSlides();
	}

    MasterNode::~MasterNode() = default;

    void MasterNode::Draw2D(FrameBuffer& fbo)
    {
        ApplicationNodeImplementation::Draw2D(fbo);
    }

	void MasterNode::loadSlides()
	{
	    auto slideNumber = 1;
		while (exists_test3("resources/slides/Folie" + std::to_string(slideNumber) + ".PNG"))
		{
		    const auto texture = GetTextureManager().GetResource("/slides/Folie" + std::to_string(slideNumber) + ".PNG");
			if (!texture) break;
			texture_slides_.push_back(texture);
			slideNumber++;
		}
		numberOfSlides_ = texture_slides_.size();
        acknowledged_.resize(6, std::make_pair(false,false));
        clientStates_ = std::vector<std::vector<bool>>(6, std::vector<bool>(numberOfSlides_, false));
		current_slide_ = 0;
		//setCurrentTexture(texture_slides_[current_slide_]);
	}

    bool MasterNode::sync(int index)
    {
        bool doSync = true;
        for(auto clientId = 0; clientId < 6; clientId++)
        {
            doSync = doSync && clientStates_[clientId][index];
        }
        return !doSync;
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
        const auto state = *reinterpret_cast<ClientState*>(receivedData);
        LOG(INFO) << "synced client: " <<  state.clientId << " index: " << state.textureIndex;
        clientStates_[state.clientId -1][state.textureIndex] = state.synced;
        return state.synced;
    }


#ifdef VISCOM_USE_SGCT
    void MasterNode::EncodeData()
    {
        ApplicationNodeImplementation::EncodeData();
        //sgct::SharedData::instance()->writeBool(&sharedInit_);
        sgct::SharedData::instance()->writeInt32(&sharedIndex_);
        sgct::SharedData::instance()->writeInt32(&sharedNumberOfSlides_);
        sgct::SharedData::instance()->writeObj(&sharedData_);
        sgct::SharedData::instance()->writeVector(&sharedVector_);
    }

    void MasterNode::PreSync()
    {
        ApplicationNodeImplementation::PreSync();
        //sharedInit_.setVal(init_);
        sharedIndex_.setVal(current_slide_);
        sharedNumberOfSlides_.setVal(numberOfSlides_);
        if (sync(current_slide_)) {
            sharedData_.setVal(getCurrentSlide()->getInfo());
            sharedVector_.setVal(getCurrentSlide()->getImageDataUC());
        } else
        {
            setCurrentTexture(getCurrentSlide());
            if(current_slide_+1 < numberOfSlides_)
                current_slide_++;
        }
    }

    /*void MasterNode::DecodeData()
    {
        ApplicationNodeImplementation::DecodeData();
    }*/
#endif
}
