<p align="center">
     <a><img src="./assets/logo.webp" alt="Al-Dente"></a>
</p>

# Al-Dente - An extremely fast directory listing 

Faster than most of the other cli tools like `fd` or `find` by more than triple!

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
hyperfine --warmup 3 'find benchmark_scenario/' 'fdfind -HI . benchmark_scenario/' 'dent benchmark_scenario/'
Benchmark 1: find benchmark_scenario/
  Time (mean ± σ):      3.178 s ±  0.037 s    [User: 0.727 s, System: 2.435 s]
  Range (min … max):    3.093 s …  3.229 s    10 runs
 
Benchmark 2: fdfind -HI . benchmark_scenario/
  Time (mean ± σ):      1.633 s ±  0.034 s    [User: 0.264 s, System: 5.336 s]
  Range (min … max):    1.581 s …  1.686 s    10 runs
 
Benchmark 3: dent benchmark_scenario/
  Time (mean ± σ):     563.9 ms ±  14.8 ms    [User: 226.4 ms, System: 1815.4 ms]
  Range (min … max):   550.7 ms … 597.7 ms    10 runs
 
Summary
  'dent benchmark_scenario/' ran
    2.90 ± 0.10 times faster than 'fdfind -HI . benchmark_scenario/'
    5.64 ± 0.16 times faster than 'find benchmark_scenario/'
```

Now combined with grep you can achieve regex searching faster!
```shell
hyperfine --warmup 3 'find benchmark_scenario/ -iregex .*file_25.txt' 'fdfind -HI -u .*file_25.txt benchmark_scenario/' 'dent benchmark_scenario/ | grep -aP .*file_25.txt'
Benchmark 1: find benchmark_scenario/ -iregex .*file_25.txt
  Time (mean ± σ):      4.231 s ±  0.080 s    [User: 1.732 s, System: 2.500 s]
  Range (min … max):    4.129 s …  4.399 s    10 runs
 
Benchmark 2: fdfind -HI -u .*file_25.txt benchmark_scenario/
  Time (mean ± σ):      1.127 s ±  0.030 s    [User: 0.197 s, System: 3.751 s]
  Range (min … max):    1.104 s …  1.207 s    10 runs
 
Benchmark 3: dent benchmark_scenario/ | grep -aP .*file_25.txt
  Time (mean ± σ):     703.9 ms ±  32.5 ms    [User: 197.8 ms, System: 1895.0 ms]
  Range (min … max):   657.1 ms … 768.2 ms    10 runs
 
Summary
  'dent benchmark_scenario/ | grep -aP .*file_25.txt' ran
    1.60 ± 0.09 times faster than 'fdfind -HI -u .*file_25.txt benchmark_scenario/'
    6.01 ± 0.30 times faster than 'find benchmark_scenario/ -iregex .*file_25.txt'
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
