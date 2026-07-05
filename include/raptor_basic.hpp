#pragma once
#include "graph_struct.hpp"
#include <vector>
#include <optional>

namespace raptor {

struct JourneyLeg {
    StopId from_stop;
    StopId to_stop;
    RouteId route; // INVALID_ROUTE oznacza przejście piesze
    Time start_time;
    Time end_time;
};

struct Journey {
    Time arrival_time;
    std::vector<JourneyLeg> legs;
};

struct BackPointer {
    StopId prev_stop;
    RouteId route;
    Time board_time;
};

class BasicRaptor {
public:
    BasicRaptor(const RaptorGraph& graph);

    // Zwraca zbiór Pareto
    std::vector<Journey> find_pareto_journeys(StopId source, StopId target, Time departure_time, uint32_t max_rounds = 4, Time min_transfer_time = 120);

private:
    const RaptorGraph& graph_;

    // Funkcja do odtwarzania trasy
    Journey reconstruct_journey(StopId target, uint32_t k, const std::vector<Time>& tau, const std::vector<BackPointer>& bp);
};

} // namespace raptor
