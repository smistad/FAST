#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include "FAST/SmartPointers.hpp"
#include "FAST/ProcessObject.hpp"
#include "FAST/Data/BoundingBox.hpp"
#include "FAST/Data/SpatialDataObject.hpp"
#include <mutex>
#include <QOpenGLFunctions>


namespace fast {

class View;
class BoundingBox;

class FAST_EXPORT  Renderer : public ProcessObject, protected QOpenGLFunctions {
    public:
        typedef SharedPointer<Renderer> pointer;
        virtual void draw() = 0;
        virtual void postDraw();
        /**
         * Adds a new input connection
         * @param port
         * @return the input nr of the new connection
         */
        virtual uint addInputConnection(DataPort::pointer port);
        /**
         * Adds a new input connection to a specific data object
         * @param data
         * @return the input nr of the new connection
         */
        virtual uint addInputData(DataObject::pointer data);
        virtual BoundingBox getBoundingBox();
        virtual void draw2D(
                cl::Buffer PBO,
                uint width,
                uint height,
                Affine3f pixelToViewportTransform,
                float PBOspacing,
                Vector2f translation
        ) {};
        virtual void stopPipeline();
    protected:
        Renderer();
        void execute() override;

        void createShaderProgram(std::vector<std::string> shaderFilenames, std::string programName = "default");
        void attachShader(std::string filename, std::string programName = "default");
        void activateShader(std::string programName = "default");
        void deactivateShader();
        uint getShaderProgram(std::string programName = "default");

        // Locking mechanisms to ensure thread safe synchronized rendering
        bool mHasRendered = true;
        std::condition_variable_any mRenderedCV;
        std::mutex mMutex;

        /**
         * This holds the current data to render for each input connection
         */
        std::unordered_map<uint, SpatialDataObject::pointer> mDataToRender;

        /**
         * This will lock the renderer mutex. Used by the compute thread.
         */
        void lock();
        /**
         * This will unlock the renderer mutex. Used by the compute thread.
         */
        void unlock();
        friend class View;
    private:

        /**
         * OpenGL shader IDs. Program name -> OpenGL ID
         */
        std::unordered_map<std::string, uint> mShaderProgramIDs;
};

}



#endif /* RENDERER_HPP_ */
