/**
 * \brief Test kernels.
 *
 * \date Feb 4, 2014
 * \author Janne Beate Bakeng, SINTEF
 */

__kernel void test_one(void){
    size_t id = get_global_id(0);
    id++;
}
