FROM debian:9

RUN apt -y update && \
    apt -y install build-essential

COPY ./ /scrutiny/

RUN cd /scrutiny && \
    make install

RUN rm -rf /scrutiny
