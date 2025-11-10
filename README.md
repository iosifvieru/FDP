# FDP

For C we use OpenMPI 4.1.6

The official documentation for OpenMPI can be found [here](https://www-lb.open-mpi.org/doc/v4.1/).

### Usage

    // compile c file
    mpicc <filename> -Wall -o <name>

    // run c program
    mpirun -np <number> file.out --oversubscribe