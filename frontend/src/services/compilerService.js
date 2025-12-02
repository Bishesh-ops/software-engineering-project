/**
 * Compiler Service - API Communication Layer
 * Handles all communication with the Flask backend API
 */

const API_BASE_URL = import.meta.env.VITE_API_URL || 'http://localhost:5001';
const COMPILE_ENDPOINT = `${API_BASE_URL}/compile`;

/**
 * Custom error class for API errors
 */
class APIError extends Error {
  constructor(message, status, details) {
    super(message);
    this.name = 'APIError';
    this.status = status;
    this.details = details;
  }
}

/**
 * Compile C source code and get visualization data
 *
 * @param {string} source - The C source code to compile
 * @returns {Promise<Object>} Compilation results with tokens, ast, assembly, hex
 * @throws {APIError} If compilation fails or network error occurs
 */
export async function compileCode(source) {
  // Validate input
  if (!source || typeof source !== 'string') {
    throw new Error('Source code must be a non-empty string');
  }

  try {
    const response = await fetch(COMPILE_ENDPOINT, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'Accept': 'application/json',
      },
      body: JSON.stringify({ source: source }),
      // Add timeout using AbortController
      signal: AbortSignal.timeout(30000), // 30 second timeout
    });

    // Parse response
    const data = await response.json();

    // Handle HTTP errors
    if (!response.ok) {
      throw new APIError(
        data.error || 'Compilation failed',
        response.status,
        data.details || null
      );
    }

    // Validate response structure
    if (!data || typeof data !== 'object') {
      throw new APIError(
        'Invalid response format from server',
        response.status,
        null
      );
    }

    return {
      success: true,
      tokens: data.tokens || null,
      ast: data.ast || null,
      assembly: data.assembly || null,
      hex: data.hexdump || data.hex || null,
      errors: data.errors || [],
      warnings: data.warnings || [],
    };
  } catch (error) {
    // Handle network errors
    if (error.name === 'AbortError' || error.name === 'TimeoutError') {
      throw new APIError(
        'Request timeout - server took too long to respond',
        408,
        { originalError: error.message }
      );
    }

    if (error.name === 'TypeError' && error.message.includes('fetch')) {
      throw new APIError(
        'Network error - unable to reach server. Make sure the API is running on port 5001.',
        0,
        { originalError: error.message, apiUrl: API_BASE_URL }
      );
    }

    // Re-throw APIError as-is
    if (error instanceof APIError) {
      throw error;
    }

    // Wrap unexpected errors
    throw new APIError(
      'Unexpected error during compilation',
      500,
      { originalError: error.message }
    );
  }
}

/**
 * Check if the API server is available
 *
 * @returns {Promise<boolean>} True if server is available
 */
export async function checkAPIHealth() {
  try {
    const response = await fetch(`${API_BASE_URL}/health`, {
      method: 'GET',
      signal: AbortSignal.timeout(5000),
    });
    return response.ok;
  } catch (error) {
    return false;
  }
}

/**
 * Get API server status and version info
 *
 * @returns {Promise<Object>} Server status information
 */
export async function getAPIStatus() {
  try {
    const response = await fetch(`${API_BASE_URL}/api/info`, {
      method: 'GET',
      headers: {
        'Accept': 'application/json',
      },
      signal: AbortSignal.timeout(5000),
    });

    if (!response.ok) {
      throw new Error('Failed to fetch API status');
    }

    return await response.json();
  } catch (error) {
    return {
      available: false,
      error: error.message,
    };
  }
}

/**
 * Compile with retry logic
 *
 * @param {string} source - The C source code
 * @param {number} maxRetries - Maximum number of retry attempts
 * @returns {Promise<Object>} Compilation results
 */
export async function compileCodeWithRetry(source, maxRetries = 2) {
  let lastError;

  for (let attempt = 0; attempt <= maxRetries; attempt++) {
    try {
      return await compileCode(source);
    } catch (error) {
      lastError = error;

      // Don't retry on validation errors or client errors (4xx)
      if (error instanceof APIError && error.status >= 400 && error.status < 500) {
        throw error;
      }

      // Wait before retry (exponential backoff)
      if (attempt < maxRetries) {
        await new Promise((resolve) => setTimeout(resolve, Math.pow(2, attempt) * 1000));
      }
    }
  }

  throw lastError;
}

/**
 * Format error message for user display
 *
 * @param {Error} error - The error object
 * @returns {string} User-friendly error message
 */
export function formatErrorMessage(error) {
  if (error instanceof APIError) {
    if (error.status === 0) {
      return '❌ Cannot connect to compiler API. Please ensure the Flask server is running on port 5001.';
    }
    if (error.status === 408) {
      return '⏱️ Compilation timeout. The code is taking too long to compile.';
    }
    if (error.status >= 500) {
      return `🔥 Server error: ${error.message}`;
    }
    return `❌ Compilation error: ${error.message}`;
  }

  return `❌ Unexpected error: ${error.message}`;
}

// Export error class for external use
export { APIError };

// Export default object with all functions
export default {
  compileCode,
  compileCodeWithRetry,
  checkAPIHealth,
  getAPIStatus,
  formatErrorMessage,
  APIError,
};
