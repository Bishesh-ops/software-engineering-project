"""
Flask API Bridge for C Compiler Visualization

This module provides an HTTP server that:
1. Receives C source code from the React frontend
2. Invokes the mycc compiler with dump flags
3. Returns consolidated JSON with tokens, AST, assembly, and hex dumps
"""

from flask import Flask, request, jsonify
from flask_cors import CORS
import os
import sys

# Add parent directory to path to import config and utils
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from config import Config
from utils.compiler import CompilerInvoker

app = Flask(__name__)

# Configure CORS for React development server
# Development: Allow localhost:3000 (React dev server)
# Production: Should be restricted to specific domain
cors_config = {
    "origins": Config.CORS_ORIGINS,
    "methods": ["GET", "POST", "OPTIONS"],
    "allow_headers": ["Content-Type"],
    "supports_credentials": False,
    "max_age": 3600,  # Cache preflight requests for 1 hour
}

CORS(app, resources={r"/*": cors_config})

# Initialize compiler invoker
compiler = CompilerInvoker(Config.COMPILER_PATH)


@app.route("/health", methods=["GET"])
def health_check():
    """Health check endpoint to verify API is running."""
    return (
        jsonify(
            {
                "status": "healthy",
                "compiler_path": Config.COMPILER_PATH,
                "compiler_exists": os.path.exists(Config.COMPILER_PATH),
            }
        ),
        200,
    )


@app.route("/compile", methods=["POST"])
def compile_code():
    """
    Compile C source code and return visualization data.

    Expected JSON payload:
    {
        "source": "int main() { return 42; }"
    }

    Returns JSON with:
    Success Response:
    {
        "success": true,
        "tokens": [...],        # Parsed from tokens.json
        "ast": {...},           # Parsed from ast.json
        "assembly": "...",      # Raw assembly text
        "hexdump": "...",       # Raw hex string
        "logs": {
            "stdout": "...",
            "stderr": ""
        }
    }

    Error Response:
    {
        "success": false,
        "error": "Syntax error at line 5...",
        "tokens": {...},        # Partial results if available
        "ast": {...},           # Partial results if available
        "logs": {
            "stdout": "...",
            "stderr": "..."
        }
    }
    """
    try:
        # Parse request - Handle invalid JSON
        data = request.get_json(force=False, silent=False)

        if data is None:
            return (
                jsonify({"success": False, "error": "Invalid JSON in request body"}),
                400,
            )

        if "source" not in data:
            return (
                jsonify(
                    {
                        "success": False,
                        "error": 'Missing "source" field in request body',
                    }
                ),
                400,
            )

        source_code = data["source"]

        # Validate source code is not empty
        if not source_code or not source_code.strip():
            return (
                jsonify({"success": False, "error": "Source code cannot be empty"}),
                400,
            )

        # Validate source code size
        if len(source_code) > Config.MAX_SOURCE_SIZE:
            return (
                jsonify(
                    {
                        "success": False,
                        "error": f"Source code too large: {len(source_code)} bytes (max: {Config.MAX_SOURCE_SIZE} bytes)",
                    }
                ),
                400,
            )

        # Invoke compiler with default filename
        result = compiler.compile(source_code, filename="input.c")

        # Check for timeout specifically
        if (
            result.get("return_code") == -1
            and "timeout" in str(result.get("errors", [])).lower()
        ):
            # Compilation timeout - return 408
            return (
                jsonify(
                    {
                        "success": False,
                        "error": result.get("errors", ["Compilation timeout"])[0],
                        "logs": {
                            "stdout": result.get("stdout", ""),
                            "stderr": result.get("stderr", ""),
                        },
                    }
                ),
                408,
            )

        # Transform result to match API contract
        if result["success"]:
            # Successful compilation
            response = {
                "success": True,
                "tokens": result.get("tokens"),
                "ast": result.get("ast"),
                "assembly": result.get("assembly"),
                "hexdump": result.get("hex"),
                "logs": {
                    "stdout": result.get("stdout", ""),
                    "stderr": result.get("stderr", ""),
                },
            }
            return jsonify(response), 200
        else:
            # Compilation failed - return partial results with error
            error_msg = (
                result.get("errors", ["Unknown compilation error"])[0]
                if result.get("errors")
                else "Compilation failed"
            )

            response = {
                "success": False,
                "error": error_msg,
                "tokens": result.get(
                    "tokens"
                ),  # Partial result: may have tokens even if failed
                "ast": result.get(
                    "ast"
                ),  # Partial result: may have AST even if codegen failed
                "assembly": result.get(
                    "assembly"
                ),  # Partial result: may have assembly even if linking failed
                "logs": {
                    "stdout": result.get("stdout", ""),
                    "stderr": result.get("stderr", ""),
                },
            }
            return jsonify(response), 400

    except ValueError as e:
        # Invalid JSON format
        return (
            jsonify(
                {"success": False, "error": f"Invalid JSON in request body: {str(e)}"}
            ),
            400,
        )

    except FileNotFoundError as e:
        # Compiler binary not found
        return (
            jsonify(
                {"success": False, "error": f"Compiler binary not found: {str(e)}"}
            ),
            500,
        )

    except Exception as e:
        # Internal server error - log and return 500
        import traceback

        print(f"Internal server error: {str(e)}")
        print(traceback.format_exc())

        return (
            jsonify({"success": False, "error": f"Internal server error: {str(e)}"}),
            500,
        )


@app.route("/api/info", methods=["GET"])
def api_info():
    """Get API information and configuration."""
    return (
        jsonify(
            {
                "name": "C Compiler Visualization API",
                "version": "1.0.0",
                "endpoints": {
                    "/health": "GET - Health check",
                    "/compile": "POST - Compile C code and get visualization data",
                    "/api/info": "GET - API information",
                },
                "config": {
                    "compiler_path": Config.COMPILER_PATH,
                    "temp_dir": Config.TEMP_DIR,
                    "timeout_seconds": Config.COMPILE_TIMEOUT,
                },
            }
        ),
        200,
    )


if __name__ == "__main__":
    # Development server
    app.run(host=Config.HOST, port=Config.PORT, debug=Config.DEBUG)
