FROM ubuntu:18.04
RUN sed -i.bak -e "s%http://us.archive.ubuntu.com/ubuntu/%http://ftp.iij.ad.jp/pub/linux/ubuntu/archive/%g" /etc/apt/sources.list
RUN apt update && apt install --no-install-recommends -y gcc gdb make git binutils libc6-dev vim clang-format
ENV LANG C.UTF-8
WORKDIR /mycc
