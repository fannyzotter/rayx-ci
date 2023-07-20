#include "PlaneMirror.h"

#include "Data/xml.h"
#include "Shared/Constants.h"

namespace RAYX {

Element makePlaneMirror(DesignObject& dobj) {
    auto behaviour = serializeMirror();
    auto surface = makePlane();
    return makeElement(dobj, behaviour, surface);
}

}  // namespace RAYX
