# BSH

Bsh (short for bad shell) is a simple shell made by me in C++. It supports basic features like pipes, redirections, environment variable expansion, script execution, and a colorful prompt.

## Features

- Command execution – Every shell can do this
- A few built-in commands
- Piping – Commands can be piped into each other by using |
- File completion using TAB - Thanks to GNU Readline
- stdout/stderr/stdin redirection using > >> 2> 2>> <
- Env variable expansion – `$VAR` or `${VAR}` will be expanded
- Script execution – Put a few commands into a file and run it
- Colored prompt – Nothing more to say about that
- Command history – Lets you run commands from your history (also some other stuff but not enough space)
- Displays the last exit code 🤯
- Multiline commands using \ and separating commands using ;

## Requirements

- A computer
- Linux

## Installation

1. Install dependencies (example for Arch Linux):
    ```bash
    sudo pacman -S cmake base-devel fmt readline
    ```
2. Clone the GitHub repository:
    ```bash
    git clone https://github.com/simon0302010/bsh.git
    cd bsh
    ```
3. Build and install:
    ```bash
    cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
    cmake --build build
    sudo cmake --install build
    ```

## Usage

Just run `bsh`.

## License

This project is licensed under the GNU General Public License Version 3. For more information, see the [LICENSE](LICENSE) file.