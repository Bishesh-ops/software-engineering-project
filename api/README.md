# C Compiler Visualization API

Flask-based HTTP API bridge that receives C source code from the React frontend, invokes the mycc compiler, and returns consolidated JSON with visualization data (tokens, AST, assembly, hex dump).

## Directory Structure

```
api/
├── app.py              # Main Flask application with endpoints
├── config.py           # Configuration (paths, timeouts, server settings)
├── requirements.txt    # Python dependencies
├── utils/
│   └── compiler.py     # Compiler invocation and output parsing logic
└── README.md           # This file
```

## Prerequisites

- Python 3.8 or higher
- The mycc compiler must be built (located at `../bin/mycc`)
- macOS or Linux (the compiler targets these platforms)

## Setup

### 1. Create Virtual Environment

```bash
# Navigate to the api directory
cd api

# Create virtual environment
python3 -m venv venv

# Activate virtual environment
# On macOS/Linux:
source venv/bin/activate

# On Windows (if supported):
# venv\Scripts\activate
```

### 2. Install Dependencies

```bash
# Make sure virtual environment is activated
pip install --upgrade pip
pip install -r requirements.txt
```

### 3. Verify Compiler

Ensure the compiler is built:

```bash
# From project root
make clean && make

# Verify compiler exists
ls -la bin/mycc
```

### 4. Run the API Server

```bash
# Make sure you're in the api/ directory with venv activated
python app.py
```

The server will start on `http://localhost:5001` by default.

## API Endpoints

### Health Check

**GET** `/health`

Check if the API is running and the compiler is accessible.

**Response:**
```json
{
  "status": "healthy",
  "compiler_path": "/path/to/bin/mycc",
  "compiler_exists": true
}
```

### API Information

**GET** `/api/info`

Get API information and configuration details.

**Response:**
```json
{
  "name": "C Compiler Visualization API",
  "version": "1.0.0",
  "endpoints": { ... },
  "config": {
    "compiler_path": "/path/to/bin/mycc",
    "temp_dir": "/path/to/tmp",
    "timeout_seconds": 30
  }
}
```

### Compile C Code

**POST** `/compile`

Compile C source code and return visualization data.

**Request Body:**
```json
{
  "source_code": "int main() { return 0; }",
  "filename": "main.c"  // optional, defaults to "input.c"
}
```

**Response (Success):**
```json
{
  "success": true,
  "tokens": [
    {
      "type": "KW_INT",
      "value": "int",
      "location": { "file": "input.c", "line": 1, "column": 1 }
    },
    ...
  ],
  "ast": {
    "stage": "parsing",
    "declaration_count": 1,
    "declarations": [ ... ]
  },
  "assembly": "    .section __TEXT,__text\n    .global _main\n...",
  "hex": "CF FA ED FE 0C 00 00 01 ...",
  "errors": [],
  "stdout": "...",
  "stderr": "",
  "return_code": 0
}
```

**Response (Compilation Error):**
```json
{
  "success": false,
  "tokens": { ... },  // May be present if lexing succeeded
  "ast": null,
  "assembly": null,
  "hex": null,
  "errors": [
    "input.c:5:10: error: expected ';' after statement"
  ],
  "stdout": "",
  "stderr": "...",
  "return_code": 1
}
```

## Configuration

Edit `config.py` to customize settings:

```python
class Config:
    COMPILER_PATH = str(PROJECT_ROOT / "bin" / "mycc")
    COMPILE_TIMEOUT = 30  # seconds
    HOST = "0.0.0.0"
    PORT = 5001
    DEBUG = True
```

## Testing with curl

```bash
# Health check
curl http://localhost:5001/health

# API info
curl http://localhost:5001/api/info

# Compile simple program
curl -X POST http://localhost:5001/compile \
  -H "Content-Type: application/json" \
  -d '{
    "source_code": "int main() { return 42; }"
  }'

# Compile with custom filename
curl -X POST http://localhost:5001/compile \
  -H "Content-Type: application/json" \
  -d '{
    "source_code": "int add(int a, int b) { return a + b; }",
    "filename": "math.c"
  }'
```

## Development

### Activating Virtual Environment

Always activate the virtual environment before working:

```bash
cd api
source venv/bin/activate
```

### Deactivating Virtual Environment

```bash
deactivate
```

### Installing New Dependencies

```bash
# Activate venv first
source venv/bin/activate

# Install package
pip install <package-name>

# Update requirements.txt
pip freeze > requirements.txt
```

### Running in Debug Mode

Debug mode is enabled by default in `config.py`. The server will auto-reload on code changes.

## Production Deployment

For production deployment:

1. Set `DEBUG = False` in `config.py`
2. Configure specific CORS origins
3. Use a production WSGI server like Gunicorn:

```bash
pip install gunicorn
gunicorn -w 4 -b 0.0.0.0:5001 app:app
```

## Troubleshooting

### "Compiler not found" error

- Make sure you've built the compiler: `make clean && make`
- Check that `bin/mycc` exists and is executable
- Verify `config.py` has the correct path

### "Address already in use" error

- Port 5001 is already in use
- Change `PORT` in `config.py` to a different value
- Or kill the process using the port: `lsof -ti:5001 | xargs kill`

### Import errors

- Make sure virtual environment is activated
- Reinstall dependencies: `pip install -r requirements.txt`

### CORS errors from frontend

- Check that `flask-cors` is installed
- Verify CORS configuration in `app.py`
- For specific origins, update `CORS_ORIGINS` in `config.py`

## Next Steps

After completing Task 2.1 (this initialization), the following tasks will be implemented:

- **Task 2.2**: Implement file handling and subprocess execution
- **Task 2.3**: Add JSON consolidation and response formatting
- **Task 2.4**: Add error handling and input validation
- **Task 2.5**: Write API tests

## License

Same as parent project (C Compiler Visualization Suite)
