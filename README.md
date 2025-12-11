# C Compiler with Interactive Web Visualization

A C compiler (`mycc`) with a React-based visualization tool that shows the compilation pipeline in real-time.

## Quick Start

```bash
# Build the compiler
make

# Run with visualization dumps
./bin/mycc program.c --dump-tokens tokens.json --dump-ast ast.json --dump-asm output.s -o program

# Start the API server
cd api && source venv/bin/activate && python app.py

# Start the frontend (separate terminal)
cd frontend && npm run dev
```

Open `http://localhost:5173` to use the visualization tool.

## Project Structure

```
├── src/          # C++ compiler source
├── include/      # C++ headers
├── bin/mycc      # Compiled compiler
├── api/          # Flask API bridge
└── frontend/     # React visualization app
```

## Compiler Flags

| Flag | Description |
|------|-------------|
| `-o <file>` | Output executable name |
| `--dump-tokens <file>` | Export tokens as JSON |
| `--dump-ast <file>` | Export AST as JSON |
| `--dump-asm <file>` | Export assembly |
| `--dump-hex <file>` | Export binary hex |

## Architecture

```
React Frontend → Flask API → mycc Compiler → JSON Dumps
     ↓              ↓            ↓
  Monaco      POST /compile   Lexer → Parser → Semantic → IR → Codegen
  Editor                              ↓
                              Token/AST/Assembly visualization
```

## Requirements

- **Compiler:** g++/clang++ with C++17
- **API:** Python 3.8+, Flask
- **Frontend:** Node.js 18+
