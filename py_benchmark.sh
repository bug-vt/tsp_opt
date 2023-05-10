
RUN=10

for (( i=5; i<=$RUN; i++ ))
do
  ./tsp.py 0 $i 
done
