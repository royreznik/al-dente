<p align="center">
     <a><img src="./assets/logo.webp" alt="Al-Dente"></a>
</p>

# Al-Dente - An extremely fast directory listing 

Faster then most of the other cli tools like `fd` or `find` by more than triple!

Have you ever had a really large file system and tried to list all the files in it? So now you can do it quickly!

This program basically a clone of my co-worker [@yehonatanz](https://github.com/yehonatanz) code, given into chatgpt in order to multithreading it.

This project implements a multithreaded directory listing program in C using the `getdent64` syscall. 

## Why the weird name?
As my co-worker called this program `dent` as for the syscall name, i thought it will be funny to give it a name with a food reference.

As written at wiki:
```
In cooking, al dente pasta or rice is cooked to be firm to the bite.
```

Also when writing it with a lower-cased `L` it seems like AI-Dent-e, which i used chatgpt to write the code for this project.

## Benchmark
We are going to compare dent with `find` and `fdfind`.

Let's create a `benchmark_scenario` folder that contains 765200 file entries (including folders in different hierarchies). this can be achived by the `benchmark_scenario.c`.

Let's test the listing itself:
```shell
hyperfine --warmup 3 'find benchmark_scenario/' 'fdfind . benchmark_scenario/' './dent benchmark_scenario/'
Benchmark 1: find benchmark_scenario/
  Time (mean ± σ):      3.203 s ±  0.056 s    [User: 0.727 s, System: 2.403 s]
  Range (min … max):    3.132 s …  3.284 s    10 runs
 
Benchmark 2: fdfind . benchmark_scenario/
  Time (mean ± σ):      2.943 s ±  0.072 s    [User: 0.449 s, System: 9.918 s]
  Range (min … max):    2.805 s …  3.056 s    10 runs
 
Benchmark 3: ./dent benchmark_scenario/
  Time (mean ± σ):     797.3 ms ±  19.0 ms    [User: 595.3 ms, System: 2293.2 ms]
  Range (min … max):   774.1 ms … 838.9 ms    10 runs
 
Summary
  './dent benchmark_scenario/' ran
    3.69 ± 0.13 times faster than 'fdfind . benchmark_scenario/'
    4.02 ± 0.12 times faster than 'find benchmark_scenario/'
```

Now combined with grep you can achieve regex searching faster!
```shell
hyperfine --warmup 3 'find benchmark_scenario/ -iregex .*file_25.txt' 'fdfind -u .*file_25.txt benchmark_scenario/' './dent benchmark_scenario/ | grep -P .*file_25.txt'
Benchmark 1: find benchmark_scenario/ -iregex .*file_25.txt
  Time (mean ± σ):      4.379 s ±  0.050 s    [User: 1.742 s, System: 2.554 s]
  Range (min … max):    4.259 s …  4.442 s    10 runs
 
Benchmark 2: fdfind -u .*file_25.txt benchmark_scenario/
  Time (mean ± σ):      1.244 s ±  0.040 s    [User: 0.167 s, System: 4.007 s]
  Range (min … max):    1.202 s …  1.327 s    10 runs
 
Benchmark 3: ./dent benchmark_scenario/ | grep -P .*file_25.txt
  Time (mean ± σ):      1.001 s ±  0.057 s    [User: 0.453 s, System: 2.609 s]
  Range (min … max):    0.955 s …  1.153 s    10 runs
 
Summary
  './dent benchmark_scenario/ | grep -P .*file_25.txt' ran
    1.24 ± 0.08 times faster than 'fdfind -u .*file_25.txt benchmark_scenario/'
    4.38 ± 0.25 times faster than 'find benchmark_scenario/ -iregex .*file_25.txt'
```

## Compilation

To compile the program, run:

```bash
make
```
This will produce an executable named dirlist.

## Usage
Run the program with:

```bash
./dent [starting_directory] [num_of_theards]
```

## Clean Up
To clean up the compiled object files and the executable, run:

```bash
make clean
```
Notes
The program uses POSIX threads (`pthread`) and semaphores (`sem_t`) to manage concurrency.
It uses the `getdents64` system call to read directory entries directly.
The maximum number of concurrent threads is limited by the `MAX_CONCURRENT_THREADS` macro defined in dirlist.h (default is 1000). You can adjust this value as needed.

## License
This project is provided as-is without any warranty.
