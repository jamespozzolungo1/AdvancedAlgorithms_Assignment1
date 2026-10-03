# AdvancedAlgorithms_Assignment1

Video walkthrough: https://youtu.be/FtlIDkTAyPs
Report: [14502850_Report_41052.pdf](14502850_Report_41052.pdf)


Bloom filter: Empirical Study (Track A)

A bloom filter which has been implemented in C++, including experiments comparing its measured false positive rate against the theoretical formula, and its memory and speed against std::unordered_set. Plotting for all data is done through python.


Files include:

bloomfilter.h - The bloom filter itself
testbloom.cpp - Tests for the filter, including no false negatives, false positive rate, invariant, and 3 edge case tests
experiments.cpp - 4 experiments that writes CSVs to results/
plot.py - Reads the CSVs and draws pllots into plots/
results/ - CSV outputs from running experiments.cpp
plots/ - PNG plots after running plot.py


Requirements:

A C++17 compiler (g++ 9 or later)
Python 3 with matplotlib (for running plots): pip install -r requirements.txt

How to build and run:

Run testbloom.cpp:

g++ -std=c++17 -O2 -o testbloom testbloom.cpp
./testbloom

Run experiments.cpp:

g++ -std=c++17 -O2 -o experiments experiments.cpp
./experiments
python plot.py

On windows, run .\testbloom.exe and .\experiments.exe instead of ./testbloom ./experiments. If a program provides no output on windows, add -static to the g++ command.


Experiments:

Experiment 1: m = 10,000 bits, k = 7, n from 100 to 2,000.
Output: results/vary_n.csv, plots/vary_n.png

Experiment 2: m = 10,000 bits, n = 1,000, k from 1 to 15.
Output: results/vary_k.csv, plots/vary_k.png

Experiment 3: 10,000 (composite), vs m 10,007 (prime), n = 1000, k from 1 to 15, averaged over 5 trials. Testst whether double hashing repeating positions affects the false positive rate.
Output: results/prime_m.csv, plots/prime_m.png

Experiment 4 (baseline): std::unordered_set<std::string> vs the bloom filter sized for 1% false positive rate, for n = 1,000 to 1,000,000. Compoares memory, insert and lookup time.
Output: results/baseline.csv, plots/baseline_memory.png, plots/baseline_lookup.png

The false positive rate is the fraction of those that the filter wrongfully reports as "probably present". This is compared against the theory: 
p = (1 - e^ (-kn / m))^k

Memory of the unordered_set is measured by counting every heap allocation (a replaced operator new in experiments.cpp). Memory for the bloom filter is the object size plus the packed bbit array (m / 8 bytes).


