#ifndef Graph_H
#define Graph_H

#include <iostream> 
#include <list> 
#include <vector>

using namespace std; 

// Graph class represents a undirected graph using adjacency list representation 
class Graph { 
	
	private:
		
		// No. of vertices 
		int V;

		// Pointer to an array containing adjacency lists 
		list<int> *adj; 

		vector<int> component;

		int nb_components;

		// A function used by DFS 
		void DFSUtil(int v, int cc, bool visited[]); 

	public:

		Graph(int V); // Constructor 

		void addEdge(int v, int w); 

		void connectedComponents(); 

		vector<int> getComponent() { return component; }

		int getNbComponents() { return nb_components; }
};

#endif