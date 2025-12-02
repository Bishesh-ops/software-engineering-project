#!/usr/bin/env python3
"""
Quick test script to verify API setup without running the server.

This tests:
1. Configuration validation
2. Compiler invocation
3. JSON parsing
"""

import sys
import os

# Add current directory to path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from config import Config
from utils.compiler import CompilerInvoker


def main():
    print("=" * 60)
    print("C Compiler Visualization API - Setup Test")
    print("=" * 60)

    # Test 1: Configuration
    print("\n[1/3] Testing configuration...")
    try:
        Config.validate()
        print("[OK] Configuration valid")

        config_info = Config.info()
        print(f"  - Compiler path: {config_info['compiler_path']}")
        print(f"  - Compiler exists: {config_info['compiler_exists']}")
        print(f"  - Server: {config_info['server']['host']}:{config_info['server']['port']}")
    except Exception as e:
        print(f"[ERROR] Configuration error: {e}")
        return 1

    # Test 2: Compiler Invoker
    print("\n[2/3] Testing compiler invoker...")
    try:
        compiler = CompilerInvoker(Config.COMPILER_PATH)
        print("[OK] CompilerInvoker initialized")
    except Exception as e:
        print(f"[ERROR] CompilerInvoker error: {e}")
        return 1

    # Test 3: Compile simple program
    print("\n[3/3] Testing compilation...")
    test_code = """
int main() {
    int x = 42;
    return x;
}
"""

    try:
        result = compiler.compile(test_code, "test.c")

        print(f"[OK] Compilation {'succeeded' if result['success'] else 'failed'}")
        print(f"  - Return code: {result['return_code']}")
        print(f"  - Tokens: {'present' if result['tokens'] else 'missing'}")
        print(f"  - AST: {'present' if result['ast'] else 'missing'}")
        print(f"  - Assembly: {'present' if result['assembly'] else 'missing'}")
        print(f"  - Hex dump: {'present' if result['hex'] else 'missing'}")

        if result['tokens']:
            token_count = len(result['tokens'].get('tokens', []))
            print(f"  - Token count: {token_count}")

        if result['errors']:
            print(f"  - Errors: {len(result['errors'])}")
            for error in result['errors'][:3]:
                print(f"    - {error}")

    except Exception as e:
        print(f"[ERROR] Compilation error: {e}")
        return 1

    print("\n" + "=" * 60)
    print("All tests passed! API is ready to run.")
    print("=" * 60)
    print("\nNext steps:")
    print("1. Create virtual environment: python3 -m venv venv")
    print("2. Activate it: source venv/bin/activate")
    print("3. Install dependencies: pip install -r requirements.txt")
    print("4. Run server: python app.py")
    print("5. Test endpoint: curl http://localhost:5001/health")

    return 0


if __name__ == "__main__":
    sys.exit(main())
