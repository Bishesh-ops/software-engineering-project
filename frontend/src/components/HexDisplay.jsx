import { useState } from 'react';

/**
 * HexDisplay - Professional hex dump viewer with hacker aesthetic
 * Displays binary data in traditional hex dump format with ASCII representation
 */
const HexDisplay = ({ data }) => {
  const [copied, setCopied] = useState(false);

  if (!data || typeof data !== 'string') {
    return (
      <div className="flex items-center justify-center h-full">
        <div className="text-center text-gray-500 font-mono">
          <div className="text-4xl mb-4">🔢</div>
          <p>No hex dump available</p>
          <p className="text-sm mt-2">Compile your code to see the binary output</p>
        </div>
      </div>
    );
  }

  // Parse hex dump: each line should have hex bytes separated by spaces
  const parseHexDump = (hexString) => {
    const lines = hexString.trim().split('\n');
    const rows = [];

    lines.forEach((line, index) => {
      const bytes = line.trim().split(/\s+/).filter(b => b.length > 0);

      // Only process lines with valid hex bytes
      if (bytes.length > 0 && bytes.every(b => /^[0-9A-Fa-f]{2}$/.test(b))) {
        rows.push({
          offset: index * 16,
          bytes: bytes,
        });
      }
    });

    return rows;
  };

  const rows = parseHexDump(data);

  // Convert hex byte to ASCII character
  const hexToAscii = (hexByte) => {
    const code = parseInt(hexByte, 16);
    // Printable ASCII range (32-126)
    if (code >= 32 && code <= 126) {
      return String.fromCharCode(code);
    }
    return '.';
  };

  // Check if character is printable
  const isPrintable = (hexByte) => {
    const code = parseInt(hexByte, 16);
    return code >= 32 && code <= 126;
  };

  // Copy hex dump to clipboard
  const handleCopy = async () => {
    try {
      await navigator.clipboard.writeText(data);
      setCopied(true);
      setTimeout(() => setCopied(false), 2000);
    } catch (err) {
      console.error('Failed to copy:', err);
    }
  };

  // Download hex dump as file
  const handleDownload = () => {
    const blob = new Blob([data], { type: 'text/plain' });
    const url = URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.href = url;
    link.download = 'hexdump.hex';
    link.click();
    URL.revokeObjectURL(url);
  };

  // Calculate statistics
  const totalBytes = rows.reduce((sum, row) => sum + row.bytes.length, 0);
  const printableCount = rows.reduce((sum, row) =>
    sum + row.bytes.filter(isPrintable).length, 0
  );

  return (
    <div className="h-full flex flex-col">
      {/* Header with controls */}
      <div className="p-4 border-b border-cyber-green/20 bg-cyber-dark/50">
        <div className="flex items-center justify-between mb-3">
          {/* Stats */}
          <div className="flex items-center space-x-4 text-sm font-mono">
            <div>
              <span className="text-cyber-green">●</span>
              <span className="text-gray-400 ml-2">{totalBytes} bytes</span>
            </div>

            <div>
              <span className="text-cyber-blue">●</span>
              <span className="text-gray-400 ml-2">{rows.length} rows</span>
            </div>

            <div>
              <span className="text-cyber-pink">●</span>
              <span className="text-gray-400 ml-2">
                {printableCount} printable ({Math.round((printableCount / totalBytes) * 100)}%)
              </span>
            </div>
          </div>

          {/* Action buttons */}
          <div className="flex items-center space-x-2">
            <button
              onClick={handleCopy}
              className="px-3 py-1 text-xs font-mono border border-cyber-green/50 rounded
                         hover:border-cyber-green hover:bg-cyber-green/10 text-cyber-green
                         transition-all flex items-center space-x-1"
            >
              <span>{copied ? '✓' : '📋'}</span>
              <span>{copied ? 'Copied!' : 'Copy'}</span>
            </button>

            <button
              onClick={handleDownload}
              className="px-3 py-1 text-xs font-mono border border-cyber-pink/50 rounded
                         hover:border-cyber-pink hover:bg-cyber-pink/10 text-cyber-pink
                         transition-all"
            >
              💾 Download
            </button>
          </div>
        </div>

        {/* Format legend */}
        <div className="text-xs font-mono text-gray-500">
          <span className="text-cyber-blue">Address</span>
          <span className="mx-2">|</span>
          <span className="text-cyber-pink">Hex Bytes (16 per line)</span>
          <span className="mx-2">|</span>
          <span className="text-cyber-green">ASCII</span>
        </div>
      </div>

      {/* Hex dump display */}
      <div className="flex-1 overflow-auto p-4 bg-black/30">
        <div className="font-mono text-sm">
          {rows.map((row, rowIndex) => (
            <div
              key={rowIndex}
              className="flex items-start hover:bg-cyber-green/5 transition-colors py-1"
            >
              {/* Address offset */}
              <div className="text-cyber-blue w-24 text-right pr-4 select-none">
                {row.offset.toString(16).padStart(8, '0').toUpperCase()}
              </div>

              {/* Hex bytes */}
              <div className="flex-1">
                <div className="grid grid-cols-16 gap-2 text-cyber-pink">
                  {row.bytes.map((byte, byteIndex) => (
                    <span
                      key={byteIndex}
                      className="text-center hover:bg-cyber-pink/20 transition-colors"
                    >
                      {byte.toUpperCase()}
                    </span>
                  ))}
                  {/* Fill empty spaces for incomplete rows */}
                  {Array(16 - row.bytes.length)
                    .fill(0)
                    .map((_, i) => (
                      <span key={`empty-${i}`} className="text-gray-800">
                        --
                      </span>
                    ))}
                </div>
              </div>

              {/* ASCII representation */}
              <div className="w-40 ml-6 pl-4 border-l border-gray-800">
                <div className="flex">
                  {row.bytes.map((byte, byteIndex) => {
                    const char = hexToAscii(byte);
                    const printable = isPrintable(byte);
                    return (
                      <span
                        key={byteIndex}
                        className={`${
                          printable
                            ? 'text-cyber-green font-bold'
                            : 'text-gray-600'
                        }`}
                      >
                        {char}
                      </span>
                    );
                  })}
                </div>
              </div>
            </div>
          ))}
        </div>

        {/* Empty state for no data */}
        {rows.length === 0 && (
          <div className="text-center text-gray-600 font-mono py-8">
            <div className="text-2xl mb-2">⚠️</div>
            <p>Invalid hex dump format</p>
            <p className="text-xs mt-2">Expected format: space-separated hex bytes</p>
          </div>
        )}
      </div>

      {/* Footer legend */}
      <div className="p-3 border-t border-cyber-green/20 bg-black/20 text-xs font-mono text-gray-500">
        <div className="flex items-center justify-between">
          <div className="flex items-center space-x-4">
            <span className="text-cyber-green">■ Printable ASCII</span>
            <span className="text-gray-600">■ Non-printable</span>
            <span className="text-cyber-pink">■ Hex Bytes</span>
            <span className="text-cyber-blue">■ Address Offset</span>
          </div>
          <span className="text-gray-600">16 bytes per row • Hover to highlight</span>
        </div>
      </div>
    </div>
  );
};

export default HexDisplay;
