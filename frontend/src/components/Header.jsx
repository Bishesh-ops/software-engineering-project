import { useState } from 'react';

const Header = () => {
  const [theme, setTheme] = useState('dark');

  const toggleTheme = () => {
    const newTheme = theme === 'dark' ? 'light' : 'dark';
    setTheme(newTheme);
    // Theme toggle functionality can be expanded later
    document.documentElement.classList.toggle('light-mode');
  };

  return (
    <header className="bg-cyber-dark border-b border-cyber-blue/30 px-6 py-4 flex items-center justify-between" style={{ backgroundColor: '#0a0e27', borderBottom: '1px solid rgba(0, 217, 255, 0.3)' }}>
      <div className="flex items-center space-x-4">
        <div>
          <h1 className="text-2xl font-bold text-glow-cyan tracking-wider" style={{ color: '#00d9ff' }}>
            C COMPILER
          </h1>
          <p className="text-xs text-cyber-green/70 font-mono fade-in" style={{ color: '#00ff9f' }}>
            Interactive Visualization Suite
          </p>
        </div>
      </div>

      <div className="flex items-center space-x-4">
        <button
          onClick={toggleTheme}
          className="px-4 py-2 bg-cyber-dark border border-cyber-purple/50 rounded-md
                     hover:border-cyber-purple hover:shadow-neon-purple transition-all duration-200
                     text-cyber-purple hover:text-white font-mono text-sm"
          aria-label="Toggle theme"
        >
          THEME
        </button>

        <div className="text-xs font-mono text-gray-400">
          READY
        </div>
      </div>
    </header>
  );
};

export default Header;
