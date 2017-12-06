/**
* @file   MasterNode.h
* @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
* @date   2016.11.25
*
* @brief  Declaration of the ApplicationNodeImplementation for the master node.
*/

#pragma once

#include "../app/ApplicationNodeImplementation.h"
#ifdef WITH_TUIO
#include "core/TuioInputWrapper.h"
#endif

namespace viscom {
    enum PackageID : int {
        Descriptor = 0,
        Data = 1
    };
    struct ClientState
    {
        ClientState() : clientId(-1), textureIndex(-1) {}
        ClientState(int cId, int tIdx) : clientId(cId), textureIndex(tIdx) {}
        int clientId;
        int textureIndex;
    };

    struct MasterMessage
    {
        MasterMessage() : hasData(false), numberOfSlide(0), index(-1), descriptor(0, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, 0, 0, 0) {}
        MasterMessage(bool hd, int nos, int i, TextureDescriptor des) : hasData(hd), numberOfSlide(nos), index(i), descriptor(des)
        {        }
        bool hasData;
        int numberOfSlide;
        int index;
        TextureDescriptor descriptor;
    };

    class MasterNode final : public ApplicationNodeImplementation
    {
    public:
        explicit MasterNode(ApplicationNodeInternal* appNode);
        virtual ~MasterNode() override;
        virtual void InitOpenGL() override;
        virtual void Draw2D(FrameBuffer& fbo) override;
        virtual bool KeyboardCallback(int key, int scancode, int action, int mods) override;
        virtual bool DataTransferCallback(void* receivedData, int receivedLength, int packageID, int clientID) override;
        // ###################### https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c ##########
        inline bool exists_test3(const std::string& name) {
            struct stat buffer;
            return (stat(name.c_str(), &buffer) == 0);
        }
        // ##########################################################################################################################################

        /** iterates over resource/slides folder and loads textures */
        void loadSlides();
        std::shared_ptr<Texture> getCurrentSlide() const { return texture_slides_[current_slide_]; }

#ifdef VISCOM_USE_SGCT
        virtual void EncodeData() override;
        virtual void PreSync() override;
#endif
    private:
#ifdef VISCOM_USE_SGCT
        sgct::SharedInt32 sharedIndex_;
#endif
        /** Holds the index of the current displayed slide */
        int current_slide_;
        /** Holds the number of slides */
        int numberOfSlides_;
        /** The vector holds all available slide textures */
        std::vector<std::shared_ptr<viscom::Texture>> texture_slides_;
        std::vector<std::vector<bool>> clientReceivedTexture_;
        bool initialized_;

    };
}
