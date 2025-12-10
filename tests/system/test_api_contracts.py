#!/usr/bin/env python3
"""
API Contract Tests - Validate request/response schemas.
"""

import pytest
import requests
import os

from jsonschema import validate, ValidationError

API_BASE_URL = os.environ.get("API_BASE_URL", "http://localhost:5000")

# JSON Schemas
COMPILE_REQUEST_SCHEMA = {
    "type": "object",
    "required": ["code"],
    "properties": {
        "code": {"type": "string"},
        "options": {"type": "object"}
    }
}

COMPILE_RESPONSE_SCHEMA = {
    "type": "object",
    "required": ["success"],
    "properties": {
        "success": {"type": "boolean"},
        "tokens": {"type": "array"},
        "ast": {"type": ["object", "null"]},
        "assembly": {"type": ["string", "null"]},
        "errors": {"type": "array"}
    }
}

ERROR_RESPONSE_SCHEMA = {
    "type": "object",
    "required": ["error"],
    "properties": {
        "error": {"type": "string"},
        "message": {"type": "string"}
    }
}

TOKEN_SCHEMA = {
    "type": "object",
    "required": ["type", "value"],
    "properties": {
        "type": {"type": "string"},
        "value": {"type": "string"},
        "line": {"type": "integer"},
        "column": {"type": "integer"}
    }
}

ERROR_MESSAGE_SCHEMA = {
    "type": "object",
    "required": ["message"],
    "properties": {
        "message": {"type": "string"},
        "line": {"type": "integer"},
        "column": {"type": "integer"},
        "severity": {"type": "string"}
    }
}


def validate_schema(data: dict, schema: dict) -> bool:
    """Validate data against a JSON schema."""
    try:
        validate(instance=data, schema=schema)
        return True
    except ValidationError:
        return False


class TestRequestSchemas:
    """Tests for API request schema validation."""

    @pytest.mark.contract
    def test_valid_compile_request(self):
        """Valid compile request matches schema."""
        request = {"code": "int main() { return 0; }"}
        assert validate_schema(request, COMPILE_REQUEST_SCHEMA)

    @pytest.mark.contract
    def test_compile_request_with_options(self):
        """Compile request with options matches schema."""
        request = {
            "code": "int main() { return 0; }",
            "options": {"dump_tokens": True}
        }
        assert validate_schema(request, COMPILE_REQUEST_SCHEMA)

    @pytest.mark.contract
    def test_missing_code_is_invalid(self):
        """Request without code field is invalid."""
        request = {"options": {"dump_tokens": True}}
        with pytest.raises(ValidationError):
            validate(instance=request, schema=COMPILE_REQUEST_SCHEMA)


class TestResponseSchemas:
    """Tests for API response schema validation."""

    @pytest.mark.contract
    def test_success_response(self):
        """Successful response matches schema."""
        response = {
            "success": True,
            "tokens": [{"type": "KEYWORD", "value": "int"}],
            "ast": {"type": "Program"},
            "errors": []
        }
        assert validate_schema(response, COMPILE_RESPONSE_SCHEMA)

    @pytest.mark.contract
    def test_error_response(self):
        """Error response matches schema."""
        response = {
            "success": False,
            "tokens": [],
            "ast": None,
            "errors": [{"message": "Syntax error", "line": 1}]
        }
        assert validate_schema(response, COMPILE_RESPONSE_SCHEMA)

    @pytest.mark.contract
    def test_api_error_response(self):
        """API error response matches schema."""
        response = {"error": "InvalidRequest", "message": "Missing code"}
        assert validate_schema(response, ERROR_RESPONSE_SCHEMA)



class TestTokenAndErrorSchemas:
    """Tests for token and error message schemas."""

    @pytest.mark.contract
    def test_token_schema(self):
        """Token object matches schema."""
        token = {"type": "KEYWORD", "value": "int", "line": 1, "column": 1}
        assert validate_schema(token, TOKEN_SCHEMA)

    @pytest.mark.contract
    def test_error_message_schema(self):
        """Error message matches schema."""
        error = {"message": "Syntax error", "line": 1, "column": 5, "severity": "error"}
        assert validate_schema(error, ERROR_MESSAGE_SCHEMA)


def check_api_available() -> bool:
    """Check if API server is running."""
    try:
        response = requests.get(f"{API_BASE_URL}/health", timeout=5)
        return response.status_code in [200, 404]
    except requests.exceptions.RequestException:
        return False


class TestLiveAPI:
    """Tests that require a running API server."""

    @pytest.mark.contract
    @pytest.mark.live
    def test_compile_endpoint(self):
        """Compile endpoint returns valid response."""
        if not check_api_available():
            pytest.skip("API not available")

        response = requests.post(
            f"{API_BASE_URL}/compile",
            json={"code": "int main() { return 0; }"},
            timeout=30
        )
        assert response.status_code in [200, 201]
        assert "application/json" in response.headers.get("Content-Type", "")

    @pytest.mark.contract
    @pytest.mark.live
    def test_invalid_request_handling(self):
        """API handles invalid requests properly."""
        if not check_api_available():
            pytest.skip("API not available")

        response = requests.post(f"{API_BASE_URL}/compile", json={}, timeout=30)
        assert response.status_code in [400, 422]
