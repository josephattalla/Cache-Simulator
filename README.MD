# HW8 Cache Simulator

This project is a write-through, no write allocate and write-back, write allocate cache simulator. The cache is dynamically allocated to support different block sizes, number of sets, and associativity.

## Input

Input is through stdin and should be in the format:

```txt
<block size>
<number of sets>
<associativity>
R/W<tab>address
...
R/W<tab>address
```

**Sample Input**

```txt
16
64
4
W 300
R 304
R 4404
W 4408
W 8496
R 8500
R 304
```
- addresses are tested to handle a maximum number of 1073741824
- addresses should not be below 0

**Sample Output**
```txt
Block size: 16
Number of sets: 64
Associativity: 4
Number of offset bits: 4
Number of index bits: 6
Number of tag bits: 22
****************************************
Write-through with No Write Allocate
****************************************
Total number of references: 7
Hits: 2
Misses: 5
Memory References: 6
****************************************
Write-back with Write Allocate
****************************************
Total number of references: 7
Hits: 3
Misses: 4
Memory References: 4
```

Sample inputs are provided as `.txt` files

## Known Problems

There are no known problems through the testing I have done. I encountered a problem with the implementation of the address parsing, the max address required to handle did not require an unsigned int, but I used right shift to get the tag which resulted in negative integers, which was fixed by using an unsigned int for the address.