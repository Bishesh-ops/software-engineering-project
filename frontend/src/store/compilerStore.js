import { create } from 'zustand';
import { persist, createJSONStorage } from 'zustand/middleware';
import { useShallow } from 'zustand/react/shallow';
import { compileCodeWithRetry, formatErrorMessage } from '../services/compilerService';

/**
 * Compiler Store - Central state management for the compiler application
 * Uses Zustand for lightweight, performant state management
 */
const useCompilerStore = create(
  persist(
    (set, get) => ({
      // ========== STATE ==========

      // Source code
      source: `int main() {
    int x = 42;
    return x;
}`,

      // Compilation status
      isCompiling: false,
      compilationStatus: 'ready', // 'ready' | 'compiling' | 'success' | 'error'

      // Compilation results
      result: {
        tokens: null,
        ast: null,
        assembly: null,
        hex: null,
        logs: {
          stdout: '',
          stderr: '',
        },
      },

      // Error state
      error: null,

      // UI state
      activeTab: 'tokens', // 'tokens' | 'ast' | 'assembly' | 'hex'

      // Log messages
      logs: [],

      // Editor settings
      editorSettings: {
        fontSize: 14,
        theme: 'cyberpunk',
        wordWrap: 'on',
      },

      // ========== ACTIONS ==========

      /**
       * Update source code
       */
      setSource: (source) => set({ source }),

      /**
       * Set active output tab
       */
      setActiveTab: (tab) => set({ activeTab: tab }),

      /**
       * Add log message
       */
      addLog: (log) =>
        set((state) => ({
          logs: [
            ...state.logs,
            {
              ...log,
              timestamp: new Date().toISOString(),
            },
          ],
        })),

      /**
       * Clear logs
       */
      clearLogs: () => set({ logs: [] }),

      /**
       * Update editor settings
       */
      updateEditorSettings: (settings) =>
        set((state) => ({
          editorSettings: {
            ...state.editorSettings,
            ...settings,
          },
        })),

      /**
       * Compile the current source code
       */
      compile: async () => {
        const { source } = get();

        // Reset state
        set({
          isCompiling: true,
          compilationStatus: 'compiling',
          error: null,
          logs: [
            {
              type: 'info',
              message: 'Starting compilation...',
              timestamp: new Date().toISOString(),
            },
          ],
        });

        try {
          // Call compiler API with retry logic
          const result = await compileCodeWithRetry(source);

          // Update state with successful result
          set({
            isCompiling: false,
            compilationStatus: 'success',
            result: {
              tokens: result.tokens,
              ast: result.ast,
              assembly: result.assembly,
              hex: result.hex,
              logs: {
                stdout: result.stdout || '',
                stderr: result.stderr || '',
              },
            },
          });

          // Add success log
          get().addLog({
            type: 'success',
            message: '✅ Compilation successful!',
          });

          // Add warnings if any
          if (result.warnings && result.warnings.length > 0) {
            result.warnings.forEach((warning) => {
              get().addLog({
                type: 'warning',
                message: `⚠️ ${warning}`,
              });
            });
          }

          return result;
        } catch (error) {
          // Format error message
          const errorMessage = formatErrorMessage(error);

          // Update state with error
          set({
            isCompiling: false,
            compilationStatus: 'error',
            error: {
              message: errorMessage,
              details: error.details || null,
              status: error.status || 0,
            },
          });

          // Add error log
          get().addLog({
            type: 'error',
            message: errorMessage,
          });

          // Add detailed error info if available
          if (error.details) {
            get().addLog({
              type: 'info',
              message: `Details: ${JSON.stringify(error.details)}`,
            });
          }

          throw error;
        }
      },

      /**
       * Reset compilation results
       */
      resetResults: () =>
        set({
          result: {
            tokens: null,
            ast: null,
            assembly: null,
            hex: null,
            logs: {
              stdout: '',
              stderr: '',
            },
          },
          error: null,
          compilationStatus: 'ready',
          logs: [],
        }),

      /**
       * Reset all state to defaults
       */
      reset: () =>
        set({
          source: `int main() {
    int x = 42;
    return x;
}`,
          isCompiling: false,
          compilationStatus: 'ready',
          result: {
            tokens: null,
            ast: null,
            assembly: null,
            hex: null,
            logs: {
              stdout: '',
              stderr: '',
            },
          },
          error: null,
          activeTab: 'tokens',
          logs: [],
        }),
    }),
    {
      name: 'compiler-storage', // localStorage key
      storage: createJSONStorage(() => localStorage),
      // Only persist certain fields
      partialize: (state) => ({
        source: state.source,
        editorSettings: state.editorSettings,
        activeTab: state.activeTab,
      }),
    }
  )
);

// ========== SELECTORS ==========
// These are helper functions to select specific parts of the state
// Using useShallow to prevent infinite re-renders when returning objects

/**
 * Select source code
 */
export const useSource = () => useCompilerStore((state) => state.source);

/**
 * Select compilation status - use useShallow to prevent infinite loops
 */
export const useCompilationStatus = () =>
  useCompilerStore(
    useShallow((state) => ({
      isCompiling: state.isCompiling,
      status: state.compilationStatus,
    }))
  );

/**
 * Select compilation results
 */
export const useCompilationResults = () =>
  useCompilerStore((state) => state.result);

/**
 * Select active tab
 */
export const useActiveTab = () => useCompilerStore((state) => state.activeTab);

/**
 * Select logs
 */
export const useLogs = () => useCompilerStore((state) => state.logs);

/**
 * Select error state
 */
export const useError = () => useCompilerStore((state) => state.error);

/**
 * Select editor settings
 */
export const useEditorSettings = () =>
  useCompilerStore((state) => state.editorSettings);

/**
 * Select actions only - use useShallow to prevent infinite loops
 */
export const useCompilerActions = () =>
  useCompilerStore(
    useShallow((state) => ({
      setSource: state.setSource,
      setActiveTab: state.setActiveTab,
      compile: state.compile,
      addLog: state.addLog,
      clearLogs: state.clearLogs,
      resetResults: state.resetResults,
      reset: state.reset,
      updateEditorSettings: state.updateEditorSettings,
    }))
  );

export default useCompilerStore;
