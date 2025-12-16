# Multi-stage Dockerfile for C Compiler with React Visualization
FROM node:22-slim AS frontend-builder

WORKDIR /app/frontend
COPY frontend/package*.json ./
RUN npm install
COPY frontend/ ./
RUN npm run build

# Main build stage
FROM python:3.11-slim

# Install build tools
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Install Node.js 22 for development
RUN curl -fsSL https://deb.nodesource.com/setup_22.x | bash - \
    && apt-get install -y nodejs \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy source code
COPY src/ ./src/
COPY include/ ./include/
COPY Makefile ./

# Build the compiler
RUN make native

# Copy API files
COPY api/ ./api/
WORKDIR /app/api
RUN pip install --no-cache-dir -r requirements.txt

# Copy frontend files
WORKDIR /app
COPY frontend/ ./frontend/

# Install frontend dependencies
WORKDIR /app/frontend
RUN npm install

# Copy built frontend from builder stage
COPY --from=frontend-builder /app/frontend/dist ./dist

WORKDIR /app

# Expose ports for API and frontend
EXPOSE 5000 5173

# Default command
CMD ["python3", "api/app.py"]
