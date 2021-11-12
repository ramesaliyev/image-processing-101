# Options
output="./output"

# Compile program
gcc main.c -std=c99 -pedantic -Wall -lm -o main

# Create output folder
rm -rf $output
mkdir $output

# Do median filter
for f in *.pgm
do
  ./main median 3 "${f}" "$output/median.${f}"
done

# Do average filter
for f in *.pgm
do
  ./main average 3 "${f}" "$output/average.${f}"
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