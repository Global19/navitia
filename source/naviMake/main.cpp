#include "config.h"
#include <iostream>
#include <boost/foreach.hpp>

#include "gtfs_parser.h"
#include "bdtopo_parser.h"
#include "osm2nav.h"
#include "utils/timer.h"

#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace po = boost::program_options;
namespace pt = boost::posix_time;

int main(int argc, char * argv[])
{
    std::string type, input, output, date, topo_path, osm_filename;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Affiche l'aide")
        ("date,d", po::value<std::string>(&date), "Date de début")
        ("input,i", po::value<std::string>(&input), "Repertoire d'entrée")
        ("topo", po::value<std::string>(&topo_path), "Repertoire contenant la bd topo")
        ("osm", po::value<std::string>(&osm_filename), "Fichier OpenStreetMap au format pbf")
        ("output,o", po::value<std::string>(&output)->default_value("data.nav"), "Fichier de sortie")
        ("version,v", "Affiche la version");


    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if(vm.count("version")){
        std::cout << argv[0] << " V" << NAVITIA_VERSION << " " << NAVITIA_BUILD_TYPE << std::endl;
        return 0;
    }

    if(vm.count("help") || !vm.count("input") || !vm.count("type")) {
        std::cout << desc <<  "\n";
        return 1;
    }
    if(!vm.count("topo") && !vm.count("osm")) {
        std::cout << "Pas de topologie chargee" << std::endl;
    }
    pt::ptime start, end;
    int read, complete, clean, sort, transform, save, autocomplete, sn;

    navimake::Data data; // Structure temporaire
    navitia::type::Data nav_data; // Structure définitive


    nav_data.meta.publication_date = pt::microsec_clock::local_time();
    
    // Est-ce que l'on charge la carto ?
    start = pt::microsec_clock::local_time();
    if(vm.count("topo")){
        nav_data.meta.data_sources.push_back(boost::filesystem::absolute(topo_path).native());

        navimake::connectors::BDTopoParser topo_parser(topo_path);
        //gtfs ne contient pas le référentiel des villes, on le charges depuis la BDTOPO
        topo_parser.load_city(data);
        topo_parser.load_georef(nav_data.geo_ref);
    } else if(vm.count("osm")){
        navitia::georef::fill_from_osm(nav_data.geo_ref, osm_filename);
    }

    sn = (pt::microsec_clock::local_time() - start).total_milliseconds();


    start = pt::microsec_clock::local_time();

    nav_data.meta.data_sources.push_back(boost::filesystem::absolute(input).native());


    navimake::connectors::GtfsParser connector(input);
    connector.fill(data, date);
    nav_data.meta.production_date = connector.production_date;

    read = (pt::microsec_clock::local_time() - start).total_milliseconds();


    std::cout << "line: " << data.lines.size() << std::endl;
    std::cout << "route: " << data.routes.size() << std::endl;
    std::cout << "stoparea: " << data.stop_areas.size() << std::endl;
    std::cout << "stoppoint: " << data.stop_points.size() << std::endl;
    std::cout << "vehiclejourney: " << data.vehicle_journeys.size() << std::endl;
    std::cout << "stop: " << data.stops.size() << std::endl;
    std::cout << "connection: " << data.connections.size() << std::endl;
    std::cout << "route points: " << data.route_points.size() << std::endl;
    std::cout << "city: " << data.cities.size() << std::endl;
    std::cout << "modes: " << data.modes.size() << std::endl;
    std::cout << "validity pattern : " << data.validity_patterns.size() << std::endl;
    std::cout << "route point connections : " << data.route_point_connections.size() << std::endl;
    std::cout << "voies (rues) : " << nav_data.geo_ref.ways.size() << std::endl;


    start = pt::microsec_clock::local_time();
    data.complete();
    complete = (pt::microsec_clock::local_time() - start).total_milliseconds();

    start = pt::microsec_clock::local_time();
    data.clean();
    clean = (pt::microsec_clock::local_time() - start).total_milliseconds();

    start = pt::microsec_clock::local_time();
    data.sort();
    sort = (pt::microsec_clock::local_time() - start).total_milliseconds();

    start = pt::microsec_clock::local_time();
    data.transform(nav_data.pt_data);

    transform = (pt::microsec_clock::local_time() - start).total_milliseconds();

    std::cout << "Construction des contours de la région" << std::endl;
    nav_data.meta.shape = data.find_shape(nav_data.pt_data);

    start = pt::microsec_clock::local_time();
    std::cout << "Construction de proximity list" << std::endl;
    nav_data.build_proximity_list();
    std::cout << "Construction de external code" << std::endl;
    nav_data.build_uri();
    std::cout << "Assigne les villes aux voiries du filaire" << std::endl;
    nav_data.set_cities(); // Assigne les villes aux voiries du filaire [depend des uri]
    std::cout << "Construction de first letter" << std::endl;
    nav_data.build_autocomplete();
    std::cout << "On va construire les correspondances" << std::endl;
    {Timer t("Construction des correspondances");  nav_data.pt_data.build_connections();}
    autocomplete = (pt::microsec_clock::local_time() - start).total_milliseconds();
    std::cout <<"Debut sauvegarde ..." << std::endl;
    start = pt::microsec_clock::local_time();

    nav_data.save(output);
    save = (pt::microsec_clock::local_time() - start).total_milliseconds();

    std::cout << "temps de traitement" << std::endl;
    std::cout << "\t lecture des fichiers " << read << "ms" << std::endl;
    std::cout << "\t completion des données " << complete << "ms" << std::endl;
    std::cout << "\t netoyage des données " << clean << "ms" << std::endl;
    std::cout << "\t trie des données " << sort << "ms" << std::endl;
    std::cout << "\t transformation " << transform << "ms" << std::endl;
    std::cout << "\t street network " << sn << "ms" << std::endl;
    std::cout << "\t construction de autocomplete " << autocomplete << "ms" << std::endl;
    std::cout << "\t serialization " << save << "ms" << std::endl;
    
    return 0;
}
