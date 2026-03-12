# C Local Version Control System (repo)

![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)

This repository provides a C-based command-line utility that acts as a lightweight, local version control system (similar to Git). It allows users to track file modifications, stage files, generate commits with detailed diffs (insertions/deletions using the LCS algorithm), and revert to previous states. 

---

## Repository Structure
```
.
├── Makefile
├── main.c
├── repo.h
├── add.c
├── remove.c
├── status.c
├── commit.c
├── revert.c
├── log.c
├── help.c
└── utils.c
```
- **main.c** The entry point of the program. It provides an interactive prompt (`> `) and uses `fork()` and `execvp()` to dispatch user inputs to the appropriate sub-commands.
- **add.c & remove.c** Manages the staging area by verifying file paths, ensuring they are within the working directory, and updating the `.repo/.staging.log`.
- **status.c** Compares the current working directory against the latest commit using MD5 hashes and file sizes to categorize files as `staged`, `untracked`, or `modified`.
- **commit.c** Creates a new versioned directory under `.repo/`. It calculates the exact lines of insertions and deletions using the Longest Common Subsequence (LCS) algorithm and physically backs up the files.
- **revert.c** Restores the working directory to a specific commit state by copying files from the `.repo/<COMMIT_NAME>` backup directory.
- **log.c & help.c** Parses `.repo/.commit.log` to display the commit history and provides usage instructions for the CLI.
- **utils.c** Contains the heavy-lifting logic: BFS directory traversal, MD5 hashing, linked list management for staging/commits, and the LCS diff calculator.

---

## Prerequisites
> **IMPORTANT** : This tool relies on the Linux `proc` filesystem (e.g., `/proc/self/exe`) and specific POSIX system calls. Execution on non-Linux environments (like Windows natively) is **NOT** recommended.
1. **Linux OS** (Kernel support for standard POSIX file and directory operations).
2. **Bash** (for terminal interaction).
3. **GCC** (version >= 7.0).
4. **OpenSSL Library** (required for `libcrypto` to calculate MD5 hashes).
---
## Unpacking Archives
If this repository is distributed as a compressed archive (e.g., `repo.tar.gz`), you must decompress it while maintaining the directory structure.

1. Using tar:
   ```bash
   tar -xzvf repo.tar.gz
   cd repo
**Note**: Must follow the extracted directory structure to ensure the `Makefile` resolves dependencies correctly.

## Installation & Setup

1. **Install dependencies** (OpenSSL and build tools):
   ```bash
   sudo apt update
   sudo apt install -y build-essential libssl-dev
2. **Clone or copy** this repository to your local machine.
3. **Compile the program**:
   ```bash
    make
    ```
    This will gererate the executable binary named **repo**.
4. **Clean build files** (optional):
    ```bash
    make clean

---

## Usage
Start the interactive version control shell by running the binary:
```bash
./repo
```
Once inside the prompt (`> `), you can use the following commands:

- `add <PATH`> : Add a file or directory to the staging area.

- `remove <PATH>` : Remove a file or directory from the staging area.

- `status` : Show the current status of the staging area (Changes to be committed & Untracked files).

- `commit <NAME>` : Commit the staged changes with a specific version name.

- `revert <NAME>` : Revert the working directory to the state of the specified commit name.

- `log [NAME]` : Show the entire commit log, or the log for a specific commit name.

- `help` : Show available commands.

- `exit` : Exit the interactive shell.

---

## Directory Layout Example
When you initialize and use the program, it automatically creates a `.repo` hidden directory to manage states and backups:
```
.
├── .repo/
│   ├── .staging.log     # Tracks files currently in the staging area
│   ├── .commit.log      # Centralized log of all commits
│   ├── v1.0/            # Backup directory for commit "v1.0"
│   │   └── src/
│   │       └── example.c
│   └── v2.0/            # Backup directory for commit "v2.0"
│       └── src/
│           └── example.c
├── src/
│   └── example.c
├── repo                 # Executable binary
└── main.c
```

---

## Example Workflow
1. **Start the program**:
    ```bash
    ./repo
    > 
    ```
2. **Check status**:
    ```bash
    > status
    untracked files:
     new file: "./main.c"
3. **Stage a file**:

    ```bash
    > add ./main.c
    add "./main.c"
4. **Commit the changes**:
    ```bash
    > commit initial_commit
    commit to "initial_commit"
    1 files changed, 150 insertions(+), 0 deletions(-)
    new file: "./main.c"
5. **View logs**:
    ```bash
    > log
    commit: "initial_commit" - new file: "/absolute/path/to/main.c"

---

- **"fatal error: openssl/md5.h: No such file or directory"** : You are missing the OpenSSL development headers. Run this.
  ```bash
  sudo apt install libssl-dev.
- **"unable to get cwd" or Path Errors** : Ensure you are running the `repo` executable from a directory where you have read/write permissions. The program strictly prevents adding files outside of the Current Working Directory (CWD).

- **"ERROR: Commit name is too long"** : Commit names are restricted to 255 bytes (`FILE_LENGTH`). Choose a shorter, concise commit name without slashes (`/`).

---

## Customization
- **Diff Calculation Memory Scaling** In `utils.c`, the `count_lines` function dynamically scales memory for file line parsing starting from a 1000-line capacity. If you regularly commit massive source files (e.g., 100,000+ lines), you can increase the initial `capacity` variable to reduce realloc overhead.

**Hash Algorithm** The integrity checks currently rely on MD5 (`calc_md5_hash` in `utils.c`). You can easily swap this out for SHA-256 by updating the OpenSSL context to `SHA256_CTX` if stronger cryptographic integrity is required.

---

## License & Acknowledgments

This source code is licensed under the **MIT License**. See the `LICENSE` file for details.  

**Acknowledgments:**
- **Code Implementation**: Wooyong Eom, CSE, Soongsil Univ.
- **Project Specification**: The architecture, requirements, and specifications for this project were provided as an assignment for the Linux System Programming course at Soongsil University, by Prof. Hong Jiman, OSLAB. This repository contains my original implementation of those requirements.
---
_Last Updated: March 11, 2026_
