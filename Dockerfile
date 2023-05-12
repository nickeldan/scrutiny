FROM debian:latest

RUN apt -y update && \
    apt -y install build-essential

COPY ./ /scrutiny/

RUN cd /scrutiny && \
    make docker_build=yes monkeypatch=yes install

RUN rm -rf /scrutiny
