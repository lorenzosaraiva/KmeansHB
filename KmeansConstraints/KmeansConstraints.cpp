#include <algorithm>
#include <cstdlib>
#include <limits>
#include <random>
#include <cmath>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <ctime>
#include "dataset.h"
#include "Graph.h"

using namespace std;

class Solution {
	
	public:
		
		vector<double> centers;
		vector<int> assignment;

		Solution(vector<double> centers_, vector<int> assignment_) {
			centers = centers_;
			assignment = assignment_;
		}

		~Solution() {

		}
};

class Constraints {
	
	public:
		
		vector< vector<int> > connected_components;
		vector< vector<int> > must_list;
		vector< vector<int> > cannot_list;
		vector< vector<double> > must_weight;
		vector< vector<double> > cannot_weight;
		int nb_must;
		int nb_cannot;
		int points;

		Constraints(int n) {
			must_weight = vector< vector<double> >(n, vector<double>(n));
			cannot_weight = vector< vector<double> >(n, vector<double>(n));
			must_list = vector< vector<int> >(n);
			cannot_list = vector< vector<int> >(n);
			nb_must = 0;
			nb_cannot = 0;
			points = n;

			for (int i = 0; i < n; i++) {
				for (int j = 0; j < n; j++) {
		            must_weight[i][j] = 0;
		            cannot_weight[i][j] = 0;
		        }
		    }
		}

		// Add must link constraint
		void addMustLink(int a, int b, float weight) {
	        //fullList to check the weight of specific broken constraint later
			must_weight[a][b] = weight;
			must_weight[b][a] = weight;
			must_list[a].push_back(b);
			must_list[b].push_back(a);
			nb_must++;
		}

		// Add cannot link constraint
		void addCannotLink(int a, int b, float weight) {
			cannot_weight[a][b] = weight;
			cannot_weight[b][a] = weight;
			cannot_list[a].push_back(b);
			cannot_list[b].push_back(a);
			nb_cannot++;
		}

		void findConnectedComponents() {
			Graph g(points);

			for (int i = 0; i < must_list.size(); i++) {
				for (int j = 0; j < must_list[i].size(); j++) {
					g.addEdge(i, must_list[i][j]);
				}
			}

			g.connectedComponents(); 
			vector<int> component = g.getComponent();
			connected_components = vector< vector<int> >(g.getNbComponents());

			for(int i = 0; i < points; i++) {
				connected_components[component[i]].push_back(i);
			}
		}
};

string replaceString(string s, const string &toReplace, const string &replaceWith) {
	return(s.replace(s.find(toReplace), toReplace.length(), replaceWith));
}

void countRandCoefficients(vector<int> y, vector<int> y_pred, int n, int& a, int& b, int& c, int& d) {
	a = 0;
	b = 0;
	c = 0;
	d = 0;
	
	for(int i = 0; i < n; i++) {
		for(int j = i+1; j < n; j++) {
			if( (y_pred[i] == y_pred[j]) && (y[i] == y[j]) ) {
				a++;
			}
			if( (y_pred[i] == y_pred[j]) && (y[i] != y[j]) ) {
				b++;
			}
			if( (y_pred[i] != y_pred[j]) && (y[i] == y[j]) ) {
				c++;
			}
			if( (y_pred[i] != y_pred[j]) && (y[i] != y[j]) ) {
				d++;
			}
		}
	}
}

// get rand indicator, a measure for partitions agreement. Useful to test accuracy
double rand(vector<int> y, vector<int> y_pred, int n) {
	int a, b, c, d;
	countRandCoefficients(y, y_pred, n, a, b, c, d);
	double rand_index = 1.0*(a + d)/(a + b + c + d);
	return rand_index;
}

// get the c-rand indicator (or adjusted rand), a measure for partitions agreement. Useful to test accuracy
double crand(vector<int> y, vector<int> y_pred, int n) {
	int a, b, c, d;
	countRandCoefficients(y, y_pred, n, a, b, c, d);
	int total = a + b + c + d;
	double crand_index = (a - (1.0*(b + a)*(c + a))/total)/((1.0*(b + a + c + a))/2 - (1.0*(b + a)*(c + a))/total);
	return crand_index;
}

double nmi(vector<int> y, vector<int> y_pred, int n) {
	// assert(y_pred.size() == y.size() && "Two partitions have different number of nodes !");
	int qa = -1, qb = -1;
	vector <int > ga;//group a
	vector <int > gb;//group b

	for (int i = 0; i < n; i++) {
		if(qa < y_pred[i]) qa = y_pred[i];
		if(qb < y[i]) qb = y[i];
	}
	qa++;
	qb++;
	if(qa == 1 && qb == 1) return 0.0;
	ga.resize(qa);

	for (int q = 0; q < qa; q++) ga[q]=0;
	gb.resize(qb);
	for (int q = 0; q < qb; q++) gb[q]=0;

	vector< vector<int> > A;
	vector< vector<int> > B;
	A.resize(qa); //existing structure
	B.resize(qa); //counting structure
	
	for (int i = 0; i < n; i++) {
		int q = y_pred[i];
		int t = y[i];
		ga[q]++;
		gb[t]++;
		int idx = -1;
		for(int j = 0; j < A[q].size(); j++) {
			if(A[q][j] == t) {
				idx = j;
				break;
			}
		}
		if(idx == -1) {//pair [x y] did not show up
			A[q].push_back(t);
			B[q].push_back(1);
		} else {// [x y] is there
			B[q][idx] += 1;
		}
	}
	double Ha = 0;
	for(int q = 0; q < qa; q++) {
		if(ga[q] == 0) continue;
		double prob = 1.0 * ga[q]/n;
		Ha += prob*log(prob);
	}
	double Hb = 0;
	for(int q = 0; q < qb; q++) {
		if(gb[q] == 0) continue;
		double prob = 1.0 * gb[q]/n;
		Hb += prob*log(prob);
	}
	double Iab = 0;
	for(int q = 0; q < qa; q++) {
		for(int idx = 0; idx < A[q].size(); idx++) {
			double prob = 1.0 * B[q][idx]/n;
			int t = A[q][idx];
			Iab += prob*log(prob/ ( 1.0*ga[q]/n*gb[t]/n ));
		}
	}
	return -2.0*Iab/(Ha+Hb);
}

vector<int> getLabels(string filename, int n, int m) {
	
	string fileLabels = "data/" + filename + ".label";
 	// Open the labels file
	ifstream inputLabels(fileLabels.c_str());
	if (! inputLabels) {
		cerr << "Unable to open labels file: " << fileLabels << endl;
	}
	
	int max = 0;
	vector<int> y(n);
	
	for (int i = 0; i < n; ++i) {
		inputLabels >> y[i];
		y[i] = y[i] - 1; // labels file starts from 1
		if(y[i] > max)
			max = y[i];
	}
	
	if(max != m-1) {
		cerr << "Number of labels does not match m" << endl;
	}
	return y;
}

double pointToCenterDist(int p, int c, vector<double> centers, const Dataset* dataset) {
	int d = dataset->d;
	double* data = dataset->data;
	double ret = 0.0;

	for (int i = 0; i < d; i++) {
		double a = data[p * d + i];
		double b = centers[c * d + i];
		double diff = a - b;
		ret += diff * diff;
	}
	return ret;
}

double pointToPointDist(int p1, int p2, const Dataset* dataset) {
	int d = dataset->d;
	double* data = dataset->data;
	double ret = 0.0;
	for (int i = 0; i < d; i++) {
		ret += (data[p1 * d + i] - data[p2 * d + i]) * (data[p1 * d + i] - data[p2 * d + i]);
	}
	return ret;
}

double centerToCenterDist(vector<double> c1, vector<double> c2, const Dataset* dataset) {
	double dist = 0.0;
	for (int i = 0; i < dataset->d; i++) {
		dist += (c1[i] - c2[i]) * (c1[i] - c2[i]);
	}
	return dist;
}

void initialize(Constraints* constraints, vector<int> &assignment, vector<double> &centers,
	const Dataset* dataset, int nb_clusters, vector<int> &randomizer, int seed) {
	
	int n = dataset->n;
	int d = dataset->d;

	static mt19937 rng(seed);
	uniform_int_distribution<int> indices(0, dataset->n - 1);
	
	for (int i = 0; i < nb_clusters; i++) {
		int p = indices(rng);
		for (int j = 0; j < d; j++) {
			centers[i * d + j] = dataset->data[p * d + j];
		}
	}
	
	// using built-in random generator:
	random_shuffle(randomizer.begin(), randomizer.end());
	
	// Assign data points to closest centroid, considering penalities related to contraints 
	for (int k = 0; k < n; k++) {
		int p = randomizer[k];
		double best_distance = numeric_limits<double>::max();
		int best_cluster = 0;
		for (int c = 0; c < nb_clusters; c++) {
			// For each point, calculate the nearest cluster
			double new_distance = pointToCenterDist(p, c, centers, dataset);
			for (int j = 0; j < constraints->must_list[p].size(); j++) {
				int i = constraints->must_list[p][j];
				if(assignment[i] != c) {
					new_distance += constraints->must_weight[p][i];
				}
			}
			for (int j = 0; j < constraints->cannot_list[p].size(); j++) {
				int i = constraints->cannot_list[p][j];
				if (assignment[i] == c) {
					new_distance += constraints->cannot_weight[p][i];
				}
			}
			if (new_distance < best_distance) {
				// If the distance is better, updates the best cluster and distance  
				best_distance = new_distance;
				best_cluster = c;
			}
		}
		assignment[p] = best_cluster;
	}	
}

void moveComponents(Constraints* constraints, vector<int> &assignment, vector<double> centers,
	const Dataset* dataset, int nb_clusters) {
	
	int d = dataset->d;

	vector<int> randomizer;
	for (int i = 0; i < nb_clusters; i++)
		randomizer.push_back(i);

	random_shuffle(randomizer.begin(), randomizer.end());

	for (int i = 0; i < constraints->connected_components.size(); i++) {

		for (int k1 = 0; k1 < nb_clusters; k1++) {
			
			// int k = k1;
			int k = randomizer[k1];

			double curr_distance = 0;
			double new_distance = 0;

			// if (constraints->connected_components[i].size() > 1) {
				for (int j = 0; j < constraints->connected_components[i].size(); j++) {
					
					int p = constraints->connected_components[i][j];
					
					new_distance += pointToCenterDist(p, k, centers, dataset);
					curr_distance += pointToCenterDist(p, assignment[p], centers, dataset);
					
					for (int i = 0; i < constraints->cannot_list[p].size(); i++) {
						int j = constraints->cannot_list[p][i];
						if (k == assignment[j]) {
							new_distance += constraints->cannot_weight[p][j];
						}
						if (assignment[p] == assignment[j]) {
							curr_distance += constraints->cannot_weight[p][j];
						}
					}
				}

				if (new_distance < curr_distance) {
					for (int j = 0; j < constraints->connected_components[i].size(); j++) {
						assignment[constraints->connected_components[i][j]] = k;
					}
				}
			// }
		}
	}	
}

void updateCentroids(vector<double> &centers, const vector<int> &assignment, const Dataset* dataset, int nb_clusters) {
	int n = dataset->n;
	int d = dataset->d;
	vector<double> new_centers(nb_clusters * d, 0);
	
	// Creates a vector to count how many points in a cluster
	vector<int> points_in_cluster(nb_clusters, 0);

	for (int p = 0; p < n; p++) {
		// Get the right cluster for each point
		int c = assignment[p];
		for (int i = 0; i < d; i++) {
			//Sums the points belonging to a cluster
			new_centers[c * d + i] += dataset->data[p * d + i];
		}
		points_in_cluster[c] += 1;
	}
	
	// Divide sums by number of points to get new centroids.
	for (int c = 0; c < nb_clusters; c++) {
		// Turn 0/0 into 0/1 to avoid zero division.
		int count = max<int>(1, points_in_cluster[c]);
		for (int i = 0; i < d; i++) {
			centers[c * d + i] = new_centers[c * d + i]/count;
		}
	}
}

void assignPoints(Constraints* constraints, vector<int> &assignment, vector<double> centers,
	const Dataset* dataset, int nb_clusters, vector<int> &randomizer) {

	int n = dataset->n;
	int d = dataset->d;

	random_shuffle(randomizer.begin(), randomizer.end());
	
	for (int k = 0; k < n; k++) {
		int p = randomizer[k];
		double best_distance = std::numeric_limits<double>::max();;
		int best_cluster = 0;
		for (int c = 0; c < nb_clusters; c++) {
			// For each point, calculate the nearest cluster
			// For each sample, in random order, calculate the minimum cost of change of assignment from its current center to 
			//another (considering the other samples as well assigned and the penalties associated to them in case we violate must-link 
			//or cannot-link constraints), if the difference of cost is smaller than 0, apply the change.
			double new_distance = pointToCenterDist(p, c, centers, dataset);
			for(int i = 0; i < constraints->must_list[p].size(); i++) {
				int j = constraints->must_list[p][i];
				if(c != assignment[j]) {
					new_distance += constraints->must_weight[p][j];
				}
			}
			for(int i = 0; i < constraints->cannot_list[p].size(); i++) {
				int j = constraints->cannot_list[p][i];
				if(c == assignment[j]) {
					new_distance += constraints->cannot_weight[p][j];
				}
			}
			if (new_distance < best_distance) {
				// If the distance is better, updates the best cluster and distance 
				best_distance = new_distance;
				best_cluster = c;
			}
		}
		assignment[p] = best_cluster;
	}
}

double costMsscStandard(const Dataset* dataset, vector<double> solution, vector<int> assignments) {
    double* data = dataset->data;
    int n = dataset->n;
    int d = dataset->d;
    double cost = 0.0;

    for (int p = 0; p < n; p++) {
		int cluster = assignments[p];
		cost += pointToCenterDist(p, cluster, solution, dataset);
    }
    return cost;
}

double costMsscConstraints(const Dataset* dataset, vector<double> solution, vector<int> assignments, Constraints* constraints) {
    double* data = dataset->data;
    int n = dataset->n;
    int d = dataset->d;
    double cost = 0.0;

    for (int p = 0; p < n; p++) {
		int cluster = assignments[p];
		cost += pointToCenterDist(p, cluster, solution, dataset);
    }

	vector< vector<int> > doubleCheckML(n, vector<int>(n));
	vector< vector<int> > doubleCheckCL(n, vector<int>(n));

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			doubleCheckML[i][j] = 0;
			doubleCheckCL[i][j] = 0;
		}
	}
	
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			if(constraints->must_weight[i][j] != 0.0) {
				if(assignments[i] != assignments[j] && doubleCheckML[i][j] == 0 && doubleCheckML[j][i] == 0) {
					cost += constraints->must_weight[i][j];
					doubleCheckML[i][j] = 1;
					doubleCheckML[j][i] = 1;
				}
			}
			if (constraints->must_weight[i][j] != 0.0) {
				if (assignments[i] == assignments[j] && doubleCheckCL[i][j] == 0 && doubleCheckCL[j][i] == 0) {
					cost += constraints->must_weight[i][j];
					doubleCheckCL[i][j] = 1;
					doubleCheckCL[j][i] = 1;
				}
			}
		}
	}
    return cost;
}

 vector< vector<int> > unify_components(Constraints* constraints, const Dataset* dataset, int k) {
	vector< vector<int> > components = constraints->connected_components;
	vector< vector<int> > components_;
	vector< vector<double> > centers_;
	double* data = dataset->data;
	int d = dataset->d;

	int ct = 0;

	for (int i = 0; i < components.size(); i++) {

		if (components[i].size() > 1) {
			vector<int> list;
			vector<double> c(d, 0.0);

			for (int j = 0; j < components[i].size(); j++) {
				int p = components[i][j];
				list.push_back(p);

				for (int q = 0; q < d; q++) {
					c[q] = c[q] + data[p*d + q];
				}
			}

			for (int q = 0; q < d; q++) {
				c[q] = 1.0 * c[q] / components[i].size();
			}

			components_.push_back(list);
			centers_.push_back(c);
		}
	}

	vector< vector<bool> > can_merge (components_.size(), vector<bool> (components_.size()));

	for (int i = 0; i < components_.size(); i++) {
		for (int j = 0; j < components_.size(); j++) {
			for (int q1 = 0; q1 < components_[i].size(); q1++) {
				for (int q2 = 0; q2 < components_[j].size(); q2++) {
					if (constraints->cannot_weight[q1][q2] > 0 || constraints->cannot_weight[q2][q1] > 0) {
						can_merge[i][j] = false;
						can_merge[j][i] = false;
					} else {
						can_merge[i][j] = true;
						can_merge[j][i] = true;
					}
				}
			}
		}
	}

	while(components_.size() > k) {
		int nb_components = components_.size();
		double min_dist = numeric_limits<double>::max();
		int c1, c2;

		// Normalized cut minimization
		for (int i = 0; i < nb_components; i++) {
			for (int j = i+1; j < nb_components; j++) {
				double dist = 0.0;
				for (int q1 = 0; q1 < components_[i].size(); q1++) {
					for (int q2 = 0; q2 < components_[j].size(); q2++) {
						dist = dist + pointToPointDist(components_[i][q1], components_[j][q2], dataset);		
					}
				}
				
				dist = ( 1.0 * dist ) / ( components_[i].size() * components_[j].size() );
				
				// get the pair of closest centers
				if (dist < min_dist && can_merge[i][j]) {
					min_dist = dist;
					c1 = i;
					c2 = j;
				}
			}		
		}

		// Distance of centers minimization
		// for (int i = 0; i < centers_.size(); i++) {
		// 	for (int j = i+1; j < centers_.size(); j++) {
		// 		double dist = centerToCenterDist(centers_[i], centers_[j], dataset);
		// 		// get the pair of closest centers
		// 		if (dist < min_dist && can_merge[i][j]) {
		// 			min_dist = dist;
		// 			c1 = i;
		// 			c2 = j;
		// 		}
		// 	}		
		// }

		// Merge clusters
		vector<int> merged_cluster;
		for (int i = 0; i < components_[c1].size(); i++) {
			merged_cluster.push_back(components_[c1][i]);
		}
		for (int i = 0; i < components_[c2].size(); i++) {
			merged_cluster.push_back(components_[c2][i]);	
		}

		// Update list of points for merged cluster
		vector< vector<int> > aux_components(nb_components - 1);
		aux_components[0] = merged_cluster;

		// Update center for merged cluster
		vector< vector<double> > aux_centers(nb_components - 1);
		vector<double> c(d, 0.0);
		for (int j = 0; j < aux_components[0].size(); j++) {
			int p = aux_components[0][j];
			for (int q = 0; q < d; q++) {
				c[q] = c[q] + data[p*d + q];
			}
		}
		for (int q = 0; q < d; q++) {
			c[q] = 1.0 * c[q] / aux_components[0].size();
		}
		aux_centers[0] = c;
		
		// Copy list of points and centers for the remaining (not changed) clusters
		int counter = 0;
		for (int i = 0; i < components_.size(); i++) {
			if (i != c1 && i != c2) {
				counter++;
				aux_components[counter] = components_[i];
				aux_centers[counter] = centers_[i];
			}
		}

		components_.resize(nb_components - 1);
		components_ = aux_components;

		centers_.resize(nb_components - 1);
		centers_ = aux_centers;
	}

	return components_;
}

Solution* kmeans(const Dataset* dataset, int nb_clusters, Constraints* constraints, int nb_it, int seed) {
	double* data = dataset->data;
	int n = dataset->n;
	int d = dataset->d;

	// Creates the clusters vector based on the number of desired clusters
	vector<double> centers(nb_clusters * d);
	vector<int> assignment(n);
	
	vector<int> randomizer;
	for (int i = 0; i < n; i++) 
		randomizer.push_back(i); 

	// STEP 1
	initialize(constraints, assignment, centers, dataset, nb_clusters, randomizer, seed);

	bool converged = false;

	for (int it = 0; (!converged && (it < nb_it)); ++it) {

		// STEP 2
		updateCentroids(centers, assignment, dataset, nb_clusters);

		vector<int> prev_assignment = assignment;

    	// STEP 3
		moveComponents(constraints, assignment, centers, dataset, nb_clusters);
		assignPoints(constraints, assignment, centers, dataset, nb_clusters, randomizer);

		// Check convergence
		if(prev_assignment == assignment) {
			converged = true;
		}
	}

	Solution* s = new Solution(centers, assignment);
	return s;
}

void inferAdditionalConstraints(Constraints* constraints, vector < pair<int, int> > cannot_pairs, int n, int W) {
	// List with the component number for each sample: index is sample, value is the cc number
	vector<int> sample_component(n);
	for (int i = 0; i < constraints->connected_components.size(); i++) {
		for (int j = 0; j < constraints->connected_components[i].size(); j++) {
			sample_component[constraints->connected_components[i][j]] = i;
		}
	}

	// Store connected components with more than 1 data point
	vector< vector<int> > multi_components; 
	for (int i = 0; i < constraints->connected_components.size(); i++) {
		if (constraints->connected_components[i].size() > 1) {
			multi_components.push_back(constraints->connected_components[i]);
		}
	}

	// "Infer" must links from transitivity
	for (int i = 0; i < multi_components.size(); i++) {
		for (int j = 0; j < multi_components[i].size(); j++) {
			for (int q = j+1; q < multi_components[i].size(); q++) {
				if (constraints->must_weight[multi_components[i][j]][multi_components[i][q]] != 0) {
					constraints->addMustLink(multi_components[i][j], multi_components[i][q], W);
				}
			}
		}
	}

	// "Infer" cannot links from existing must links
	for (int i = 0; i < cannot_pairs.size(); i++) {
		int p1 = cannot_pairs[i].first;
		int p2 = cannot_pairs[i].second;
		if (sample_component[p1] != sample_component[p2]) {
			// Connected component of p1
			vector<int> C1 = constraints->connected_components[sample_component[p1]];
			// Connected component of p2
			vector<int> C2 = constraints->connected_components[sample_component[p2]];
			// Add cannot-link for each pair a,b; where a \in C1 and b \in C2
			for (int q1 = 0; q1 < C1.size(); q1++) {
				for (int q2 = 0; q2 < C2.size(); q2++) {
					if (constraints->cannot_weight[C1[q1]][C2[q2]] != 0) {
						constraints->addCannotLink(C1[q1], C2[q2], W);
					}
				}
			}
		}
	}
}

int main(int argc, char** argv) {

	//Get the data file name
	string file_name = argv[1];
	
	//Get the solution file name
	string solution_name = argv[2];
	
	//Amount of Kmeans calls in each loop
	int nb_kmeans = atoi(argv[3]);
	
	//The amount of loops
	int nb_runs = atoi(argv[4]);
	
	//Seed
	int seed = atoi(argv[5]);
	
	// Open the data file
	ifstream dataset_file(file_name);
	
	// Read the parameters
	int n, d;
	const int W = 100;

	dataset_file >> n;
	dataset_file >> d;
	
	// Allocate storage
	Dataset *x = new Dataset(n, d);
	
	vector<double> mean(d,0.0);
	vector<double> stdev(d,0.0);

	vector<double> max(d,0.0);
	vector<double> min(d,numeric_limits<double>::max());

	// Read the data values directly into the dataset
	for (int i = 0; i < n * d; ++i) {
		dataset_file >> x->data[i];
		if(x->data[i] < min[i % d]) {
			min[i % d] = x->data[i];
		}
		if(x->data[i] > max[i % d]) {
			max[i % d] = x->data[i];
		}
		mean[i % d] = mean[i % d] + x->data[i];
	}

	for (int i = 0; i < d; i++) {
		mean[i] = (1.0 * mean[i]) / n;
	}

    for (int i = 0; i < n * d; i++) {
        stdev[i % d] = stdev[i % d] + pow(x->data[i] - mean[i % d], 2);
    }

    for (int i = 0; i < d; i++) {
        stdev[i] = sqrt((1.0 * stdev[i]) / (n-1));
    }

    // Standardization: (x - avg) / stdev
	// for (int i = 0; i < n * d; ++i) {
	// 	if (stdev[i % d] == 0.0) {
	// 		x->data[i] = 0.0;
	// 	} else {
	// 		x->data[i] = (1.0*(x->data[i] - mean[i % d])) / (stdev[i % d]);
	// 	}
	// }

    // Re-scaling (min-max normalization): (x - x_min) / (x_max - x_min)
	for (int i = 0; i < n * d; ++i) {
		if( (max[i % d] - min[i % d]) == 0 ) {
			x->data[i] = 0.0;
		} else {
			x->data[i] = 1.0 * (x->data[i] - min[i % d]) / (max[i % d] - min[i % d]);
		}
	}

	// Open a file with the constraints
	ifstream constraints_file(solution_name);

	//Read the parameters
	int nb_clusters;
	int nb_must;
	int nb_cannot;
	double informativeness;
	double coherence;
	
	constraints_file >> nb_clusters;
	constraints_file >> nb_must;
	constraints_file >> nb_cannot;
	constraints_file >> informativeness;
	constraints_file >> coherence;

	int nb_constraints = nb_must + nb_cannot;
	
	Constraints *constraints = new Constraints(n);
	
	int a, b;
	double w;
	vector < pair<int, int> > cannot_pairs(nb_cannot);

	// Read the assignments
	for(int i = 0; i < nb_must; i++) {
		constraints_file >> a;
		constraints_file >> b;
		constraints_file >> w;
		constraints->addMustLink(a, b, W);
	}
	
	for(int i = 0; i < nb_cannot; i++) {
		constraints_file >> a;
		constraints_file >> b;
		constraints_file >> w;
		constraints->addCannotLink(a, b, W);
		cannot_pairs[i].first = a; 
		cannot_pairs[i].second = b;
	}
	
	constraints->findConnectedComponents();
	inferAdditionalConstraints(constraints, cannot_pairs, n, W);
	constraints->findConnectedComponents();

	string instance = replaceString(file_name, "data/", "");
	instance = replaceString(instance, ".data", "");
	vector<int> y = getLabels(instance, n, nb_clusters);
	
	ifstream out_hgmeans(replaceString(file_name, ".data", ".out"));
	ifstream out_kmeans(replaceString(file_name, ".data", ".out2"));
	
	vector<int> y_hgmeans(n);
	vector<int> y_kmeans(n);

	for (int i = 0; i < n; ++i) {
		out_hgmeans >> y_hgmeans[i];
	}

	for (int i = 0; i < n; ++i) {
		out_kmeans >> y_kmeans[i];
	}
	
	double sum_rand = 0.0;
	double sum_crand = 0.0;
	double sum_nmi = 0.0;

	double best_rand = 0.0;
	double best_crand = 0.0;
	double best_nmi = 0.0;
	
	double best_cost;
	double elapsed_secs;

	int counter = 0;
	Solution* best_s;

	// constraints->connected_components = unify_components(constraints, x, nb_clusters);

	while(counter < nb_runs) {
		
		best_cost = numeric_limits<double>::max();
		clock_t begin = clock();
		
		for (int i = 0; i < nb_kmeans; i++) {

			Solution* s = kmeans(x, nb_clusters, constraints, 1000, seed);

			double cost = costMsscConstraints(x, s->centers, s->assignment, constraints);
			double rand_index = rand(s->assignment, y, n);
			double crand_index = crand(s->assignment, y, n);
			double nmi_index = nmi(s->assignment, y, n);
			
			sum_rand = sum_rand + rand_index;
			sum_crand = sum_crand + crand_index;
			sum_nmi = sum_nmi + nmi_index;

			if(cost < best_cost) {
				if(i > 0)
					delete best_s;
				best_s = s;
				best_cost = cost;
				best_rand = rand_index;
				best_crand = crand_index;
				best_nmi = nmi_index;
			} else {
				delete s;
			}
		}

		vector<int> list_must; //list of points belonging to at least one must link
		vector<int> y_must; //ground truth of points belonging to at least one must link
		vector<int> y_must_pred; //prediction of points belonging to at least one must link

		for (int i = 0; i < constraints->connected_components.size(); i++) {
			if (constraints->connected_components[i].size() > 1) {
				for (int j = 0; j < constraints->connected_components[i].size(); j++) {
					list_must.push_back(constraints->connected_components[i][j]);
					y_must.push_back(y[constraints->connected_components[i][j]]);
					y_must_pred.push_back(best_s->assignment[constraints->connected_components[i][j]]);
				}
			}
		}

		double crand_connected = crand(y_must_pred, y_must, list_must.size());
		// cout << list_must.size() << endl;
		// cout << "c-rand for CC @" << instance << " = " << crand_connected << endl;

		clock_t end = clock();
		elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
		counter++;
	}

	double cost_mssc = costMsscStandard(x, best_s->centers, best_s->assignment);
	
	cout << "S-Kmeans " << instance	<< setw(20 - instance.length())
						<< "\t" << std::setprecision(4) << best_cost
						<< "\t" << std::setprecision(4) << cost_mssc
						//<< "\t" << std::setprecision(4) << best_rand 
						<< "\t" << std::setprecision(4) << best_crand
						// << "\t" << std::setprecision(4) << sum_crand/nb_kmeans
						<< "\t" << std::setprecision(4) << best_nmi
						// << "\t" << std::setprecision(4) << sum_nmi/nb_kmeans
						<< "\t" << std::setprecision(4) << 1.0 * elapsed_secs << endl;
	
	// cout << "HG-means " << instance	<< setw(20 - instance.length())
	// 							<< "\t" << std::setprecision(4) << "####"
	// 							// << rand(y_hgmeans, y, n)
	// 							<< "\t" << std::setprecision(4) << crand(y_hgmeans, y, n) << "\t" << std::setprecision(4) << "####"
	// 							<< "\t" << std::setprecision(4) << nmi(y_hgmeans, y, n) << "\t" << std::setprecision(4) << "####"
	// 							<< "\t" << std::setprecision(4) << "####" << endl;
	
	// cout << "K--means " << instance	<< setw(20 - instance.length())
	// 							<< "\t" << std::setprecision(4) << "####"
	// 							// << rand(y_kmeans, y, n)
	// 							<< "\t" << std::setprecision(4) << crand(y_kmeans, y, n) << "\t" << std::setprecision(4) << "####"
	// 							<< "\t" << std::setprecision(4) << nmi(y_kmeans, y, n) << "\t" << std::setprecision(4) << "####"
	// 							<< "\t" << std::setprecision(4) << "####" << endl;

	// cout << "------------------------------------------------------------" << endl;

	// Save outputs
	ofstream myfile;
    stringstream outfile;

    string fileData = solution_name;
    fileData = replaceString(fileData, "data/", "");
    fileData = replaceString(fileData, ".link", "");

    outfile << "output/001/" << fileData << ".out";

    myfile.open (outfile.str().c_str(), ofstream::out | ofstream::app);  
	myfile << best_crand << " ";
	myfile << crand(y_hgmeans, y, n) << " ";
	myfile << crand(y_kmeans, y, n) << " ";
	myfile << best_nmi << " ";
	myfile << nmi(y_hgmeans, y, n) << " ";
	myfile << nmi(y_kmeans, y, n) << " ";
	myfile << best_cost << " ";
	myfile << 1.0 * elapsed_secs << "\n";
	myfile.close();

	delete x;
	delete constraints;
}
