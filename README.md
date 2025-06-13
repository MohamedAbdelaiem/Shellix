# **Shellix**

Shellix is a lightweight, custom shell implementation written in C that delivers essential shell functionality with a clean and intuitive interface. Built as a learning project to understand system programming fundamentals and shell internals.

## **✨ Features**

Shellix provides comprehensive shell capabilities including:

* **Command Execution**: Execute external programs with full argument support
* **Built-in Commands**:
   * `exit` - Gracefully terminate the shell
   * `cd <directory>` - Change current working directory (supports HOME navigation)
   * `pwd` - Display current working directory path
   * `paths <path>...` - Configure executable search paths
* **I/O Redirection**: Redirect command output using `>` operator
* **Path Resolution**: Intelligent executable discovery across configured directories
* **Signal Management**: Proper handling of SIGINT (Ctrl+C) and SIGTSTP (Ctrl+Z)
* **Dual Operation Modes**: 
   * Interactive mode with command prompt
   * Non-interactive mode for script execution
* **Robust Error Handling**: Comprehensive error reporting and graceful failure recovery

## **📦 Installation**

Clone and build Shellix in just a few steps:

```bash
git clone https://github.com/yourusername/shellix.git
cd shellix
make
```

The build process creates an executable named `shellix` ready for use.

## **🚀 Usage**

### Interactive Mode
Launch the shell for interactive command execution:

```bash
./shellix
```

### Non-Interactive Mode
Execute commands from a script file:

```bash
./shellix script.sh
```

## **💻 Command Examples**

### Basic Operations
```bash
shellix> ls -la                    # List files with details
shellix> cd /home/user/documents   # Navigate to directory
shellix> cd                        # Return to HOME directory
shellix> pwd                       # Show current location
shellix> exit                      # Close the shell
```

### Output Redirection
```bash
shellix> ls -la /tmp > output.txt  # Save directory listing to file
shellix> echo "Hello World" > greeting.txt
shellix> wc -l myfile.txt > linecount.txt
```

### Path Configuration
```bash
shellix> paths /bin /usr/bin /usr/local/bin  # Set executable search paths
shellix> paths                               # Clear all paths (disables external commands)
```

## **🔧 Technical Architecture**

Shellix follows a modular design pattern with clear separation of concerns:

### Core Components
* **Main Loop**: Handles user input, command parsing, and execution flow
* **Command Parser**: Tokenizes input and identifies command types
* **Built-in Handler**: Processes internal shell commands
* **External Executor**: Manages child processes for external programs
* **Path Resolver**: Locates executables in configured directories
* **Signal Controller**: Manages interrupt and control signals

### Process Management
* **Fork/Exec Model**: Creates child processes for external command execution
* **Signal Propagation**: Forwards signals to active child processes
* **Process Cleanup**: Ensures proper resource cleanup and zombie prevention

### Error Handling
* **Syntax Validation**: Catches malformed commands before execution
* **System Call Monitoring**: Handles system-level errors gracefully
* **User Feedback**: Provides clear error messages via stderr

## **⚙️ System Requirements**

* **Operating System**: Linux/Unix-compatible system
* **Compiler**: GCC or compatible C compiler
* **Dependencies**: Standard C library (libc)
* **Memory**: Minimal footprint, suitable for resource-constrained environments


## **🔐 Security Considerations**

* **Path Validation**: Prevents execution of unauthorized programs
* **Input Sanitization**: Protects against command injection
* **Process Isolation**: Maintains secure process boundaries
* **Signal Safety**: Implements signal-safe operations

## **🚦 Error Handling**

Shellix implements comprehensive error detection:
* **Command Not Found**: Clear feedback when executables aren't located
* **Permission Denied**: Appropriate handling of access restrictions
* **Syntax Errors**: Validation of command structure before execution
* **System Failures**: Graceful handling of system call failures

All error messages follow the format: `"An error has occurred!"` printed to stderr.

## **📚 Development Notes**

### Key System Calls Used
* `fork()` - Process creation
* `execv()` - Program execution
* `wait()/waitpid()` - Process synchronization
* `chdir()` - Directory navigation
* `getcwd()` - Path resolution
* `access()` - Executable validation
* `strtok()` - Input tokenization

### Safety Features
* **Fork Bomb Protection**: Designed to prevent uncontrolled process creation
* **Resource Limits**: Respects system ulimits
* **Signal Safety**: Proper signal handler implementation

## **🤝 Contributing**

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Implement your changes with proper testing
4. Submit a pull request with detailed description

## **📄 License**

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for complete details.

## **🙏 Acknowledgments**

Built as an educational project to explore:
* Unix system programming concepts
* Process management and IPC
* Shell design and implementation patterns
* C programming best practices

---

**Note**: This shell is designed for educational purposes and basic usage. For production environments, consider using established shells like bash, zsh, or fish.
