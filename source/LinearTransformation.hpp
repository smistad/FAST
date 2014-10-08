#ifndef LINEARTRANSFORMATION_HPP_
#define LINEARTRANSFORMATION_HPP_

#include <boost/numeric/ublas/matrix.hpp>
#include "DataTypes.hpp"

namespace fast {

class LinearTransformation : public boost::numeric::ublas::matrix<float> {
    public:
        LinearTransformation();
        // Copy
        LinearTransformation(boost::numeric::ublas::matrix<float> m) : boost::numeric::ublas::matrix<float>(m) {};
        LinearTransformation getInverse();
        LinearTransformation operator*(const LinearTransformation &other);
        boost::numeric::ublas::matrix<float> getMatrix() const;
        Float3 operator*(Float3 vertex);
};

Float3 operator*(const Float3& vertex, const LinearTransformation& transform);

} // end namespace fast

#endif /* LINEARTRANSFORMATION_HPP_ */
