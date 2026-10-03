import csv
import math
import os
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

# reads the csv files in experiments.cpp and draws the plots 

#returns the csv as a list of rows
def read_csv(filename):
    rows = []
    f = open(filename)
    reader = csv.DictReader(f)
    for row in reader:
        values = {}
        for key in row:
            values[key] = float(row[key])
        rows.append(values)
    f.close()
    return rows

# pull one column out of the rows as a list
def column(rows, name): 
    values = []
    for i in range(len(rows)):
        values.append(rows[i][name])
    return values

#plot of experiment 1
def plot_vary_n(): 
    rows = read_csv("results/vary_n.csv")
    plt.figure()
    plt.plot(column(rows, "n"), column(rows, "measured"), "o", label = "measured")
    plt.plot(column(rows, "n"), column(rows, "theory"), "-", label = "theory")
    plt.xlabel("items inserted (n)")
    plt.ylabel("false positive rate")
    plt.title("False positive rate vs n (m = 10000, k = 7)")
    plt.legend()
    plt.grid(True)
    plt.savefig("plots/vary_n.png", dpi=150)
    plt.close()

#plot of experiment 2
def plot_vary_k():
    rows = read_csv("results/vary_k.csv")
    best_k = (10000 / 1000) * math.log(2)
    plt.figure()
    plt.plot(column(rows, "k"), column(rows, "measured"), "o", label = "measured")
    plt.plot(column(rows, "k"), column(rows, "theory"), "-", label = "theory")
    plt.axvline(best_k, linestyle = "--", color = "gray", label = "theoretical best k")
    plt.xlabel("number of hash functions (k)")
    plt.ylabel("false positive rate")
    plt.title("False positive rate vs k (m = 10000, n = 1000)")
    plt.legend()
    plt.grid(True)
    plt.savefig("plots/vary_k.png", dpi=150)
    plt.close()

#plot of experiment 3
def plot_prime_m():
    rows = read_csv("results/prime_m.csv")
    k = column(rows, "k")
    plt.figure()
    plt.plot(k, column(rows, "composite_m_10000"), "o", label = "measured m = 10000 (composite)")
    plt.plot(k, column(rows, "prime_m_10007"), "s", label = "measured, m = 10007 (prime)")
    plt.plot(k, column(rows, "theory"), "-", label = "theory")
    plt.xlabel("number of hash functions (k)")
    plt.ylabel("false positive rate (average of 5 trials)")
    plt.title("Composite vs prime m (n = 1000)")
    plt.legend()
    plt.grid(True)
    plt.savefig("plots/prime_m.png", dpi=150)
    plt.close()

#plot of experiment 4
def plot_baseline():
    rows = read_csv("results/baseline.csv")
    labels = []
    for i in range(len(rows)):
        labels.append(str(int(rows[i]["n"])))
    positions = []
    left = []
    right = []
    width = 0.38
    for i in range(len(rows)):
        positions.append(i)
        left.append(i - width / 2)
        right.append(i + width / 2)

    # memory

    plt.figure()
    plt.bar(left, column(rows, "set_bytes"), width, label="std::unordered_set (baseline)")
    plt.bar(right, column(rows, "bloom_bytes"), width, label = "Bloom filter")
    plt.xticks(positions, labels)
    plt.yscale("log")
    plt.xlabel("items stored (n)")
    plt.ylabel("memory (bytes, log scale)")
    plt.title("Memory use (bloom filter sized for 1 percent false positives)")
    plt.legend()
    plt.grid(True, axis = "y")
    plt.savefig("plots/baseline_memory.png", dpi = 150)
    plt.close()

    # lookup time (20k look ups of items that were never inserted)

    plt.figure()
    plt.bar(left, column(rows, "set_lookup_ms"), width, label = "std::unordered_set (baseline)")
    plt.bar(right, column(rows, "bloom_lookup_ms"), width, label = "Bloom filter")
    plt.xticks(positions, labels)
    plt.xlabel("items sorted (n)")
    plt.ylabel("time for 20,000 lookups (ms)")
    plt.title("Lookup time")
    plt.legend()
    plt.grid(True, axis = "y")
    plt.savefig("plots/baseline_lookup.png", dpi = 150)
    plt.close()

if __name__ == "__main__":
    os.makedirs("plots", exist_ok = True)
    plot_vary_n()
    plot_vary_k()
    plot_prime_m()
    plot_baseline()
    print("Plots saved in plots/")