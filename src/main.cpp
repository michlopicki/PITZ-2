#include <iostream>
#include <string>
#include <cstdio>
#include <random>

#include "just_gtfs.h"
#include "builder.hpp"
#include "raptor_basic.hpp"

inline std::string format_time(raptor::Time t) {
    if (t == raptor::INVALID_TIME) return "--:--:--";
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02u:%02u:%02u", t / 3600, (t % 3600) / 60, t % 60);
    return std::string(buf);
}

#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;

inline raptor::Time parse_time(const std::string& time_str) {
    if (time_str.size() < 5) return raptor::INVALID_TIME;
    try {
        uint32_t h = std::stoi(time_str.substr(0, 2));
        uint32_t m = std::stoi(time_str.substr(3, 2));
        return h * 3600 + m * 60;
    } catch (...) {
        return raptor::INVALID_TIME;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Użycie: " << argv[0] << " <plik_gtfs_1> [<plik_gtfs_2> ...]\n";
        return 1;
    }

    raptor::GraphBuilder builder;

    for (int i = 1; i < argc; ++i) {
        std::string path = argv[i];
        std::cout << "Wczytywanie GTFS z: " << path << "...\n";
        
        gtfs::Feed feed(path);
        feed.read_feed();
        
        std::cout << "Wczytano: ";
        std::cout << "routes: " << feed.get_routes().size();
        std::cout << ", stops: " << feed.get_stops().size() << " ";
        std::cout << ", trips: " << feed.get_trips().size() << "\n\n";

        builder.append_feed(feed);
    }

    // Budowa struktur dla RAPTOR-a
    auto raptor_graph = builder.build();

    std::cout << "\nZbudowano RaptorGraph\n";
    std::cout << "Liczba RAPTOR routes: " << raptor_graph.route_count() << "\n";
    std::cout << "Liczba przystanków: " << raptor_graph.stop_count() << "\n\n";

    if (raptor_graph.stop_count() < 2) return 0;

    httplib::Server svr;

    svr.set_mount_point("/", "../web");

    // Zwraca listę wszystkich przystanków
    svr.Get("/api/stops", [&raptor_graph](const httplib::Request&, httplib::Response& res) {
        json stops = json::array();
        for (raptor::StopId i = 0; i < raptor_graph.stop_count(); ++i) {
            stops.push_back({
                {"id", i},
                {"name", raptor_graph.stop_name(i)}
            });
        }
        res.set_content(stops.dump(), "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
    });

    // Wyszukuje połączenie
    svr.Get("/api/route", [&raptor_graph](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_param("source") || !req.has_param("target") || !req.has_param("time")) {
            res.status = 400;
            res.set_content("Missing params", "text/plain");
            return;
        }
        
        raptor::StopId source = std::stoi(req.get_param_value("source"));
        raptor::StopId target = std::stoi(req.get_param_value("target"));
        std::string time_str = req.get_param_value("time");
        
        raptor::Time dep_time = parse_time(time_str);
        if (dep_time == raptor::INVALID_TIME) {
            res.status = 400;
            res.set_content("Invalid time format (use HH:MM)", "text/plain");
            return;
        }
        
        raptor::BasicRaptor raptor_algo(raptor_graph);
        auto pareto_journeys = raptor_algo.find_pareto_journeys(source, target, dep_time);
        
        if (pareto_journeys.empty()) {
            json error = {{"error", "Brak połączenia"}};
            res.set_content(error.dump(), "application/json");
            res.set_header("Access-Control-Allow-Origin", "*");
            return;
        }

        json j_result;
        json j_journeys = json::array();

        for (const auto& journey : pareto_journeys) {
            json j_journey;
            j_journey["arrival_time"] = format_time(journey.arrival_time);
            
            json j_legs = json::array();
            for (const auto& leg : journey.legs) {
                json j_leg;
                std::string from_name = raptor_graph.stop_name(leg.from_stop);
                std::string to_name = raptor_graph.stop_name(leg.to_stop);
                
                j_leg["start_time"] = format_time(leg.start_time);
                j_leg["end_time"] = format_time(leg.end_time);
                
                if (leg.route == raptor::INVALID_ROUTE) {
                    j_leg["type"] = "walk";
                    j_leg["route_name"] = "Pieszo";
                    j_leg["from_stop"] = from_name;
                    j_leg["to_stop"] = to_name;
                } else {
                    j_leg["type"] = "transit";
                    j_leg["route_name"] = raptor_graph.route_name(leg.route);
                    j_leg["from_stop"] = from_name;
                    j_leg["to_stop"] = to_name;
                }
                j_legs.push_back(j_leg);
            }
            j_journey["legs"] = j_legs;
            j_journeys.push_back(j_journey);
        }

        j_result["journeys"] = j_journeys;
        
        res.set_content(j_result.dump(), "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
    });

    std::cout << "Uruchamianie serwera na porcie 8080... (http://localhost:8080/)\n";
    svr.listen("0.0.0.0", 8080);

    return 0;
}
