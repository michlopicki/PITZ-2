#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <span>

namespace raptor {

using StopId = uint32_t;
using RouteId = uint32_t;
using TripId = uint32_t;
using Time = uint32_t; // sekundy od północy

constexpr Time INVALID_TIME = 0xFFFFFFFF;
constexpr StopId INVALID_STOP = 0xFFFFFFFF;
constexpr RouteId INVALID_ROUTE = 0xFFFFFFFF;

struct Stop {
    uint32_t ptr_stop_routes;
    uint32_t num_routes;
    uint32_t ptr_transfers;
    uint32_t num_transfers;
};

struct Route {
    uint32_t ptr_route_stops;
    uint32_t num_stops;
    uint32_t ptr_stop_times;
    uint32_t num_trips;
};

struct StopTime {
    Time arrival;
    Time departure;
};

struct Transfer {
    StopId target;
    Time duration;
};

class RaptorGraph {
    std::vector<Stop> stops_;
    std::vector<Route> routes_;
    
    std::vector<RouteId> stop_routes_;
    std::vector<StopId> route_stops_;
    std::vector<Transfer> transfers_;
    std::vector<StopTime> stop_times_;

    std::vector<std::string> stop_names_;
    std::vector<std::string> route_names_;

public:
    RaptorGraph() = default;

    void set_stops(std::vector<Stop> stops) { stops_ = std::move(stops); }
    void set_routes(std::vector<Route> routes) { routes_ = std::move(routes); }
    void set_stop_routes(std::vector<RouteId> stop_routes) { stop_routes_ = std::move(stop_routes); }
    void set_route_stops(std::vector<StopId> route_stops) { route_stops_ = std::move(route_stops); }
    void set_transfers(std::vector<Transfer> transfers) { transfers_ = std::move(transfers); }
    void set_stop_times(std::vector<StopTime> stop_times) { stop_times_ = std::move(stop_times); }
    void set_stop_names(std::vector<std::string> names) { stop_names_ = std::move(names); }
    void set_route_names(std::vector<std::string> names) { route_names_ = std::move(names); }


    size_t stop_count() const { return stops_.size(); }
    size_t route_count() const { return routes_.size(); }

    std::span<const RouteId> routes_for_stop(StopId stop) const {
        const auto& s = stops_[stop];
        return std::span<const RouteId>(stop_routes_.data() + s.ptr_stop_routes, s.num_routes);
    }

    std::span<const Transfer> transfers_for_stop(StopId stop) const {
        const auto& s = stops_[stop];
        return std::span<const Transfer>(transfers_.data() + s.ptr_transfers, s.num_transfers);
    }

    std::span<const StopId> stops_for_route(RouteId route) const {
        const auto& r = routes_[route];
        return std::span<const StopId>(route_stops_.data() + r.ptr_route_stops, r.num_stops);
    }

    std::span<const StopTime> stop_times_for_trip(RouteId route, uint32_t trip_idx) const {
        const auto& r = routes_[route];
        uint32_t start_idx = r.ptr_stop_times + trip_idx * r.num_stops;
        return std::span<const StopTime>(stop_times_.data() + start_idx, r.num_stops);
    }

    uint32_t trips_for_route(RouteId route) const {
        return routes_[route].num_trips;
    }

    const std::string& stop_name(StopId stop) const { return stop_names_[stop]; }
    const std::string& route_name(RouteId route) const { return route_names_[route]; }
};

} // namespace raptor
