import { useState } from 'react';
import LogDisplay from './LogDisplay';

const BottomBar = ({ onCompile, isCompiling, status, logs, error, onScrollToLine }) => {
  const [isExpanded, setIsExpanded] = useState(false);
  const getStatusColor = () => {
    switch (status) {
      case 'success':
        return 'text-cyber-green';
      case 'error':
        return 'text-cyber-pink';
      case 'compiling':
        return 'text-cyber-blue animate-pulse';
      default:
        return 'text-gray-400';
    }
  };

  const getStatusIcon = () => {
    switch (status) {
      case 'success':
        return '✓';
      case 'error':
        return '✗';
      case 'compiling':
        return '⟳';
      default:
        return '●';
    }
  };

  const getStatusText = () => {
    switch (status) {
      case 'success':
        return 'COMPILATION SUCCESSFUL';
      case 'error':
        return 'COMPILATION FAILED';
      case 'compiling':
        return 'COMPILING...';
      default:
        return 'READY';
    }
  };

  const errorCount = logs.filter((log) => log.type === 'error').length + (error ? 1 : 0);
  const warningCount = logs.filter((log) => log.type === 'warning').length;

  return (
    <div className="border-t border-cyber-blue/30 bg-cyber-dark">
      {/* Status Bar */}
      <div className="px-6 py-3 flex items-center justify-between">
        <div className="flex items-center space-x-4">
          <button
            onClick={onCompile}
            disabled={isCompiling}
            className={`px-6 py-2 font-mono font-bold text-sm tracking-wider
                       border rounded-md transition-all duration-300 gpu-accelerated
                       ${
                         isCompiling
                           ? 'bg-gray-700 border-gray-600 text-gray-400 cursor-not-allowed'
                           : 'bg-cyber-dark border-cyber-blue text-cyber-blue hover:bg-cyber-blue hover:text-black hover-brightness'
                       }`}
          >
            {isCompiling ? (
              <span className="flex items-center space-x-2">
                <span className="animate-spin">⟳</span>
                <span>COMPILING...</span>
              </span>
            ) : (
              <span className="flex items-center space-x-2">
                <span>▶</span>
                <span>COMPILE</span>
              </span>
            )}
          </button>

          <div className={`flex items-center space-x-2 font-mono text-sm ${getStatusColor()}`}>
            <span>{getStatusIcon()}</span>
            <span>{getStatusText()}</span>
          </div>

          {/* Error/Warning counts */}
          {(errorCount > 0 || warningCount > 0) && (
            <div className="flex items-center space-x-3 text-xs font-mono">
              {errorCount > 0 && (
                <span className="text-red-400">❌ {errorCount} errors</span>
              )}
              {warningCount > 0 && (
                <span className="text-yellow-400">⚠️ {warningCount} warnings</span>
              )}
            </div>
          )}

          {/* Toggle logs button */}
          {logs.length > 0 && (
            <button
              onClick={() => setIsExpanded(!isExpanded)}
              className="px-3 py-1 text-xs font-mono border border-gray-600 rounded
                         hover:border-cyber-blue hover:bg-cyber-blue/10 text-gray-400
                         hover:text-cyber-blue transition-all"
            >
              {isExpanded ? '▼ Hide Logs' : '▲ Show Logs'}
            </button>
          )}
        </div>

        <div className="flex items-center space-x-4 text-xs font-mono text-gray-500">
          <span>API: localhost:5001</span>
          <span>|</span>
          <span>mycc compiler</span>
        </div>
      </div>

      {/* Expandable Log Display */}
      {isExpanded && logs.length > 0 && (
        <div className="border-t border-cyber-blue/20 bg-cyber-dark/50 h-80">
          <LogDisplay logs={logs} error={error} onScrollToLine={onScrollToLine} />
        </div>
      )}
    </div>
  );
};

export default BottomBar;
