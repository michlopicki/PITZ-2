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

    raptor::BasicRaptor raptor_algo(raptor_graph);
    std::mt19937 gen(321745922);
    raptor::StopId source = gen() % raptor_graph.stop_count();
    raptor::StopId target = gen() % raptor_graph.stop_count();
    raptor::Time dep_time = 6 * 3600 + gen() % (10*3600); // 06:00:00 - 16:00:00

    std::cout << "--- Basic RAPTOR ---\n";
    std::cout << "Od: " << raptor_graph.stop_name(source) 
              << "\nDo: " << raptor_graph.stop_name(target) << "\nCzas Wyjazdu: " << format_time(dep_time) << "\n";
    
    auto result = raptor_algo.find_earliest_arrival(source, target, dep_time);
    
    if (result) {
        std::cout << "Znaleziono połączenie: Czas przyjazdu: " << format_time(result->arrival_time) << "\n";
        std::cout << "Trasa:\n";
        for (const auto& leg : result->legs) {
            std::cout << "    * " << raptor_graph.stop_name(leg.from_stop)
                      << " ---> " << raptor_graph.stop_name(leg.to_stop);
            if (leg.route == raptor::INVALID_ROUTE) {
                std::cout << " (Pieszo)";
            } else {
                std::cout << " (Linia: " << raptor_graph.route_name(leg.route) << ")";
            }
            std::cout << " [" << format_time(leg.start_time) << " -> " << format_time(leg.end_time) << "]\n";
        }
    } else {
        std::cout << "Brak połączenia.\n";
    }

    return 0;
}
