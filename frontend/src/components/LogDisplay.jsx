import { useState } from 'react';

/**
 * LogDisplay - Professional error and log viewer
 * Displays compilation errors, warnings, and stdout/stderr with proper formatting
 */
const LogDisplay = ({ logs = [], error = null, onScrollToLine }) => {
  const [expandedSections, setExpandedSections] = useState({
    errors: true,
    warnings: true,
    stdout: false,
    stderr: false,
  });

  const toggleSection = (section) => {
    setExpandedSections((prev) => ({
      ...prev,
      [section]: !prev[section],
    }));
  };

  // Parse error/warning from different formats
  const parseLogEntry = (log) => {
    if (typeof log === 'string') {
      // Try to parse format: "file.c:line:col: error: message"
      const match = log.match(/^(.+?):(\d+):(\d+):\s*(error|warning):\s*(.+)$/);
      if (match) {
        return {
          file: match[1],
          line: parseInt(match[2]),
          column: parseInt(match[3]),
          type: match[4],
          message: match[5],
          raw: log,
        };
      }
      // Simple format: just the message
      return {
        type: log.toLowerCase().includes('error') ? 'error' : 'info',
        message: log,
        raw: log,
      };
    }
    // Already parsed object
    return {
      type: log.type || 'info',
      message: log.message || log.raw || '',
      line: log.line,
      column: log.column,
      file: log.file,
      raw: log.raw || log.message,
    };
  };

  // Categorize logs
  const categorizedLogs = {
    errors: [],
    warnings: [],
    info: [],
    stdout: [],
    stderr: [],
  };

  logs.forEach((log) => {
    const parsed = parseLogEntry(log);
    if (parsed.type === 'error') {
      categorizedLogs.errors.push(parsed);
    } else if (parsed.type === 'warning') {
      categorizedLogs.warnings.push(parsed);
    } else if (parsed.type === 'stdout') {
      categorizedLogs.stdout.push(parsed);
    } else if (parsed.type === 'stderr') {
      categorizedLogs.stderr.push(parsed);
    } else {
      categorizedLogs.info.push(parsed);
    }
  });

  // Add error prop to errors list
  if (error) {
    const parsedError = parseLogEntry(error);
    if (!categorizedLogs.errors.some((e) => e.raw === parsedError.raw)) {
      categorizedLogs.errors.unshift(parsedError);
    }
  }

  const hasErrors = categorizedLogs.errors.length > 0;
  const hasWarnings = categorizedLogs.warnings.length > 0;
  const hasStdout = categorizedLogs.stdout.length > 0;
  const hasStderr = categorizedLogs.stderr.length > 0;

  // Render a single log entry
  const renderLogEntry = (entry, index) => {
    const isError = entry.type === 'error';
    const isWarning = entry.type === 'warning';

    const bgColor = isError
      ? 'bg-red-900/20 border-red-500/30'
      : isWarning
      ? 'bg-yellow-900/20 border-yellow-500/30'
      : 'bg-blue-900/20 border-blue-500/30';

    const textColor = isError
      ? 'text-red-400'
      : isWarning
      ? 'text-yellow-400'
      : 'text-blue-400';

    const icon = isError ? '❌' : isWarning ? '⚠️' : 'ℹ️';

    const handleClick = () => {
      if (entry.line && onScrollToLine) {
        onScrollToLine(entry.line);
      }
    };

    return (
      <div
        key={index}
        className={`p-3 mb-2 border rounded ${bgColor} font-mono text-sm transition-all
                   ${entry.line ? 'cursor-pointer hover:brightness-125' : ''}`}
        onClick={handleClick}
      >
        <div className="flex items-start space-x-3">
          <span className="text-xl">{icon}</span>
          <div className="flex-1">
            {/* Header with type and location */}
            <div className="flex items-center space-x-2 mb-1">
              <span className={`font-bold uppercase text-xs ${textColor}`}>
                {entry.type}
              </span>
              {entry.file && (
                <span className="text-gray-500 text-xs">
                  {entry.file}
                  {entry.line && `:${entry.line}`}
                  {entry.column && `:${entry.column}`}
                </span>
              )}
            </div>

            {/* Message */}
            <div className="text-white">{entry.message}</div>

            {/* Click hint */}
            {entry.line && (
              <div className="text-xs text-gray-500 mt-2">
                Click to jump to line {entry.line}
              </div>
            )}
          </div>
        </div>
      </div>
    );
  };

  // Collapsible section component
  const CollapsibleSection = ({ title, count, isExpanded, onToggle, color, children }) => {
    if (count === 0) return null;

    return (
      <div className="mb-4">
        <button
          onClick={onToggle}
          className={`w-full flex items-center justify-between p-3 rounded border ${color}
                     hover:brightness-125 transition-all`}
        >
          <div className="flex items-center space-x-3">
            <span className="text-lg">{isExpanded ? '▼' : '▶'}</span>
            <span className="font-bold font-mono">{title}</span>
            <span className="text-sm text-gray-400">({count})</span>
          </div>
        </button>

        {isExpanded && <div className="mt-2">{children}</div>}
      </div>
    );
  };

  // Render stdout/stderr as collapsible raw text
  const renderRawOutput = (entries) => {
    return (
      <div className="bg-black/50 p-4 rounded border border-gray-700 font-mono text-xs">
        <pre className="whitespace-pre-wrap text-gray-300 overflow-auto max-h-60">
          {entries.map((e) => e.message || e.raw).join('\n')}
        </pre>
      </div>
    );
  };

  return (
    <div className="h-full flex flex-col bg-cyber-dark/30 overflow-auto p-4">
      {/* Header */}
      <div className="mb-4 pb-3 border-b border-gray-700">
        <div className="flex items-center justify-between">
          <h3 className="text-lg font-bold font-mono text-cyber-blue">
            📊 Compilation Logs
          </h3>
          <div className="flex items-center space-x-3 text-sm font-mono">
            {hasErrors && (
              <span className="text-red-400">
                ❌ {categorizedLogs.errors.length} errors
              </span>
            )}
            {hasWarnings && (
              <span className="text-yellow-400">
                ⚠️ {categorizedLogs.warnings.length} warnings
              </span>
            )}
            {!hasErrors && !hasWarnings && (
              <span className="text-cyber-green">✅ No issues</span>
            )}
          </div>
        </div>
      </div>

      {/* No logs message */}
      {logs.length === 0 && !error && (
        <div className="flex-1 flex items-center justify-center">
          <div className="text-center text-gray-500 font-mono">
            <div className="text-4xl mb-4">📋</div>
            <p>No logs yet</p>
            <p className="text-sm mt-2">Compile your code to see output</p>
          </div>
        </div>
      )}

      {/* Errors Section */}
      <CollapsibleSection
        title="Errors"
        count={categorizedLogs.errors.length}
        isExpanded={expandedSections.errors}
        onToggle={() => toggleSection('errors')}
        color="bg-red-900/20 border-red-500/30 text-red-400"
      >
        {categorizedLogs.errors.map(renderLogEntry)}
      </CollapsibleSection>

      {/* Warnings Section */}
      <CollapsibleSection
        title="Warnings"
        count={categorizedLogs.warnings.length}
        isExpanded={expandedSections.warnings}
        onToggle={() => toggleSection('warnings')}
        color="bg-yellow-900/20 border-yellow-500/30 text-yellow-400"
      >
        {categorizedLogs.warnings.map(renderLogEntry)}
      </CollapsibleSection>

      {/* Info/Success Messages */}
      {categorizedLogs.info.length > 0 && (
        <div className="mb-4">
          {categorizedLogs.info.map(renderLogEntry)}
        </div>
      )}

      {/* Stdout Section */}
      <CollapsibleSection
        title="Standard Output (stdout)"
        count={categorizedLogs.stdout.length}
        isExpanded={expandedSections.stdout}
        onToggle={() => toggleSection('stdout')}
        color="bg-gray-800 border-gray-600 text-gray-300"
      >
        {renderRawOutput(categorizedLogs.stdout)}
      </CollapsibleSection>

      {/* Stderr Section */}
      <CollapsibleSection
        title="Standard Error (stderr)"
        count={categorizedLogs.stderr.length}
        isExpanded={expandedSections.stderr}
        onToggle={() => toggleSection('stderr')}
        color="bg-gray-800 border-gray-600 text-gray-400"
      >
        {renderRawOutput(categorizedLogs.stderr)}
      </CollapsibleSection>

      {/* Help text */}
      {(hasErrors || hasWarnings) && (
        <div className="mt-4 p-3 bg-gray-800/30 border border-gray-700 rounded text-xs font-mono text-gray-500">
          <div className="font-bold text-cyber-blue mb-2">💡 Tips:</div>
          <ul className="space-y-1">
            <li>• Click on errors/warnings with line numbers to jump to source code</li>
            <li>• Errors are highlighted in red, warnings in yellow</li>
            <li>• Expand stdout/stderr sections to see full compiler output</li>
          </ul>
        </div>
      )}
    </div>
  );
};

export default LogDisplay;
