import { useState, useRef } from 'react';
import Editor from '@monaco-editor/react';

/**
 * AssemblyDisplay - Professional assembly code viewer with syntax highlighting
 * Uses Monaco Editor for advanced features like syntax highlighting and line numbers
 */
const AssemblyDisplay = ({ data }) => {
  const [copied, setCopied] = useState(false);
  const [stats, setStats] = useState({ lines: 0, sections: [], labels: [] });
  const editorRef = useRef(null);

  if (!data || typeof data !== 'string') {
    return (
      <div className="flex items-center justify-center h-full">
        <div className="text-center text-gray-500 font-mono">
          <div className="text-4xl mb-4">⚙️</div>
          <p>No assembly code available</p>
          <p className="text-sm mt-2">Compile your code to see generated assembly</p>
        </div>
      </div>
    );
  }

  // Analyze assembly code for stats
  const analyzeAssembly = (code) => {
    const lines = code.split('\n');
    const sections = [];
    const labels = [];

    lines.forEach((line) => {
      const trimmed = line.trim();

      // Find sections
      if (trimmed.match(/^\.(text|data|bss|rodata)/)) {
        sections.push(trimmed);
      }

      // Find labels (lines ending with :)
      if (trimmed.match(/^[a-zA-Z_][a-zA-Z0-9_]*:$/)) {
        labels.push(trimmed.replace(':', ''));
      }
    });

    return {
      lines: lines.length,
      sections: [...new Set(sections)],
      labels: [...new Set(labels)],
    };
  };

  // Copy assembly to clipboard
  const handleCopy = async () => {
    try {
      await navigator.clipboard.writeText(data);
      setCopied(true);
      setTimeout(() => setCopied(false), 2000);
    } catch (err) {
      console.error('Failed to copy:', err);
    }
  };

  // Download assembly as file
  const handleDownload = () => {
    const blob = new Blob([data], { type: 'text/plain' });
    const url = URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.href = url;
    link.download = 'assembly.s';
    link.click();
    URL.revokeObjectURL(url);
  };

  // Handle editor mount
  const handleEditorDidMount = (editor, monaco) => {
    editorRef.current = editor;

    // Configure custom assembly language support
    monaco.languages.register({ id: 'x86asm' });

    // Define assembly syntax highlighting
    monaco.languages.setMonarchTokensProvider('x86asm', {
      tokenizer: {
        root: [
          // Comments
          [/#.*$/, 'comment'],
          [/;.*$/, 'comment'],
          [/\/\/.*$/, 'comment'],

          // Sections
          [/\.(text|data|bss|rodata|section)/, 'keyword.section'],

          // Directives
          [/\.(globl|global|extern|align|byte|word|long|quad|ascii|asciz|string)/, 'keyword.directive'],

          // Labels
          [/^[a-zA-Z_][a-zA-Z0-9_]*:/, 'type.identifier'],

          // Registers
          [/%?(rax|rbx|rcx|rdx|rsi|rdi|rbp|rsp|r[8-9]|r1[0-5])/, 'variable.register'],
          [/%?(eax|ebx|ecx|edx|esi|edi|ebp|esp)/, 'variable.register'],
          [/%?(ax|bx|cx|dx|si|di|bp|sp)/, 'variable.register'],
          [/%?(al|bl|cl|dl|ah|bh|ch|dh)/, 'variable.register'],
          [/%?(x[0-9]|x[12][0-9]|x30|x31)/, 'variable.register'], // ARM64 registers

          // Instructions - Data Movement
          [/\b(mov|movq|movl|movw|movb|lea|push|pop|xchg)\b/, 'keyword.instruction'],

          // Instructions - Arithmetic
          [/\b(add|sub|mul|imul|div|idiv|inc|dec|neg)\b/, 'keyword.instruction'],

          // Instructions - Logical
          [/\b(and|or|xor|not|shl|shr|sal|sar|rol|ror)\b/, 'keyword.instruction'],

          // Instructions - Comparison and Test
          [/\b(cmp|test|bt|bts|btr|btc)\b/, 'keyword.instruction'],

          // Instructions - Control Flow
          [/\b(jmp|je|jne|jz|jnz|jg|jge|jl|jle|ja|jae|jb|jbe|call|ret|leave|enter)\b/, 'keyword.instruction'],

          // Instructions - Stack
          [/\b(pushq|popq|pushf|popf)\b/, 'keyword.instruction'],

          // ARM64 instructions
          [/\b(ldr|str|ldp|stp|adr|adrp|bl|br|blr|cbz|cbnz)\b/, 'keyword.instruction'],

          // Numbers - Hex
          [/0x[0-9a-fA-F]+/, 'number.hex'],

          // Numbers - Decimal
          [/\$?-?[0-9]+/, 'number'],

          // Memory operands
          [/\(.*?\)/, 'string.operand'],

          // Strings
          [/"([^"\\]|\\.)*$/, 'string.invalid'],
          [/"/, 'string', '@string'],
        ],

        string: [
          [/[^\\"]+/, 'string'],
          [/\\./, 'string.escape'],
          [/"/, 'string', '@pop'],
        ],
      },
    });

    // Set theme colors for assembly
    monaco.editor.defineTheme('assembly-dark', {
      base: 'vs-dark',
      inherit: true,
      rules: [
        { token: 'comment', foreground: '6A9955', fontStyle: 'italic' },
        { token: 'keyword.section', foreground: 'FF006E', fontStyle: 'bold' },
        { token: 'keyword.directive', foreground: 'B026FF' },
        { token: 'keyword.instruction', foreground: '00D9FF', fontStyle: 'bold' },
        { token: 'type.identifier', foreground: '00FF9F', fontStyle: 'bold' },
        { token: 'variable.register', foreground: 'FFD700' },
        { token: 'number', foreground: 'FF006E' },
        { token: 'number.hex', foreground: 'FF006E' },
        { token: 'string', foreground: 'B026FF' },
        { token: 'string.operand', foreground: 'E0E0E0' },
      ],
      colors: {
        'editor.background': '#0a0e27',
        'editor.foreground': '#E0E0E0',
        'editor.lineHighlightBackground': '#1a1e37',
        'editorLineNumber.foreground': '#4a5568',
        'editorLineNumber.activeForeground': '#00D9FF',
      },
    });

    monaco.editor.setTheme('assembly-dark');

    // Analyze assembly on mount
    const assemblyStats = analyzeAssembly(data);
    setStats(assemblyStats);
  };

  return (
    <div className="h-full flex flex-col">
      {/* Header with controls */}
      <div className="p-4 border-b border-cyber-purple/20 bg-cyber-dark/50">
        <div className="flex items-center justify-between mb-3">
          {/* Stats */}
          <div className="flex items-center space-x-4 text-sm font-mono">
            <div>
              <span className="text-cyber-purple">●</span>
              <span className="text-gray-400 ml-2">{stats.lines} lines</span>
            </div>

            {stats.sections.length > 0 && (
              <div>
                <span className="text-cyber-pink">●</span>
                <span className="text-gray-400 ml-2">
                  {stats.sections.length} sections
                </span>
              </div>
            )}

            {stats.labels.length > 0 && (
              <div>
                <span className="text-cyber-green">●</span>
                <span className="text-gray-400 ml-2">{stats.labels.length} labels</span>
              </div>
            )}
          </div>

          {/* Action buttons */}
          <div className="flex items-center space-x-2">
            <button
              onClick={handleCopy}
              className="px-3 py-1 text-xs font-mono border border-cyber-blue/50 rounded
                         hover:border-cyber-blue hover:bg-cyber-blue/10 text-cyber-blue
                         transition-all flex items-center space-x-1"
            >
              <span>{copied ? '✓' : '📋'}</span>
              <span>{copied ? 'Copied!' : 'Copy'}</span>
            </button>

            <button
              onClick={handleDownload}
              className="px-3 py-1 text-xs font-mono border border-cyber-purple/50 rounded
                         hover:border-cyber-purple hover:bg-cyber-purple/10 text-cyber-purple
                         transition-all"
            >
              💾 Download
            </button>
          </div>
        </div>

        {/* Section/Label info */}
        {(stats.sections.length > 0 || stats.labels.length > 0) && (
          <div className="text-xs font-mono space-y-1">
            {stats.sections.length > 0 && (
              <div className="text-gray-500">
                <span className="text-cyber-pink">Sections:</span>{' '}
                {stats.sections.join(', ')}
              </div>
            )}
            {stats.labels.length > 0 && (
              <div className="text-gray-500">
                <span className="text-cyber-green">Labels:</span>{' '}
                {stats.labels.slice(0, 5).join(', ')}
                {stats.labels.length > 5 && ` +${stats.labels.length - 5} more`}
              </div>
            )}
          </div>
        )}
      </div>

      {/* Monaco Editor for assembly code */}
      <div className="flex-1 overflow-hidden">
        <Editor
          height="100%"
          language="x86asm"
          value={data}
          theme="assembly-dark"
          onMount={handleEditorDidMount}
          options={{
            readOnly: true,
            minimap: { enabled: true },
            fontSize: 13,
            fontFamily: "'Fira Code', 'Courier New', monospace",
            fontLigatures: true,
            lineNumbers: 'on',
            glyphMargin: false,
            folding: true,
            lineDecorationsWidth: 0,
            lineNumbersMinChars: 4,
            renderLineHighlight: 'all',
            scrollBeyondLastLine: false,
            automaticLayout: true,
            wordWrap: 'off',
            overviewRulerBorder: false,
            hideCursorInOverviewRuler: true,
            renderWhitespace: 'selection',
            contextmenu: true,
            mouseWheelZoom: true,
            smoothScrolling: true,
          }}
        />
      </div>

      {/* Help/Legend */}
      <div className="p-3 border-t border-cyber-purple/20 bg-cyber-dark/30 text-xs font-mono text-gray-500">
        <div className="flex items-center justify-between">
          <div className="flex items-center space-x-4">
            <span className="text-cyber-pink">■ Sections</span>
            <span className="text-cyber-purple">■ Directives</span>
            <span className="text-cyber-blue">■ Instructions</span>
            <span className="text-cyber-green">■ Labels</span>
            <span style={{ color: '#FFD700' }}>■ Registers</span>
          </div>
          <span className="text-gray-600">Read-only view • Scroll to zoom</span>
        </div>
      </div>
    </div>
  );
};

export default AssemblyDisplay;
