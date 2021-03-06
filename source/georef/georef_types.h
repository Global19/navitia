
#pragma once

#include "georef/edge.h"
#include <boost/graph/adjacency_list.hpp>

namespace navitia {
namespace georef {

class Vertex;

// Plein de typedefs pour nous simpfilier un peu la vie

/** Définit le type de graph que l'on va utiliser
 *
 * Les arcs sortants et la liste des nœuds sont représentés par des vecteurs
 * les arcs sont orientés
 * les propriétés des nœuds et arcs sont les classes définies précédemment
 */
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, Vertex, Edge> Graph;

/// Représentation d'un nœud dans le g,raphe
typedef boost::graph_traits<Graph>::vertex_descriptor vertex_t;

/// Représentation d'un arc dans le graphe
typedef boost::graph_traits<Graph>::edge_descriptor edge_t;

/// Pour parcourir les segements du graphe
typedef boost::graph_traits<Graph>::edge_iterator edge_iterator;

}  // namespace georef
}  // namespace navitia
