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
CORS(app)  # Enable CORS for React frontend

# Initialize compiler invoker
compiler = CompilerInvoker(Config.COMPILER_PATH)


@app.route('/health', methods=['GET'])
def health_check():
    """Health check endpoint to verify API is running."""
    return jsonify({
        'status': 'healthy',
        'compiler_path': Config.COMPILER_PATH,
        'compiler_exists': os.path.exists(Config.COMPILER_PATH)
    }), 200


@app.route('/compile', methods=['POST'])
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
        "logs": {
            "stdout": "...",
            "stderr": "..."
        }
    }
    """
    try:
        # Parse request
        data = request.get_json()

        if not data or 'source' not in data:
            return jsonify({
                'success': False,
                'error': 'Missing "source" field in request body'
            }), 400

        source_code = data['source']

        # Invoke compiler with default filename
        result = compiler.compile(source_code, filename='input.c')

        # Transform result to match API contract
        if result['success']:
            response = {
                'success': True,
                'tokens': result.get('tokens'),
                'ast': result.get('ast'),
                'assembly': result.get('assembly'),
                'hexdump': result.get('hex'),
                'logs': {
                    'stdout': result.get('stdout', ''),
                    'stderr': result.get('stderr', '')
                }
            }
            return jsonify(response), 200
        else:
            # Compilation failed - return error response
            error_msg = result.get('errors', ['Unknown compilation error'])[0] if result.get('errors') else 'Compilation failed'
            response = {
                'success': False,
                'error': error_msg,
                'logs': {
                    'stdout': result.get('stdout', ''),
                    'stderr': result.get('stderr', '')
                }
            }
            return jsonify(response), 400

    except Exception as e:
        return jsonify({
            'success': False,
            'error': f'Internal server error: {str(e)}'
        }), 500


@app.route('/api/info', methods=['GET'])
def api_info():
    """Get API information and configuration."""
    return jsonify({
        'name': 'C Compiler Visualization API',
        'version': '1.0.0',
        'endpoints': {
            '/health': 'GET - Health check',
            '/compile': 'POST - Compile C code and get visualization data',
            '/api/info': 'GET - API information'
        },
        'config': {
            'compiler_path': Config.COMPILER_PATH,
            'temp_dir': Config.TEMP_DIR,
            'timeout_seconds': Config.COMPILE_TIMEOUT
        }
    }), 200


if __name__ == '__main__':
    # Development server
    app.run(
        host=Config.HOST,
        port=Config.PORT,
        debug=Config.DEBUG
    )
