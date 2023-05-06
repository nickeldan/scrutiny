FROM debian:latest

RUN apt -y update && \
    apt -y install build-essential

COPY ./ /scrutiny/

RUN cd /scrutiny && \
    make monkeypatch=yes install

RUN rm -rf /scrutiny
