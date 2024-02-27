#include "Helper.h"

void init(Inv& inv) {
    inv.finalized = false;

    // sets all output rays controlled by this shader call to ETYPE_UNINIT.
    for (uint i = uint(inv.pushConstants.startEventID); i < inv.pushConstants.maxEvents; i++) {
        inv.outputData[output_index(i, inv)].m_eventType = ETYPE_UNINIT;
    }
    inv.nextEventIndex = 0;

    // ray specific "seed" for random numbers -> every ray has a different starting value for the counter that creates the random number
    // TODO Random seeds should probably not be doubles! Casting MAX_UINT64 to double loses precision.
    const uint64_t MAX_UINT64 = ~(uint64_t(0));
    const double MAX_UINT64_DOUBLE = 18446744073709551616.0;
    uint64_t workerCounterNum = MAX_UINT64 / uint64_t(inv.pushConstants.numRays);
    inv.ctr = rayId(inv) * workerCounterNum + uint64_t(inv.pushConstants.randomSeed * MAX_UINT64_DOUBLE);
}

uint64_t rayId(Inv& inv) { return uint64_t(inv.pushConstants.rayIdStart) + uint64_t(gl_GlobalInvocationID); }

// `i in [0, maxEvents-1]`.
// Will return the index in outputData to access the `i'th` output ray belonging to this shader call.
// Typically used as `outputData[output_index(i)]`.
uint output_index(uint i, Inv& inv) {
    return uint(gl_GlobalInvocationID) * uint(inv.pushConstants.maxEvents - inv.pushConstants.startEventID) + i -
           uint(inv.pushConstants.startEventID);
}

// record an event and store it in the next free spot in outputData.
// `r` will typically be _ray, or some related ray.
void recordEvent(Ray r, double w, Inv& inv) {
    if (inv.nextEventIndex < inv.pushConstants.startEventID) {
        inv.nextEventIndex += 1;
        return;
    }
    if (inv.finalized) {
        return;
    }

    // recording of event type ETYPE_UINIT is forbidden.
    if (w == ETYPE_UNINIT) {
        RAYX_ERR << "recordEvent failed: weight UNINIT is invalid in recordEvent";

        return;
    }

    // the outputData array might be full!
    if (inv.nextEventIndex >= inv.pushConstants.maxEvents) {
        inv.finalized = true;

        // change the last event to "ETYPE_TOO_MANY_EVENTS".
        uint idx = output_index(uint(inv.pushConstants.maxEvents - 1), inv);
        inv.outputData[idx].m_eventType = ETYPE_TOO_MANY_EVENTS;

        RAYX_ERR << "recordEvent failed: too many events!";

        return;
    }

    r.m_eventType = w;

    uint idx = output_index(uint(inv.nextEventIndex), inv);
    inv.outputData[idx] = r;

    inv.nextEventIndex += 1;
}

// Like `recordEvent` above, but it will prevent recording more events after this.
// Is used for events terminating the path of the ray.
void recordFinalEvent(Ray r, double w, Inv& inv) {
    recordEvent(r, w, inv);
    inv.finalized = true;
}
