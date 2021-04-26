#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @defgroup exporters Exporters
 * Process objects used to export FAST data to files on disk or other formats.
 */

/**
 * @brief Abstract base class for exporters
 *
 * @ingroup exporters
 */
class FAST_EXPORT Exporter : public ProcessObject {

};

}