# Frontend Integration Documentation

This document describes how all components are integrated and wired together in the C Compiler Visualization Frontend.

## Architecture Overview

```
App.jsx (Root)
  ├── Scanline Overlay (Visual Effect)
  └── Layout.jsx (Main Container)
      ├── Header.jsx (Title Bar)
      ├── CodeEditor.jsx (Monaco Editor)
      ├── OutputPanel.jsx (Tabbed Display)
      │   ├── TokenView
      │   ├── ASTVisualization
      │   ├── AssemblyDisplay
      │   └── HexDisplay
      └── BottomBar.jsx (Controls & Logs)
          └── LogDisplay.jsx (Expandable)

State Management: compilerStore.js (Zustand)
API Layer: compilerService.js
Theme: theme.js + global.css
```

## Data Flow

### 1. Code Input Flow
```
User types in CodeEditor
  → onChange event
  → setSource(newCode) action
  → Updates compilerStore.source
  → CodeEditor re-renders with new value
  → Auto-saved to localStorage
```

**Files Involved:**
- `CodeEditor.jsx:101` - onChange handler
- `Layout.jsx:46` - setSource prop
- `compilerStore.js:50` - setSource action
- `compilerStore.js:234-238` - localStorage persistence

### 2. Compilation Flow
```
User clicks "COMPILE" button or presses Ctrl+Enter
  → onCompile() callback
  → compile() action in store
  → compileCodeWithRetry(source) in service
  → POST /compile to Flask API
  → Receive JSON response
  → Update store with results
  → Components re-render with new data
```

**Files Involved:**
- `CodeEditor.jsx:45-49` - Keyboard shortcut (Ctrl/Cmd+Enter)
- `BottomBar.jsx:54` - Compile button click
- `Layout.jsx:26,60` - compile action passed as prop
- `compilerStore.js:97-182` - compile action implementation
- `compilerService.js:15-65` - API communication
- `compilerStore.js:122-131` - Result storage

### 3. Display Flow
```
Store updates result state
  → useCompilationResults() selector fires
  → OutputPanel receives new data prop
  → activeTab determines which view to show
  → View components render data
```

**Files Involved:**
- `compilerStore.js:264-265` - useCompilationResults selector
- `Layout.jsx:21,54` - result passed to OutputPanel
- `OutputPanel.jsx:28-53` - renderContent() switches on activeTab
- `OutputPanel.jsx:6-7` - activeTab from store

### 4. Error Handling Flow
```
API request fails
  → catch block in compile action
  → formatErrorMessage(error)
  → Update store with error state
  → BottomBar shows error status
  → LogDisplay shows error details
  → Error can be clicked to jump to line
```

**Files Involved:**
- `compilerStore.js:151-181` - Error handling in compile
- `compilerService.js:105-131` - formatErrorMessage
- `Layout.jsx:23,64` - error passed to BottomBar
- `BottomBar.jsx:117` - LogDisplay with error prop
- `LogDisplay.jsx:26-39` - Error parsing and display

### 5. Loading State Flow
```
Compile button clicked
  → set isCompiling: true, status: 'compiling'
  → Compile button disabled, shows spinner
  → StatusBar shows "COMPILING..."
  → API call completes
  → set isCompiling: false, status: 'success'/'error'
  → Compile button re-enabled
```

**Files Involved:**
- `compilerStore.js:101-103` - Loading state set
- `compilerStore.js:119-121` - Loading cleared (success)
- `compilerStore.js:156-158` - Loading cleared (error)
- `BottomBar.jsx:55,59` - disabled state
- `BottomBar.jsx:64-75` - Conditional rendering

### 6. Log Display Flow
```
Compilation events occur
  → addLog() action called
  → logs array updated
  → BottomBar shows log count
  → User clicks "Show Logs"
  → LogDisplay expands
  → Categorized logs displayed
```

**Files Involved:**
- `compilerStore.js:68-75` - addLog action
- `compilerStore.js:105-110,135-147,167-177` - Logs added
- `Layout.jsx:22,63` - logs passed to BottomBar
- `BottomBar.jsx:45-46,95-104` - Toggle button
- `LogDisplay.jsx:48-76` - Log categorization

### 7. Scroll-to-Line Flow
```
User clicks error in LogDisplay
  → onScrollToLine(lineNumber) callback
  → handleScrollToLine in Layout
  → editorRef.current.revealLineInCenter(line)
  → Monaco editor scrolls and focuses
```

**Files Involved:**
- `LogDisplay.jsx:98` - onClick handler
- `BottomBar.jsx:117` - onScrollToLine prop
- `Layout.jsx:29-35,65` - handleScrollToLine implementation
- `CodeEditor.jsx:7-9` - editorRef stored
- `Layout.jsx:48` - editorRef passed to CodeEditor

## State Management

### Zustand Store (`compilerStore.js`)

**State Structure:**
```javascript
{
  source: string,                    // C source code
  isCompiling: boolean,              // Compilation in progress
  compilationStatus: string,         // 'ready' | 'compiling' | 'success' | 'error'
  result: {
    tokens: array | null,            // Token stream
    ast: object | null,              // Abstract syntax tree
    assembly: string | null,         // Assembly code
    hex: string | null,              // Hex dump
    logs: { stdout, stderr }         // Compiler output
  },
  error: object | null,              // Error details
  activeTab: string,                 // 'tokens' | 'ast' | 'assembly' | 'hex'
  logs: array,                       // UI log messages
  editorSettings: object             // Editor preferences
}
```

**Actions:**
- `setSource(source)` - Update source code
- `setActiveTab(tab)` - Switch output tab
- `compile()` - Trigger compilation (async)
- `addLog(log)` - Add log entry
- `clearLogs()` - Clear log history
- `resetResults()` - Clear compilation results
- `reset()` - Reset all state
- `updateEditorSettings(settings)` - Update editor config

**Selectors:**
- `useSource()` - Get source code
- `useCompilationStatus()` - Get compilation status
- `useCompilationResults()` - Get compilation results
- `useActiveTab()` - Get active output tab
- `useLogs()` - Get log entries
- `useError()` - Get error state
- `useEditorSettings()` - Get editor settings
- `useCompilerActions()` - Get all actions (non-reactive)

## API Integration

### Service Layer (`compilerService.js`)

**Endpoints:**
- `POST /compile` - Compile C source code
- `GET /health` - Health check (used in App.jsx)

**Request Format:**
```json
{
  "source_code": "int main() { return 0; }"
}
```

**Response Format (Success):**
```json
{
  "success": true,
  "tokens": [...],
  "ast": {...},
  "assembly": "...",
  "hex": "...",
  "stdout": "...",
  "stderr": "...",
  "warnings": [...]
}
```

**Response Format (Error):**
```json
{
  "success": false,
  "error": "Error message",
  "details": {...}
}
```

**Error Handling:**
- Network errors (timeout, connection refused)
- HTTP errors (4xx, 5xx)
- Invalid JSON responses
- Retry logic with exponential backoff (max 2 retries)

## Component Props Flow

### Layout → CodeEditor
```javascript
<CodeEditor
  code={source}              // FROM: useSource()
  onChange={setSource}       // FROM: useCompilerActions()
  onCompile={compile}        // FROM: useCompilerActions()
  editorRef={editorRef}      // FROM: useRef() in Layout
/>
```

### Layout → OutputPanel
```javascript
<OutputPanel
  data={result}              // FROM: useCompilationResults()
/>
```

### Layout → BottomBar
```javascript
<BottomBar
  onCompile={compile}        // FROM: useCompilerActions()
  isCompiling={isCompiling}  // FROM: useCompilationStatus()
  status={status}            // FROM: useCompilationStatus()
  logs={logs}                // FROM: useLogs()
  error={error}              // FROM: useError()
  onScrollToLine={handleScrollToLine} // FROM: Layout callback
/>
```

### BottomBar → LogDisplay
```javascript
<LogDisplay
  logs={logs}                // FROM: BottomBar props
  error={error}              // FROM: BottomBar props
  onScrollToLine={onScrollToLine} // FROM: BottomBar props
/>
```

## Event Handlers

### User Interactions
1. **Type in Editor**: `onChange` → `setSource`
2. **Click Compile**: `onClick` → `compile`
3. **Press Ctrl+Enter**: Monaco command → `compile`
4. **Switch Tab**: `onClick` → `setActiveTab`
5. **Click Error**: `onClick` → `onScrollToLine`
6. **Toggle Logs**: `onClick` → local state in BottomBar
7. **Export AST**: `onClick` → local file download
8. **Copy Assembly**: `onClick` → clipboard API
9. **Download Hex**: `onClick` → local file download

## Loading States

### Compilation Loading
- **Button**: Disabled, shows spinner
- **Status**: "COMPILING..." with animated icon
- **Store**: `isCompiling: true`

### Tab Loading
- No explicit loading (tabs show empty state if no data)

## Error States

### API Errors
- **Network Error**: "Failed to connect to server"
- **Timeout**: "Request timed out after 30 seconds"
- **Server Error**: Shows error from API response
- **Parse Error**: "Invalid C syntax: [details]"

### Error Display
- **BottomBar**: Shows error count and status
- **LogDisplay**: Full error details with location
- **CodeEditor**: Can scroll to error line on click

## Empty States

### No Compilation Yet
- **OutputPanel**: "No data available" with icon
- **Logs**: "No logs yet" message

### No Errors
- **LogDisplay**: "✅ No issues" message

## Responsive Behavior

### Layout Breakpoints
- **Desktop (>1024px)**: 40/60 split (editor/output)
- **Tablet (768-1024px)**: Same split (may add stacking later)
- **Mobile (<768px)**: Scanlines disabled for performance

## Keyboard Shortcuts

- **Ctrl/Cmd + Enter**: Compile code
- **Monaco built-in shortcuts**: Standard editor shortcuts

## LocalStorage Persistence

**Persisted:**
- `source` - C source code
- `editorSettings` - Editor preferences
- `activeTab` - Last selected output tab

**Not Persisted:**
- `result` - Compilation results (cleared on reload)
- `logs` - Log messages (cleared on reload)
- `isCompiling` - Compilation state (reset on reload)
- `error` - Error state (cleared on reload)

## Integration Checklist

- [x] Code editor connects to state
- [x] Compile button triggers API call
- [x] API responses route to correct displays
- [x] Loading states work across all components
- [x] Error states handled gracefully
- [x] Error-to-line navigation works
- [x] Tab switching works
- [x] LocalStorage persistence works
- [x] Theme applied consistently
- [x] Animations and effects applied
- [x] Responsive layout functional
- [x] Keyboard shortcuts work
- [x] Export/download features work

## Testing the Integration

### Manual Test Flow

1. **Start Application**
   - Open `http://localhost:3000`
   - Check console for "Backend server is running"
   - Verify scanline effect visible

2. **Test Code Input**
   - Type in editor
   - Check code persists on page reload
   - Verify syntax highlighting works

3. **Test Compilation**
   - Click "COMPILE" button
   - Verify button becomes disabled
   - Check status shows "COMPILING..."
   - Wait for completion

4. **Test Success Flow**
   - Write valid C code: `int main() { return 42; }`
   - Compile
   - Check TOKENS tab shows token list
   - Check AST tab shows tree structure
   - Check ASSEMBLY tab shows assembly code
   - Check HEX DUMP tab shows hex output

5. **Test Error Flow**
   - Write invalid C code: `int main() { undefined_func(); }`
   - Compile
   - Check status shows "COMPILATION FAILED"
   - Click "Show Logs"
   - Verify error displayed in red
   - Click error to jump to line

6. **Test Tab Switching**
   - Switch between all tabs
   - Verify each tab shows correct data
   - Check activeTab persists on reload

7. **Test Export Features**
   - Export AST as JSON
   - Copy assembly code
   - Download hex dump
   - Verify file contents

## Troubleshooting

### Backend Not Running
**Symptom**: API calls fail with network error
**Solution**: Start Flask API with `cd api && python3 app.py`

### Port Already in Use
**Symptom**: Frontend fails to start
**Solution**: Kill process on port 3000 or change port in vite.config.js

### CORS Errors
**Symptom**: API calls blocked by browser
**Solution**: Ensure Flask API has CORS enabled (should be in api/app.py)

### State Not Persisting
**Symptom**: Code resets on reload
**Solution**: Check browser localStorage (should have 'compiler-storage' key)

### Theme Not Applied
**Symptom**: No neon colors or animations
**Solution**: Check that global.css is imported in index.css
