FROM ubuntu:22.04 AS builder
RUN apt update
RUN apt-get install g++ make --no-install-recommends -y
RUN rm -rf /var/lib/apt/lists/*
COPY .. /project/
WORKDIR /project
RUN make OUT_DIR=.
CMD ["./wakeonlan"]