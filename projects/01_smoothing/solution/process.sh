# Options
output="./output"

# Compile program
gcc main.c -std=c99 -pedantic -Wall -lm -o main

# Create output folder
rm -rf $output
mkdir $output

# Do filters.
for f in *.pgm
do
  ./main median 3 "${f}" "$output/median.3.${f}"
  ./main median 5 "${f}" "$output/median.5.${f}"
  ./main average 3 "${f}" "$output/average.3.${f}"
  ./main average 5 "${f}" "$output/average.5.${f}"
done

# Copy source pgms.
cp *.pgm $output

# Convert to png
for f in $output/*.pgm
do
  convert "${f}" "${f%.pgm}.png"
done

rm $output/*.pgm

echo "Done"