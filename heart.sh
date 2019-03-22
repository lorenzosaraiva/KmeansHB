amount=0.1 # The number of constraints. This number will be multiplied by the number of points in the dataset - 0.1, 0.05, 0.01, 0
weight=10 # The weight of the constraints. This number will be multiplied by cost/constraints - 10, 1, 0.1
runs=4000 # The number of times KmeansHBWilcoxon will be called in each loop
cruns=1 # The number of loops


./KmeansHBWilcoxon "dataset/liver.txt" "solutions/liver/liver50.txt" 0.1 10 4000 1
./KmeansHBWilcoxon "dataset/liver.txt" "solutions/liver/liver50.txt" 0.05 10 4000 1
./KmeansHBWilcoxon "dataset/liver.txt" "solutions/liver/liver50.txt" 0.01 10 4000 1
./KmeansHBWilcoxon "dataset/liver.txt" "solutions/liver/liver50.txt" 0.1 1 4000 1
./KmeansHBWilcoxon "dataset/liver.txt" "solutions/liver/liver50.txt" 0.05 1 4000 1
./KmeansHBWilcoxon "dataset/liver.txt" "solutions/liver/liver50.txt" 0.01 1 4000 1
./KmeansHBWilcoxon "dataset/liver.txt" "solutions/liver/liver50.txt" 0.1 0.1 4000 1
./KmeansHBWilcoxon "dataset/liver.txt" "solutions/liver/liver50.txt" 0.05 0.1 4000 1
./KmeansHBWilcoxon "dataset/liver.txt" "solutions/liver/liver50.txt" 0.01 0.1 4000 1
./KmeansHBWilcoxon "dataset/liver.txt" "solutions/liver/liver50.txt" 0 0 4000 1