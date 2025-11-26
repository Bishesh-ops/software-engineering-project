// ============================================================================
// Example: External Math Library Integration
// ============================================================================
// Demonstrates calling external math library functions (libm)
//
// This example shows:
// 1. Calling standard math functions (sqrt, pow, sin, etc.)
// 2. Multiple external library dependencies
// 3. Floating-point return values (future enhancement)
// 4. Linking with -lm flag

#include "../include/ir.h"
#include "../include/codegen.h"
#include <iostream>
#include <memory>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "External Math Library Integration\n";
    cout << "========================================\n\n";

    // ========================================================================
    // Example: Calling math library functions
    // ========================================================================
    // C equivalent:
    //   double calculate() {
    //       double x = 16.0;
    //       double y = sqrt(x);      // Call sqrt from libm
    //       double z = pow(y, 2.0);  // Call pow from libm
    //       return z;
    //   }
    // ========================================================================

    cout << "C Code:\n";
    cout << "-------\n";
    cout << "double calculate() {\n";
    cout << "    double x = 16.0;\n";
    cout << "    double y = sqrt(x);\n";
    cout << "    double z = pow(y, 2.0);\n";
    cout << "    return z;\n";
    cout << "}\n\n";

    CodeGenerator codegen;

    // Create calculate function
    auto calcFunc = make_unique<IRFunction>("calculate", "double");
    auto block = make_unique<IRBasicBlock>("entry");

    // Note: Full floating-point support would require XMM register handling
    // For this example, we demonstrate the external symbol mechanism

    // Call sqrt(16.0)
    SSAValue xValue("x", "double", 0);
    SSAValue yValue("y", "double", 0);

    auto sqrtCall = make_unique<CallInst>("sqrt", &yValue);
    sqrtCall->addOperand(IROperand(xValue));
    block->addInstruction(move(sqrtCall));

    // Call pow(y, 2.0)
    SSAValue zValue("z", "double", 0);
    auto powCall = make_unique<CallInst>("pow", &zValue);
    powCall->addOperand(IROperand(yValue));
    block->addInstruction(move(powCall));

    // Return z
    block->addInstruction(make_unique<ReturnInst>(IROperand(zValue)));

    calcFunc->addBasicBlock(move(block));

    // Generate assembly
    vector<unique_ptr<IRFunction>> functions;
    functions.push_back(move(calcFunc));

    string assembly = codegen.generateProgram(functions);

    cout << "Generated x86-64 Assembly:\n";
    cout << "==========================\n";
    cout << assembly << "\n";

    cout << "Key Features:\n";
    cout << "-------------\n";
    cout << "1. .extern sqrt - declares sqrt as external symbol\n";
    cout << "2. .extern pow - declares pow as external symbol\n";
    cout << "3. Both calls follow System V AMD64 ABI\n";
    cout << "4. External math functions properly marked\n\n";

    // ========================================================================
    // Example 2: Multiple math functions
    // ========================================================================

    cout << "========================================\n";
    cout << "Example 2: Comprehensive Math Operations\n";
    cout << "========================================\n\n";

    cout << "C Code:\n";
    cout << "-------\n";
    cout << "void math_demo() {\n";
    cout << "    double angle = 3.14159 / 4.0;  // 45 degrees\n";
    cout << "    double s = sin(angle);\n";
    cout << "    double c = cos(angle);\n";
    cout << "    double t = tan(angle);\n";
    cout << "    double e = exp(1.0);           // Euler's number\n";
    cout << "    double l = log(e);             // Natural log\n";
    cout << "    double sq = sqrt(2.0);         // Square root\n";
    cout << "}\n\n";

    CodeGenerator codegen2;

    auto mathDemo = make_unique<IRFunction>("math_demo", "void");
    auto demoBlock = make_unique<IRBasicBlock>("entry");

    // Create calls to various math functions
    SSAValue angleVal("angle", "double", 0);

    // sin
    SSAValue sinVal("s", "double", 0);
    auto sinCall = make_unique<CallInst>("sin", &sinVal);
    sinCall->addOperand(IROperand(angleVal));
    demoBlock->addInstruction(move(sinCall));

    // cos
    SSAValue cosVal("c", "double", 0);
    auto cosCall = make_unique<CallInst>("cos", &cosVal);
    cosCall->addOperand(IROperand(angleVal));
    demoBlock->addInstruction(move(cosCall));

    // tan
    SSAValue tanVal("t", "double", 0);
    auto tanCall = make_unique<CallInst>("tan", &tanVal);
    tanCall->addOperand(IROperand(angleVal));
    demoBlock->addInstruction(move(tanCall));

    // exp
    SSAValue expVal("e", "double", 0);
    auto expCall = make_unique<CallInst>("exp", &expVal);
    demoBlock->addInstruction(move(expCall));

    // log
    SSAValue logVal("l", "double", 0);
    auto logCall = make_unique<CallInst>("log", &logVal);
    logCall->addOperand(IROperand(expVal));
    demoBlock->addInstruction(move(logCall));

    // sqrt
    SSAValue sqrtVal("sq", "double", 0);
    auto sqrtCall2 = make_unique<CallInst>("sqrt", &sqrtVal);
    demoBlock->addInstruction(move(sqrtCall2));

    // Return
    demoBlock->addInstruction(make_unique<ReturnInst>());

    mathDemo->addBasicBlock(move(demoBlock));

    // Generate assembly
    vector<unique_ptr<IRFunction>> functions2;
    functions2.push_back(move(mathDemo));

    string assembly2 = codegen2.generateProgram(functions2);

    cout << "Generated x86-64 Assembly:\n";
    cout << "==========================\n";
    cout << assembly2 << "\n";

    cout << "External Math Functions Used:\n";
    cout << "----------------------------\n";
    cout << "✓ sin()  - Sine function\n";
    cout << "✓ cos()  - Cosine function\n";
    cout << "✓ tan()  - Tangent function\n";
    cout << "✓ exp()  - Exponential function\n";
    cout << "✓ log()  - Natural logarithm\n";
    cout << "✓ sqrt() - Square root\n\n";

    // ========================================================================
    // Compilation Instructions
    // ========================================================================

    cout << "========================================\n";
    cout << "Compilation and Linking with Math Library\n";
    cout << "========================================\n\n";

    cout << "Important: Math library functions require linking with -lm\n\n";

    cout << "Step 1: Generate assembly\n";
    cout << "  [Assembly code generated by compiler]\n\n";

    cout << "Step 2: Assemble\n";
    cout << "  $ as -o output.o output.s\n\n";

    cout << "Step 3: Link with math library\n";
    cout << "  $ gcc -o program output.o -lm\n";
    cout << "  \n";
    cout << "  The -lm flag links with libm (math library)\n\n";

    cout << "Alternative explicit linking:\n";
    cout << "  $ ld -o program output.o -lc -lm \\\n";
    cout << "      -dynamic-linker /lib64/ld-linux-x86-64.so.2\n\n";

    cout << "Step 4: Run\n";
    cout << "  $ ./program\n\n";

    // ========================================================================
    // Additional Information
    // ========================================================================

    cout << "========================================\n";
    cout << "Math Library Functions Support\n";
    cout << "========================================\n\n";

    cout << "Trigonometric Functions:\n";
    cout << "  sin(x)    - Sine\n";
    cout << "  cos(x)    - Cosine\n";
    cout << "  tan(x)    - Tangent\n";
    cout << "  asin(x)   - Arc sine\n";
    cout << "  acos(x)   - Arc cosine\n";
    cout << "  atan(x)   - Arc tangent\n";
    cout << "  atan2(y,x)- Arc tangent of y/x\n\n";

    cout << "Exponential and Logarithmic:\n";
    cout << "  exp(x)    - e^x\n";
    cout << "  log(x)    - Natural logarithm\n";
    cout << "  log10(x)  - Base-10 logarithm\n";
    cout << "  pow(x,y)  - x raised to power y\n";
    cout << "  sqrt(x)   - Square root\n";
    cout << "  cbrt(x)   - Cube root\n\n";

    cout << "Rounding and Remainder:\n";
    cout << "  ceil(x)   - Ceiling\n";
    cout << "  floor(x)  - Floor\n";
    cout << "  round(x)  - Round to nearest\n";
    cout << "  fabs(x)   - Absolute value\n";
    cout << "  fmod(x,y) - Floating-point remainder\n\n";

    cout << "Hyperbolic Functions:\n";
    cout << "  sinh(x)   - Hyperbolic sine\n";
    cout << "  cosh(x)   - Hyperbolic cosine\n";
    cout << "  tanh(x)   - Hyperbolic tangent\n\n";

    cout << "ABI Notes for Floating-Point:\n";
    cout << "----------------------------\n";
    cout << "- Floating-point args passed in XMM0-XMM7 registers\n";
    cout << "- Return values in XMM0 for float/double\n";
    cout << "- Integer and float args use different register sets\n";
    cout << "- AL register contains count of vector arguments\n";
    cout << "- Stack must be 16-byte aligned\n\n";

    cout << "Note: Full floating-point support requires:\n";
    cout << "  1. XMM register allocation\n";
    cout << "  2. SSE/AVX instructions\n";
    cout << "  3. Proper type classification for ABI\n";
    cout << "  4. Mixed integer/float argument handling\n\n";

    cout << "Current Implementation:\n";
    cout << "  ✓ Correctly marks math functions as external\n";
    cout << "  ✓ Generates proper call instructions\n";
    cout << "  ✓ ABI-compliant for integer arguments\n";
    cout << "  ⚠ Floating-point arguments need XMM register support\n\n";

    cout << "========================================\n";
    cout << "Example Complete!\n";
    cout << "========================================\n";

    return 0;
}
