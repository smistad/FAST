%include "FAST/eigen.i"

namespace fast {

class Plane {
    public:
        Plane(Eigen::Vector3f normal);
        void setPosition(Eigen::Vector3f position);
        void setNormal(Eigen::Vector3f normal);
        bool hasPosition() const;
        static Plane Sagittal();
        static Plane Coronal();
        static Plane Axial();

};

}
