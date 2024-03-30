# BranchPredictor

This project is built base on https://github.com/prannoy/CSE240A.

## Introduction

We implemented two branch prediction method: Gshare and Tournament. And implemented a custom branch predictor based on Perceptrons to beat these two methods.

## Get Started

First follow the guidance in https://github.com/prannoy/CSE240A to start working with Docker.

To run the branch predictor, execute the following command under the `src` folder.
```
make
```

Here are some examples to run Gshare, Tournament and Custom (based on Perceptrons). You can modify the traces used and the parameters.
```
bunzip2 -kc "../traces/fp_1.bz2" | ./predictor --gshare:13
bunzip2 -kc "../traces/fp_1.bz2" | ./predictor --tournament:9:10:10
bunzip2 -kc "../traces/fp_1.bz2" | ./predictor --custom:13:9
```

## Reference
https://course.ece.cmu.edu/~ece740/f15/lib/exe/fetch.php?media=18-740-fall15-lecture05-branch-prediction-afterlecture.pdf

https://www.cs.utexas.edu/~lin/papers/hpca01.pdf