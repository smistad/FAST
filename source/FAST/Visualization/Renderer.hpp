#pragma once

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/BoundingBox.hpp"
#include "FAST/Data/SpatialDataObject.hpp"
#include <mutex>
#include <QOpenGLFunctions_3_3_Core>

namespace fast {

class View;
class BoundingBox;

class FAST_EXPORT  Renderer : public ProcessObject, protected QOpenGLFunctions_3_3_Core {
    public:
        typedef SharedPointer<Renderer> pointer;
        virtual void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) = 0;
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
        virtual BoundingBox getBoundingBox(bool transform = true);
        virtual void stopPipeline();
        virtual void reset();
    protected:
        Renderer();
        void execute() override;

        /**
         * Creates an OpenGL shader program. Should be used in the renderer constructor.
         * @param shaderFilenames
         * @param programName
         */
        void createShaderProgram(std::vector<std::string> shaderFilenames, std::string programName = "default");
        void attachShader(std::string filename, std::string programName = "default");
        void activateShader(std::string programName = "default");
        void deactivateShader();
        uint getShaderProgram(std::string programName = "default");
        void setShaderUniform(std::string name, Matrix4f matrix, std::string shaderProgramName = "default");
        void setShaderUniform(std::string name, Affine3f matrix, std::string shaderProgramName = "default");
        void setShaderUniform(std::string name, Vector3f vector, std::string shaderProgramName = "default");
        void setShaderUniform(std::string name, float value, std::string shaderProgramName = "default");
        void setShaderUniform(std::string name, bool value, std::string shaderProgramName = "default");
        void setShaderUniform(std::string name, int value, std::string shaderProgramName = "default");
        int getShaderUniformLocation(std::string name, std::string shaderProgramName = "default");

        // Locking mechanisms to ensure thread safe synchronized rendering
        bool mHasRendered = true;
        bool mStop = false;
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
