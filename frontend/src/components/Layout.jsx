import { useRef } from 'react';
import Header from './Header';
import CodeEditor from './CodeEditor';
import OutputPanel from './OutputPanel';
import BottomBar from './BottomBar';
import {
  useSource,
  useCompilationStatus,
  useCompilationResults,
  useLogs,
  useError,
  useCompilerActions,
} from '../store/compilerStore';

const Layout = () => {
  const editorRef = useRef(null);

  // Get state from Zustand store
  const source = useSource();
  const { isCompiling, status } = useCompilationStatus();
  const result = useCompilationResults();
  const logs = useLogs();
  const error = useError();

  // Get actions from store
  const { setSource, compile } = useCompilerActions();

  // Scroll to line in editor
  const handleScrollToLine = (line) => {
    if (editorRef.current) {
      editorRef.current.revealLineInCenter(line);
      editorRef.current.setPosition({ lineNumber: line, column: 1 });
      editorRef.current.focus();
    }
  };

  return (
    <div className="h-screen flex flex-col bg-cyber-dark" style={{ backgroundColor: '#0a0e27', minHeight: '100vh' }}>
      <Header />

      <div className="flex-1 flex flex-col lg:flex-row overflow-hidden">
        {/* Left Panel - Code Editor (40% on desktop, full width on mobile) */}
        <div className="w-full lg:w-2/5 border-r border-cyber-blue/30 flex flex-col min-h-[40vh] lg:min-h-0">
          <CodeEditor
            code={source}
            onChange={setSource}
            onCompile={compile}
            editorRef={editorRef}
          />
        </div>

        {/* Right Panel - Output Display (60% on desktop, full width on mobile) */}
        <div className="w-full lg:w-3/5 flex flex-col min-h-[40vh] lg:min-h-0">
          <OutputPanel data={result} />
        </div>
      </div>

      {/* Bottom Bar */}
      <BottomBar
        onCompile={compile}
        isCompiling={isCompiling}
        status={status}
        logs={logs}
        error={error}
        onScrollToLine={handleScrollToLine}
      />
    </div>
  );
};

export default Layout;
