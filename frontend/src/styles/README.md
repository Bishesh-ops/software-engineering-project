# Cyberpunk Theme System

This directory contains the complete theme and styling system for the C Compiler Visualization Frontend.

## Files

### `theme.js`
JavaScript theme configuration with constants and helper functions.

**Exports:**
- `colors` - Color palette (background, neon, semantic, text, border)
- `fonts` - Font families (mono, sans)
- `spacing` - Spacing scale (xs to xxl)
- `borderRadius` - Border radius values
- `shadows` - Shadow and glow effects
- `animations` - Animation durations and easing functions
- `breakpoints` - Responsive breakpoints
- `createNeonGlow()` - Helper to create neon glow effects
- `createNeonBorder()` - Helper to create borders with glow

**Usage Example:**
```javascript
import theme from '../styles/theme';

const MyComponent = () => (
  <div style={{
    color: theme.colors.neon.cyan,
    fontFamily: theme.fonts.mono,
    ...theme.createNeonBorder('cyan', '2px')
  }}>
    Neon Text
  </div>
);
```

### `global.css`
Global CSS styles including animations, utilities, and effects.

**Includes:**
1. **Animations:**
   - `textFlicker` - Subtle text flicker effect
   - `neonPulse` - Neon glow pulse
   - `scanline` - Moving scanline animation
   - `glitch` - Glitch effect
   - `fadeIn`, `fadeInUp`, `slideInRight` - Entry animations
   - `spin` - Loading spinner
   - `cursorBlink` - Terminal cursor blink

2. **Utility Classes:**
   - `.text-flicker` - Apply flicker animation
   - `.neon-pulse` - Apply pulse animation
   - `.glitch` - Apply glitch effect
   - `.fade-in`, `.fade-in-up`, `.slide-in-right` - Entry animations
   - `.spin` - Spinning animation
   - `.cursor-blink` - Blinking cursor

3. **Effects:**
   - `.scanline-overlay` - CRT scanline overlay (applied to entire app)
   - `.text-glow-cyan`, `.text-glow-green`, `.text-glow-pink`, `.text-glow-purple` - Neon text glows
   - `.hover-brightness`, `.hover-scale` - Hover effects

4. **Scrollbar Styling:**
   - Custom webkit scrollbar with cyan theme
   - Firefox scrollbar support

5. **Responsive Utilities:**
   - `.hide-mobile`, `.hide-tablet`, `.hide-desktop` - Responsive visibility
   - `.grid-cols-16` - 16-column grid for hex display

6. **Performance:**
   - `.gpu-accelerated` - Hardware acceleration
   - `@media (prefers-reduced-motion)` - Respects user motion preferences

## Color Palette

### Neon Colors
- **Cyan**: `#00d9ff` - Primary accent, borders, highlights
- **Green**: `#00ff9f` - Success, secondary accent
- **Pink**: `#ff006e` - Errors, warnings
- **Purple**: `#b026ff` - Special elements
- **Yellow**: `#ffd700` - Warnings, highlights

### Backgrounds
- **Primary**: `#0a0e27` - Main background (dark blue-black)
- **Secondary**: `#0a0a0a` - Pure black for overlays
- **Tertiary**: `#1a1e37` - Hover states

### Text
- **Primary**: `#e0e0e0` - Main text (off-white)
- **Secondary**: `#a0a0a0` - Secondary text (gray)
- **Muted**: `#6a6a6a` - Muted text (dark gray)
- **Bright**: `#ffffff` - Highlights

## Typography

### Fonts
- **Monospace**: `'Fira Code', 'JetBrains Mono', 'Courier New', monospace`
  - Used for: code, data, technical output
- **Sans-serif**: `'Inter', 'Roboto', system fonts`
  - Used for: UI text (currently unused)

## Effects

### CRT Scanline Overlay
The `.scanline-overlay` creates a retro CRT monitor effect:
- Horizontal scanlines across the entire viewport
- Moving cyan scan line
- Automatically disabled on mobile for performance
- Can be disabled with `prefers-reduced-motion`

**Implementation:**
```jsx
// In App.jsx
<div className="scanline-overlay" aria-hidden="true" />
```

### Neon Glow Effects
Multiple intensity levels available:

**Text Glow:**
```jsx
<h1 className="text-glow-cyan">Glowing Text</h1>
```

**Glow Pulse:**
```jsx
<div className="neon-pulse border border-cyber-blue">Pulsing Element</div>
```

### Animations

**Text Flicker:**
```jsx
<span className="text-flicker">⚡</span>
```

**Fade In:**
```jsx
<div className="fade-in">Content</div>
<div className="fade-in-up">Slides up while fading</div>
```

**Loading Spinner:**
```jsx
<span className="spin">⟳</span>
```

## Responsive Design

### Breakpoints
- **sm**: 640px (small phones)
- **md**: 768px (tablets)
- **lg**: 1024px (laptops)
- **xl**: 1280px (desktops)
- **xxl**: 1536px (large monitors)

### Responsive Utilities
```jsx
<div className="hide-mobile">Desktop only</div>
<div className="hide-desktop">Mobile only</div>
```

## Accessibility

### Motion Preferences
The system respects `prefers-reduced-motion`:
- Disables all animations
- Removes scanline overlay
- Reduces transition durations to minimal

### Focus Styles
Custom focus ring using cyan color:
```css
*:focus-visible {
  outline: 2px solid #00d9ff;
  outline-offset: 2px;
}
```

## Performance Optimizations

### GPU Acceleration
Use `.gpu-accelerated` for animated elements:
```jsx
<button className="gpu-accelerated hover-scale">Button</button>
```

### Hardware Acceleration
Enabled for transform animations using:
- `will-change: transform`
- `transform: translateZ(0)`
- `backface-visibility: hidden`

## Browser Support

### CSS Features
- ✅ Webkit scrollbar styling (Chrome, Safari, Edge)
- ✅ Firefox scrollbar styling
- ✅ CSS animations and keyframes
- ✅ CSS grid (hex display)
- ✅ Media queries

### Tested On
- Chrome/Edge 100+
- Firefox 100+
- Safari 15+

## Tips

1. **Consistent Spacing**: Use `theme.spacing` values instead of arbitrary numbers
2. **Color Usage**: Use semantic colors from `theme.colors.semantic` for consistent meaning
3. **Animations**: Apply sparingly - too many animations can be distracting
4. **Performance**: Add `gpu-accelerated` class to frequently animated elements
5. **Accessibility**: Always test with `prefers-reduced-motion` enabled

## Examples

### Creating a Neon Button
```jsx
<button className="
  px-6 py-2
  border border-cyber-blue rounded
  text-cyber-blue
  hover:bg-cyber-blue hover:text-black
  hover-brightness
  transition-all duration-300
  gpu-accelerated
">
  Click Me
</button>
```

### Glowing Header
```jsx
<h1 className="text-glow-cyan text-flicker">
  SYSTEM ONLINE
</h1>
```

### Card with Neon Border
```jsx
<div className="
  bg-cyber-dark/50
  border border-cyber-green/30
  rounded-lg p-4
  hover-brightness
  transition-all
">
  Card Content
</div>
```
