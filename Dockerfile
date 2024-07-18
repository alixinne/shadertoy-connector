ARG base_image=debian:buster
FROM $base_image
ENV DEBIAN_FRONTEND=noninteractive
RUN mkdir /build
WORKDIR /build
COPY debian ./debian
RUN mv /etc/apt/sources.list /etc/apt/sources.list.save && \
    awk '/^deb /{print;sub(/^deb/,"deb-src",$0);print}' /etc/apt/sources.list.save >/etc/apt/sources.list && \
    apt-get update -qq && \
    apt-get install --no-install-recommends -y \
        git \
        ca-certificates \
        curl \
        devscripts \
        equivs \
        lsb-release \
        wget \
        gnupg2 \
        apt-transport-https && \
    wget -qO - 'https://bintray.com/user/downloadSubjectPublicKey?username=alixinne' | apt-key add - && \
    echo "deb https://dl.bintray.com/alixinne/libshadertoy $(lsb_release -cs) main" >/etc/apt/sources.list.d/libshadertoy.list && \
    apt-get update -qq && \
    mk-build-deps --install --remove ./debian/control -t 'apt-get -o Debug::pkgProblemResolver=yes --no-install-recommends -y'
