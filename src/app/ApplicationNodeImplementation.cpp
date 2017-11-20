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
		ApplicationNodeBase{ appNode }, hasTexture_{false}
    {
    }

    ApplicationNodeImplementation::~ApplicationNodeImplementation() = default;

    void ApplicationNodeImplementation::InitOpenGL()
    {
        quad_ = std::make_shared<FullscreenQuad>("slide.frag", GetApplication());
        slideProgram_ = quad_->GetGPUProgram();
        slideTextureLoc_ = slideProgram_->getUniformLocation("slide"); 
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
		if (hasTexture_) {
			fbo.DrawToFBO([this]() {
				auto windowId = GetApplication()->GetEngine()->getCurrentWindowPtr()->getId();
				auto viewportPosition = -GetApplication()->GetViewportScreen(windowId).position_;
				auto viewportSize = GetApplication()->GetViewportScreen(windowId).size_;
				glViewport(viewportPosition.x, viewportPosition.y, viewportSize.x, viewportSize.y);
				glUseProgram(slideProgram_->getProgramId());


				glActiveTexture(GL_TEXTURE0 + 0);
				glBindTexture(GL_TEXTURE_2D, texture_->getTextureId());
				glUniform1i(slideTextureLoc_, 0);
				quad_->Draw();

				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
				glUseProgram(0);

			});
		}
    }

    void ApplicationNodeImplementation::CleanUp()
    {

    }
}
