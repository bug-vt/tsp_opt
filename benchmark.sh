
RUN=25

for (( i=8; i<=$RUN; i++ ))
do
  ./tsp.py $i 
done
