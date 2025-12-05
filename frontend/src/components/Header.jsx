import { useState, useEffect } from 'react';

const Header = () => {
  const [theme, setTheme] = useState('dark');

  // Initialize theme from localStorage or system preference
  useEffect(() => {
    const savedTheme = localStorage.getItem('theme');
    if (savedTheme) {
      setTheme(savedTheme);
      if (savedTheme === 'light') {
        document.documentElement.classList.add('light-mode');
      }
    }
  }, []);

  const toggleTheme = () => {
    const newTheme = theme === 'dark' ? 'light' : 'dark';
    setTheme(newTheme);
    localStorage.setItem('theme', newTheme);

    if (newTheme === 'light') {
      document.documentElement.classList.add('light-mode');
    } else {
      document.documentElement.classList.remove('light-mode');
    }
  };

  return (
    <header className="bg-cyber-dark border-b border-cyber-blue/30 px-6 py-4 flex items-center justify-between" style={{ backgroundColor: theme === 'dark' ? '#0a0e27' : '#f5f5f7', borderBottom: '1px solid rgba(0, 217, 255, 0.3)' }}>
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
          className={`px-4 py-2 border rounded-md font-mono text-sm transition-all duration-200
                     ${theme === 'dark'
              ? 'bg-cyber-dark border-cyber-purple/50 text-cyber-purple hover:border-cyber-purple hover:text-white'
              : 'bg-white border-purple-400 text-purple-600 hover:border-purple-600 hover:bg-purple-50'}`}
          aria-label="Toggle theme"
        >
          {theme === 'dark' ? 'LIGHT' : 'DARK'}
        </button>

        <div className="text-xs font-mono text-gray-400">
          READY
        </div>
      </div>
    </header>
  );
};

export default Header;
