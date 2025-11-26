#include "../include/ir.h"
#include "../include/codegen.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sys/wait.h>

// Simple executable generation test using manual IR

int main() {
    std::cout << "========================================\n";
    std::cout << "EXECUTABLE GENERATION TEST\n";
    std::cout << "User Story 4: Generate Executable Output File\n";
    std::cout << "========================================\n\n";

    // Create IR for: int main() { return 42; }
    IRFunction func("main", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue result("result", "int", 0);
    block->addInstruction(std::make_unique<MoveInst>(
        &result, IROperand("42", IROperand::OperandType::CONSTANT)));
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(result)));

    func.addBasicBlock(std::move(block));

    // Generate assembly
    CodeGenerator codegen;
    std::vector<std::unique_ptr<IRFunction>> functions;

    // We need to create a copy since generateProgram takes ownership
    IRFunction* funcPtr = &func;
    std::string assembly = codegen.generateFunction(funcPtr);

    std::cout << "Generated Assembly:\n";
    std::cout << "-------------------\n";
    std::cout << assembly << "\n\n";

    // Write to file
    std::ofstream asmFile("/tmp/test_simple.s");
    asmFile << ".text\n";
    asmFile << assembly;
    asmFile.close();

    std::cout << "[INFO] Assembly written to /tmp/test_simple.s\n";

    // Assemble and link
    std::cout << "[INFO] Assembling...\n";
    int status = system("as -o /tmp/test_simple.o /tmp/test_simple.s 2>&1");
    if (status != 0) {
        std::cerr << "[FAIL] Assembly failed\n";
        return 1;
    }

    std::cout << "[INFO] Linking...\n";
    status = system("gcc -o /tmp/test_simple /tmp/test_simple.o 2>&1");
    if (status != 0) {
        std::cerr << "[FAIL] Linking failed\n";
        return 1;
    }

    std::cout << "[INFO] Running executable...\n";
    int exitCode = system("/tmp/test_simple");
    exitCode = WEXITSTATUS(exitCode);

    std::cout << "\n========================================\n";
    std::cout << "Program exited with code: " << exitCode << "\n";
    std::cout << "Expected: 42\n";

    if (exitCode == 42) {
        std::cout << "[PASS] Executable generation successful!\n";
        std::cout << "========================================\n";
        std::cout << "\n✅ User Story 4 COMPLETE:\n";
        std::cout << "- Generated valid x86-64 assembly\n";
        std::cout << "- Assembled with 'as' tool\n";
        std::cout << "- Linked with 'gcc'\n";
        std::cout << "- Created working executable\n";
        std::cout << "- Program ran with correct output\n";
        return 0;
    } else {
        std::cout << "[FAIL] Wrong exit code\n";
        std::cout << "========================================\n";
        return 1;
    }
}
