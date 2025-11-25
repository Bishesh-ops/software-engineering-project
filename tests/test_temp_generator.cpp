#include "ir.h"
#include <iostream>
#include <cassert>

// ============================================================================
// Test Temporary Variable Generator
// ============================================================================

void testBasicTempGeneration() {
    std::cout << "Testing Basic Temp Generation...\n";

    TempVarGenerator gen;

    // Test sequential temp generation
    std::string t0 = gen.newTemp();
    std::cout << "  Generated: " << t0 << "\n";
    assert(t0 == "t0");

    std::string t1 = gen.newTemp();
    std::cout << "  Generated: " << t1 << "\n";
    assert(t1 == "t1");

    std::string t2 = gen.newTemp();
    std::cout << "  Generated: " << t2 << "\n";
    assert(t2 == "t2");

    std::string t3 = gen.newTemp();
    std::cout << "  Generated: " << t3 << "\n";
    assert(t3 == "t3");

    // Verify count
    assert(gen.getTempCount() == 4);
    std::cout << "  Total temps generated: " << gen.getTempCount() << "\n";

    std::cout << "Basic Temp Generation: PASSED\n\n";
}

void testTempCounterTracking() {
    std::cout << "Testing Temp Counter Tracking...\n";

    TempVarGenerator gen;

    // Initial count should be 0
    assert(gen.getTempCount() == 0);
    std::cout << "  Initial count: " << gen.getTempCount() << "\n";

    // Generate 10 temps
    for (int i = 0; i < 10; i++) {
        std::string temp = gen.newTemp();
        std::cout << "  [" << i << "] Generated: " << temp << "\n";
        assert(temp == "t" + std::to_string(i));
    }

    // Count should be 10
    assert(gen.getTempCount() == 10);
    std::cout << "  Final count: " << gen.getTempCount() << "\n";

    std::cout << "Temp Counter Tracking: PASSED\n\n";
}

void testTempReset() {
    std::cout << "Testing Temp Reset...\n";

    TempVarGenerator gen;

    // Generate some temps
    gen.newTemp();
    gen.newTemp();
    gen.newTemp();
    assert(gen.getTempCount() == 3);
    std::cout << "  Count before reset: " << gen.getTempCount() << "\n";

    // Reset
    gen.reset();
    assert(gen.getTempCount() == 0);
    std::cout << "  Count after reset: " << gen.getTempCount() << "\n";

    // Generate new temps - should start from t0 again
    std::string t0 = gen.newTemp();
    assert(t0 == "t0");
    std::cout << "  Generated after reset: " << t0 << "\n";

    std::string t1 = gen.newTemp();
    assert(t1 == "t1");
    std::cout << "  Generated after reset: " << t1 << "\n";

    std::cout << "Temp Reset: PASSED\n\n";
}

void testCustomPrefix() {
    std::cout << "Testing Custom Prefix...\n";

    TempVarGenerator gen("temp");

    std::string temp0 = gen.newTemp();
    std::cout << "  Generated with 'temp' prefix: " << temp0 << "\n";
    assert(temp0 == "temp0");

    std::string temp1 = gen.newTemp();
    std::cout << "  Generated with 'temp' prefix: " << temp1 << "\n";
    assert(temp1 == "temp1");

    // Test changing prefix
    gen.setPrefix("tmp");
    std::string tmp2 = gen.newTemp();
    std::cout << "  Generated with 'tmp' prefix: " << tmp2 << "\n";
    assert(tmp2 == "tmp2");

    std::cout << "Custom Prefix: PASSED\n\n";
}

void testTempSSAGeneration() {
    std::cout << "Testing Temp SSA Value Generation...\n";

    TempVarGenerator gen;

    // Generate SSA temp values with types
    SSAValue t0 = gen.newTempSSA("int");
    std::cout << "  Generated SSA: " << t0.getSSAName() << " (type: " << t0.getType() << ")\n";
    assert(t0.getName() == "t0");
    assert(t0.getType() == "int");
    assert(t0.getSSAName() == "t0_0");

    SSAValue t1 = gen.newTempSSA("float");
    std::cout << "  Generated SSA: " << t1.getSSAName() << " (type: " << t1.getType() << ")\n";
    assert(t1.getName() == "t1");
    assert(t1.getType() == "float");
    assert(t1.getSSAName() == "t1_0");

    SSAValue t2 = gen.newTempSSA("int*");
    std::cout << "  Generated SSA: " << t2.getSSAName() << " (type: " << t2.getType() << ")\n";
    assert(t2.getName() == "t2");
    assert(t2.getType() == "int*");
    assert(t2.getSSAName() == "t2_0");

    std::cout << "Temp SSA Value Generation: PASSED\n\n";
}

void testLabelGeneration() {
    std::cout << "Testing Label Generation...\n";

    LabelGenerator gen;

    // Test sequential label generation
    std::string L0 = gen.newLabel();
    std::cout << "  Generated: " << L0 << "\n";
    assert(L0 == "L0");

    std::string L1 = gen.newLabel();
    std::cout << "  Generated: " << L1 << "\n";
    assert(L1 == "L1");

    std::string L2 = gen.newLabel();
    std::cout << "  Generated: " << L2 << "\n";
    assert(L2 == "L2");

    // Test named labels
    std::string loop0 = gen.newLabel("loop");
    std::cout << "  Generated named: " << loop0 << "\n";
    assert(loop0 == "loop_3");

    std::string if1 = gen.newLabel("if");
    std::cout << "  Generated named: " << if1 << "\n";
    assert(if1 == "if_4");

    // Verify count
    assert(gen.getLabelCount() == 5);
    std::cout << "  Total labels generated: " << gen.getLabelCount() << "\n";

    std::cout << "Label Generation: PASSED\n\n";
}

void testLabelReset() {
    std::cout << "Testing Label Reset...\n";

    LabelGenerator gen;

    // Generate some labels
    gen.newLabel();
    gen.newLabel();
    gen.newLabel("test");
    assert(gen.getLabelCount() == 3);
    std::cout << "  Count before reset: " << gen.getLabelCount() << "\n";

    // Reset
    gen.reset();
    assert(gen.getLabelCount() == 0);
    std::cout << "  Count after reset: " << gen.getLabelCount() << "\n";

    // Generate new labels - should start from 0 again
    std::string L0 = gen.newLabel();
    assert(L0 == "L0");
    std::cout << "  Generated after reset: " << L0 << "\n";

    std::cout << "Label Reset: PASSED\n\n";
}

void testMultipleGenerators() {
    std::cout << "Testing Multiple Independent Generators...\n";

    TempVarGenerator gen1;
    TempVarGenerator gen2;

    // Gen1 generates some temps
    std::string g1_t0 = gen1.newTemp();
    std::string g1_t1 = gen1.newTemp();
    std::cout << "  Gen1: " << g1_t0 << ", " << g1_t1 << "\n";

    // Gen2 should have independent counter
    std::string g2_t0 = gen2.newTemp();
    std::string g2_t1 = gen2.newTemp();
    std::cout << "  Gen2: " << g2_t0 << ", " << g2_t1 << "\n";

    assert(g1_t0 == "t0" && g1_t1 == "t1");
    assert(g2_t0 == "t0" && g2_t1 == "t1");

    // More from gen1
    std::string g1_t2 = gen1.newTemp();
    std::cout << "  Gen1 continued: " << g1_t2 << "\n";
    assert(g1_t2 == "t2");

    // Gen2 should still be at t2
    std::string g2_t2 = gen2.newTemp();
    std::cout << "  Gen2 continued: " << g2_t2 << "\n";
    assert(g2_t2 == "t2");

    std::cout << "Multiple Independent Generators: PASSED\n\n";
}

void testRealWorldUsageExample() {
    std::cout << "Testing Real-World Usage Example...\n";
    std::cout << "Example: Generating IR for expression (a + b) * (c - d)\n\n";

    TempVarGenerator tempGen;

    // Simulate generating IR for: result = (a + b) * (c - d)

    // t0 = a + b
    std::string t0 = tempGen.newTemp();
    std::cout << "  " << t0 << " = add a, b\n";

    // t1 = c - d
    std::string t1 = tempGen.newTemp();
    std::cout << "  " << t1 << " = sub c, d\n";

    // t2 = t0 * t1
    std::string t2 = tempGen.newTemp();
    std::cout << "  " << t2 << " = mul " << t0 << ", " << t1 << "\n";

    // result = t2
    std::cout << "  result = move " << t2 << "\n\n";

    assert(t0 == "t0");
    assert(t1 == "t1");
    assert(t2 == "t2");
    assert(tempGen.getTempCount() == 3);

    std::cout << "  Total temporaries used: " << tempGen.getTempCount() << "\n";
    std::cout << "Real-World Usage Example: PASSED\n\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Temporary Variable Generator Test Suite\n";
    std::cout << "========================================\n\n";

    testBasicTempGeneration();
    testTempCounterTracking();
    testTempReset();
    testCustomPrefix();
    testTempSSAGeneration();
    testLabelGeneration();
    testLabelReset();
    testMultipleGenerators();
    testRealWorldUsageExample();

    std::cout << "========================================\n";
    std::cout << "All tests PASSED!\n";
    std::cout << "========================================\n";

    return 0;
}
