#pragma once

#include "graph_struct.hpp"
#include "just_gtfs.h"
#include <unordered_map>
#include <vector>
#include <string>

namespace raptor {

struct VectorHash {
    size_t operator()(const std::vector<StopId>& v) const {
        size_t seed = v.size();
        for(auto& i : v) {
            seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

class GraphBuilder {
public:
    GraphBuilder(double walk_radius_km = 1.5, double walk_speed_km_h = 4.0);

    // Wczytuje i dokleja kolejne dane GTFS
    void append_feed(const gtfs::Feed& feed);

    // Buduje i zwraca ostateczny graf
    RaptorGraph build();

private:
    double walk_radius_km_;
    double walk_speed_km_h_;

    std::unordered_map<std::string, StopId> stop_name_to_id_;
    std::vector<gtfs::Stop> all_gtfs_stops_;

    struct RouteInfo {
        std::vector<std::vector<StopTime>> trips_st;
        std::string name;
    };

    // Grupowanie kursów (do unikalnych)
    std::unordered_map<std::vector<StopId>, RouteInfo, VectorHash> grouped_routes_;

    // Funkcja pomocnicza liczenie czasu przejścia
    static double haversine(double lat1, double lon1, double lat2, double lon2);
};

} // namespace raptor
