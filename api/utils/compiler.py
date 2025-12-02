"""
Compiler Invocation Logic

This module handles:
1. Creating temporary files for source code
2. Invoking the mycc compiler with appropriate dump flags
3. Reading and parsing all dump outputs
4. Cleaning up temporary files
5. Error handling and timeout management
"""

import os
import subprocess
import tempfile
import json
from pathlib import Path
from typing import Dict, Any, Optional


class CompilerInvoker:
    """Handles invocation of the mycc C compiler and collection of dump outputs."""

    def __init__(self, compiler_path: str):
        """
        Initialize the compiler invoker.

        Args:
            compiler_path: Absolute path to the mycc compiler executable
        """
        self.compiler_path = compiler_path

        if not os.path.exists(compiler_path):
            raise FileNotFoundError(f"Compiler not found at: {compiler_path}")

        if not os.access(compiler_path, os.X_OK):
            raise PermissionError(f"Compiler not executable: {compiler_path}")

    def compile(
        self,
        source_code: str,
        filename: str = "input.c",
        timeout: int = 30
    ) -> Dict[str, Any]:
        """
        Compile C source code and collect all visualization dumps.

        Args:
            source_code: C source code as string
            filename: Name for the temporary source file
            timeout: Compilation timeout in seconds

        Returns:
            Dictionary containing:
            {
                "success": bool,
                "tokens": dict or None,
                "ast": dict or None,
                "assembly": str or None,
                "hex": str or None,
                "errors": list of str,
                "stdout": str,
                "stderr": str
            }
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_path = Path(temp_dir)

            # File paths
            source_file = temp_path / filename
            tokens_file = temp_path / "tokens.json"
            ast_file = temp_path / "ast.json"
            assembly_file = temp_path / "assembly.s"
            hex_file = temp_path / "executable.hex"
            output_exe = temp_path / "output"

            # Write source code to file
            source_file.write_text(source_code)

            # Build compiler command with absolute paths
            cmd = [
                self.compiler_path,
                str(source_file),
                "--dump-tokens", str(tokens_file),
                "--dump-ast", str(ast_file),
                "--dump-asm", str(assembly_file),
                "--dump-hex", str(hex_file),
                "-o", str(output_exe)
            ]

            # Execute compiler
            # Remove TMPDIR from environment to prevent path doubling
            # The compiler's getTempFileName() prepends $TMPDIR, but we're providing
            # absolute paths for all dumps. If TMPDIR points to a temp directory
            # and we pass absolute paths from that same temp directory, we get
            # doubled paths like: /tmp/xxx//tmp/xxx/file.s
            # Solution: Remove TMPDIR so compiler uses /tmp as default
            try:
                env = os.environ.copy()
                if 'TMPDIR' in env:
                    del env['TMPDIR']  # Let compiler use default /tmp

                result = subprocess.run(
                    cmd,
                    capture_output=True,
                    text=True,
                    timeout=timeout,
                    env=env
                )

                # Parse outputs
                tokens = self._read_json_file(tokens_file)
                ast = self._read_json_file(ast_file)
                assembly = self._read_text_file(assembly_file)
                hex_dump = self._read_text_file(hex_file)

                # Determine success
                success = result.returncode == 0

                # Collect errors from stderr
                errors = self._parse_errors(result.stderr)

                return {
                    "success": success,
                    "tokens": tokens,
                    "ast": ast,
                    "assembly": assembly,
                    "hex": hex_dump,
                    "errors": errors,
                    "stdout": result.stdout,
                    "stderr": result.stderr,
                    "return_code": result.returncode
                }

            except subprocess.TimeoutExpired:
                return {
                    "success": False,
                    "tokens": None,
                    "ast": None,
                    "assembly": None,
                    "hex": None,
                    "errors": [f"Compilation timeout after {timeout} seconds"],
                    "stdout": "",
                    "stderr": "",
                    "return_code": -1
                }

            except Exception as e:
                return {
                    "success": False,
                    "tokens": None,
                    "ast": None,
                    "assembly": None,
                    "hex": None,
                    "errors": [f"Unexpected error: {str(e)}"],
                    "stdout": "",
                    "stderr": str(e),
                    "return_code": -1
                }

    def _read_json_file(self, filepath: Path) -> Optional[Dict]:
        """
        Read and parse a JSON file.

        Args:
            filepath: Path to JSON file

        Returns:
            Parsed JSON as dict, or None if file doesn't exist or is invalid
        """
        try:
            if filepath.exists():
                with open(filepath, 'r') as f:
                    return json.load(f)
        except (json.JSONDecodeError, IOError) as e:
            print(f"Warning: Failed to read JSON file {filepath}: {e}")

        return None

    def _read_text_file(self, filepath: Path) -> Optional[str]:
        """
        Read a text file.

        Args:
            filepath: Path to text file

        Returns:
            File contents as string, or None if file doesn't exist
        """
        try:
            if filepath.exists():
                return filepath.read_text()
        except IOError as e:
            print(f"Warning: Failed to read text file {filepath}: {e}")

        return None

    def _parse_errors(self, stderr: str) -> list:
        """
        Parse compiler error messages from stderr.

        Args:
            stderr: Standard error output from compiler

        Returns:
            List of error message strings
        """
        if not stderr.strip():
            return []

        # Split by lines and filter out empty lines
        errors = [line.strip() for line in stderr.split('\n') if line.strip()]

        return errors
