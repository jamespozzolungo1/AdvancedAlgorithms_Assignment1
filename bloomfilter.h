
#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

// bloom filters store only m bits and never  the items themselves.
// the invariant is that bits only ever go from 0 to 1 and nothing sets a bit back t 0, so an item that was added will always 
// be found, meaning no false positives

class BloomFilter {
    public:
    BloomFilter(uint64_t m, int k) {
        // m = number of bits, k = number of hash functions (positions per item)
        if (m == 0) {
            throw std::invalid_argument("Number of bits must be atleast 1");
        }
        if (k <= 0) {
            throw std::invalid_argument("Number of hash functions must be atleast 1");
        }

        this -> m = m;
        this -> k = k;
        
        // std::vector<bool> packs the bits 8 per bite with every bit starting at 0
        bits.assign(m, false);
    }

        // hash item to k bit positions and set them to 1, only place bits are written; keeping the invariant
        void add(const std::string& item) {
            std::vector<uint64_t> pos = positions(item);
            for (int i = 0; i < (int)pos.size(); i++) {
                bits[pos[i]] = true;
            }
        }

        // if any of hte item k's bits is 0 = definently not added, if all bits = 1 probably added (possible false positive)
        bool mightContain(const std::string& item) const {
            std::vector<uint64_t> pos = positions(item);
            for (int i = 0; i < (int)pos.size(); i++) {
                if (bits[pos[i]] == false) {
                    return false;
                }
            }
            return true;
        }
        
        // counts how many bits are 1 (used by the tests to check the invariant)
        uint64_t bitsSet() const {
            uint64_t count = 0;
            for (uint64_t i = 0; i < m; i++) {
                if (bits[i]) {
                    count++;
                }
            }
            return count;
        }

        // works out k bit positions for an item using double hash
        std::vector<uint64_t> positions(const std::string& item) const {
            uint64_t h1 = mix(hashString(item));
            uint64_t h2 = mix(h1);

            h1 = h1 % m;
            h2 = h2 % m;
            
            // if h2 = 0, every position would be h1, however this doesnt stop repeats when h2 shares a factor with m
            if (h2 == 0) {
                h2 = 1;
            }

            std::vector<uint64_t> pos;
            for (int i = 0; i < k; i++) {
                pos.push_back((h1 + (uint64_t)i * h2) % m);
            }
            return pos;
        }
    

    private: 
        // private so outside code doesnt alter bits directly
        uint64_t m;
        int k;
        std::vector<bool> bits;
        
        // FNV-1a  string hash (64bit)
        static uint64_t hashString(const std::string& s) {
            uint64_t h = 14695981039346656037ULL;
            
            for (int i = 0; i < (int)s.size(); i++) {
                h = h ^ (unsigned char)s[i];
                h = h * 1099511628211ULL;
            }
            return h;
        }

        // scrambles bits so similar strings get diff hashes
        static uint64_t mix(uint64_t x) {
            x = x + 0x9E3779B97F4A7C15ULL;
            x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
            x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
            x = (x ^ (x >> 31));
            return x;
        }
};

#endif