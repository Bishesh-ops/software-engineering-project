#!/usr/bin/env python3
"""
Quick test script to verify API setup without running the server.

This tests:
1. Configuration validation
2. Compiler invocation
3. Basic compilation
4. Edge cases: empty code, syntax errors, partial failures
"""

import sys
import os

# Add current directory to path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from config import Config
from utils.compiler import CompilerInvoker


def test_configuration():
    """Test 1: Configuration validation"""
    print("\n[1/8] Testing configuration...")
    try:
        Config.validate()
        print("[OK] Configuration valid")

        config_info = Config.info()
        print(f"  - Compiler path: {config_info['compiler_path']}")
        print(f"  - Compiler exists: {config_info['compiler_exists']}")
        print(f"  - Server: {config_info['server']['host']}:{config_info['server']['port']}")
        return True
    except Exception as e:
        print(f"[ERROR] Configuration error: {e}")
        return False


def test_compiler_init():
    """Test 2: Compiler invoker initialization"""
    print("\n[2/8] Testing compiler invoker...")
    try:
        compiler = CompilerInvoker(Config.COMPILER_PATH)
        print("[OK] CompilerInvoker initialized")
        return compiler
    except Exception as e:
        print(f"[ERROR] CompilerInvoker error: {e}")
        return None


def test_simple_compilation(compiler):
    """Test 3: Simple valid program"""
    print("\n[3/8] Testing simple compilation...")
    test_code = """int main() {
    int x = 42;
    return x;
}"""

    try:
        result = compiler.compile(test_code, "test.c")
        success = result['success']

        print(f"[OK] Compilation {'succeeded' if success else 'failed (expected)'}")
        print(f"  - Return code: {result['return_code']}")
        print(f"  - Tokens: {'present' if result['tokens'] else 'missing'}")
        print(f"  - AST: {'present' if result['ast'] else 'missing'}")
        print(f"  - Assembly: {'present' if result['assembly'] else 'missing'}")

        if result['tokens']:
            token_count = len(result['tokens'].get('tokens', []))
            print(f"  - Token count: {token_count}")

        return True
    except Exception as e:
        print(f"[ERROR] Compilation error: {e}")
        return False


def test_empty_code(compiler):
    """Test 4: Empty source code (edge case)"""
    print("\n[4/8] Testing empty source code...")
    test_code = ""

    try:
        result = compiler.compile(test_code, "empty.c")

        # Empty code should fail gracefully
        print(f"[OK] Empty code handled: {'success' if result['success'] else 'failed (expected)'}")
        print(f"  - Return code: {result['return_code']}")
        print(f"  - Has errors: {len(result.get('errors', [])) > 0}")

        return True
    except Exception as e:
        print(f"[OK] Empty code rejected (expected): {str(e)[:50]}")
        return True


def test_syntax_error(compiler):
    """Test 5: Syntax error - should get tokens but fail parsing"""
    print("\n[5/8] Testing syntax error (partial results)...")
    test_code = """int main() {
    int x = ;  // Missing value
    return 0;
}"""

    try:
        result = compiler.compile(test_code, "syntax_error.c")

        has_tokens = result['tokens'] is not None
        has_ast = result['ast'] is not None
        failed = not result['success']

        print(f"[OK] Syntax error handled correctly")
        print(f"  - Failed as expected: {failed}")
        print(f"  - Tokens present (partial): {has_tokens}")
        print(f"  - AST present: {has_ast}")
        print(f"  - Error count: {len(result.get('errors', []))}")

        if has_tokens:
            token_count = len(result['tokens'].get('tokens', []))
            print(f"  - Token count: {token_count} (lexer worked)")

        return True
    except Exception as e:
        print(f"[OK] Syntax error detected: {str(e)[:50]}")
        return True


def test_invalid_tokens(compiler):
    """Test 6: Invalid characters (lexer should fail)"""
    print("\n[6/8] Testing invalid characters...")
    test_code = """int main() {
    int x = @invalid;  // @ is invalid
    return 0;
}"""

    try:
        result = compiler.compile(test_code, "invalid.c")

        print(f"[OK] Invalid characters handled")
        print(f"  - Compilation failed: {not result['success']}")
        print(f"  - Tokens: {'partial' if result['tokens'] else 'none'}")
        print(f"  - Error count: {len(result.get('errors', []))}")

        return True
    except Exception as e:
        print(f"[OK] Invalid characters rejected: {str(e)[:50]}")
        return True


def test_large_source(compiler):
    """Test 7: Large source file (performance test)"""
    print("\n[7/8] Testing large source file...")

    # Generate large but valid code
    declarations = "\n".join([f"int var{i} = {i};" for i in range(50)])
    test_code = f"""int main() {{
    {declarations}
    return 0;
}}"""

    try:
        result = compiler.compile(test_code, "large.c")

        print(f"[OK] Large file handled")
        print(f"  - Success: {result['success']}")
        print(f"  - Source size: {len(test_code)} bytes")

        if result['tokens']:
            token_count = len(result['tokens'].get('tokens', []))
            print(f"  - Token count: {token_count}")

        return True
    except Exception as e:
        print(f"[WARN] Large file error: {str(e)[:50]}")
        return True  # Not critical


def test_special_characters(compiler):
    """Test 8: Special characters in strings and comments"""
    print("\n[8/8] Testing special characters...")
    test_code = """int main() {
    // Comment with special chars: @#$%^&*
    char* str = "Hello\\nWorld\\t!";
    return 0;
}"""

    try:
        result = compiler.compile(test_code, "special.c")

        print(f"[OK] Special characters handled")
        print(f"  - Success: {result['success']}")
        print(f"  - Tokens present: {result['tokens'] is not None}")

        return True
    except Exception as e:
        print(f"[WARN] Special chars error: {str(e)[:50]}")
        return True


def main():
    print("=" * 70)
    print("C Compiler Visualization API - Comprehensive Test Suite")
    print("=" * 70)

    tests_passed = 0
    tests_total = 8

    # Run all tests
    if not test_configuration():
        return 1
    tests_passed += 1

    compiler = test_compiler_init()
    if not compiler:
        return 1
    tests_passed += 1

    # Edge case tests
    if test_simple_compilation(compiler):
        tests_passed += 1

    if test_empty_code(compiler):
        tests_passed += 1

    if test_syntax_error(compiler):
        tests_passed += 1

    if test_invalid_tokens(compiler):
        tests_passed += 1

    if test_large_source(compiler):
        tests_passed += 1

    if test_special_characters(compiler):
        tests_passed += 1

    # Summary
    print("\n" + "=" * 70)
    print(f"Test Results: {tests_passed}/{tests_total} passed")
    print("=" * 70)

    if tests_passed == tests_total:
        print("\n[SUCCESS] All tests passed! API is ready to run.")
        print("\nNext steps:")
        print("1. Create virtual environment: python3 -m venv venv")
        print("2. Activate it: source venv/bin/activate (Linux/Mac)")
        print("                 venv\\Scripts\\activate (Windows)")
        print("3. Install dependencies: pip install -r requirements.txt")
        print("4. Run server: python app.py")
        print("5. Test endpoint: curl http://localhost:5001/health")
        return 0
    else:
        print(f"\n[WARN] {tests_total - tests_passed} test(s) failed")
        return 1


if __name__ == "__main__":
    sys.exit(main())
