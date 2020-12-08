/**
 * @file   ApplicationNodeImplementation.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.30
 *
 * @brief  Declaration of the application node implementation common for coordinator and worker nodes.
 */

#pragma once

#include "core/app/ApplicationNodeBase.h"

namespace viscom {

    class MeshRenderable;

    enum class SlideMessages : std::uint16_t {
        RequestSlideNames,
        SlideNamesTransfer,
        ResetPresentation
    };

    class ApplicationNodeImplementation : public ApplicationNodeBase
    {
    public:
        explicit ApplicationNodeImplementation(ApplicationNodeInternal* appNode);
        ApplicationNodeImplementation(const ApplicationNodeImplementation&) = delete;
        ApplicationNodeImplementation(ApplicationNodeImplementation&&) = delete;
        ApplicationNodeImplementation& operator=(const ApplicationNodeImplementation&) = delete;
        ApplicationNodeImplementation& operator=(ApplicationNodeImplementation&&) = delete;
        virtual ~ApplicationNodeImplementation() override;

        virtual void UpdateFrame(double currentTime, double elapsedTime) override;
        virtual void ClearBuffer(FrameBuffer& fbo) override;
        virtual void DrawFrame(FrameBuffer& fbo) override;

        void LoadTextures(const std::vector<std::string>& textureNames, bool resetSlides = true);
        void SetCurrentSlide(int slide) { current_slide_ = std::min(slide, static_cast<int>(texture_slides_.size()) - 1); };
        int GetCurrentSlide() const { return current_slide_; }
        void NextSlide();
        void PreviousSlide();

        std::vector<std::uint8_t> GetTextureSlideNameData() const;

    private:
        /** The GPU program used for drawing. */
        const GPUProgram *slideProgram_;
        /** Texture location */
        GLint slideTextureLoc_;
        /** Holds the index of the current displayed slide */
        int current_slide_ = -1;
        /** The vector holds all available slide textures */
        std::vector<std::shared_ptr<viscom::Texture>> texture_slides_;
        /** Holds the geometry which gets textured with a slide*/
        std::shared_ptr<FullscreenQuad> quad_;
    };
}
