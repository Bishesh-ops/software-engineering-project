import { useState } from 'react';

/**
 * ASTNode - Recursive component for rendering AST tree nodes
 * Handles all node types: Declarations, Statements, Expressions
 */
const ASTNode = ({
  node,
  level = 0,
  isLast = false,
  expandAll = false,
  searchTerm = '',
  maxDepth = 10,
}) => {
  const [isExpanded, setIsExpanded] = useState(level < 2); // Auto-expand first 2 levels
  const [showDetails, setShowDetails] = useState(false);

  if (!node) return null;

  // Prevent rendering beyond max depth to avoid performance issues
  if (level > maxDepth) {
    return (
      <div className="font-mono text-xs text-gray-600 ml-4">
        ... (max depth {maxDepth} reached)
      </div>
    );
  }

  const nodeType = node.node_type || node.type || 'UNKNOWN';

  // Get color based on node type category
  const getNodeColor = () => {
    // Declarations (green)
    if (nodeType.includes('DECL')) return 'text-cyber-green';
    // Statements (purple)
    if (nodeType.includes('STMT')) return 'text-cyber-purple';
    // Expressions (blue)
    if (nodeType.includes('EXPR')) return 'text-cyber-blue';
    // Types (pink)
    if (nodeType.includes('TYPE')) return 'text-cyber-pink';
    // Default (gray)
    return 'text-gray-400';
  };

  // Get icon based on node type
  const getNodeIcon = () => {
    if (nodeType.includes('FUNCTION')) return '⚙️';
    if (nodeType.includes('VAR')) return '📦';
    if (nodeType.includes('BINARY')) return '➕';
    if (nodeType.includes('LITERAL')) return '🔢';
    if (nodeType.includes('IDENTIFIER')) return '🏷️';
    if (nodeType.includes('RETURN')) return '↩️';
    if (nodeType.includes('IF')) return '❓';
    if (nodeType.includes('WHILE') || nodeType.includes('FOR')) return '🔄';
    if (nodeType.includes('CALL')) return '📞';
    if (nodeType.includes('COMPOUND')) return '📑';
    return '●';
  };

  // Extract displayable properties
  const getNodeLabel = () => {
    const parts = [nodeType];

    if (node.name) parts.push(`"${node.name}"`);
    if (node.operator) parts.push(`(${node.operator})`);
    if (node.value !== undefined) parts.push(`= ${JSON.stringify(node.value)}`);
    if (node.return_type) parts.push(`→ ${node.return_type}`);

    return parts.join(' ');
  };

  // Get all children nodes
  const getChildren = () => {
    const children = [];

    // Common child properties
    if (node.body) children.push({ key: 'body', node: node.body });
    if (node.declarations) {
      node.declarations.forEach((decl, i) =>
        children.push({ key: `decl_${i}`, node: decl })
      );
    }
    if (node.statements) {
      node.statements.forEach((stmt, i) =>
        children.push({ key: `stmt_${i}`, node: stmt })
      );
    }
    if (node.parameters) {
      node.parameters.forEach((param, i) =>
        children.push({ key: `param_${i}`, node: param })
      );
    }

    // Expression children
    if (node.left) children.push({ key: 'left', node: node.left });
    if (node.right) children.push({ key: 'right', node: node.right });
    if (node.operand) children.push({ key: 'operand', node: node.operand });
    if (node.expression) children.push({ key: 'expr', node: node.expression });
    if (node.condition) children.push({ key: 'condition', node: node.condition });
    if (node.then_branch) children.push({ key: 'then', node: node.then_branch });
    if (node.else_branch) children.push({ key: 'else', node: node.else_branch });
    if (node.init) children.push({ key: 'init', node: node.init });
    if (node.increment) children.push({ key: 'increment', node: node.increment });

    // Function call arguments
    if (node.arguments) {
      node.arguments.forEach((arg, i) =>
        children.push({ key: `arg_${i}`, node: arg })
      );
    }

    return children;
  };

  const children = getChildren();
  const hasChildren = children.length > 0;

  // Tree branch characters
  const indent = '  '.repeat(level);
  const branch = isLast ? '└─' : '├─';
  const vertical = isLast ? '  ' : '│ ';

  return (
    <div className="font-mono text-sm">
      <div className="flex items-start hover:bg-cyber-blue/5 transition-colors">
        {/* Tree structure indicators */}
        <span className="text-gray-600 select-none whitespace-pre">{indent}</span>
        <span className="text-gray-600 select-none">{branch}</span>

        {/* Expand/collapse button */}
        {hasChildren && (
          <button
            onClick={() => setIsExpanded(!isExpanded)}
            className="w-4 h-4 flex items-center justify-center text-cyber-blue hover:text-white mx-1"
          >
            {isExpanded ? '▼' : '▶'}
          </button>
        )}

        {/* Node content */}
        <div className="flex-1">
          <button
            onClick={() => setShowDetails(!showDetails)}
            className={`${getNodeColor()} hover:brightness-150 transition-all`}
          >
            <span className="mr-2">{getNodeIcon()}</span>
            <span className="font-bold">{getNodeLabel()}</span>
          </button>

          {/* Node details (on click) */}
          {showDetails && (
            <div className="ml-6 mt-2 p-3 bg-gray-800/50 border border-cyber-blue/30 rounded text-xs">
              <div className="text-cyber-blue font-bold mb-2">Node Details:</div>
              {node.location && (
                <div className="text-gray-400">
                  📍 Location: Line {node.location.line}, Column {node.location.column}
                </div>
              )}
              {node.data_type && (
                <div className="text-gray-400">🏷️ Type: {node.data_type}</div>
              )}
              <details className="mt-2">
                <summary className="cursor-pointer text-cyber-purple hover:text-white">
                  Full JSON
                </summary>
                <pre className="mt-1 text-gray-500 overflow-x-auto">
                  {JSON.stringify(node, null, 2)}
                </pre>
              </details>
            </div>
          )}
        </div>
      </div>

      {/* Render children recursively */}
      {hasChildren && isExpanded && (
        <div>
          {children.map((child, index) => (
            <ASTNode
              key={child.key}
              node={child.node}
              level={level + 1}
              isLast={index === children.length - 1}
              expandAll={expandAll}
              searchTerm={searchTerm}
              maxDepth={maxDepth}
            />
          ))}
        </div>
      )}
    </div>
  );
};

export default ASTNode;
