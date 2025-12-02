"""
Configuration for Flask API Bridge

This module contains all configuration settings for the API server,
including paths, timeouts, and server settings.
"""

import os
from pathlib import Path


class Config:
    """Configuration class for the API server."""

    # Project root (parent of api/ directory)
    PROJECT_ROOT = Path(__file__).parent.parent.absolute()

    # Compiler settings
    COMPILER_PATH = str(PROJECT_ROOT / "bin" / "mycc")

    # Temporary file settings
    TEMP_DIR = str(PROJECT_ROOT / "tmp")

    # Compilation settings
    COMPILE_TIMEOUT = 30  # seconds

    # Flask server settings
    HOST = "0.0.0.0"  # Listen on all interfaces
    PORT = 5001  # Port 5000 is often used by AirPlay on macOS
    DEBUG = True  # Enable debug mode for development

    # CORS settings
    CORS_ORIGINS = "*"  # Allow all origins in development
    # For production, set to specific frontend URL:
    # CORS_ORIGINS = "http://localhost:3000"

    # Logging
    LOG_LEVEL = "INFO"

    @classmethod
    def validate(cls):
        """
        Validate configuration settings.

        Raises:
            FileNotFoundError: If compiler binary doesn't exist
            ValueError: If configuration values are invalid
        """
        if not os.path.exists(cls.COMPILER_PATH):
            raise FileNotFoundError(
                f"Compiler not found at: {cls.COMPILER_PATH}\n"
                f"Please build the compiler first with: make"
            )

        if not os.access(cls.COMPILER_PATH, os.X_OK):
            raise PermissionError(
                f"Compiler is not executable: {cls.COMPILER_PATH}"
            )

        if cls.COMPILE_TIMEOUT <= 0:
            raise ValueError("COMPILE_TIMEOUT must be positive")

        if not (1024 <= cls.PORT <= 65535):
            raise ValueError("PORT must be between 1024 and 65535")

    @classmethod
    def info(cls):
        """
        Print configuration information.

        Returns:
            Dictionary with configuration details
        """
        return {
            "project_root": str(cls.PROJECT_ROOT),
            "compiler_path": cls.COMPILER_PATH,
            "compiler_exists": os.path.exists(cls.COMPILER_PATH),
            "temp_dir": cls.TEMP_DIR,
            "compile_timeout": cls.COMPILE_TIMEOUT,
            "server": {
                "host": cls.HOST,
                "port": cls.PORT,
                "debug": cls.DEBUG
            },
            "cors_origins": cls.CORS_ORIGINS
        }


# Validate configuration on import (development mode)
if __name__ != "__main__":
    try:
        Config.validate()
    except Exception as e:
        print(f"Warning: Configuration validation failed: {e}")
        print("The API may not work correctly until the compiler is built.")
