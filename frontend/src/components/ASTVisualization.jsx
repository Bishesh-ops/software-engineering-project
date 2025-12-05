import { useState } from 'react';
import ASTNode from './ASTNode';

/**
 * ASTVisualization - Main wrapper component for AST tree visualization
 * Provides controls and displays the root AST structure
 */
const ASTVisualization = ({ data }) => {
  const [expandAll, setExpandAll] = useState(false);
  const [searchTerm, setSearchTerm] = useState('');
  const [maxDepth, setMaxDepth] = useState(10); // Limit depth for performance

  if (!data) {
    return (
      <div className="flex items-center justify-center h-full">
        <div className="text-center text-gray-500 font-mono">
          <div className="text-4xl mb-4">🌳</div>
          <p>No AST data available</p>
          <p className="text-sm mt-2">Compile your code to see the syntax tree</p>
        </div>
      </div>
    );
  }

  // Handle different AST data structures
  const getASTRoot = () => {
    // If data is the full response with stage/declaration_count
    if (data.declarations) {
      return {
        node_type: 'PROGRAM',
        declarations: data.declarations,
        declaration_count: data.declaration_count || data.declarations.length,
      };
    }

    // If data is already a node
    if (data.node_type) {
      return data;
    }

    // If data is an array of declarations
    if (Array.isArray(data)) {
      return {
        node_type: 'PROGRAM',
        declarations: data,
        declaration_count: data.length,
      };
    }

    // Fallback - wrap in program node
    return {
      node_type: 'PROGRAM',
      declarations: [data],
      declaration_count: 1,
    };
  };

  const astRoot = getASTRoot();
  const nodeCount = countNodes(astRoot);

  // Recursively count all nodes
  function countNodes(node) {
    if (!node) return 0;

    let count = 1;

    // Count children
    if (node.declarations) count += node.declarations.reduce((sum, n) => sum + countNodes(n), 0);
    if (node.statements) count += node.statements.reduce((sum, n) => sum + countNodes(n), 0);
    if (node.parameters) count += node.parameters.reduce((sum, n) => sum + countNodes(n), 0);
    if (node.arguments) count += node.arguments.reduce((sum, n) => sum + countNodes(n), 0);
    if (node.body) count += countNodes(node.body);
    if (node.left) count += countNodes(node.left);
    if (node.right) count += countNodes(node.right);
    if (node.operand) count += countNodes(node.operand);
    if (node.expression) count += countNodes(node.expression);
    if (node.condition) count += countNodes(node.condition);
    if (node.then_branch) count += countNodes(node.then_branch);
    if (node.else_branch) count += countNodes(node.else_branch);
    if (node.init) count += countNodes(node.init);
    if (node.increment) count += countNodes(node.increment);

    return count;
  }

  // Export AST as JSON file
  const handleExport = () => {
    const jsonStr = JSON.stringify(astRoot, null, 2);
    const blob = new Blob([jsonStr], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.href = url;
    link.download = 'ast-export.json';
    link.click();
    URL.revokeObjectURL(url);
  };

  return (
    <div className="h-full flex flex-col">
      {/* Controls Header */}
      <div className="p-4 bg-transparent">
        <div className="flex items-center justify-between mb-3">
          <div className="flex items-center space-x-4">
            <div className="text-sm font-mono">
              <span className="text-cyber-green">●</span>
              <span className="text-gray-400 ml-2">{nodeCount} nodes</span>
            </div>

            {astRoot.declaration_count !== undefined && (
              <div className="text-sm font-mono">
                <span className="text-cyber-purple">●</span>
                <span className="text-gray-400 ml-2">
                  {astRoot.declaration_count} declarations
                </span>
              </div>
            )}
          </div>

          <div className="flex items-center space-x-2">
            <button
              onClick={handleExport}
              className="px-3 py-1 text-xs font-mono border border-cyber-blue/50 rounded
                         hover:border-cyber-blue hover:bg-cyber-blue/10 text-cyber-blue
                         transition-all"
            >
              Export JSON
            </button>

            <button
              onClick={() => setExpandAll(!expandAll)}
              className="px-3 py-1 text-xs font-mono border border-cyber-purple/50 rounded
                         hover:border-cyber-purple hover:bg-cyber-purple/10 text-cyber-purple
                         transition-all"
            >
              {expandAll ? 'Collapse All' : 'Expand All'}
            </button>
          </div>
        </div>

        {/* Search bar */}
        <input
          type="text"
          placeholder="Search nodes... (e.g., function name, variable)"
          value={searchTerm}
          onChange={(e) => setSearchTerm(e.target.value)}
          className="w-full px-3 py-2 bg-gray-800 border border-gray-700 rounded
                     text-sm font-mono text-white placeholder-gray-500
                     focus:outline-none focus:border-cyber-blue focus:ring-1 focus:ring-cyber-blue"
        />
      </div>

      {/* Tree Display */}
      <div className="flex-1 overflow-auto p-4">
        <ASTNode
          node={astRoot}
          level={0}
          isLast={true}
          expandAll={expandAll}
          searchTerm={searchTerm}
          maxDepth={maxDepth}
        />
      </div>
    </div>
  );
};

export default ASTVisualization;
