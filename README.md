# Overview

Pablo Lomeli CS453 Fall 2020

In this program a set array size will be sorted by single or multiple threads. Timing will be provided to compare the performance difference between single and multiple threads.

## Make Concurrent (ABET outcome 1)

I have designed my program to be based on how many levels are inputted to decide how many threads to use when sorting. 

To use the program use this usage guide: 
mytests <input size> [<seed>] [<number of lvls>] [<enable helper printf()>] [<enable efficiency data mode>]

The arguments after "input size" are optional and have default values. For instance, by default
the helper console print methods and dataMode are disabled.

## Implementation Analysis (ABET outcome 6)

Create a file named speedup.pdf (in the root directory) that analyses
the following two aspects of your mergesort solution.

**Efficiency** - For efficiency testing, I used levels 1, 2, 3, 4, and 5 to sort 100 million elements and compared how much faster it was than a single thread. More information is provided by "data analysis efficiency.pdf"

**Effectiveness** - For effectiveness testing, I used level 3, so only 4 threads, and did a 100 single threaded sorts and 100 threaded sorts. More information is provided by "data analysis effectiveness.pdf". Data used to create the pdf is "data.txt".

