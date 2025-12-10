import sys
import os
import json
from pathlib import Path

# Add api directory to path
sys.path.insert(0, os.path.join(os.getcwd(), "api"))

from utils.compiler import CompilerInvoker
from config import Config


def test_compiler_integration():
    print("Testing Compiler Integration...")

    # Check if compiler exists
    if not os.path.exists(Config.COMPILER_PATH):
        print(f"Error: Compiler not found at {Config.COMPILER_PATH}")
        return False

    invoker = CompilerInvoker(Config.COMPILER_PATH)

    source_code = """
    int main() {
        int x = 42;
        return x;
    }
    """

    print("Compiling sample code...")
    result = invoker.compile(source_code)

    if result["success"]:
        print("Compilation Successful!")
        print(f"Tokens: {len(result['tokens']) if result['tokens'] else 0} items")
        print(f"AST: {result['ast'] is not None}")
        print(f"Assembly: {len(result['assembly']) if result['assembly'] else 0} bytes")
        print(f"Hex: {len(result['hex']) if result['hex'] else 0} bytes")
        return True
    else:
        print("Compilation Failed!")
        print("Errors:", result["errors"])
        print("Stderr:", result["stderr"])
        return False


if __name__ == "__main__":
    success = test_compiler_integration()
    sys.exit(0 if success else 1)
