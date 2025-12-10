#include "compiler_driver.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <getopt.h>

// ============================================================================
// Main Entry Point - C Compiler CLI
// ============================================================================

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options] <source-file>\n\n";
    std::cout << "Options:\n";
    std::cout << "  -o <file>           Write output to <file> (default: a.out)\n";
    std::cout << "  -S                  Compile only; do not assemble or link\n";
    std::cout << "  -c                  Compile and assemble, but do not link\n";
    std::cout << "  -O0                 Disable optimizations\n";
    std::cout << "  -v, --verbose       Enable verbose output\n";
    std::cout << "  -k, --keep          Keep intermediate files (.s, .o)\n";
    std::cout << "  -w                  Disable warnings\n";
    std::cout << "  --dump-tokens <file>  Dump token stream to JSON file\n";
    std::cout << "  --dump-ast <file>     Dump AST to JSON file\n";
    std::cout << "  --dump-asm <file>     Dump assembly to text file\n";
    std::cout << "  --dump-hex <file>     Dump hex output to file\n";
    std::cout << "  -h, --help          Display this help message\n";
    std::cout << "\n";
    std::cout << "Example:\n";
    std::cout << "  " << programName << " -v --dump-tokens tokens.json program.c\n";
    std::cout << "\n";
}

int main(int argc, char* argv[]) {
    CompilerDriver::Options options;
    std::string sourceFile;

    // Define long options
    static struct option long_options[] = {
        {"verbose",     no_argument,       nullptr, 'v'},
        {"keep",        no_argument,       nullptr, 'k'},
        {"help",        no_argument,       nullptr, 'h'},
        {"dump-tokens", required_argument, nullptr, 1000},
        {"dump-ast",    required_argument, nullptr, 1001},
        {"dump-asm",    required_argument, nullptr, 1002},
        {"dump-hex",    required_argument, nullptr, 1003},
        {nullptr,       0,                 nullptr, 0}
    };

    // Parse command-line arguments
    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "o:Scvkhw", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'o':
                options.outputFile = optarg;
                break;
            case 'S':
                options.emitAssembly = true;
                break;
            case 'c':
                options.emitObject = true;
                break;
            case 'v':
                options.verbose = true;
                break;
            case 'k':
                options.keepIntermediateFiles = true;
                break;
            case 'w':
                options.warningsEnabled = false;
                break;
            case 1000:  // --dump-tokens
                options.dumpTokensPath = optarg;
                break;
            case 1001:  // --dump-ast
                options.dumpAstPath = optarg;
                break;
            case 1002:  // --dump-asm
                options.dumpAsmPath = optarg;
                break;
            case 1003:  // --dump-hex
                options.dumpHexPath = optarg;
                break;
            case 'h':
                printUsage(argv[0]);
                return 0;
            default:
                printUsage(argv[0]);
                return 1;
        }
    }

    // Check if source file is provided
    if (optind >= argc) {
        std::cerr << "Error: No source file specified\n\n";
        printUsage(argv[0]);
        return 1;
    }

    sourceFile = argv[optind];

    // Read source file
    std::ifstream inFile(sourceFile);
    if (!inFile) {
        std::cerr << "Error: Cannot open source file: " << sourceFile << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << inFile.rdbuf();
    std::string sourceCode = buffer.str();
    inFile.close();

    // Create compiler driver and compile
    CompilerDriver driver(options);
    bool success = driver.compile(sourceCode, sourceFile);

    if (!success) {
        std::cerr << "\nCompilation failed with " << driver.getErrorCount() << " error(s)\n";
        return 1;
    }

    if (options.verbose) {
        std::cout << "\nCompilation completed successfully!\n";
    }

    return 0;
}
