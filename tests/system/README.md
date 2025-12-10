# System-Level Tests

System-level tests for the C Compiler Visualization Suite.

## Test Files

| File | Description |
|------|-------------|
| `test_full_stack_e2e.py` | E2E tests for compilation flow |
| `test_api_contracts.py` | API schema validation tests |

## Setup

```bash
pip install -r requirements.txt
```

## Running Tests

```bash
# Run all tests
pytest -v

# E2E tests only
pytest -v -m e2e

# Contract tests only
pytest -v -m contract

# Live API tests (requires running server)
pytest -v -m live
```

## Test Markers

- `@pytest.mark.e2e` - End-to-end tests
- `@pytest.mark.contract` - API contract validation
- `@pytest.mark.live` - Requires live API server

