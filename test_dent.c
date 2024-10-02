#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

void test_basic_functionality();
void test_specify_starting_directory();
void test_limit_max_concurrent_threads();
void test_invalid_max_concurrent_threads();
void test_deep_directory_structure();
void test_large_number_of_files();
void test_permission_denied();
void test_non_existent_directory();
void test_symbolic_links();
void test_circular_symbolic_links();

char* execute_dent(const char* args);

void compare_output(const char* actual_output, const char* expected_output);

int main() {
    printf("Starting Tests\n");

    test_basic_functionality();
    test_specify_starting_directory();
    test_limit_max_concurrent_threads();
    test_invalid_max_concurrent_threads();
    test_deep_directory_structure();
    test_large_number_of_files();
    test_permission_denied();
    test_non_existent_directory();
    test_symbolic_links();
    test_circular_symbolic_links();


    printf("All Tests Completed Successfully\n");
    return 0;
}


void test_basic_functionality() {
    printf("Test 1: Basic Functionality Test\n");
    char* output = execute_dent("");
    assert(output != NULL);
    free(output);
    printf("Test 1 Passed\n");
}

void test_specify_starting_directory() {
    printf("Test 2: Specify Starting Directory\n");
    system("mkdir -p test_dir2");
    char* output = execute_dent(" test_dir2");
    assert(output != NULL);
    assert(strstr(output, "test_dir2") != NULL);
    free(output);
    system("rm -r test_dir2");
    printf("Test 2 Passed\n");
}

void test_limit_max_concurrent_threads() {
    printf("Test 3: Limit Max Concurrent Threads\n");
    char* output = execute_dent(" . 10");
    assert(output != NULL);
    free(output);
    printf("Test 3 Passed\n");
}

void test_invalid_max_concurrent_threads() {
    printf("Test 4: Invalid Max Concurrent Threads\n");
    char* output_zero = execute_dent(" . 0");
    assert(strstr(output_zero, "Invalid value for max_concurrent_threads"));
    char* output_negative = execute_dent(" . -5");
    assert(strstr(output_negative, "Invalid value for max_concurrent_threads"));
    char* output_nonint = execute_dent(" . abc");
    assert(strstr(output_nonint, "Invalid value for max_concurrent_threads"));
    printf("Test 4 Passed\n");
}

void test_deep_directory_structure() {
    printf("Test 5: Deep Directory Structure\n");
    system("mkdir -p test_dir/depth1/depth2/depth3/depth4/depth5");
    char* output = execute_dent(" test_dir");
    assert(output != NULL);
    assert(strstr(output, "test_dir/depth1/depth2/depth3/depth4/depth5") != NULL);
    free(output);
    system("rm -r test_dir");
    printf("Test 5 Passed\n");
}

void test_large_number_of_files() {
    printf("Test 6: Large Number of Files\n");
    system("mkdir test_large");
    system("bash -c 'for i in {1..100}; do touch test_large/file_$i.txt; done'");
    char* output = execute_dent(" test_large");
    assert(output != NULL);
    assert(strstr(output, "test_large/file_1.txt") != NULL);
    assert(strstr(output, "test_large/file_100.txt") != NULL);
    free(output);
    system("rm -r test_large");
    printf("Test 6 Passed\n");
}

void test_permission_denied() {
    printf("Test 7: Permission Denied Test\n");
    system("mkdir test_no_permission");
    system("chmod 000 test_no_permission");
    char* output = execute_dent(" test_no_permission");
    assert(output != NULL);
    assert(strstr(output, "opendir: Permission denied") != NULL);
    free(output);
    system("chmod 755 test_no_permission");
    system("rm -r test_no_permission");
    printf("Test 7 Passed\n");
}

void test_non_existent_directory() {
    printf("Test 8: Non-existent Directory\n");
    char* output = execute_dent(" non_existent_directory");
    assert(output != NULL);
    assert(strstr(output, "opendir: No such file or directory") != NULL);
    free(output);
    printf("Test 8 Passed\n");
}

void test_symbolic_links() {
    printf("Test 9: Symbolic Links Test\n");
    system("mkdir test_symlinks");
    system("touch test_symlinks/real_file.txt");
    system("ln -s real_file.txt test_symlinks/symlink_to_file");
    char* output = execute_dent(" test_symlinks");
    assert(output != NULL);
    assert(strstr(output, "test_symlinks/real_file.txt") != NULL);
    assert(strstr(output, "test_symlinks/symlink_to_file") != NULL);
    free(output);
    system("rm -r test_symlinks");
    printf("Test 9 Passed\n");
}

void test_circular_symbolic_links() {
    printf("Test 10: Circular Symbolic Links Test\n");
    system("mkdir test_circular");
    system("ln -s ../test_circular test_circular/loop");
    char* output = execute_dent(" test_circular");
    assert(output != NULL);
    free(output);
    system("rm -r test_circular");
    printf("Test 10 Passed\n");
}


char* execute_dent(const char* args) {
    char command[256];
    snprintf(command, sizeof(command), "./dent%s 2>&1", args);

    FILE* fp = popen(command, "r");
    if (fp == NULL) {
        perror("Failed to run dent");
        return NULL;
    }

    size_t size = 0;
    size_t capacity = 4096;
    char* output = malloc(capacity);
    if (!output) {
        perror("Malloc failed");
        pclose(fp);
        return NULL;
    }

    size_t len;
    while ((len = fread(output + size, 1, capacity - size - 1, fp)) > 0) {
        size += len;
        if (size >= capacity - 1) {
            capacity *= 2;
            char* temp = realloc(output, capacity);
            if (!temp) {
                perror("Realloc failed");
                free(output);
                pclose(fp);
                return NULL;
            }
            output = temp;
        }
    }
    output[size] = '\0';

    int ret = pclose(fp);
    if (ret != 0) {


    }

    return output;
}

void compare_output(const char* actual_output, const char* expected_output) {
    assert(strcmp(actual_output, expected_output) == 0);
}