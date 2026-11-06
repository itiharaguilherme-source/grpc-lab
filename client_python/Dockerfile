FROM python:3.10-slim
RUN apt-get update && apt-get install -y build-essential && rm -rf /var/lib/apt/lists/*
COPY client_python/requirements.txt /tmp/
RUN pip install --no-cache-dir -r /tmp/requirements.txt
WORKDIR /workspace
