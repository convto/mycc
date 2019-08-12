FROM ubuntu:18.04
RUN apt update && apt install -y gcc make git binutils libc6-dev vim
ENV LANG C.UTF-8
WORKDIR /mycc
