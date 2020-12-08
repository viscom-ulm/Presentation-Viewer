/**
* @file   ApplicationNodeImplementation.cpp
* @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
* @date   2016.11.30
*
* @brief  Implementation of the application node class.
*/

#include "core/open_gl.h"
#include "ApplicationNodeImplementation.h"
#include "core/gfx/mesh/MeshRenderable.h"
#include <glm/gtc/matrix_inverse.hpp>
#include "core/gfx/mesh/AnimMeshRenderable.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <iostream>

namespace viscom {

    ApplicationNodeImplementation::ApplicationNodeImplementation(ApplicationNodeInternal* appNode) :
        ApplicationNodeBase{appNode}, slideProgram_(nullptr), slideTextureLoc_(0)
    {
        quad_ = std::make_shared<FullscreenQuad>("slide.frag", this);
        slideProgram_ = quad_->GetGPUProgram();
        slideTextureLoc_ = slideProgram_->getUniformLocation("slide");
    }

    ApplicationNodeImplementation::~ApplicationNodeImplementation() = default;

    void ApplicationNodeImplementation::UpdateFrame(double currentTime, double)
    {
    }

    void ApplicationNodeImplementation::ClearBuffer(FrameBuffer& fbo)
    {
        fbo.DrawToFBO([]() {
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        });
    }

    void ApplicationNodeImplementation::DrawFrame(FrameBuffer& fbo)
    {

        fbo.DrawToFBO([this]() {
#ifdef VISCOM_USE_SGCT
            auto windowId = GetApplication()->GetFramework().GetEngine()->getCurrentWindowPtr()->getId();
            auto viewportPosition = -GetApplication()->GetFramework().GetViewportScreen(windowId).position_;
            auto viewportSize = GetApplication()->GetFramework().GetViewportScreen(windowId).size_;
            glViewport(viewportPosition.x, viewportPosition.y, viewportSize.x, viewportSize.y);
#endif
            glUseProgram(slideProgram_->getProgramId());
            if (current_slide_ >= 0) {
                glActiveTexture(GL_TEXTURE0 + 0);
                glBindTexture(GL_TEXTURE_2D, texture_slides_[current_slide_]->getTextureId());
                glUniform1i(slideTextureLoc_, 0);
                quad_->Draw();
            }
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            glUseProgram(0);

        });

    }

    void ApplicationNodeImplementation::LoadTextures(const std::vector<std::string>& textureNames, bool resetSlides)
    {
        if (resetSlides) current_slide_ = -1;
        texture_slides_.clear();

        for (const auto& texName : textureNames) texture_slides_.emplace_back(GetTextureManager().GetSynchronizedResource(texName));
    }

    void ApplicationNodeImplementation::NextSlide()
    {
        if (current_slide_ + 1 < texture_slides_.size()) {
            current_slide_++;
        }
    }

    void ApplicationNodeImplementation::PreviousSlide()
    {
        if (current_slide_ - 1 >= 0) {
            current_slide_--;
        }
    }
    std::vector<std::uint8_t> ApplicationNodeImplementation::GetTextureSlideNameData() const
    {
        std::vector<std::uint8_t> slideNames;
        std::size_t slideNamesSize = sizeof(std::size_t);
        for (const auto& tex : texture_slides_) {
            slideNamesSize += sizeof(std::size_t);
            slideNamesSize += tex->GetId().size();
        }

        slideNames.resize(slideNamesSize);
        auto dataPtr = slideNames.data();
        reinterpret_cast<std::size_t*>(dataPtr)[0] = texture_slides_.size();
        dataPtr += sizeof(std::size_t);

        for (const auto& tex : texture_slides_) {
            reinterpret_cast<std::size_t*>(dataPtr)[0] = tex->GetId().size();
            dataPtr += sizeof(std::size_t);
            memcpy(dataPtr, tex->GetId().data(), tex->GetId().size());
            dataPtr += tex->GetId().size();
        }

        return slideNames;
    }
}
