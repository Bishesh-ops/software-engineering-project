import { useEffect, Component } from 'react';
import Layout from './components/Layout';
import './App.css';

// Error Boundary to catch React errors
class ErrorBoundary extends Component {
  constructor(props) {
    super(props);
    this.state = { hasError: false, error: null };
  }

  static getDerivedStateFromError(error) {
    return { hasError: true, error };
  }

  componentDidCatch(error, errorInfo) {
    console.error('App Error:', error, errorInfo);
  }

  render() {
    if (this.state.hasError) {
      return (
        <div style={{
          backgroundColor: '#0a0e27',
          color: '#ff006e',
          minHeight: '100vh',
          padding: '40px',
          fontFamily: 'monospace'
        }}>
          <h1 style={{ color: '#00d9ff' }}>⚠️ Something went wrong</h1>
          <pre style={{ color: '#ff006e', marginTop: '20px' }}>
            {this.state.error?.toString()}
          </pre>
          <button
            onClick={() => window.location.reload()}
            style={{
              marginTop: '20px',
              padding: '10px 20px',
              backgroundColor: '#00d9ff',
              color: '#0a0e27',
              border: 'none',
              cursor: 'pointer'
            }}
          >
            Reload Page
          </button>
        </div>
      );
    }
    return this.props.children;
  }
}

/**
 * Main Application Component
 * Integrates all components with state management and theming
 */
function App() {
  useEffect(() => {
    // Log application initialization
    console.log('🚀 C Compiler Visualization Frontend initialized');
    console.log('📡 API Endpoint:', import.meta.env.VITE_API_URL || 'http://localhost:5001');

    // Check if backend is running (optional health check)
    const checkBackend = async () => {
      try {
        const response = await fetch('http://localhost:5001/health', {
          method: 'GET',
          signal: AbortSignal.timeout(2000),
        });
        if (response.ok) {
          console.log('✅ Backend server is running');
        }
      } catch (error) {
        console.warn('⚠️ Backend server not responding. Make sure Flask API is running on port 5001.');
      }
    };

    checkBackend();
  }, []);

  return (
    <ErrorBoundary>
      {/* CRT Scanline Overlay Effect - Retro Terminal Aesthetic */}
      <div className="scanline-overlay" aria-hidden="true" />

      {/* Main Application Layout */}
      <Layout />
    </ErrorBoundary>
  );
}

export default App;
