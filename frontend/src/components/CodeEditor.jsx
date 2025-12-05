import Editor from '@monaco-editor/react';

const CodeEditor = ({ code, onChange, onCompile, editorRef }) => {
  // Handle editor mount
  const handleEditorDidMount = (editor, monaco) => {
    // Store editor instance in ref passed from parent
    if (editorRef) {
      editorRef.current = editor;
    }

    // Define custom cyberpunk theme
    monaco.editor.defineTheme('cyberpunk', {
      base: 'vs-dark',
      inherit: true,
      rules: [
        { token: 'comment', foreground: '6A9955', fontStyle: 'italic' },
        { token: 'keyword', foreground: '00D9FF', fontStyle: 'bold' },
        { token: 'string', foreground: 'B026FF' },
        { token: 'number', foreground: 'FF006E' },
        { token: 'type', foreground: '00FF9F' },
        { token: 'identifier', foreground: 'E0E0E0' },
        { token: 'delimiter', foreground: '00D9FF' },
        { token: 'operator', foreground: 'FF006E' },
      ],
      colors: {
        'editor.background': '#0a0e27',
        'editor.foreground': '#E0E0E0',
        'editor.lineHighlightBackground': '#1a1e37',
        'editor.selectionBackground': '#00D9FF33',
        'editor.inactiveSelectionBackground': '#00D9FF22',
        'editorCursor.foreground': '#00D9FF',
        'editorLineNumber.foreground': '#4a5568',
        'editorLineNumber.activeForeground': '#00D9FF',
        'editorIndentGuide.background': '#2d3748',
        'editorIndentGuide.activeBackground': '#4a5568',
        'editorBracketMatch.background': '#00D9FF22',
        'editorBracketMatch.border': '#00D9FF',
      },
    });

    // Set the custom theme
    monaco.editor.setTheme('cyberpunk');

    // Add keyboard shortcut for compile (Ctrl+Enter / Cmd+Enter)
    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.Enter, () => {
      if (onCompile) {
        onCompile();
      }
    });

    // Add additional C language features
    monaco.languages.setLanguageConfiguration('c', {
      comments: {
        lineComment: '//',
        blockComment: ['/*', '*/'],
      },
      brackets: [
        ['{', '}'],
        ['[', ']'],
        ['(', ')'],
      ],
      autoClosingPairs: [
        { open: '{', close: '}' },
        { open: '[', close: ']' },
        { open: '(', close: ')' },
        { open: '"', close: '"' },
        { open: "'", close: "'" },
      ],
      surroundingPairs: [
        { open: '{', close: '}' },
        { open: '[', close: ']' },
        { open: '(', close: ')' },
        { open: '"', close: '"' },
        { open: "'", close: "'" },
      ],
      indentationRules: {
        increaseIndentPattern: /^.*\{[^}"']*$/,
        decreaseIndentPattern: /^\s*\}/,
      },
    });

    // Focus the editor
    editor.focus();
  };

  const handleEditorChange = (value) => {
    onChange(value || '');
  };

  return (
    <div className="flex-1 flex flex-col">
      <div className="px-4 py-2 bg-cyber-dark/50 border-b border-cyber-blue/30 flex items-center justify-between">
        <h2 className="text-sm font-bold text-cyber-blue font-mono tracking-wider">
          C SOURCE CODE
        </h2>
        <div className="text-xs text-gray-500 font-mono">
          <kbd className="px-2 py-1 bg-gray-800 border border-gray-700 rounded">
            {navigator.platform.includes('Mac') ? '⌘' : 'Ctrl'}+Enter
          </kbd>
          <span className="ml-2">to compile</span>
        </div>
      </div>

      <div className="flex-1 overflow-hidden">
        <Editor
          height="100%"
          defaultLanguage="c"
          value={code}
          onChange={handleEditorChange}
          onMount={handleEditorDidMount}
          theme="cyberpunk"
          options={{
            // Display
            minimap: { enabled: true, scale: 1 },
            fontSize: 14,
            fontFamily: "'Fira Code', 'Courier New', monospace",
            fontLigatures: true,
            lineNumbers: 'on',
            lineNumbersMinChars: 4,
            glyphMargin: true,
            folding: true,
            foldingStrategy: 'indentation',
            showFoldingControls: 'always',

            // Scrolling
            scrollBeyondLastLine: false,
            scrollbar: {
              vertical: 'visible',
              horizontal: 'visible',
              verticalScrollbarSize: 10,
              horizontalScrollbarSize: 10,
            },

            // Auto-indentation
            automaticLayout: true,
            autoIndent: 'full',
            formatOnPaste: true,
            formatOnType: true,
            tabSize: 4,
            insertSpaces: true,

            // Word wrap
            wordWrap: 'on',
            wordWrapColumn: 80,
            wrappingIndent: 'indent',

            // Padding and spacing
            padding: { top: 16, bottom: 16 },
            lineHeight: 20,

            // Visual enhancements
            renderWhitespace: 'selection',
            renderLineHighlight: 'all',
            cursorBlinking: 'smooth',
            cursorSmoothCaretAnimation: 'on',
            smoothScrolling: true,
            roundedSelection: true,

            // Bracket matching
            matchBrackets: 'always',
            bracketPairColorization: {
              enabled: true,
            },

            // Suggestions
            suggestOnTriggerCharacters: true,
            acceptSuggestionOnCommitCharacter: true,
            acceptSuggestionOnEnter: 'on',
            quickSuggestions: {
              other: true,
              comments: false,
              strings: false,
            },

            // Other features
            contextmenu: true,
            mouseWheelZoom: true,
            links: false,
            colorDecorators: true,
          }}
        />
      </div>
    </div>
  );
};

export default CodeEditor;
