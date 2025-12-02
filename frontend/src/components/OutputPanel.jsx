import { useActiveTab, useCompilerActions, useCompilationStatus } from '../store/compilerStore';
import ASTVisualization from './ASTVisualization';
import AssemblyDisplay from './AssemblyDisplay';
import HexDisplay from './HexDisplay';

const OutputPanel = ({ data }) => {
  const activeTab = useActiveTab();
  const { setActiveTab } = useCompilerActions();
  const { isCompiling } = useCompilationStatus();

  const tabs = [
    { id: 'tokens', label: 'TOKENS', icon: '🔤', color: 'blue' },
    { id: 'ast', label: 'AST', icon: '🌳', color: 'green' },
    { id: 'assembly', label: 'ASSEMBLY', icon: '⚙️', color: 'purple' },
    { id: 'hex', label: 'HEX DUMP', icon: '🔢', color: 'pink' },
  ];

  const getTabColorClass = (color, isActive) => {
    if (isActive) {
      return {
        blue: 'border-cyber-blue text-cyber-blue neon-border-blue',
        green: 'border-cyber-green text-cyber-green neon-border-green',
        purple: 'border-cyber-purple text-cyber-purple neon-border-purple',
        pink: 'border-cyber-pink text-cyber-pink neon-border-pink',
      }[color];
    }
    return 'border-gray-600 text-gray-500 hover:text-white hover:border-gray-400';
  };

  const renderContent = () => {
    // Show loading state while compiling
    if (isCompiling) {
      return (
        <div className="h-full flex items-center justify-center p-4">
          <div className="text-center text-gray-400 font-mono fade-in">
            <div className="text-6xl mb-4 spin">⟳</div>
            <p className="text-lg">Compiling...</p>
            <p className="text-sm mt-2 text-gray-600">
              Analyzing {activeTab === 'tokens' ? 'tokens' : activeTab === 'ast' ? 'syntax tree' : activeTab === 'assembly' ? 'assembly' : 'hex dump'}
            </p>
          </div>
        </div>
      );
    }

    if (!data[activeTab]) {
      return (
        <div className="h-full flex items-center justify-center p-4">
          <div className="text-center text-gray-500 font-mono fade-in">
            <div className="text-4xl mb-4">📋</div>
            <p>No data available</p>
            <p className="text-sm mt-2">Compile your code to see results</p>
          </div>
        </div>
      );
    }

    switch (activeTab) {
      case 'tokens':
        return <TokenView data={data.tokens} />;
      case 'ast':
        return <ASTView data={data.ast} />;
      case 'assembly':
        return <AssemblyView data={data.assembly} />;
      case 'hex':
        return <HexView data={data.hex} />;
      default:
        return null;
    }
  };

  return (
    <div className="flex-1 flex flex-col">
      {/* Tab Headers */}
      <div className="flex border-b border-cyber-blue/30 bg-cyber-dark/50">
        {tabs.map((tab) => (
          <button
            key={tab.id}
            onClick={() => setActiveTab(tab.id)}
            className={`px-6 py-3 font-mono text-sm font-bold tracking-wider transition-all duration-200
                       border-b-2 ${getTabColorClass(tab.color, activeTab === tab.id)}`}
          >
            <span className="flex items-center space-x-2">
              <span>{tab.icon}</span>
              <span>{tab.label}</span>
            </span>
          </button>
        ))}
      </div>

      {/* Tab Content */}
      <div className="flex-1 overflow-hidden bg-cyber-dark/30">
        {renderContent()}
      </div>
    </div>
  );
};

// Token View Component
const TokenView = ({ data }) => {
  const tokens = Array.isArray(data) ? data : data?.tokens || [];

  return (
    <div className="h-full flex flex-col">
      {/* Header with stats */}
      <div className="p-4 border-b border-cyber-blue/20 bg-cyber-dark/50">
        <div className="flex items-center space-x-4 text-sm font-mono">
          <div>
            <span className="text-cyber-blue">●</span>
            <span className="text-gray-400 ml-2">{tokens.length} tokens</span>
          </div>
        </div>
      </div>

      {/* Scrollable table container */}
      <div className="flex-1 overflow-auto p-4">
        <table className="w-full text-sm font-mono">
          <thead className="sticky top-0 bg-cyber-dark">
            <tr className="text-cyber-blue border-b border-cyber-blue/30">
              <th className="text-left py-2 px-4">#</th>
              <th className="text-left py-2 px-4">TYPE</th>
              <th className="text-left py-2 px-4">VALUE</th>
              <th className="text-left py-2 px-4">LINE</th>
              <th className="text-left py-2 px-4">COLUMN</th>
            </tr>
          </thead>
          <tbody>
            {tokens.map((token, index) => (
              <tr
                key={index}
                className="border-b border-gray-800 hover:bg-cyber-blue/5 transition-colors"
              >
                <td className="py-2 px-4 text-gray-500">{index + 1}</td>
                <td className="py-2 px-4 text-cyber-green">{token.type}</td>
                <td className="py-2 px-4 text-white">{token.value}</td>
                <td className="py-2 px-4 text-gray-400">{token.location?.line || '-'}</td>
                <td className="py-2 px-4 text-gray-400">{token.location?.column || '-'}</td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
};

// AST View Component
const ASTView = ({ data }) => {
  return <ASTVisualization data={data} />;
};

// Assembly View Component
const AssemblyView = ({ data }) => {
  return <AssemblyDisplay data={data} />;
};

// Hex View Component
const HexView = ({ data }) => {
  return <HexDisplay data={data} />;
};

export default OutputPanel;
