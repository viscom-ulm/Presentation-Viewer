/**
* @file   MasterNode.h
* @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
* @date   2016.11.25
*
* @brief  Declaration of the ApplicationNodeImplementation for the master node.
*/

#pragma once

#include "../app/ApplicationNodeImplementation.h"

#include "core/open_gl.h"

namespace viscom {

    enum PackageID : int {
        PresentationData = 0,
        TextureData = 1
    };
    struct ClientState
    {
        ClientState() : clientId(-1), textureIndex(-1) {}
        ClientState(int cId) : clientId(cId), textureIndex(-1), numberOfSlides(-1) {}
        int clientId;
        int textureIndex;
        int numberOfSlides;
    };

    struct SlideTexDescriptor
    {
        SlideTexDescriptor() noexcept : desc_{ 0, 0, 0, 0 } {}
        SlideTexDescriptor(const TextureDescriptor& desc) noexcept : desc_{ desc } {}

        TextureDescriptor desc_;
        std::size_t width_ = 0;
        std::size_t height_ = 0;
    };

    struct TextureHeaderMessage
    {
        TextureHeaderMessage() : index(-1), descriptor(TextureDescriptor{ 0, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE }) {}
        TextureHeaderMessage(std::size_t nos, std::size_t i, SlideTexDescriptor des) : index(i), descriptor(des) { }
        std::size_t index;
        SlideTexDescriptor descriptor;
    };

    class MasterNode final : public ApplicationNodeImplementation
    {
    public:
        explicit MasterNode(ApplicationNodeInternal* appNode);
        virtual ~MasterNode() override;
        virtual void InitOpenGL() override;
        virtual void UpdateFrame(double currentTime, double elapsedTime) override;
        virtual void Draw2D(FrameBuffer& fbo) override;
        virtual bool KeyboardCallback(int key, int scancode, int action, int mods) override;
        std::vector<std::string> getDirectoryContent(const std::string &dir, bool returnFiles = false) const;
        std::vector<std::string> getFiles(const std::string &dir) { return getDirectoryContent(dir, true); }
        

        /** iterates over resource/slides folder and loads textures */
        void loadSlides();
        std::shared_ptr<Texture> getCurrentSlide() const { return texture_slides_[current_slide_]; }

#ifdef VISCOM_USE_SGCT
        virtual void EncodeData() override;
        virtual void PreSync() override;
        virtual bool DataTransferCallback(void* receivedData, int receivedLength, int packageID, int clientID) override;
#endif
    private:
#ifdef VISCOM_USE_SGCT
        sgct::SharedInt32 sharedIndex_;
#endif

        std::vector<std::vector<bool>> clientReceivedTexture_;
        std::vector<bool> presentationInitialized_;
        bool allTexturesInitialized_;
        void TransferSlide(std::size_t slideID, std::size_t clientId) const;
        void GetTextureData(std::vector<uint8_t>& data, const SlideTexDescriptor& desc, GLuint textureId) const;

        /** Holds the index of the current displayed slide */
        int current_slide_;
        /** Holds the number of slides */
        size_t numberOfSlides_;
        /** The vector holds all available slide textures */
        std::vector<std::shared_ptr<viscom::Texture>> texture_slides_;
        /* Holds the input directory containing slides*/
        std::string inputDir_;
        /**/
        bool inputDirectorySelected_;
        bool animationChanged_ = false;
    };
}
