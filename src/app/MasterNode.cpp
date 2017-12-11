/**
* @file   MasterNode.cpp
* @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
* @date   2016.11.25
*
* @brief  Implementation of the master application node.
*/

#include "MasterNode.h"
#include <imgui.h>
#include "Filesystem.h"
#include <windows.h>


namespace viscom {

    MasterNode::MasterNode(ApplicationNodeInternal* appNode)
        : ApplicationNodeImplementation{appNode}
          , current_slide_(0)
          , numberOfSlides_(0)
          , inputDir_("D:/dev")
          , inputDirectorySelected_(false)
#ifdef VISCOM_USE_SGCT
	, initialized_(false)
#endif
    {
    }

    void MasterNode::InitOpenGL()
    {
        ApplicationNodeImplementation::InitOpenGL();
    }

    MasterNode::~MasterNode() = default;

    void MasterNode::Draw2D(FrameBuffer& fbo)
    {
        fbo.DrawToFBO([&]() {
            
            
            ImGui::SetNextWindowPos(ImVec2(700, 60), ImGuiSetCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
            if (!inputDirectorySelected_ && ImGui::Begin("Select input directory", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_ShowBorders))
            {
                ImGui::Text(inputDir_.data());
                for(auto &path : getDirectoryContent(inputDir_))
                {
                    bool selected = false;
                    ImGui::Selectable(path.data(), &selected);
                    if(selected)
                    {
                        if(path == "..")
                        {
                            removeLastDir(inputDir_);
                        } else
                        {
                            inputDir_.append("/").append(path);
                        }
                    }
                }

                inputDirectorySelected_ = ImGui::Button("Select Folder", ImVec2(50, 20));
                if(inputDirectorySelected_)
                {
                    loadSlides();
                }
                ImGui::End();
            }
            
        });
        ApplicationNodeImplementation::Draw2D(fbo);
    }

    void MasterNode::loadSlides() {
        auto slideNumber = 1;
        std::vector<std::string> slides = getFiles(inputDir_);
        std::sort(slides.begin(), slides.end(), viscom::comparePaths);

        for(auto& slide : slides)
        {
            const auto texture = GetTextureManager().GetResource(slide);
            if (!texture) break;
            texture_slides_.push_back(texture);
        }
        numberOfSlides_ = texture_slides_.size();
#ifdef VISCOM_USE_SGCT
        clientReceivedTexture_ = std::vector<std::vector<bool>>(6,std::vector<bool>(numberOfSlides_,false));
        for (auto clientId = 0; clientId < 6; ++clientId)
            {
                for (auto i = 0; i < numberOfSlides_; ++i)
                {
                    if (!clientReceivedTexture_[clientId][i])
                    {
                        const auto tex = texture_slides_[i];
                        MasterMessage mm(numberOfSlides_, i, tex->getDescriptor());
                        sgct::Engine::instance()->transferDataToNode(tex->data(), tex->getImageData().size(), PackageID::Data, clientId);
                        sgct::Engine::instance()->transferDataToNode(&mm, sizeof(MasterMessage), PackageID::Descriptor, clientId);
                    }
                }
            }
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
                setCurrentTexture(texture_slides_[current_slide_]);
                return true;
            }
        default: return false;
        }
    }

    std::vector<std::string> MasterNode::getDirectoryContent(const std::string& dir, const bool returnFiles) const
    {
        std::vector<std::string> content;
        std::string path = dir;
        path.append("\\*");
        WIN32_FIND_DATA data;
        HANDLE hFind;
        if ((hFind = FindFirstFile(path.c_str(), &data)) != INVALID_HANDLE_VALUE) {
            do {
                if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && !returnFiles)
                {
                    content.push_back(std::string(data.cFileName));
                } 
                
                if(returnFiles)
                {
                    const auto file = dir + std::string("/").append(data.cFileName);
                    if(Resource::IsResource(file, GetApplication()))
                        content.push_back(file);
                }
            } while (FindNextFile(hFind, &data) != 0);
            FindClose(hFind);
        }
        return content;
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

        if (!initialized_)
        {
            for (auto clientId = 0; clientId < 6; ++clientId)
            {
                for (auto i = 0; i < numberOfSlides_; ++i)
                {
                    if (!clientReceivedTexture_[clientId][i])
                    {
                        const auto tex = texture_slides_[i];
                        MasterMessage mm(numberOfSlides_, i, tex->getDescriptor());
                        sgct::Engine::instance()->transferDataToNode(tex->data(), tex->getImageData().size(), PackageID::Data, clientId);
                        sgct::Engine::instance()->transferDataToNode(&mm, sizeof(MasterMessage), PackageID::Descriptor, clientId);
                    }
                }
            }
        }

		return true;
	}


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
