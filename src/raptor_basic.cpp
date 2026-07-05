#include "raptor_basic.hpp"
#include <algorithm>
#include <iostream>

namespace raptor {

BasicRaptor::BasicRaptor(const RaptorGraph& graph) : graph_(graph) {}

std::vector<Journey> BasicRaptor::find_pareto_journeys(StopId source, StopId target, Time departure_time, uint32_t max_rounds, Time min_transfer_time) {
    uint32_t num_stops = graph_.stop_count();
    
    // Tablice 1D tau[k * num_stops + i]
    // k = 0 to stan startowy, k = 1..max_rounds to właściwe rundy
    std::vector<Time> tau((max_rounds + 1) * num_stops, INVALID_TIME);
    std::vector<BackPointer> bp((max_rounds + 1) * num_stops, {INVALID_STOP, INVALID_ROUTE, INVALID_TIME});
    std::vector<Time> best_tau(num_stops, INVALID_TIME);

    // Inicjalizacja
    tau[0 * num_stops + source] = departure_time;
    best_tau[source] = departure_time;
    
    std::vector<bool> marked_stops(num_stops, false);
    marked_stops[source] = true;

    // Footpaths ze źródła
    for (const auto& transfer : graph_.transfers_for_stop(source)) {
        Time arrival = departure_time + transfer.duration;
        if (arrival < tau[0 * num_stops + transfer.target]) {
            tau[0 * num_stops + transfer.target] = arrival;
            best_tau[transfer.target] = arrival;
            marked_stops[transfer.target] = true;
            bp[0 * num_stops + transfer.target] = {source, INVALID_ROUTE, departure_time};
        }
    }

    for (uint32_t k = 1; k <= max_rounds; ++k) {
        std::vector<bool> new_marked_stops(num_stops, false);

        std::vector<uint32_t> route_active_idx(graph_.route_count(), 0xFFFFFFFF);
        for (StopId p = 0; p < num_stops; ++p) {
            if (marked_stops[p]) {
                for (RouteId r : graph_.routes_for_stop(p)) {
                    auto stops = graph_.stops_for_route(r);
                    for (uint32_t idx = 0; idx < stops.size(); ++idx) {
                        if (stops[idx] == p) {
                            if (idx < route_active_idx[r]) {
                                route_active_idx[r] = idx;
                            }
                            break;
                        }
                    }
                }
            }
        }

        // czasy z poprzedniej rundy
        for (StopId p = 0; p < num_stops; ++p) {
            tau[k * num_stops + p] = tau[(k - 1) * num_stops + p];
            bp[k * num_stops + p] = bp[(k - 1) * num_stops + p];
        }

        // Przejście przez aktywne trasy
        for (RouteId r = 0; r < graph_.route_count(); ++r) {
            uint32_t start_idx = route_active_idx[r];
            if (start_idx == 0xFFFFFFFF) continue;

            auto stops = graph_.stops_for_route(r);
            uint32_t current_trip_idx = 0xFFFFFFFF;
            StopId board_stop = INVALID_STOP;
            Time board_time = INVALID_TIME;

            for (uint32_t idx = start_idx; idx < stops.size(); ++idx) {
                StopId p_i = stops[idx];

                // Czy możemy wysiąść
                if (current_trip_idx != 0xFFFFFFFF) {
                    Time arrival = graph_.stop_times_for_trip(r, current_trip_idx)[idx].arrival;
                    
                    if (arrival < std::min(tau[k * num_stops + p_i], best_tau[p_i])) {
                        tau[k * num_stops + p_i] = arrival;
                        best_tau[p_i] = arrival;
                        new_marked_stops[p_i] = true;
                        bp[k * num_stops + p_i] = {board_stop, r, board_time};
                    }
                }

                // Czy możemy wsiąść do wcześniejszego kursu albo pierwszy raz
                Time prev_arr = tau[(k - 1) * num_stops + p_i];
                if (prev_arr != INVALID_TIME) {
                    Time required_board_time = prev_arr;
                    if (k > 1) {
                        required_board_time += min_transfer_time;
                    }

                    uint32_t num_trips = graph_.trips_for_route(r);
                    for (uint32_t t = 0; t < num_trips; ++t) {
                        Time dep = graph_.stop_times_for_trip(r, t)[idx].departure;
                        if (dep >= required_board_time) {
                            if (current_trip_idx == 0xFFFFFFFF || t < current_trip_idx) {
                                current_trip_idx = t;
                                board_stop = p_i;
                                board_time = dep;
                            }
                            break; 
                        }
                    }
                }
            }
        }

        // Przejścia piesze
        std::vector<bool> current_new_marked = new_marked_stops;
        for (StopId p = 0; p < num_stops; ++p) {
            if (current_new_marked[p]) {
                Time arrival = tau[k * num_stops + p];
                for (const auto& transfer : graph_.transfers_for_stop(p)) {
                    if (transfer.target != p) {
                        Time walk_arr = arrival + transfer.duration;
                        if (walk_arr < std::min(tau[k * num_stops + transfer.target], best_tau[transfer.target])) {
                            tau[k * num_stops + transfer.target] = walk_arr;
                            best_tau[transfer.target] = walk_arr;
                            new_marked_stops[transfer.target] = true;
                            bp[k * num_stops + transfer.target] = {p, INVALID_ROUTE, arrival};
                        }
                    }
                }
            }
        }

        marked_stops = new_marked_stops;
        
        bool any_marked = false;
        for (bool m : marked_stops) if (m) { any_marked = true; break; }
        if (!any_marked) break;
    }

    std::vector<Journey> pareto_journeys;
    Time best_time_so_far = INVALID_TIME;

    // Przechodzimy po wszystkich rundach k. Im większe k,
    // tym mniejszy musi być czas przyjazdu
    for (uint32_t k = 0; k <= max_rounds; ++k) {
        Time current_arr = tau[k * num_stops + target];
        if (current_arr != INVALID_TIME) {
            if (best_time_so_far == INVALID_TIME || current_arr < best_time_so_far) {
                best_time_so_far = current_arr;
                pareto_journeys.push_back(reconstruct_journey(target, k, tau, bp));
            }
        }
    }

    return pareto_journeys;
}

Journey BasicRaptor::reconstruct_journey(StopId target, uint32_t k, const std::vector<Time>& tau, const std::vector<BackPointer>& bp) {
    uint32_t num_stops = graph_.stop_count();
    
    Journey journey;
    journey.arrival_time = tau[k * num_stops + target];

    StopId current_stop = target;
    uint32_t current_k = k;

    while (current_k > 0) {
        const auto& ptr = bp[current_k * num_stops + current_stop];
        if (ptr.prev_stop == INVALID_STOP) {
            current_k--;
            continue;
        }

        JourneyLeg leg;
        leg.from_stop = ptr.prev_stop;
        leg.to_stop = current_stop;
        leg.route = ptr.route;
        leg.start_time = ptr.board_time;
        leg.end_time = tau[current_k * num_stops + current_stop];

        journey.legs.push_back(leg);
        
        current_stop = ptr.prev_stop;
        if (ptr.route != INVALID_ROUTE) {
            current_k--;
        }
    }

    // footpaths
    if (current_stop != INVALID_STOP && bp[0 * num_stops + current_stop].prev_stop != INVALID_STOP) {
        const auto& ptr = bp[0 * num_stops + current_stop];
        JourneyLeg leg;
        leg.from_stop = ptr.prev_stop;
        leg.to_stop = current_stop;
        leg.route = ptr.route;
        leg.start_time = ptr.board_time;
        leg.end_time = tau[0 * num_stops + current_stop];
        journey.legs.push_back(leg);
    }

    std::reverse(journey.legs.begin(), journey.legs.end());

    // Przesunięcie czasu wyjścia
    if (!journey.legs.empty() && journey.legs[0].route == INVALID_ROUTE && journey.legs.size() > 1) {
        Time walk_duration = journey.legs[0].end_time - journey.legs[0].start_time;
        Time next_departure = journey.legs[1].start_time;
        
        journey.legs[0].end_time = next_departure;
        journey.legs[0].start_time = next_departure - walk_duration;
    }

    return journey;
}

} // namespace raptor
