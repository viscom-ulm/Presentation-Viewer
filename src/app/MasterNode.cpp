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

    MasterNode::MasterNode(ApplicationNodeInternal* appNode) 
	: ApplicationNodeImplementation{appNode}
	, current_slide_(0)
	, numberOfSlides_(0)
#ifdef VISCOM_USE_SGCT
	, initialized_(false)
#endif
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

    void MasterNode::loadSlides() {
        auto slideNumber = 1;
        while (exists_test3("../resources/slides/Slide" + std::to_string(slideNumber) + ".PNG"))
        {
            const auto texture = GetTextureManager().GetResource("/slides/Slide" + std::to_string(slideNumber) + ".PNG");
            if (!texture) break;
            texture_slides_.push_back(texture);
            slideNumber++;
        }
        numberOfSlides_ = texture_slides_.size();
		if(numberOfSlides_ > 0)
			setCurrentTexture(texture_slides_[0]);
#ifdef VISCOM_USE_SGCT
        clientReceivedTexture_ = std::vector<std::vector<bool>>(6,std::vector<bool>(numberOfSlides_,false));
#endif
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
				auto data = texture_slides_[current_slide_]->getImageDataUC();
                setCurrentTexture(texture_slides_[current_slide_]);
                return true;
            }
        default: return false;
        }
    }
#ifdef VISCOM_USE_SGCT
	bool MasterNode::DataTransferCallback(void* receivedData, int receivedLength, int packageID, int clientID)
	{
		const auto state = *reinterpret_cast<ClientState*>(receivedData);
		LOG(INFO) << "client " << state.clientId << " synced texture " << state.textureIndex;
		clientReceivedTexture_[state.clientId - 1][state.textureIndex] = true;
		bool initialized = true;
		for (auto& vec : clientReceivedTexture_)
		{
			for (auto& isInitialized : vec)
			{
				initialized = initialized && isInitialized;
				if (initialized) {
					initialized_ = true;
					setCurrentTexture(getCurrentSlide());
				}
			}
		}
		return true;
	}


    void MasterNode::EncodeData()
    {
        ApplicationNodeImplementation::EncodeData();
        sgct::SharedData::instance()->writeInt32(&sharedIndex_);
        if(!initialized_)
        {
            for (auto clientId = 0; clientId < 6; ++clientId)
            {
                for (auto i = 0; i < numberOfSlides_; ++i)
                {
                    if (!clientReceivedTexture_[clientId][i])
                    {
                        const auto tex = texture_slides_[i];
                        MasterMessage mm(true, numberOfSlides_, i, tex->getDescriptor());
                        sgct::Engine::instance()->transferDataToNode(tex->getImageDataUC(), tex->getDescriptor().length(), PackageID::Data, clientId);
                        sgct::Engine::instance()->transferDataToNode(&mm, sizeof(MasterMessage), PackageID::Descriptor, clientId);
                    }
                }
            }
        }
    }

    void MasterNode::PreSync()
    {
        ApplicationNodeImplementation::PreSync();
        sharedIndex_.setVal(current_slide_);
    }
#endif
}
