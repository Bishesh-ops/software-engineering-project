/**
 * Cyberpunk/Hacker Theme Configuration
 * Centralized theme constants for consistent styling across the application
 */

export const colors = {
  // Background colors
  background: {
    primary: '#0a0e27',      // Deep dark blue-black (main background)
    secondary: '#0a0a0a',    // Pure black (overlays, panels)
    tertiary: '#1a1e37',     // Slightly lighter (hover states)
    overlay: 'rgba(10, 14, 39, 0.95)', // Translucent overlay
  },

  // Primary neon colors
  neon: {
    green: '#00ff9f',        // Bright neon green
    cyan: '#00d9ff',         // Electric cyan/blue
    pink: '#ff006e',         // Hot pink/magenta
    purple: '#b026ff',       // Neon purple
    yellow: '#ffd700',       // Bright yellow/gold
  },

  // Semantic colors
  semantic: {
    success: '#00ff9f',      // Green for success
    error: '#ff006e',        // Pink/red for errors
    warning: '#ffd700',      // Yellow for warnings
    info: '#00d9ff',         // Cyan for information
  },

  // Text colors
  text: {
    primary: '#e0e0e0',      // Main text (off-white)
    secondary: '#a0a0a0',    // Secondary text (gray)
    muted: '#6a6a6a',        // Muted text (darker gray)
    bright: '#ffffff',       // Bright white (highlights)
  },

  // Border colors with opacity
  border: {
    primary: 'rgba(0, 217, 255, 0.3)',   // Cyan border
    secondary: 'rgba(0, 255, 159, 0.3)', // Green border
    muted: 'rgba(255, 255, 255, 0.1)',   // Subtle white border
  },
};

export const fonts = {
  mono: "'Fira Code', 'JetBrains Mono', 'Courier New', monospace",
  sans: "'Inter', 'Roboto', -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif",
};

export const spacing = {
  xs: '0.25rem',   // 4px
  sm: '0.5rem',    // 8px
  md: '1rem',      // 16px
  lg: '1.5rem',    // 24px
  xl: '2rem',      // 32px
  xxl: '3rem',     // 48px
};

export const borderRadius = {
  sm: '0.25rem',   // 4px
  md: '0.5rem',    // 8px
  lg: '0.75rem',   // 12px
  full: '9999px',  // Fully rounded
};

export const shadows = {
  glow: {
    cyan: '0 0 10px rgba(0, 217, 255, 0.5)',
    green: '0 0 10px rgba(0, 255, 159, 0.5)',
    pink: '0 0 10px rgba(255, 0, 110, 0.5)',
    purple: '0 0 10px rgba(176, 38, 255, 0.5)',
    yellow: '0 0 10px rgba(255, 215, 0, 0.5)',
  },
  glowStrong: {
    cyan: '0 0 20px rgba(0, 217, 255, 0.8)',
    green: '0 0 20px rgba(0, 255, 159, 0.8)',
    pink: '0 0 20px rgba(255, 0, 110, 0.8)',
    purple: '0 0 20px rgba(176, 38, 255, 0.8)',
  },
  elevation: {
    sm: '0 2px 8px rgba(0, 0, 0, 0.5)',
    md: '0 4px 16px rgba(0, 0, 0, 0.6)',
    lg: '0 8px 32px rgba(0, 0, 0, 0.7)',
  },
};

export const animations = {
  duration: {
    fast: '150ms',
    normal: '300ms',
    slow: '500ms',
  },
  easing: {
    default: 'cubic-bezier(0.4, 0, 0.2, 1)',
    in: 'cubic-bezier(0.4, 0, 1, 1)',
    out: 'cubic-bezier(0, 0, 0.2, 1)',
    inOut: 'cubic-bezier(0.4, 0, 0.2, 1)',
  },
};

export const breakpoints = {
  sm: '640px',
  md: '768px',
  lg: '1024px',
  xl: '1280px',
  xxl: '1536px',
};

// Helper function to create neon glow effect
export const createNeonGlow = (color, intensity = 'normal') => {
  const baseGlow = shadows.glow[color] || shadows.glow.cyan;
  const strongGlow = shadows.glowStrong[color] || shadows.glowStrong.cyan;

  return intensity === 'strong' ? strongGlow : baseGlow;
};

// Helper function to create border with glow
export const createNeonBorder = (color, width = '1px') => {
  const colorMap = {
    cyan: colors.neon.cyan,
    green: colors.neon.green,
    pink: colors.neon.pink,
    purple: colors.neon.purple,
    yellow: colors.neon.yellow,
  };

  const borderColor = colorMap[color] || colorMap.cyan;

  return {
    border: `${width} solid ${borderColor}`,
    boxShadow: createNeonGlow(color),
  };
};

export default {
  colors,
  fonts,
  spacing,
  borderRadius,
  shadows,
  animations,
  breakpoints,
  createNeonGlow,
  createNeonBorder,
};
