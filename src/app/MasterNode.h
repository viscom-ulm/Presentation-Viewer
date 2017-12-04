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
    struct ClientState
    {
        ClientState() : clientId(-1), textureIndex(-1), synced(false){}
        int clientId;
        int textureIndex;
        bool synced;
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
        bool sync(int index);
        std::shared_ptr<Texture> getCurrentSlide() const { return texture_slides_[current_slide_]; }
		
#ifdef VISCOM_USE_SGCT
        virtual void EncodeData() override;
        virtual void PreSync() override;
        //virtual void DecodeData() override;
#endif
    private:
#ifdef VISCOM_USE_SGCT
        /** Holds the data the master shares. */
        sgct::SharedObject<TextureInfo> sharedData_;
        sgct::SharedVector<unsigned char> sharedVector_;
        sgct::SharedInt32 sharedIndex_;
        sgct::SharedInt32 sharedNumberOfSlides_;
        //sgct::SharedInt32 sharedClientID_;
        //sgct::SharedBool sharedInit_;
#endif
		/** Holds the index of the current displayed slide */
		int current_slide_;
		/** Holds the number of slides */
		int numberOfSlides_;
		/** The vector holds all available slide textures */
		std::vector<std::shared_ptr<viscom::Texture>> texture_slides_;
        /* Holds if there is on going initiation. */
        bool init_;
        /* Holds state of clients. */
        std::vector<std::vector<bool>> clientStates_;
        std::vector<std::pair<bool,bool>> acknowledged_;
    };
}
