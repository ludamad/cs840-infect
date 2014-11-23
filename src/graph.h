#ifndef GENERATE_GRAPH_H_
#define GENERATE_GRAPH_H_

#include "config.h"

// Contains the infection probability.
struct Edge {
	double prob;
	int node;
	Edge(double prob, int node) : prob(prob), node(node) {
	}
};

typedef std::vector<Edge> Node;
typedef std::vector<Node> Graph;

// Based on the passed settings, create a random initial state.
// Right now, we just generate a directed graph (the network state) of some average connectivity and uniformly weighted connections
Graph generate_graph(Config& config);

#endif /* GENERATE_GRAPH_H_ */
