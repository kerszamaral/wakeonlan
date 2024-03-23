FROM ubuntu:22.04
RUN apt update
RUN apt install -y g++ make
COPY . /project/
WORKDIR /project/
RUN make
ENTRYPOINT ./build/wakeonlan