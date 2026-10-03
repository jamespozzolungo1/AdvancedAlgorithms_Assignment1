#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <new>
#include <string>
#include <unordered_set>
#include <vector>
#include "bloomfilter.h"

// does the false-positive rate measured in these experiments match the theory, and how does the bloom filter compare
// to std::unordered_set

// theory assumes hash positoins are random and independent: 
// false positive rate p = (1 - e^(-k*n/m))^k
// best k = (m / n) * ln 2

// how many never inserted items to check per data point
const int NUM_TESTS = 20000;
// memory counting for the baseline
static size_t g_bytesAllocated = 0;

void* operator new(size_t size) {
    g_bytesAllocated += size;
    void* p = std::malloc(size);
    if (p == nullptr) {
        throw std::bad_alloc();
    }
    return p;
}

// matching deleetes so memory from operator new is freed properly
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, size_t) noexcept { std::free(p); }

// textbook false positive rate 
double theoryRate(double m, double n, double k) {
    return std::pow(1.0 - std::exp(-k * n / m), k);
}

// builds a filter, inserts n items, then checks NUM_TESTS items that were never inserted, any probably yes is a false positive. 
// trial changes the item names so repeated trials use fresh data
double measureRate(uint64_t m, int n, int k, int trial) {
    BloomFilter filter(m, k);
    std::string prefix = std::to_string(trial) + "_";
    for (int i = 0; i < n; i++) {
        filter.add("item_" + prefix + std::to_string(i));
    }

    int falsePositives = 0;
    for (int i = 0; i < NUM_TESTS ; i++) { 
        if (filter.mightContain("test_" + prefix + std::to_string(i))) {
            falsePositives++;
        }
    }
    return (double)falsePositives / NUM_TESTS;
}

// exp1: fix m and k, vary n (number of items inserted). as more items are added, more bits are 1, so rate should rise
void experimentVaryN() {
    uint64_t m = 10000;
    int k = 7;
    std::cout << "Expreiment 1: m = " << m << ", k" << k << ", varying n" << std::endl;
    std::cout << "     n   measured      theory" << std::endl;
    std::ofstream out("results/vary_n.csv");
    out << "n,measured,theory\n";
    for (int n = 100; n <= 2000; n += 100) {
        double meRate = measureRate(m, n, k, 0);
        double thRate = theoryRate(m, n, k);
        out << n << "," << meRate << "," << thRate << "\n";
        printf("%6d  %.5f  %.5f\n", n, meRate, thRate);
    }
}

// exp2: fix m and n, vary k (number of hash functions), too few hashes = easy accidentl matches, too many = array fills up
// theory predicts U- shape with the lowest rate near k = (m/n) ln 2
void experimentVaryK() {
    uint64_t m = 10000;
    int n = 1000;
    double bestK = ((double)m / n) * std::log(2.0);
    std::cout << "\nExperiment 2: m = " << m << ", n = " << n << ", varying k" << std::endl;
    printf("Theory says the best k is about %.2f\n", bestK);
    std::cout << "     k   measured      theory" << std::endl;
    std::ofstream out("results/vary_k.csv");
    out << "k,measured,theory\n";
    for (int k = 1; k <= 15; k++) {
        double meRate = measureRate(m, n, k, 0);
        double thRate = theoryRate(m, n, k);
        out << k << "," << meRate << "," << thRate << "\n";
        printf("%6d  %.5f  %.5f\n", k, meRate, thRate);
    }
}

// exp3: composite m (10000) vs prime m (10007) averaged over 5 trials. with double hashing, if h2 shares a factor with m,
// the positions can repeat, 10000 has many factors, 10007 is prime (no repeats possible).
// Tests whether that pushes the measured rate above theory
void experimentPrimeM() {
    int n = 1000;
    int trials = 5;
    std::cout << "\nExperiment 3: composite m = 10000 vs prime m = 10007, n = " << n << ", " 
    << trials << " trials each" << std::endl;
    std::cout <<"    k  m=10000   m=10007   theory" << std::endl;
    std::ofstream out("results/prime_m.csv");
    out << "k,composite_m_10000,prime_m_10007,theory\n";
    for (int k = 1; k <= 15; k++) {
        double totalC = 0;
        double totalP = 0;
        for (int t = 0; t < trials; t++) {
            totalC += measureRate(10000, n, k, t);
            totalP += measureRate(10007, n, k, t);
        }

        double avgC = totalC / trials;
        double avgP = totalP / trials;
        double thRate = theoryRate(10000, n, k);
        out << k << "," << avgC << "," << avgP << "," << thRate << "\n";
        printf("%6d   %.5f   %.5f   %.5f\n", k, avgC, avgP, thRate);
    }
}

// exp4: basline, memory and speed vs std::unordered_set. 
void experimentBaseLine() {
    // size filter for 1% false positive rate
    double targetP = 0.01;
    std::cout << "\nExperiment 5: Bloom filter vs std::unordered_set<std::string>" 
    << " (filter sized for 1 percent false positives) " << std::endl;
    std::cout << "      n   set bytes   bloom bytes   set insert ms   bloom insert ms" 
    << "   set lookup ms   bloom lookupp ms   bloom FP" << std::endl;
    std::ofstream out("results/baseline.csv");
    out << "n,m,k,set_bytes,bloom_bytes,set_insert_ms,bloom_insert_ms," 
    << "set_lookup_ms,bloom_lookup_ms,bloom_fp_rate\n";

    std::vector<int> sizes = {1000, 10000, 100000, 1000000};
    for (int s = 0; s < (int)sizes.size(); s++) {
        int n = sizes[s];
        // standard sizing formulas
        uint64_t m = (uint64_t)std::ceil(-n * std::log(targetP) / (std::log(2.0) * std::log(2.0)));
        int k = (int)std::round(((double)m / n) * std::log(2.0));
        
        // make all strings up front so creating isnt counted in the timings
        std::vector<std::string> items;
        std::vector<std::string> tests;

        for (int i = 0; i < n; i++) {
            items.push_back("item_" + std::to_string(i));
        }
        for (int i = 0; i < NUM_TESTS; i++) {
            tests.push_back("test_" + std::to_string(i));
        }

        // resets the counter so only the sets own allocations are measured
        g_bytesAllocated = 0;
        auto start = std::chrono::steady_clock::now();
        std::unordered_set<std::string> set;

        for (int i =0; i < n; i++) {
            set.insert(items[i]);
        }

        auto end = std::chrono::steady_clock::now();
        size_t setBytes = g_bytesAllocated;
        double setInsertMs = std::chrono::duration<double, std::milli>(end - start).count();

        start = std::chrono::steady_clock::now();
        int setHits = 0;
        for (int i = 0; i < NUM_TESTS; i++) {
            if (set.count(tests[i]) > 0 ) {
                setHits++;
            }
        }
        end = std::chrono::steady_clock::now();
        double setLookupMs = std::chrono::duration<double, std::milli>(end - start).count();

        // BLOOM FILTER
        g_bytesAllocated = 0;
        start = std::chrono::steady_clock::now();
        BloomFilter filter(m, k);

        for (int i = 0; i < n; i++) {
            filter.add(items[i]);
        }

        end = std::chrono::steady_clock::now();
        // counted directly rather than with g_bytesAllocated as add() can inflate count
        size_t bloomBytes = sizeof(BloomFilter) + (m+7) / 8;
        double bloomInsertMs = std::chrono::duration<double, std::milli>(end - start).count();

        start = std::chrono::steady_clock::now();
        int bloomHits = 0;
        
        for (int i = 0; i < NUM_TESTS; i++) {
            if (filter.mightContain(tests[i])) {
                bloomHits++;
            }
        }
        end = std::chrono::steady_clock::now();
        double bloomLookupMs = std::chrono::duration<double, std::milli>(end - start).count();
        // test items were never inserted, so every hit is a false positive (fp)
        double fpRate = (double)bloomHits / NUM_TESTS;
        
        // checks the set stores exact items, meaning it should never find a test item
        if (setHits != 0) {
            std::cout << "Warning: set found " << setHits << " test items " << std::endl;
        }

        out << n << "," << m << "," << k << "," << setBytes << "," << bloomBytes << "," 
        << setInsertMs << "," << bloomInsertMs << "," << setLookupMs << "," << bloomLookupMs << "," << fpRate << "\n";
        printf("%8d    %9zu    %11zu    %13.2f    %15.2f    %13.2f    %15.2f    %.4f\n", n, 
        setBytes, bloomBytes, setInsertMs, bloomInsertMs, setLookupMs, bloomLookupMs, fpRate);
 
    }
}

int main() {
    std::filesystem::create_directories("results");
    experimentVaryN();
    experimentVaryK();
    experimentPrimeM();
    experimentBaseLine();
    std::cout << "\nDone, results are in results/. Run python plot.py to make the plots." << std::endl;
    return 0;
}