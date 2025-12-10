# Component Integration Diagram

## Visual Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                           App.jsx                                │
│  - Health check on mount                                         │
│  - Scanline overlay                                              │
└───────────────────────────┬─────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│                         Layout.jsx                               │
│  - Main container                                                │
│  - Manages editor ref                                            │
│  - Connects all state selectors                                  │
└─────┬──────────────┬────────────────┬─────────────┬─────────────┘
      │              │                │             │
      ▼              ▼                ▼             ▼
┌──────────┐  ┌──────────┐  ┌─────────────┐  ┌──────────┐
│ Header   │  │  Code    │  │   Output    │  │  Bottom  │
│          │  │  Editor  │  │   Panel     │  │   Bar    │
└──────────┘  └─────┬────┘  └──────┬──────┘  └────┬─────┘
                    │              │              │
                    │              │              │
              ┌─────┴──────┐  ┌────┴──────┐  ┌───┴─────┐
              │ Monaco     │  │  Tabs:    │  │  Log    │
              │ Editor     │  │ - Tokens  │  │ Display │
              └────────────┘  │ - AST     │  └─────────┘
                              │ - Assembly│
                              │ - Hex     │
                              └───────────┘
```

## State Management Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                    compilerStore.js (Zustand)                    │
│                                                                   │
│  State:                          Actions:                        │
│  ├─ source                       ├─ setSource()                  │
│  ├─ isCompiling                  ├─ compile()                    │
│  ├─ compilationStatus            ├─ setActiveTab()               │
│  ├─ result                       ├─ addLog()                     │
│  ├─ error                        ├─ clearLogs()                  │
│  ├─ activeTab                    ├─ resetResults()               │
│  ├─ logs                         └─ reset()                      │
│  └─ editorSettings                                               │
│                                                                   │
│  Selectors (Subscriptions):                                      │
│  ├─ useSource()                  ← CodeEditor                    │
│  ├─ useCompilationStatus()       ← BottomBar                     │
│  ├─ useCompilationResults()      ← OutputPanel                   │
│  ├─ useActiveTab()               ← OutputPanel                   │
│  ├─ useLogs()                    ← BottomBar                     │
│  ├─ useError()                   ← BottomBar                     │
│  └─ useCompilerActions()         ← Layout, CodeEditor, BottomBar │
└─────────────────────────────────────────────────────────────────┘
```

## Compilation Flow (Happy Path)

```
User Action: Click "COMPILE" or press Ctrl+Enter
     │
     ▼
[BottomBar] or [CodeEditor]
     │ onCompile()
     ▼
[Layout] compile action
     │
     ▼
[compilerStore] compile() async action
     │
     ├─ Set isCompiling: true
     ├─ Set status: 'compiling'
     ├─ Clear error
     └─ Add log: "Starting compilation..."
     │
     ▼
[compilerService] compileCodeWithRetry(source)
     │
     ├─ Retry 1: POST /compile
     │   ├─ Timeout: 30s
     │   └─ AbortController
     │
     ├─ On Failure: Wait & Retry 2
     │
     └─ On Success: Return result
     │
     ▼
[compilerStore] Update state
     │
     ├─ Set isCompiling: false
     ├─ Set status: 'success'
     ├─ Set result: { tokens, ast, assembly, hex }
     └─ Add log: "✅ Compilation successful!"
     │
     ▼
[Components Re-render]
     │
     ├─ [OutputPanel] Shows new data in tabs
     ├─ [BottomBar] Shows "SUCCESS" status
     └─ [BottomBar] Shows success log
```

## Error Flow

```
Compilation fails (syntax error, network error, etc.)
     │
     ▼
[compilerService] catch block
     │
     ├─ formatErrorMessage(error)
     └─ Throw APIError
     │
     ▼
[compilerStore] catch block in compile()
     │
     ├─ Set isCompiling: false
     ├─ Set status: 'error'
     ├─ Set error: { message, details, status }
     └─ Add log: error message
     │
     ▼
[Components Re-render]
     │
     ├─ [BottomBar] Shows "FAILED" status
     ├─ [BottomBar] Shows error count badge
     ├─ [LogDisplay] Shows error in red
     └─ [LogDisplay] Clickable to jump to line
     │
     ▼
User clicks error in LogDisplay
     │
     ▼
[LogDisplay] onScrollToLine(lineNumber)
     │
     ▼
[BottomBar] passes to Layout
     │
     ▼
[Layout] handleScrollToLine(line)
     │
     ▼
[CodeEditor] editorRef.current.revealLineInCenter(line)
     │
     ▼
Monaco Editor scrolls and focuses line
```

## Data Persistence Flow

```
[User Action] Edit code, change tab, etc.
     │
     ▼
[compilerStore] State update
     │
     ▼
[Zustand Persist Middleware]
     │
     ├─ Filter state (partialize)
     │   ├─ Include: source
     │   ├─ Include: editorSettings
     │   ├─ Include: activeTab
     │   ├─ Exclude: result
     │   ├─ Exclude: logs
     │   └─ Exclude: isCompiling, error
     │
     ▼
localStorage.setItem('compiler-storage', JSON.stringify(state))
     │
     ▼
[Page Reload]
     │
     ▼
[compilerStore] Initialize
     │
     ▼
[Zustand Persist Middleware]
     │
     ▼
localStorage.getItem('compiler-storage')
     │
     ▼
Restore: source, editorSettings, activeTab
```

## API Integration

```
Frontend                          Backend (Flask)
────────                          ───────────────

[compilerService.js]
     │
     │ POST /compile
     │ Content-Type: application/json
     │ Body: { "source_code": "..." }
     ├──────────────────────────────────────────►
     │                                          [app.py]
     │                                             │
     │                                          [compile_code()]
     │                                             │
     │                                          ├─ Write source to temp file
     │                                          ├─ Run: ./bin/mycc --dump-tokens ...
     │                                          ├─ Read: tokens.json, ast.json, etc.
     │                                          └─ Return JSON response
     │
     │ Response:
     │ {
     │   "success": true,
     │   "tokens": [...],
     │   "ast": {...},
     │   "assembly": "...",
     │   "hex": "...",
     │   "stdout": "...",
     │   "stderr": "..."
     │ }
     ◄──────────────────────────────────────────┤
     │
     ▼
Parse & validate response
     │
     ▼
Return to compilerStore
```

## Component Communication Patterns

### Parent → Child (Props)
```
Layout
  ├─► CodeEditor: code, onChange, onCompile, editorRef
  ├─► OutputPanel: data
  └─► BottomBar: onCompile, isCompiling, status, logs, error, onScrollToLine
```

### Child → Parent (Callbacks)
```
CodeEditor
  └─ onChange(newCode) ──► Layout ──► setSource(newCode)
  └─ onCompile() ──────► Layout ──► compile()

LogDisplay
  └─ onScrollToLine(line) ──► BottomBar ──► Layout ──► handleScrollToLine()
```

### Sibling Communication (via Store)
```
CodeEditor
  └─ setSource(newCode)
       ↓
  [compilerStore]
       ↓
  OutputPanel (receives updated data after compile)

BottomBar
  └─ setActiveTab('ast')
       ↓
  [compilerStore]
       ↓
  OutputPanel (switches to AST view)
```

## Event Timeline (Typical Session)

```
Time  Event                           Component         State Update
────  ─────────────────────────────   ───────────────   ──────────────
0ms   User opens app                  App.jsx           -
10ms  Health check initiated          App.jsx           -
15ms  Store initialized               compilerStore     localStorage loaded
20ms  Components render               All               Initial render
100ms Health check completes          App.jsx           Console log

2s    User types "int main() {"       CodeEditor        source updated
3s    User types more code            CodeEditor        source updated

5s    User clicks COMPILE             BottomBar         isCompiling: true
5ms   API request sent                compilerService   -
5ms   Loading state shown             BottomBar         status: 'compiling'

500ms API response received           compilerService   -
501ms State updated                   compilerStore     result populated
502ms Components re-render            OutputPanel       Shows data
502ms Success log added               BottomBar         "✅ Compilation successful!"

10s   User switches to AST tab        OutputPanel       activeTab: 'ast'
10ms  AST view rendered               ASTVisualization  -

15s   User exports AST                ASTVisualization  -
15ms  File download triggered         Browser           -

20s   User introduces syntax error    CodeEditor        source updated
21s   User compiles again             BottomBar         isCompiling: true
22s   Error returned                  compilerService   -
22ms  Error state shown               BottomBar         status: 'error'
22ms  Error displayed                 LogDisplay        error object

25s   User clicks error               LogDisplay        -
25ms  Editor scrolls to line          CodeEditor        Monaco scroll
```

## Integration Testing Matrix

| Component A   | Component B   | Integration Point        | Status |
|---------------|---------------|--------------------------|--------|
| CodeEditor    | Store         | setSource on onChange    | ✅     |
| CodeEditor    | Store         | compile on Ctrl+Enter    | ✅     |
| BottomBar     | Store         | compile on button click  | ✅     |
| Store         | API Service   | compile action calls API | ✅     |
| API Service   | Backend       | POST /compile            | ✅     |
| Store         | OutputPanel   | result passed via prop   | ✅     |
| Store         | BottomBar     | status passed via prop   | ✅     |
| LogDisplay    | Layout        | onScrollToLine callback  | ✅     |
| Layout        | CodeEditor    | editorRef for scroll     | ✅     |
| OutputPanel   | Store         | activeTab subscription   | ✅     |
| Store         | localStorage  | persist middleware       | ✅     |
| App           | Backend       | health check on mount    | ✅     |

## Summary

All components are properly integrated with:
- **Unidirectional data flow** via Zustand store
- **Type-safe props** passed between components
- **Callback functions** for child-to-parent communication
- **Store subscriptions** for reactive updates
- **API layer** abstracted in service
- **Error handling** at every level
- **Loading states** managed centrally
- **Persistence** via localStorage middleware
