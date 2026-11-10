*This project has been created as part of the 42 curriculum by aahmamou.*

# Get Next Line

## Description

Get Next Line is a C project that implements a function capable of reading a file descriptor one line at a time.

The goal is to return one line per function call, regardless of its length or the buffer size used for reading.

The function returns:
- The next line, including the newline character (`\n`) when present.
- The last line even if it does not end with a newline.
- `NULL` when there is nothing left to read or when an error occurs.

The returned line is dynamically allocated and must be freed by the caller.

A bonus version can read from several file descriptors in turns without mixing their data.

**Function prototype:**

```c
char *get_next_line(int fd);
```

## Instructions

### Requirements

- A C compiler (such as `cc`).
- A Unix-like operating system.
- [Valgrind](https://valgrind.org/) for memory leak testing (optional).

### Compilation

Compile the project with a custom `BUFFER_SIZE`:

```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 \
get_next_line.c get_next_line_utils.c main.c
```

If `BUFFER_SIZE` is not defined during compilation, its default value is `42`.

You can change the buffer size to test different configurations:

```bash
-D BUFFER_SIZE=1
-D BUFFER_SIZE=42
-D BUFFER_SIZE=1000
-D BUFFER_SIZE=10000000
```

The compiler generates an executable named `a.out` by default.

To compile the bonus version (the bonus files are compiled separately from the mandatory ones):

```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 \
get_next_line_bonus.c get_next_line_utils_bonus.c main.c
```

(`main.c` must then include `get_next_line_bonus.h`.)

### Example main.c

Create a `main.c` file to test the function:

```c
#include "get_next_line.h"
#include <stdio.h>
#include <fcntl.h>

int main(void)
{
    int     fd;
    char    *line;

    fd = open("file.txt", O_RDONLY);
    if (fd < 0)
        return (1);
    line = get_next_line(fd);
    while (line)
    {
        printf("%s", line);
        free(line);
        line = get_next_line(fd);
    }
    close(fd);
    return (0);
}
```

The program opens `file.txt`, retrieves one line at a time, prints it, and frees the allocated memory before requesting the next line.

### Example file.txt

Create a `file.txt` file in the project directory:

```text
Hello World
This is a Get Next Line test.
You can modify this file to test different inputs.
Try adding more lines or removing the final newline.
```

You can modify `file.txt` to test:
- A single line.
- Multiple lines.
- Empty lines.
- Very long lines.
- A file without a newline at the end.
- An empty file.

### Execution

After compilation, run:

```bash
./a.out
```

## Algorithm

The implementation uses a static variable called `stash` to preserve unread data between function calls.

Instead of reading the entire file at once, it reads fixed-size chunks and stores them until a complete line is available.

### 1. Persistent storage: `stash`

```c
static char *stash;
```

The static variable keeps its value between calls to `get_next_line()`.

It stores the data already read from the file descriptor that has not yet been returned to the caller.

This allows the function to continue reading from where the previous call stopped without losing data.

### 2. Checking for an existing newline: `fill_stash()`

Before reading more data, `fill_stash()` checks whether the stash already contains a newline.

- If a newline exists, there is already enough data to extract a line.
- Otherwise, it allocates a temporary buffer of `BUFFER_SIZE + 1` bytes and calls `read_chunks()`.

This avoids unnecessary calls to `read()` when a complete line is already available.

### 3. Reading data: `read_chunks()`

The function reads up to `BUFFER_SIZE` bytes using:

```c
bytes = read(fd, buffer, BUFFER_SIZE);
```

While `read()` returns a positive value:

1. The buffer is null-terminated at the number of bytes actually read.
2. The new chunk is appended to the stash.
3. The previous stash is freed by `append_chunk()`.
4. The function checks whether the newly read chunk contains a newline.

Reading stops when a newline is found in the new chunk or when `read()` reaches EOF.

Importantly, the entire chunk is appended even if the newline occurs in the middle of it. Any characters after that newline remain in the stash for the next call.

If `read()` returns a negative value, the stash is freed and the function returns `NULL`.

### 4. Appending data: `append_chunk()`

This function combines the existing stash with the newly read chunk using `ft_strjoin()`.

The process is:

1. Allocate a new string containing both strings.
2. If allocation fails, free the old stash and return `NULL`.
3. If allocation succeeds, free the old stash and return the new string.

This ensures that the stash always points to the latest combined data.

### 5. Extracting a line: `extract_line()`

Once the stash contains a complete line or the file has reached EOF, `extract_line()` creates a new allocated string.

It scans the stash until it finds:

- A newline character, which is included in the returned line.
- The null terminator, meaning the remaining data is the final line without a newline.

It then allocates the required memory, copies the line, and adds a null terminator.

The original stash is not modified by this function.

### 6. Preserving remaining data: `trim_stash()`

After extracting a line, the stash may still contain data belonging to the next line.

`trim_stash()`:

1. Finds the first newline in the stash.
2. Checks whether any characters remain after it.
3. Allocates memory for the remaining characters.
4. Copies the remaining data into the new allocation.
5. Frees the old stash.
6. Returns the new stash.

If there is no newline or no remaining data after the newline, the old stash is freed and `NULL` is returned.

This ensures that the next call starts with the unread data rather than reading the same characters again.

### 7. Main function: `get_next_line()`

The main function coordinates the entire process:

1. Checks whether the file descriptor is valid and whether `BUFFER_SIZE` is positive.
2. Calls `fill_stash()` to obtain enough data.
3. Returns `NULL` if no data is available.
4. Calls `extract_line()` to create the next line.
5. Calls `trim_stash()` to preserve the remaining data.
6. Returns the extracted line to the caller.

### Algorithm justification

The chunk-based reading approach avoids loading the entire file into memory before returning the first line.

The static stash preserves unread data between calls, while the configurable `BUFFER_SIZE` allows the implementation to work with different reading sizes.

Separating reading, appending, extraction, and trimming into different functions keeps each operation focused and makes memory management easier to follow.

Two properties keep the algorithm correct. After every call, the stash holds exactly the bytes that were read but not yet returned, and an empty stash is always `NULL`. A `read()` is only made when the stash does not already contain a newline, so only the newly read chunk needs to be checked for one. `extract_line()` runs before `trim_stash()` because `trim_stash()` frees the stash.

Memory follows one rule: whoever replaces a pointer frees the old one. `ft_strjoin()` frees nothing, `append_chunk()` and `trim_stash()` free the old stash, the read buffer is freed in `fill_stash()`, and the returned line belongs to the caller. On any failure, everything owned is freed and `NULL` is returned.

The one trade-off is speed: each append copies the whole stash, so a very long line read with a very small `BUFFER_SIZE` is slow. This was accepted in exchange for a simple and safe design.

## Bonus

The bonus reads several file descriptors in turns, using a single static variable:

```c
static char *stash[MAX_FD];
```

- The array holds one stash per file descriptor: `stash[3]` for descriptor 3, `stash[4]` for descriptor 4, and so on, so their data never mixes.
- Only `get_next_line()` changes: the check becomes `fd < 0 || fd >= MAX_FD || BUFFER_SIZE <= 0`, and `stash` becomes `stash[fd]`. The other functions are the same as in the mandatory part.
- `MAX_FD` is `1024`, the usual default limit of open files on Linux. A descriptor greater than or equal to `MAX_FD` returns `NULL`.
- Limit: the state is stored by descriptor number, so if a file is closed before its end and another file receives the same number, the old leftover is still in that slot.

## Helper Functions

| Function | Purpose |
|---|---|
| `ft_strlen()` | Calculates the length of a string. Returns `0` for `NULL`. |
| `has_newline()` | Checks whether a string contains `\n`. |
| `ft_strjoin()` | Allocates and combines two strings. |
| `append_chunk()` | Appends a newly read chunk to the stash and manages the previous allocation. |
| `read_chunks()` | Reads data from the file descriptor until a newline, EOF, or error. |
| `fill_stash()` | Ensures that the stash contains enough data to extract a line. |
| `extract_line()` | Allocates and returns the next line. |
| `trim_stash()` | Removes the returned line from the stash and preserves the remaining data. |

## Project Structure

```text
gnl/
├── get_next_line.c
├── get_next_line.h
├── get_next_line_utils.c
├── get_next_line_bonus.c
├── get_next_line_bonus.h
├── get_next_line_utils_bonus.c
└── README.md
```

`main.c` and `file.txt` are local testing files and are not part of the submission.

## Resources

### Documentation

- [read() — Linux manual](https://man7.org/linux/man-pages/man2/read.2.html)
- [open() — Linux manual](https://man7.org/linux/man-pages/man2/open.2.html)
- [malloc() — Linux manual](https://man7.org/linux/man-pages/man3/malloc.3.html)
- [Static variable — Wikipedia](https://en.wikipedia.org/wiki/Static_variable)
- [C reference — cppreference](https://en.cppreference.com/w/c.html)
- [Valgrind documentation](https://valgrind.org/docs/manual/quick-start.html)

### AI Usage

AI tools were used as a learning, review, and debugging aid during this project.

They were used to understand `read()`, file descriptors, static variables, and memory ownership; to discuss the design of the algorithm (the stash, the split into fill, extract, and trim, and the array used for the bonus); to review the code and find bugs (a leak in `append_chunk()`, problems in `trim_stash()`, and an `int` overflow for very large `BUFFER_SIZE` values); to choose test cases and read Valgrind output; and to review this README.