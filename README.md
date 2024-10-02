<p align="center">
     <a><img src="./assets/logo.webp" alt="Al-Dente"></a>
</p>

# Al-Dente - An extremely fast directory listing 

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
