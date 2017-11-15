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
        void setCurrentSlide(int slide) { current_slide_ = slide; };
        int getCurrentSlide() const { return current_slide_; }
        virtual bool KeyboardCallback(int key, int scancode, int action, int mods) override;


        // ###################### https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c ##########
        inline bool exists_test3(const std::string& name) {
            struct stat buffer;
            return (stat(name.c_str(), &buffer) == 0);
        }
        // ##########################################################################################################################################
    private:
        /** Holds the index of the current displayed slide */
        int current_slide_;
        /** Holds the number of slides */
        int numberOfSlides_;
        /** The vector holds all available slide textures */
        std::vector<std::shared_ptr<viscom::Texture>> texture_slides_;
        /** The GPU program used for drawing. */
        const GPUProgram *slideProgram_;
        /** Texture location */
        GLint slideTextureLoc_;
        /** iterates over resource/slides folder and loads textures */
        void loadSlides();
        /** Holds the geometry which gets textured with a slide*/
        std::shared_ptr<FullscreenQuad> quad_;
    };
}
