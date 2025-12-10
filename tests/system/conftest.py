#!/usr/bin/env python3
"""
==============================================================================
System Tests - pytest Configuration
==============================================================================

Configures pytest markers for system-level tests.

==============================================================================
"""

import pytest


def pytest_configure(config):
    """Configure custom pytest markers."""
    config.addinivalue_line(
        "markers", "e2e: End-to-end tests that test the full stack"
    )
    config.addinivalue_line(
        "markers", "contract: API contract validation tests"
    )
    config.addinivalue_line(
        "markers", "live: Tests that require a live API server"
    )

