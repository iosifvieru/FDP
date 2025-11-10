# FDP - Fundamentals of Distributed Processing

For C we use OpenMPI 4.1.6

The official documentation for OpenMPI can be found [here](https://www-lb.open-mpi.org/doc/v4.1/).

### Usage

    // compile c file
    mpicc <filename> -Wall -o <name>

    // run openmpi program
    mpirun -np <number> --oversubscribe file.out <program args>