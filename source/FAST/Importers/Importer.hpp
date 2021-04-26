#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

/**
 * @defgroup importers Importers
 * Process objects used to load and output data such as images, tensors and geometry.
 */

/**
 * @brief Abstract base class for @ref importers
 *
 * @ingroup importers
 */
class FAST_EXPORT  Importer : public ProcessObject {

};

} // end namespace fast

