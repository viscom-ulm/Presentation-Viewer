/**
 * @file   ApplicationNodeImplementation.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.30
 *
 * @brief  Implementation of the application node class.
 */

#include "ApplicationNodeImplementation.h"
#include "Vertices.h"
#include <imgui.h>
#include "core/gfx/mesh/MeshRenderable.h"
#include "core/imgui/imgui_impl_glfw_gl3.h"
#include <iostream>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace viscom {

    ApplicationNodeImplementation::ApplicationNodeImplementation(ApplicationNodeInternal* appNode) :
        ApplicationNodeBase{ appNode }
    {
    }

    ApplicationNodeImplementation::~ApplicationNodeImplementation() = default;

    void ApplicationNodeImplementation::InitOpenGL()
    {
        quad_ = std::make_shared<FullscreenQuad>("slide.frag", GetApplication());
        slideProgram_ = quad_->GetGPUProgram();
        slideTextureLoc_ = slideProgram_->getUniformLocation("slide");
        
        loadSlides();
    }

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
            auto windowId = GetApplication()->GetEngine()->getCurrentWindowPtr()->getId();
            auto viewportPosition = -GetApplication()->GetViewportScreen(windowId).position_;
            auto viewportSize = GetApplication()->GetViewportScreen(windowId).size_;
            glViewport(viewportPosition.x, viewportPosition.y, viewportSize.x, viewportSize.y);
            glUseProgram(slideProgram_->getProgramId());

          
            glActiveTexture(GL_TEXTURE0 + 0);
            glBindTexture(GL_TEXTURE_2D, texture_slides_[current_slide_]->getTextureId());
            glUniform1i(slideTextureLoc_, 0);
            quad_->Draw();
            
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            glUseProgram(0);
            
        });
    }

    void ApplicationNodeImplementation::CleanUp()
    {

    }

    bool ApplicationNodeImplementation::KeyboardCallback(int key, int scancode, int action, int mods)
    {
        if (ApplicationNodeBase::KeyboardCallback(key, scancode, action, mods)) return true;

        switch (key)
        {
        case GLFW_KEY_LEFT:
            if (action == GLFW_REPEAT || action == GLFW_PRESS) {
                if (current_slide_ - 1 >= 0) {
                    current_slide_--;
                }
                return true;
            }

        case GLFW_KEY_RIGHT:
            if (action == GLFW_REPEAT || action == GLFW_PRESS) {
                if (current_slide_ + 1 < numberOfSlides_) {
                    current_slide_++;
                }
                return true;
            } 
        }
        return false;
    }

    void ApplicationNodeImplementation::loadSlides()
    {
        int slideNumber = 1;
        while (exists_test3("resources/slides/Folie" + std::to_string(slideNumber) + ".PNG"))
        {
            auto texture = GetTextureManager().GetResource("/slides/Folie" + std::to_string(slideNumber) + ".PNG");
            if (!texture) break;
            texture_slides_.push_back(texture);
            slideNumber++;
        }
        numberOfSlides_ = texture_slides_.size();
        current_slide_ = 0;
    }
}
