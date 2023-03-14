#pragma once

#include <array>
#include <glm.hpp>
#include <vector>

#include "Core.h"

namespace RAYX {
/** An abstract class used as a baseline for all surfaces.
 *
 *  # This is a headline
 *  TODO(Jannis): Here will be a more detailed description,
 *  which will continue here.
 *
 */
class RAYX_API Surface {
  public:
    virtual std::array<double, 16> getParams() const = 0;
    virtual int getSurfaceType() const = 0;

    Surface();
    virtual ~Surface();

  private:
};
}  // namespace RAYX
