/**
 * @file   CoordinatorNode.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.25
 *
 * @brief  Implementation of the coordinator application node.
 */

#include "CoordinatorNode.h"
#include "Filesystem.h"
#include <imgui.h>
#include <iostream>
#include <experimental/filesystem>


namespace viscom {

    CoordinatorNode::CoordinatorNode(ApplicationNodeInternal* appNode) :
        ApplicationNodeImplementation{appNode},
        inputDir_("D:/dev"),
        inputDirectorySelected_(false)
    {
#ifdef VISCOM_USE_SGCT
        sharedIndex_.setVal(-1);
#endif
    }

    void CoordinatorNode::InitOpenGL()
    {
        ApplicationNodeImplementation::InitOpenGL();
    }

    CoordinatorNode::~CoordinatorNode() = default;

    void CoordinatorNode::Draw2D(FrameBuffer& fbo)
    {
        std::vector<std::string> supportedDriveLetters = { "A:/", "B:/", "C:/", "D:/", "E:/", "F:/", "G:/", "H:/" };
        namespace fs = std::experimental::filesystem;
        fbo.DrawToFBO([&]() {
            ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiSetCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
            ImGui::StyleColorsClassic();
            if (!inputDirectorySelected_ && ImGui::Begin("Select input directory", nullptr, ImGuiWindowFlags_MenuBar))
            {
                ImGui::Text(inputDir_.data());
                for (const auto& dl : supportedDriveLetters) {
                    if (fs::is_directory(dl)) {
                        bool selected = false;
                        ImGui::Selectable(dl.c_str(), &selected);
                        if (selected) inputDir_ = dl;
                    }
                }
                for(auto &path : getDirectoryContent(inputDir_))
                {
                    bool selected = false;
                    ImGui::Selectable(path.data(), &selected);
                    if(selected) inputDir_ = fs::canonical(fs::path(inputDir_) / path).string();
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

    void CoordinatorNode::loadSlides() {
        std::vector<std::string> slides = getFiles(inputDir_);
        std::sort(slides.begin(), slides.end(), viscom::comparePaths);

        LoadTextures(slides);

        NextSlide();
        int tmp = 0;
        TransferData(&tmp, sizeof(int), static_cast<std::uint16_t>(SlideMessages::ResetPresentation));
    }

    bool CoordinatorNode::KeyboardCallback(int key, int scancode, int action, int mods)
    {
        if (ApplicationNodeBase::KeyboardCallback(key, scancode, action, mods)) return true;

        switch (key)
        {
        case GLFW_KEY_LEFT:
            if (action == GLFW_REPEAT || action == GLFW_PRESS) {
                PreviousSlide();
                return true;
            }
            break;
        case GLFW_KEY_RIGHT:
            if (action == GLFW_REPEAT || action == GLFW_PRESS) {
                NextSlide();
                return true;
            }
            break;
        case GLFW_KEY_L:
            if (action == GLFW_PRESS && mods == GLFW_MOD_CONTROL) {
                inputDirectorySelected_ = false;
                SetCurrentSlide(-1);
                return true;
            }
            break;
        default: return false;
        }

        return false;
    }

    std::vector<std::string> CoordinatorNode::getDirectoryContent(const std::string& dir, const bool returnFiles) const
    {
        namespace fs = std::experimental::filesystem;

        std::vector<std::string> content;
        if (!returnFiles) content.emplace_back("..");

        // apparently stb image seems to have problems loading jpg files...
        auto checkImageFile = [](const fs::path& p) { return (p.extension().string() == ".png" || p.extension().string() == ".PNG"); }; // || p.extension().string() == ".jpg" || p.extension().string() == ".jpeg"); };

        for (auto& p : fs::directory_iterator(dir)) {
            if (returnFiles && fs::is_regular_file(p) && checkImageFile(p)) content.push_back(fs::absolute(p, dir).string());
            else if (!returnFiles && fs::is_directory(p)) content.push_back(p.path().filename().string());
        }
        return content;
    }
#ifdef VISCOM_USE_SGCT
    bool CoordinatorNode::DataTransferCallback(void* receivedData, int receivedLength, std::uint16_t packageID, int clientID)
    {
        switch (static_cast<SlideMessages>(packageID))
        {
        case SlideMessages::RequestSlideNames:
        {
            auto slideNames = GetTextureSlideNameData();
            TransferDataToNode(slideNames.data(), slideNames.size(), static_cast<std::uint16_t>(SlideMessages::SlideNamesTransfer), clientID);
            break;
        }
        default : break;
        }

        return true;
    }

    void CoordinatorNode::EncodeData()
    {
        ApplicationNodeImplementation::EncodeData();
        sgct::SharedData::instance()->writeInt32(&sharedIndex_);
    }

    void CoordinatorNode::PreSync()
    {
        ApplicationNodeImplementation::PreSync();
        sharedIndex_.setVal(GetCurrentSlide());
    }
#endif

    void CoordinatorNode::UpdateFrame(double currentTime, double elapsedTime)
    {
        ApplicationNodeImplementation::UpdateFrame(currentTime, elapsedTime);
    }

}
