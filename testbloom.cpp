#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "bloomfilter.h"

// test to run for the bloom filter

// prints a  message if a test is failed
void check(bool condition, const std::string& message) {
    if (!condition) {
        std::cout << "Fail: " << message << std::endl;
        std::exit(1);
    }
}

// every item that was added must always be found
void testNoFalseNegatives() {
    BloomFilter filter(10000, 7);
    for (int i = 0; i < 1000; i++) {
        filter.add("item_" + std::to_string(i));
    }
    
    for (int i = 0; i < 1000; i++) {
        check(filter.mightContain("item_" + std::to_string(i)),
        "false negative for item_" + std::to_string(i));
    }
    std::cout << "Pass: no false negatives" << std::endl;
}

// catches broken hashing
void testFalsePositiveRate() {
    BloomFilter filter(10000, 7);
        for (int i = 0; i < 1000; i++) {
            filter.add("item_" + std::to_string(i));
        }
    int falsePositives = 0;
    for (int i = 0; i < 20000; i++) {
        if (filter.mightContain("test_" + std::to_string(i))) {
            falsePositives++;
        }
    }
    double rate = (double)falsePositives / 20000;
    check(rate < 0.02, "false positive rate far too high, possible issue with hashing");
    std::cout << "Pass: false positive rate is reasonable" << std::endl;

}

// the invariant of adding items never turning a 1 back into a 0
void testBitsOnlyGoUp() {
    BloomFilter filter(500, 4); 
    uint64_t previous = filter.bitsSet();
    for (int i = 0; i < 200; i++) {
        filter.add("item_" + std::to_string(i));
        uint64_t now = filter.bitsSet();
        check(now >= previous, "a bit went fro m1 back to 0");
        previous = now;
    }
    std::cout << "Pass: bits only ever go from 0 to 1" << std::endl;
}

// m = 0 or k = 0 should  be rejected with an exception
void testBadArguments() { 
    bool threwForM = false;
    try {
        BloomFilter filter(0, 3);
    } catch (const std::invalid_argument& e) {
        threwForM = true;
    }
    bool threwForK = false;
    try { 
        BloomFilter filter(10, 0);
    } catch (const std::invalid_argument&e) {
                threwForK = true; 
    }
    check(threwForM, "m = 0 should be rejected");
    check(threwForK, "k = 0 should be rejected");
    std::cout << "Pass: rejects m = 0 or k = 0" << std::endl;
    
}

// edge case tests

// an empty filter has all bits at 0 so it must say no to all
void testEmptyFilter() {
    BloomFilter filter(1000, 5);
    for (int i = 0; i < 100; i++) {
        check(!filter.mightContain("test_" + std::to_string(i)), "empty filter said yes");
    }
    std::cout << "Pass: empty filter says  no to everything" << std::endl;
}

// m = 1, after one add the bit is 1, so everything says probably yes
void testTinyFilter() {
    BloomFilter filter (1, 3);
    check(!filter.mightContain("anything"), "empty m = 1 filter said yes");
    filter.add("a");
    check(filter.mightContain("a"), "m = 1 lst the item");
    check(filter.mightContain("never_added"), "m = 1 should give a false positive");
    std::cout << "Pass: tiny filter (m = 1), behaves as expected" << std::endl;
}

// one hash function should still work and set exactly one bit
void testKEqualsOne() {
    BloomFilter filter(100, 1);
    filter.add("hello");
    check(filter.mightContain("hello"), "k = 1 lost the item");
    check(filter.bitsSet() == 1, "k = 1 shouldset exactly one bit");
    std::cout << "Pass: k = 1 works" << std::endl;
}

int main() {
    testNoFalseNegatives();
    testFalsePositiveRate();
    testBitsOnlyGoUp();
    testBadArguments();
    testEmptyFilter();
    testTinyFilter();
    testKEqualsOne();
    std::cout << std::endl << "All tests passes." << std::endl;
    return 0;
}