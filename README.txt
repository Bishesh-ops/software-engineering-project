# C Compiler with Interactive Web Visualization

A C compiler (`mycc`) with a React-based visualization tool that shows the compilation pipeline in real-time.

## Quick Start

```bash
# Build the compiler (runs in Docker automatically)
make

# Start the API and frontend servers
make docker-dev

# Run with visualization dumps
./mycc program.c --dump-tokens tokens.json --dump-ast ast.json --dump-asm output.s -o program

# Get hex dump (works on Mac M1 via Docker)
./mycc program.c --dump-hex output.hex -o program
```

Open `http://localhost:5173` to use the visualization tool.

### Alternative: Local Build (Mac native - no hex dump support)
```bash
make native
./bin/mycc program.c --dump-tokens tokens.json --dump-ast ast.json -o program
```

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

- **Docker** (recommended - provides hex dump support on Mac M1)
- **Alternative:** g++/clang++ with C++17, Python 3.8+, Node.js 18+

## Useful Commands

```bash
make                  # Build compiler in Docker
make clean            # Clean build files
make docker-shell     # Open interactive shell in Docker container
make docker-dev       # Start API + frontend servers
./mycc <file>         # Run compiler (via Docker)
```
