#pragma once

#include <FAST/Data/SpatialDataObject.hpp>
#include <FAST/Data/SimpleDataObject.hpp>
#include <FAST/Data/Access/BoundingBoxSetAccess.hpp>
#include <thread>
#include <FAST/ProcessObject.hpp>

namespace fast {

class FAST_EXPORT BoundingBox : public SpatialDataObject {
    FAST_OBJECT(BoundingBox)
    public:
        /**
         * Create bounding box object with position and size set in millimeters
         */
        void create(Vector2f position, Vector2f size, uchar label = 1, float score = 0.0f);
        void setLabel(uchar label);
        uchar getLabel();
        /**
         * Set position in millimeters
         */
        void setPosition(Vector2f position);
        Vector2f getPosition();
        /**
         * Set size in millimeters
         */
        void setSize(Vector2f size);
        Vector2f getSize();

        void setScore(float score);
        float getScore();

        void free(ExecutionDevice::pointer device) override {};
        void freeAll() override {};
        float intersectionOverUnion(BoundingBox::pointer bbox2) const;
    protected:
        BoundingBox();

        bool m_initialized = false;
        uchar m_label = 1;
        Vector2f m_position;
        Vector2f m_size;
        float m_score;

        std::mutex m_mutex;
};

class FAST_EXPORT BoundingBoxSet : public SpatialDataObject {
    FAST_OBJECT(BoundingBoxSet)
    public:
        void create();
        int getNrOfLines();
        int getNrOfVertices();
        BoundingBoxSetAccess::pointer getAccess(accessType type);
        BoundingBoxSetOpenGLAccess::pointer getOpenGLAccess(accessType type);
        void freeAll() override;
        void free(ExecutionDevice::pointer device) override;
        ~BoundingBoxSet();
        virtual DataBoundingBox getBoundingBox() const override;
    protected:
        BoundingBoxSet();
        void setAllDataToOutOfDate();

		// OpenGL data
        bool mVBOHasData;
        bool mVBODataIsUpToDate;
        GLuint mCoordinateVBO = 0;
        GLuint mLineEBO = 0;
        GLuint m_labelVBO = 0;

        // Host data
        bool mHostHasData;
        bool mHostDataIsUpToDate;
        std::vector<float> mCoordinates;
        std::vector<uint> mLines;
        std::vector<uchar> m_labels;
        std::vector<float> m_scores;

        uint mNrOfVertices;
        uint mNrOfLines;

        bool mIsInitialized;
};

/**
 * Process object which accumulates incoming bounding box sets to a single bounding box set.
 */
class FAST_EXPORT BoundingBoxSetAccumulator : public ProcessObject {
    FAST_OBJECT(BoundingBoxSetAccumulator)
	public:
	protected:
        BoundingBoxSetAccumulator();
        void execute() override;

        BoundingBoxSet::pointer m_accumulatedBBset;
};

}
