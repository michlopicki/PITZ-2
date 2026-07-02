#include "builder.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace raptor {

GraphBuilder::GraphBuilder(double walk_radius_km, double walk_speed_km_h)
    : walk_radius_km_(walk_radius_km), walk_speed_km_h_(walk_speed_km_h) {}

double GraphBuilder::haversine(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371.0;
    const double to_rad = M_PI / 180.0;
    
    double dLat = (lat2 - lat1) * to_rad;
    double dLon = (lon2 - lon1) * to_rad;
    
    double a = std::sin(dLat / 2.0) * std::sin(dLat / 2.0) +
               std::cos(lat1 * to_rad) * std::cos(lat2 * to_rad) *
               std::sin(dLon / 2.0) * std::sin(dLon / 2.0);
               
    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    return R * c;
}

void GraphBuilder::append_feed(const gtfs::Feed& feed) {
    const auto& gtfs_stops = feed.get_stops();
    
    for (const auto& stop : gtfs_stops) {
        if (stop_name_to_id_.find(stop.stop_id) == stop_name_to_id_.end()) {
            StopId id = all_gtfs_stops_.size();
            stop_name_to_id_[stop.stop_id] = id;
            all_gtfs_stops_.push_back(stop);
        }
    }

    const auto& gtfs_trips = feed.get_trips();
    for (const auto& trip : gtfs_trips) {
        auto st_range = feed.get_stop_times_for_trip(trip.trip_id);
        std::vector<StopId> seq;
        std::vector<StopTime> st_vec;
        
        for (auto it = st_range.first; it != st_range.second; ++it) {
            const auto& st = *it;
            seq.push_back(stop_name_to_id_[st.stop_id]);
            st_vec.push_back({
                static_cast<Time>(st.arrival_time.get_total_seconds()),
                static_cast<Time>(st.departure_time.get_total_seconds())
            });
        }
        
        if (!seq.empty()) {
            if (grouped_routes_.find(seq) == grouped_routes_.end()) {
                const auto& route = feed.get_route(trip.route_id);
                std::string r_name = route.route_short_name.empty() ? route.route_id : route.route_short_name;
                grouped_routes_[seq].name = r_name;
            }
            grouped_routes_[seq].trips_st.push_back(std::move(st_vec));
        }
    }
}

RaptorGraph GraphBuilder::build() {
    std::cout << "Budowanie grafu...\n";
    RaptorGraph graph;
    
    std::vector<Route> out_routes;
    std::vector<StopId> out_route_stops;
    std::vector<StopTime> out_stop_times;
    std::vector<std::vector<RouteId>> tmp_stop_routes(all_gtfs_stops_.size());
    std::vector<std::string> out_route_names;

    RouteId route_idx = 0;
    for (auto& pair : grouped_routes_) {
        const auto& seq = pair.first;
        auto& trips_st = pair.second.trips_st;
        out_route_names.push_back(pair.second.name);

        // Sortujemy tripy po czasie odjazdu z pierwszego przystanku
        std::sort(trips_st.begin(), trips_st.end(), [](const auto& a, const auto& b) {
            if(a.empty() || b.empty()) return false;
            return a[0].departure < b[0].departure;
        });

        Route r;
        r.ptr_route_stops = out_route_stops.size();
        r.num_stops = seq.size();
        r.ptr_stop_times = out_stop_times.size();
        r.num_trips = trips_st.size();
        out_routes.push_back(r);

        for (StopId s : seq) {
            out_route_stops.push_back(s);
            if (tmp_stop_routes[s].empty() || tmp_stop_routes[s].back() != route_idx) {
                tmp_stop_routes[s].push_back(route_idx);
            }
        }

        for (const auto& st_vec : trips_st) {
            for (const auto& st : st_vec) {
                out_stop_times.push_back(st);
            }
        }
        route_idx++;
    }

    std::vector<RouteId> out_stop_routes;
    std::vector<Transfer> out_transfers;
    std::vector<Stop> out_stops;
    std::vector<std::string> out_stop_names;

    double walk_speed_m_s = (walk_speed_km_h_ * 1000.0) / 3600.0;

    for (StopId i = 0; i < all_gtfs_stops_.size(); ++i) {
        Stop s;
        const auto& stop1 = all_gtfs_stops_[i];
        out_stop_names.push_back(stop1.stop_name.empty() ? stop1.stop_id : stop1.stop_name);
        
        s.ptr_stop_routes = out_stop_routes.size();
        s.num_routes = tmp_stop_routes[i].size();
        for (RouteId rid : tmp_stop_routes[i]) {
            out_stop_routes.push_back(rid);
        }

        s.ptr_transfers = out_transfers.size();
        
        // Sam do siebie (pętla 0)
        out_transfers.push_back({i, 0});
        
        // Pieszo do sąsiednich przystanków
        if (stop1.coordinates_present) {
            for (StopId j = 0; j < all_gtfs_stops_.size(); ++j) {
                if (i == j) continue;
                const auto& stop2 = all_gtfs_stops_[j];
                if (stop2.coordinates_present) {
                    double dist = haversine(stop1.stop_lat, stop1.stop_lon, stop2.stop_lat, stop2.stop_lon);
                    if (dist <= walk_radius_km_) {
                        Time duration = static_cast<Time>((dist * 1000.0) / walk_speed_m_s);
                        out_transfers.push_back({j, duration});
                    }
                }
            }
        }
        
        s.num_transfers = out_transfers.size() - s.ptr_transfers;
        out_stops.push_back(s);
    }

    graph.set_stops(std::move(out_stops));
    graph.set_routes(std::move(out_routes));
    graph.set_stop_routes(std::move(out_stop_routes));
    graph.set_route_stops(std::move(out_route_stops));
    graph.set_stop_times(std::move(out_stop_times));
    graph.set_transfers(std::move(out_transfers));
    graph.set_stop_names(std::move(out_stop_names));
    graph.set_route_names(std::move(out_route_names));

    std::cout << "Koniec budowy grafu.\n";
    return graph;
}

} // namespace raptor
