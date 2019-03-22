#include <algorithm>
#include <cstdlib>
#include <limits>
#include <random>
#include <vector>
#include <cmath>
#include <map>
#include <random>
#include <stdio.h>      
#include <stdlib.h>
#include <iostream>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "dataset.h"
#include <ctime>



using namespace std;

class Solution
{ 
    public:
      double* solution;
      std::vector<int> assignments;
      Solution(double* sol, std::vector<int> assign){
        solution = sol;
        assignments = assign;
      }
};

class Constraints{
    public:
      vector<vector<int> > connectedComponents;
      vector<vector<int> > mlAdjacencyList;
      vector<vector<int> > clAdjacencyList;
      int cannotLinksAmount;
      int mustLinksAmount;
      float** mustLinks;
      float** cannotLinks;
      int** jointConstraints;
      int** fixedConstraints;
      int points;
      int uniqueLinks;

      Constraints(int n){
        points = n;
        jointConstraints = new int*[n];
        mustLinks = new float*[n];
        cannotLinks = new float*[n];
        uniqueLinks = 0;
        mustLinksAmount = 0;
        cannotLinksAmount = 0;


        for (int i = 0; i < n; i++){
          vector<int> v1;
          mlAdjacencyList.push_back(v1);

          vector<int> v2;
          clAdjacencyList.push_back(v2);

          jointConstraints[i] = new int[n];
          mustLinks[i] = new float[n];
          cannotLinks[i] = new float[n];

          for (int j = 0; j < n; j++){
            jointConstraints[i][j] = 0;
            mustLinks[i][j] = 0;
            cannotLinks[i][j] = 0;

          }
        }

      }

      void addCannotLink(int a, int b, float weight){
        cannotLinks[a][b] = weight;
        cannotLinks[b][a] = weight;
        clAdjacencyList[a].push_back(b);
        clAdjacencyList[b].push_back(a);
        cannotLinksAmount++;
      }

      // Add pairs of contraints

      void addMustLink(int a, int b, float weight){

        //fullList to check the weight of specific broken constraint later
        mustLinks[a][b] = weight;
        mustLinks[b][a] = weight;
        mlAdjacencyList[a].push_back(b);
        mlAdjacencyList[b].push_back(a);
        mustLinksAmount++;
        for(int i = 0; i < uniqueLinks; i++){
          //check if one of the points is already in one of the must-link

          if (jointConstraints[i][a] == 1 || jointConstraints[i][b] == 1){
            // if yes, add both points to the must-link
            jointConstraints[i][a] = 1;
            jointConstraints[i][b] = 1;
            return;
          }
        }

        // if not, create a new link
        jointConstraints[uniqueLinks][a] = 1;
        jointConstraints[uniqueLinks][b] = 1;
        uniqueLinks++;
        //fixConstraints() still has to be called

      }

      void fixConstraints(){
        int* discarded = new int[points];
        for (int i = 0; i < points; i ++){
          discarded[i] = 0;
        }
        int fixedLinks = uniqueLinks;
        // what is the complexity order of this monstruosity ?? uniqueLinks ˆ 2 * points ˆ 2?? Is that tolerable? ceiling of pointsˆ4?
        for (int i = 0; i < uniqueLinks; i++){
          for(int j = 0; j < uniqueLinks; j++){
            for (int k = 0; k < points; k++){
              if (jointConstraints[i][k] == 1 && jointConstraints[j][k] == 1 && i != j){
                discarded[j] = 1;
                fixedLinks--;
                for (int l = 0; l < points; l++){
                  if (jointConstraints[j][l] == 1){
                    jointConstraints[i][l] = 1;
                    jointConstraints[j][l] = 0;
                  }
                } 
              }
            }
          }
        }

        fixedConstraints = new int*[fixedLinks];
        int index = 0;
        for (int i = 0; i < uniqueLinks; i++){
          if (discarded[i] == 0){
              fixedConstraints[index] = jointConstraints[i];
              index++;
          }
        }

        uniqueLinks = fixedLinks;


      }

      void checkIntegrity(){

        for (int i = 0; i < uniqueLinks; i++){
          for(int j = 0; j < uniqueLinks; j++){
            for (int k = 0; k < points; k++){
              if (fixedConstraints[i][k] == 1 && fixedConstraints[j][k] == 1 && i != j){
                cout << "Erro nos must link " << i << " e " << j << " na posição " << k << endl;
              }
            }
          }
        }

        //cout << "Fim do teste." << endl;
      }

      void printConstraints(){
        for (int i = 0; i < uniqueLinks; i++){
          //cout << "Must-link " << i << endl;
          std::vector<int> v;
          connectedComponents.push_back(v);
          for (int j = 0; j < points; j++){
            if(fixedConstraints[i][j] == 1){
              //cout << j << " ";
              connectedComponents[i].push_back(j);

            }
          }

          cout << endl;
        }

        for(int i = 0; i < 0; i++){
          for(int j = 0; j < clAdjacencyList[i].size(); j++){
            cout << "Cannot-Link " << i << " and " << clAdjacencyList[i][j] << endl;
          }
        }
      }      
};

// Calculates the sum-of-squares of the Euclidean distance between 2 points

double pointToPointDistance(double* data, double* solution, int p, int cluster, int d) {

  double ret = 0.0;
  for (int i = 0; i < d; i++){

    double a = data[p * d + i];
    double b = solution[cluster * d + i];

    double diff = a - b;
    ret += diff * diff;
  }

  return ret;
}
double getCostWithoutConstraints(const Dataset* dataset, double* solution, std::vector<int> assignments){
    double* data = dataset->data;
    int n = dataset->n;
    int d = dataset->d;

    double cost = 0.0;

    for (int p = 0; p < n; p++){
      int cluster = assignments[p];
      cost += pointToPointDistance(data, solution, p, cluster, d);
    }

    return cost;
}
double getCost(const Dataset* dataset, double* solution, std::vector<int> assignments, Constraints* constraints){
    double* data = dataset->data;
    int n = dataset->n;
    int d = dataset->d;

    double cost = 0.0;

    for (int p = 0; p < n; p++){
      int cluster = assignments[p];
      cost += pointToPointDistance(data, solution, p, cluster, d);
    }

    // create a nxn matrix do
    int** doubleCheckML = new int*[n];
    int** doubleCheckCL = new int*[n];
    for(int i = 0; i < n; i++){
      doubleCheckML[i] = new int[n];
      doubleCheckCL[i] = new int[n];
      for (int j = 0; j < n; j++){
            doubleCheckML[i][j] = 0;
            doubleCheckCL[i][j] = 0;
      }
    }


    for(int i = 0; i < n; i++){
      for (int j = 0; j < n; j++){
        if(constraints->mustLinks[i][j] != 0.0){
          if(assignments[i] != assignments[j] && doubleCheckML[i][j] == 0 && doubleCheckML[j][i] == 0){
            cost += constraints->mustLinks[i][j];
            doubleCheckML[i][j] = 1;
            doubleCheckML[j][i] = 1;

          }
        }

        if(constraints->cannotLinks[i][j] != 0.0){
          if(assignments[i] == assignments[j] && doubleCheckCL[i][j] == 0 && doubleCheckCL[j][i] == 0){
            cost += constraints->cannotLinks[i][j];
            doubleCheckCL[i][j] = 1;
            doubleCheckCL[j][i] = 1;

          }
        }
      }
    }

    return cost;
}

string replaceString(string s, const string &toReplace, const string &replaceWith) {
    return(s.replace(s.find(toReplace), toReplace.length(), replaceWith));
}

double getCrand(std::vector<int> bestSolution, std::vector<int> foundSolution, int n){
  int trueNegative = 0;
  int falseNegative = 0;
  int truePositive = 0;
  int falsePositive = 0;
  for(int i = 0; i < n; i++){
    for(int j = 0; j < n; j++){
      if(i != j){

        if(bestSolution[i] == bestSolution[j]){
          if(foundSolution[i] == foundSolution[j]){
            truePositive++;
          }
          else{
            falseNegative++;
          }
        }else{
          if(bestSolution[i] != bestSolution[j]){
            if(foundSolution[i] != foundSolution[j]){
              trueNegative++;  
            }else{
              falsePositive++;
            }
          }
        }
      }
    }
  }
  double rand = (trueNegative + truePositive)/(truePositive + trueNegative + falsePositive + falseNegative);
  int total = truePositive + falseNegative + falsePositive + trueNegative;
  double crand = (truePositive - (1.0*(falseNegative + truePositive)*(falsePositive + truePositive))/total)/((1.0*(falseNegative + truePositive + falsePositive + truePositive))/2 - (1.0*(falseNegative + truePositive)*(falsePositive + truePositive))/total);

  return crand;
}

Solution* kmeans(const Dataset* dataset, int clusters, int numIt, int n, int d, Constraints* constraints) {


  static std::random_device seed;
  static std::mt19937 rng(1607);
  std::uniform_int_distribution<int> indices(0, n - 1);

  double* data = dataset->data;

  // START OF STEP 1) INITIALIZATION


  // Creates the clusters vector based on the number of desired clusters
  double* solution = new double[clusters * d];


  //- Pick K random samples to define the initial center positions
  for (int i = 0; i < clusters; i++) {
    int p = indices(rng);
    for (int j = 0; j < d; j++){
      solution[i * d + j] = data[p * d + j];
    }
  }

  std::vector<int> assignments(n);
  std::vector<double> bestDistances(n);




  //- For each sample, in random order, calculate the minimum cost of an assignment of this sample to a center 

  std::vector<int> randomizer;
  for (int i = 0; i < n; i++) 
    randomizer.push_back(i); 


  // using built-in random generator:
  std::random_shuffle(randomizer.begin(), randomizer.end());

  for (int k = 0; k < n; k++) {

      int p = randomizer[k];

      double bestDistance = std::numeric_limits<double>::max();

      int bestCluster = 0;

      for (int cluster = 0; cluster < clusters; cluster++) {
        // For each point, calculate the nearest cluster
        double newDistance = pointToPointDistance(data, solution, p, cluster, d);


        for(int j = 0; j < constraints->mlAdjacencyList[p].size(); j++){
          int i = constraints->mlAdjacencyList[p][j];
          if(assignments[i] != cluster){
            newDistance += constraints->mustLinks[p][i];
          }
        }

        for(int j = 0; j < constraints->clAdjacencyList[p].size(); j++){
          int i = constraints->clAdjacencyList[p][j];
          if(assignments[i] == cluster){
              newDistance += constraints->cannotLinks[p][i];
          }
        }

        if (newDistance < bestDistance){
          // If the distance is better, updates the best cluster and distance  
          bestDistance = newDistance;
          bestCluster = cluster;
        }


      }
      assignments[p] = bestCluster;
      bestDistances[p] = bestDistance;

  }

  // END OF STEP 1



  // Creates the assignment vector based on the number of points

  for (int iteration = 0; iteration < numIt; ++iteration) {

    // START OF STEP 2)  Update centers positions to be in the barycenter of the points currently assigned to it

    // Creates the vector for the new clusters
    std::vector<double> newSolution(clusters * d, 0);

    // Creates a vector to count how many points in a cluster
    std::vector<int> pointsInCluster(clusters, 0);

    for (int p = 0; p < n; p++) {
      // Get the right cluster for each point
      int cluster = assignments[p];

      for (int i = 0; i < d; i++){
        //Sums the points belonging to a cluster
        newSolution[cluster * d + i] += data[p * d + i];
      }

      pointsInCluster[cluster] += 1;
    }

    // Divide sums by number of points to get new centroids.
    for (int cluster = 0; cluster < clusters; cluster++) {
      // Turn 0/0 into 0/1 to avoid zero division.
      int count = std::max<int>(1, pointsInCluster[cluster]);

      for (int i = 0; i < d; i++){
          solution[cluster * d + i] = newSolution[cluster * d + i]/count;
      }
    }



    // END OF STEP 2
    

    // START OF STEP 3) Update assignment of samples to centers



    // Find assignments.

    std::random_shuffle(randomizer.begin(), randomizer.end());
    std::vector<int> oldAssignments(n);
    oldAssignments = assignments;


    for (int k = 0; k < n; k++) {

      int p = randomizer[k];
      double bestDistance = std::numeric_limits<double>::max();;
      int bestCluster = 0;

      for (int cluster = 0; cluster < clusters; cluster++) {
        // For each point, calculate the nearest cluster


        // - For each sample, in random order, calculate the minimum cost of change of assignment from its current center to 
        //another (considering the other samples as well assigned and the penalties associated to them in case we violate must-link 
        //or cannot-link constraints), if the difference of cost is smaller than 0, apply the change.

        double newDistance = pointToPointDistance(data, solution, p, cluster, d);    


        for(int i = 0; i < constraints->mlAdjacencyList[p].size(); i++){
          int j = constraints->mlAdjacencyList[p][i];
          if(cluster != assignments[j]){
            newDistance += constraints->mustLinks[p][j];
          }
        }

        for(int i = 0; i < constraints->clAdjacencyList[p].size(); i++){
          int j = constraints->clAdjacencyList[p][i];
          if(cluster == assignments[j]){
              newDistance += constraints->cannotLinks[p][j];
          }
        }

        
        if (newDistance < bestDistance){
          // If the distance is better, updates the best cluster and distance 
          bestDistance = newDistance;
          bestCluster = cluster;
        }
      }
      assignments[p] = bestCluster;
      bestDistances[p] = bestDistance;
    }


    // ##TODO: -  In addition, calculate the connected components using BFS relative to the must-link constraints. And for each connected component,
    // calculate the cost of relocating all the samples of the component to a different cluster. If the difference of cost is smaller than 0, apply the change


    for(int i = 0; i < constraints->connectedComponents.size(); i++){
      for(int k = 0; k < clusters; k++){
        double currentDistance = 0;
        double newDistance = 0;

        for(int j = 0; j < constraints->connectedComponents[i].size(); j++){
          int p = constraints->connectedComponents[i][j];
          newDistance += pointToPointDistance(data, solution, p, k, d);
          currentDistance += pointToPointDistance(data, solution, p, assignments[p], d);
          for(int i = 0; i < constraints->clAdjacencyList[p].size(); i++){
            int j = constraints->clAdjacencyList[p][i];
            if(k == assignments[j]){
              newDistance += constraints->cannotLinks[p][j];
            }
            if(assignments[p] == assignments[j]){
              currentDistance += constraints->cannotLinks[p][j];
            }
          }
        }

        if (newDistance < currentDistance){
          for(int j = 0; j < constraints->connectedComponents[i].size(); j++){
            assignments[constraints->connectedComponents[i][j]] = k;
          }
        }
      }
    }

    // If there was no change, stop 
    if(oldAssignments == assignments){
    //cout << "There were " << iteration << " iterations." << endl;
      break;

    }

    // END OF STEP 3


  }

  Solution* s = new Solution(solution, assignments);
  return s;
}


int main(int argc, char** argv){

  //Get the data file name
  string fileName = argv[1];

  //Get the solution file name
  string solutionName = argv[2];

  //Create a variable to hold the constraints amount multiplying factor
  //Possible choices are 0.1, 0.05 and 0.01
  double cAmountMultiplier = atof(argv[3]);

  //Create a variable to hold the constraints weight multiplying factor
  //possible choices are 0.1, 1, 10
  double cWeightMultiplier = atof(argv[4]);
  
  //Amount of KmeansHB calls in each loop
  int runs = atoi(argv[5]);

  //The amount of loops
  int cruns = atoi(argv[6]);

  // Open the data file
  ifstream datasetFile(fileName);
    
  // Read the parameters
  int n, d;
  datasetFile >> n;
  datasetFile >> d;

    // Allocate storage
  Dataset *x = new Dataset(n, d);
    
    // Read the data values directly into the dataset
  for (int i = 0; i < n * d; ++i) {
      datasetFile >> x->data[i];
  }

  // Open a file with a good solution
  ifstream solutionFile(solutionName);

  //Read the parameters
  int clusters;
  double solutionCost;
  double f_best;
  solutionFile >> clusters;
  solutionFile >> solutionCost;
  solutionFile >> f_best;

  std::vector<int> solutionAssignments(n);

  // Read the assignments
  for(int i = 0; i < n; i++){
      solutionFile >> solutionAssignments[i];
  }

  //Decides the amount of constraints based on number of points in dataset
  int constraintsAmount = floor(n * cAmountMultiplier);

  //Decides the weight of each constraint 
  int constraintsWeight = (solutionCost/constraintsAmount) * cWeightMultiplier;

  //Create constraints based on the solution

  Constraints *constraints = new Constraints(n);


  // using built-in random generator to avoid skeweing results towards first points
  
  int currentAmount = 0;
  while(currentAmount < constraintsAmount){

    static std::random_device seed;
    static std::mt19937 rng(seed());
    std::uniform_int_distribution<int> points(0, n - 1);

    int selected = points(rng);
    int p = -1;
    do{p = points(rng);}while(p == selected);

    if(solutionAssignments[p] == solutionAssignments[selected]){
        constraints->addMustLink(p, selected, constraintsWeight);
    }else{
        constraints->addCannotLink(p, selected, constraintsWeight);
    }
    currentAmount++;
  }
  


  constraints->fixConstraints();
  constraints->printConstraints();
  constraints->checkIntegrity();



  double totalGap = 0.0;
  double totalTime = 0.0;
  double totalCrand = 0.0;
  int counter = 0;

  fileName = replaceString(fileName, "dataset/", "out/");
  fileName = replaceString(fileName, ".txt", "");

  std::ofstream pairfile (fileName + "_" + to_string(cAmountMultiplier) + "_" + to_string(cWeightMultiplier) + "_" + to_string(clusters) + "_PairTest.txt");


  while(counter < cruns){
    
    double best = std::numeric_limits<double>::max();; 

    clock_t begin = clock();

    Solution* bestSol;
    for(int i = 0; i < runs; i++){
      Solution* sol = kmeans(x, clusters, 100, n, d, constraints);
      double solutionCost = getCostWithoutConstraints(x, sol->solution, sol->assignments);
      double g = 100 * (solutionCost - f_best) / f_best;
      double c = getCrand(solutionAssignments, sol->assignments, n);

      pairfile << g << " ";
      //pairfile << c << " ";
      if (solutionCost < best){
         best = solutionCost;
         bestSol = sol;
      }
    }

    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

    double gap = 100 * (best - f_best) / f_best;
    double crand = getCrand(solutionAssignments, bestSol->assignments, n);


    
    counter++;
    totalTime += elapsed_secs;
    totalGap += gap;
    totalCrand += crand;

  }

  pairfile.close();

}

