FROM ubuntu:23.10 AS builder
RUN apt update
RUN apt-get install g++-13 make --no-install-recommends -y
RUN rm -rf /var/lib/apt/lists/*
COPY .. /project/
WORKDIR /project
RUN make debug OUT_DIR=.
CMD ["./wakeonlan"]