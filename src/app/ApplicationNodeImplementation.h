/**
* @file   ApplicationNodeImplementation.h
* @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
* @date   2016.11.30
*
* @brief  Declaration of the application node implementation common for master and slave nodes.
*/

#pragma once

#include "core/ApplicationNodeInternal.h"
#include "core/ApplicationNodeBase.h"

namespace viscom {

    class MeshRenderable;

    class ApplicationNodeImplementation : public ApplicationNodeBase
    {
    public:
        explicit ApplicationNodeImplementation(ApplicationNodeInternal* appNode);
        ApplicationNodeImplementation(const ApplicationNodeImplementation&) = delete;
        ApplicationNodeImplementation(ApplicationNodeImplementation&&) = delete;
        ApplicationNodeImplementation& operator=(const ApplicationNodeImplementation&) = delete;
        ApplicationNodeImplementation& operator=(ApplicationNodeImplementation&&) = delete;
        virtual ~ApplicationNodeImplementation() override;

        virtual void InitOpenGL() override;
        virtual void UpdateFrame(double currentTime, double elapsedTime) override;
        virtual void ClearBuffer(FrameBuffer& fbo) override;
        virtual void DrawFrame(FrameBuffer& fbo) override;
        virtual void CleanUp() override;

        void setCurrentTexture(std::shared_ptr<Texture> tex) { texture_ = tex; };

    private:
        /** The GPU program used for drawing. */
        const GPUProgram *slideProgram_;
        /** Texture location */
        GLint slideTextureLoc_;
        /** Holds the current texture to be rendered*/
        std::shared_ptr<Texture> texture_;
        /** Holds the geometry which gets textured with a slide*/
        std::shared_ptr<FullscreenQuad> quad_;
    };
}
