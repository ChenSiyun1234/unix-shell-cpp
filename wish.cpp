#include <cerrno>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

static void error() {
    const char message[] = "An error has occurred\n";
    const char *p = message;
    size_t remaining = sizeof(message) - 1;
    while (remaining > 0) {
        ssize_t count = write(STDERR_FILENO, p, remaining);
        if (count < 0) {
            if (errno == EINTR) continue;
            break;
        }
        if (count == 0) break;
        p += count;
        remaining -= static_cast<size_t>(count);
    }
}

static std::vector<std::string> tokenize(const std::string &line) {
    std::vector<std::string> tokens;
    std::string word;
    for (unsigned char c : line) {
        if (std::isspace(c) || c == '>' || c == '&') {
            if (!word.empty()) {
                tokens.push_back(word);
                word.clear();
            }
            if (c == '>' || c == '&') tokens.emplace_back(1, c);
        } else {
            word += static_cast<char>(c);
        }
    }
    if (!word.empty()) tokens.push_back(word);
    return tokens;
}

static bool run_command(const std::vector<std::string> &tokens,
                        std::vector<std::string> &paths,
                        std::vector<pid_t> &children) {
    if (tokens.empty()) return false;
    std::vector<std::string> args;
    std::string output;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i] == ">") {
            if (args.empty() || i + 2 != tokens.size() ||
                tokens[i + 1] == ">") {
                error();
                return false;
            }
            output = tokens[i + 1];
            break;
        }
        args.push_back(tokens[i]);
    }

    if (args[0] == "exit") {
        if (args.size() != 1 || !output.empty()) {
            error();
            return false;
        }
        return true;
    }
    if (args[0] == "cd") {
        if (args.size() != 2 || !output.empty() ||
            chdir(args[1].c_str()) != 0) error();
        return false;
    }
    if (args[0] == "path") {
        if (!output.empty()) {
            error();
        } else {
            paths.assign(args.begin() + 1, args.end());
        }
        return false;
    }

    std::string executable;
    for (const std::string &directory : paths) {
        std::string candidate = directory + "/" + args[0];
        if (access(candidate.c_str(), X_OK) == 0) {
            executable = candidate;
            break;
        }
    }
    if (executable.empty()) {
        error();
        return false;
    }
    std::vector<char *> argv;
    for (std::string &arg : args) argv.push_back(&arg[0]);
    argv.push_back(nullptr);

    pid_t child = fork();
    if (child < 0) {
        error();
    } else if (child == 0) {
        if (!output.empty()) {
            int fd = open(output.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd < 0) {
                error();
                _exit(1);
            }
            if (dup2(fd, STDOUT_FILENO) < 0 || dup2(fd, STDERR_FILENO) < 0) {
                error();
                close(fd);
                _exit(1);
            }
            if (fd != STDOUT_FILENO && fd != STDERR_FILENO) close(fd);
        }
        execv(executable.c_str(), argv.data());
        error();
        _exit(1);
    } else {
        children.push_back(child);
    }
    return false;
}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        error();
        return 1;
    }
    FILE *input = stdin;
    const bool interactive = argc == 1;
    if (!interactive) {
        input = fopen(argv[1], "r");
        if (input == nullptr) {
            error();
            return 1;
        }
    }

    std::vector<std::string> paths = {"/bin"};
    char *buffer = nullptr;
    size_t capacity = 0;
    bool exiting = false;
    while (!exiting) {
        if (interactive) {
            fputs("wish> ", stdout);
            fflush(stdout);
        }
        if (getline(&buffer, &capacity, input) < 0) break;
        const auto tokens = tokenize(buffer);
        std::vector<pid_t> children;
        std::vector<std::string> command;
        for (size_t i = 0; i <= tokens.size(); ++i) {
            if (i == tokens.size() || tokens[i] == "&") {
                exiting = run_command(command, paths, children);
                command.clear();
                if (exiting) break;
            } else {
                command.push_back(tokens[i]);
            }
        }
        for (pid_t child : children) {
            pid_t result;
            do {
                result = waitpid(child, nullptr, 0);
            } while (result < 0 && errno == EINTR);
            if (result < 0) error();
        }
    }
    free(buffer);
    if (!interactive) fclose(input);
    return 0;
}
