FROM centos:centos8

RUN yum install -y gcc make wget \
    && wget -P /etc/yum.repos.d https://www.nasm.us/nasm.repo \
    && yum install -y nasm