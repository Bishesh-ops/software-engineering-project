#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "ir_codegen.h"
#include "ir_optimizer.h"
#include "codegen.h"
#include <iostream>
#include <fstream>
#include <cstdlib>

// ============================================================================
// End-to-End Compilation Example
// ============================================================================
// Demonstrates the complete compilation pipeline from C source to executable

void printSeparator() {
    std::cout << "========================================\n";
}

int main() {
    printSeparator();
    std::cout << "C COMPILER - END-TO-END COMPILATION DEMO\n";
    printSeparator();
    std::cout << "\n";

    // ========================================================================
    // Sample C Program
    // ========================================================================
    std::string sourceCode = R"(
int add(int a, int b) {
    return a + b;
}

int main() {
    int x = 10;
    int y = 20;
    int result = add(x, y);
    return result;
}
)";

    std::cout << "Source Code:\n";
    std::cout << "------------\n";
    std::cout << sourceCode << "\n\n";

    // ========================================================================
    // Stage 1: Lexical Analysis
    // ========================================================================
    std::cout << "Stage 1: Lexical Analysis\n";

    Lexer lexer(sourceCode, "example.c");
    std::vector<Token> tokens = lexer.tokenize();

    std::cout << "  ✓ Generated " << tokens.size() << " tokens\n\n";

    // ========================================================================
    // Stage 2: Parsing
    // ========================================================================
    std::cout << "Stage 2: Parsing\n";

    Parser parser(tokens);
    std::unique_ptr<Program> ast = parser.parseProgram();

    if (parser.hasErrors()) {
        std::cerr << "  ✗ Parsing errors:\n";
        for (const auto& err : parser.getErrors()) {
            std::cerr << "    " << err << "\n";
        }
        return 1;
    }

    std::cout << "  ✓ AST constructed successfully\n\n";

    // ========================================================================
    // Stage 3: Semantic Analysis
    // ========================================================================
    std::cout << "Stage 3: Semantic Analysis\n";

    SemanticAnalyzer analyzer;
    analyzer.analyze(*ast);

    if (analyzer.hasErrors()) {
        std::cerr << "  ✗ Semantic errors:\n";
        for (const auto& err : analyzer.getErrors()) {
            std::cerr << "    " << err << "\n";
        }
        return 1;
    }

    std::cout << "  ✓ Type checking passed\n\n";

    // ========================================================================
    // Stage 4: IR Generation
    // ========================================================================
    std::cout << "Stage 4: IR Generation\n";

    IRCodeGenerator irGen;
    irGen.generate(*ast);
    auto& functions = irGen.getFunctions();

    std::cout << "  ✓ Generated " << functions.size() << " function(s)\n";
    std::cout << "\n  IR Code:\n";
    for (const auto& func : functions) {
        std::cout << "  " << func->toString() << "\n";
    }
    std::cout << "\n";

    // ========================================================================
    // Stage 5: Optimization
    // ========================================================================
    std::cout << "Stage 5: Optimization\n";

    IROptimizer optimizer;
    for (auto& func : functions) {
        optimizer.optimize(func.get());
    }

    std::cout << "  ✓ Optimization passes completed\n\n";

    // ========================================================================
    // Stage 6: Code Generation
    // ========================================================================
    std::cout << "Stage 6: x86-64 Code Generation\n";

    CodeGenerator codegen;
    std::string assembly = codegen.generateProgram(functions);

    std::cout << "  ✓ Generated " << assembly.size() << " bytes of assembly\n\n";

    std::cout << "Generated Assembly:\n";
    std::cout << "-------------------\n";
    std::cout << assembly << "\n";

    // ========================================================================
    // Stage 7: Write Assembly File
    // ========================================================================
    std::cout << "Stage 7: Writing Assembly File\n";

    std::string asmFile = "output.s";
    std::ofstream outFile(asmFile);
    if (!outFile) {
        std::cerr << "  ✗ Failed to write assembly file\n";
        return 1;
    }

    outFile << assembly;
    outFile.close();

    std::cout << "  ✓ Assembly written to: " << asmFile << "\n\n";

    // ========================================================================
    // Stage 8: Assembly (Optional - requires system tools)
    // ========================================================================
    std::cout << "Stage 8: Assembling (optional)\n";
    std::cout << "  To assemble manually:\n";
    std::cout << "    as -o output.o output.s\n";
    std::cout << "    ld -o program output.o -lSystem -syslibroot $(xcrun --show-sdk-path) -e _main -arch arm64\n";
    std::cout << "    # Or use gcc: gcc -o program output.s\n\n";

    printSeparator();
    std::cout << "COMPILATION PIPELINE COMPLETE!\n";
    printSeparator();
    std::cout << "\nOutput file: " << asmFile << "\n";
    std::cout << "This assembly can be assembled and linked to create an executable.\n\n";

    return 0;
}
