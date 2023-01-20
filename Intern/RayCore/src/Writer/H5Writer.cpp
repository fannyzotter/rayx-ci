#ifndef CI  // highfive doesn't work in CI

#include "H5Writer.h"

#include <highfive/H5DataSpace.hpp>
#include <highfive/H5File.hpp>
#include <string>

#include "Debug/Debug.h"

using uint = unsigned int;

/// writeH5 stores the rays as 18 doubles: ray_id, snapshot_id, [... contents of the RAYX::Ray struct]
struct SerializedRay {
    double ray_id;
    double snapshot_id;
    RAYX::Ray ray;
};

int count(const RAYX::Rays& rays) {
    int c = 0;
    for (auto& r : rays) {
        c += r.size();
    }
    return c;
}

std::vector<SerializedRay> serialize(const RAYX::Rays& rays) {
    std::vector<SerializedRay> serialized;
    serialized.reserve(count(rays));
    for (uint ray_id = 0; ray_id < rays.size(); ray_id++) {
        auto& snapshots = rays[ray_id];
        for (uint snapshot_id = 0; snapshot_id < snapshots.size(); snapshot_id++) {
            auto& ray = snapshots[snapshot_id];
            SerializedRay sray = {.ray_id = (double)ray_id, .snapshot_id = (double)snapshot_id, .ray = ray};
            serialized.push_back(sray);
        }
    }

    return serialized;
}

void writeH5(const RAYX::Rays& rays, std::string filename) {
    static_assert(sizeof(SerializedRay) == 18 * sizeof(double));

    HighFive::File file(filename, HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate);

    const uint32_t numRays = rays.size();
    auto srays = serialize(rays);

    try {
        auto dataspace = HighFive::DataSpace({numRays, 18});
        auto dataset = file.createDataSet<double>("0", dataspace);
        auto ptr = (double*)srays.data();
        dataset.write_raw(ptr);
    } catch (HighFive::Exception& err) {
        RAYX_ERR << err.what();
    }
}

#endif
