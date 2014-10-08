#include "LinearTransformation.hpp"

#include <boost/numeric/ublas/operation.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>

using namespace boost::numeric::ublas;
namespace fast {

/**
 * Initializes linear transformation object to identity matrix
 */
LinearTransformation::LinearTransformation() : boost::numeric::ublas::matrix<float>(4,4) {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            this->operator()(i,j) = i == j ? 1 : 0;
        }
    }
}

LinearTransformation LinearTransformation::getInverse() {
    typedef permutation_matrix<std::size_t> pmatrix;

    // create a working copy
    matrix<float> A = getMatrix();

    // create a permutation matrix for the LU-factorization
    pmatrix pm(A.size1());

    // perform LU-factorization
    int res = lu_factorize(A, pm);
    if (res != 0)
        throw Exception("Unable to invert matrix");

    // create identity matrix of "inverse"
    LinearTransformation inverse;
    inverse.assign(identity_matrix<float> (A.size1()));

    // backsubstitute to get the inverse
    lu_substitute(A, pm, inverse);

    return inverse;
}

LinearTransformation LinearTransformation::operator *(
        const LinearTransformation& other) {
    LinearTransformation T(boost::numeric::ublas::prod((boost::numeric::ublas::matrix<float>)*this, (boost::numeric::ublas::matrix<float>)other));
    return T;
}

boost::numeric::ublas::matrix<float> LinearTransformation::getMatrix() const {
    return matrix<float>(*this);
}


Float3 operator*(const Float3& vertex, const LinearTransformation& transform) {
    vector<float> boostVertex(4);
    boostVertex(0) = vertex[0];
    boostVertex(1) = vertex[1];
    boostVertex(2) = vertex[2];
    boostVertex(3) = 1;
    boostVertex = prod((matrix<float>)transform, boostVertex);
    Float3 result;
    result[0] = boostVertex(0);
    result[1] = boostVertex(1);
    result[2] = boostVertex(2);
    return result;
}
Float3 LinearTransformation::operator*(Float3 vertex) {
    vector<float> boostVertex(4);
    boostVertex(0) = vertex[0];
    boostVertex(1) = vertex[1];
    boostVertex(2) = vertex[2];
    boostVertex(3) = 1;
    boostVertex = prod((matrix<float>)*this, boostVertex);
    Float3 result;
    result[0] = boostVertex(0);
    result[1] = boostVertex(1);
    result[2] = boostVertex(2);
    return result;
}

}
