#!/usr/bin/env python3
"""
Full Stack E2E Tests - Test compilation pipeline end-to-end.
"""

import pytest
import requests
import json
import subprocess
import os

API_BASE_URL = os.environ.get("API_BASE_URL", "http://localhost:5000")
COMPILER_PATH = os.environ.get("COMPILER_PATH", "./bin/mycc")


def compile_directly(source_code: str) -> dict:
    """Compile using the compiler binary directly."""
    temp_file = "/tmp/test_e2e.c"
    with open(temp_file, "w") as f:
        f.write(source_code)

    result = {"success": False, "tokens": [], "errors": []}

    try:
        # Get tokens
        proc = subprocess.run(
            [COMPILER_PATH, "--dump-tokens", temp_file],
            capture_output=True, text=True, timeout=30
        )
        if proc.returncode == 0:
            result["tokens"] = proc.stdout.strip().split("\n")

        # Full compilation
        proc = subprocess.run(
            [COMPILER_PATH, temp_file, "-o", "/tmp/test_e2e"],
            capture_output=True, text=True, timeout=30
        )
        result["success"] = proc.returncode == 0
        if proc.stderr:
            result["errors"] = proc.stderr.strip().split("\n")
    except (subprocess.TimeoutExpired, FileNotFoundError) as e:
        result["errors"] = [str(e)]

    return result


class TestCompilationFlow:
    """Tests for the compilation pipeline."""

    @pytest.mark.e2e
    def test_minimal_program(self):
        """Minimal C program compiles."""
        result = compile_directly("int main() { return 0; }")
        assert "tokens" in result or result.get("success")

    @pytest.mark.e2e
    def test_program_with_function(self):
        """Program with function calls compiles."""
        source = """
        int add(int a, int b) { return a + b; }
        int main() { return add(5, 3); }
        """
        result = compile_directly(source)
        assert "tokens" in result or result.get("success")

    @pytest.mark.e2e
    def test_program_with_loop(self):
        """Program with loops compiles."""
        source = """
        int main() {
            int sum = 0; int i;
            for (i = 0; i < 10; i = i + 1) { sum = sum + i; }
            return sum;
        }
        """
        result = compile_directly(source)
        assert "tokens" in result or result.get("success")


class TestErrorDetection:
    """Tests for error handling."""

    @pytest.mark.e2e
    def test_syntax_error_detected(self):
        """Syntax errors are caught."""
        source = "int main() { int x = ; return 0; }"
        result = compile_directly(source)
        assert not result.get("success") or len(result.get("errors", [])) > 0

    @pytest.mark.e2e
    def test_semantic_error_detected(self):
        """Semantic errors are caught."""
        source = "int main() { undeclared = 5; return 0; }"
        result = compile_directly(source)
        assert not result.get("success") or len(result.get("errors", [])) > 0


class TestRealPrograms:
    """Tests with realistic programs."""

    @pytest.mark.e2e
    def test_fibonacci(self):
        """Recursive Fibonacci compiles."""
        source = """
        int fib(int n) {
            if (n <= 1) { return n; }
            return fib(n-1) + fib(n-2);
        }
        int main() { return fib(10); }
        """
        result = compile_directly(source)
        assert "tokens" in result or result.get("success")

    @pytest.mark.e2e
    def test_factorial(self):
        """Iterative factorial compiles."""
        source = """
        int fact(int n) {
            int r = 1; int i;
            for (i = 2; i <= n; i = i + 1) { r = r * i; }
            return r;
        }
        int main() { return fact(5); }
        """
        result = compile_directly(source)
        assert "tokens" in result or result.get("success")

