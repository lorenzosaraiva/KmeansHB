#include "Graph.h"

// Method to print connected components in an undirected graph 
void Graph::connectedComponents() {
	// Mark all the vertices as not visited 
	bool *visited = new bool[V]; 
	for(int v = 0; v < V; v++) 
		visited[v] = false;

	int cc = 0;

	for (int v = 0; v < V; v++) { 
		if (visited[v] == false) { 
			// Print all reachable vertices from v 
			DFSUtil(v, cc, visited);
			cc++;
		}
	}

	nb_components = cc;
}

void Graph::DFSUtil(int v, int cc, bool visited[]) { 
	// Mark the current node as visited and print it 
	visited[v] = true; 
	component[v] = cc;

	// Recur for all the vertices adjacent to this vertex 
	list<int>::iterator i; 
	for(i = adj[v].begin(); i != adj[v].end(); ++i) {
		if(!visited[*i]) {
			DFSUtil(*i, cc, visited); 
		}
	}
}

Graph::Graph(int V) { 
	this->V = V; 
	adj = new list<int>[V];
	component = vector<int>(V);
	nb_components = 0;
}

// method to add an undirected edge 
void Graph::addEdge(int v, int w) { 
	adj[v].push_back(w); 
	adj[w].push_back(v); 
}
