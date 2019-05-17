# ELEN4020-Transposition of Big Data

## Link to repo
git@github.com:SboneloMdluli/ELEN4020-Project.git
https://github.com/SboneloMdluli/ELEN4020-Project
```
The program was run using hornet in ubuntu 18.02
```

### Installing MPICH3.3
https://www.mpich.org/static/downloads/3.2.1/mpich-3.2.1-installguide.pdf

In NetBeans right click on the project file -----> Properties ------> libraries -----> add JAR

The following commands are used to run the prgrams in Message Passing Interface (MPI)

```
mpicc -o file trans.c -lm
mpiexec -n p ./file

```
