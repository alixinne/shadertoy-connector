# Known build distributions
ALL_DISTS=bionic stretch buster

# Known package types
PKG_TYPES=amd64 i386 source

# Possible combinations of distributions-types
ALL_BIONIC_ARCHS=$(patsubst %,bionic-%,$(PKG_TYPES))
ALL_STRETCH_ARCHS=$(patsubst %,stretch-%,$(PKG_TYPES))
ALL_BUSTER_ARCHS=$(patsubst %,buster-%,$(PKG_TYPES))
ALL_LOCAL=local-amd64 local-source
ALL_PKGS=$(ALL_BIONIC_ARCHS) $(ALL_STRETCH_ARCHS) $(ALL_BUSTER_ARCHS) $(ALL_LOCAL)

# Test settings
IGNORE_TEST_FAILURES?=
SKIP_TESTS?=

# CI settings
VERS=$(shell head -n 1 debian/changelog | awk '{gsub("[()]","",$$2);print $$2}')
OS_DIST=$(shell . /etc/os-release && (test -n "$$VERSION_CODENAME" && echo -n "$$VERSION_CODENAME") || \
	   (test -n "$$VERSION" && echo -n "$$VERSION" | cut -d' ' -f2 | sed 's/[()]//g') || \
	   (echo -n "$$PRETTY_NAME" | cut -d' ' -f3 | cut -d/ -f1))
export OS_DIST
CI_BUILD_TYPE?=ci-src

GIT_PREFIX=$(shell git rev-parse --short HEAD)
export GIT_PREFIX

all: $(ALL_DISTS)

bionic: $(ALL_BIONIC_ARCHS)

stretch: $(ALL_STRETCH_ARCHS)

buster: $(ALL_BUSTER_ARCHS)

local: $(ALL_LOCAL)

$(ALL_PKGS):
	./buildpackage.sh $@

ci: $(CI_BUILD_TYPE)

# ci-src builds the project from source
ci-src:
	./buildpackage.sh ci-src

# ci-pkg builds packages for the current distribution, on supported hosts
ci-pkg: $(patsubst %,$(OS_DIST)-%,$(PKG_TYPES))
	tar -cjvf shadertoy-connector-$(VERS)-$(OS_DIST)-$(GIT_PREFIX).tar.bz2 ../shadertoy-connector-$(VERS)-$(OS_DIST)-$(GIT_PREFIX)

gl:
	./buildpackage.sh $@
	mv ../shadertoy-connector-$(VERS)-$(OS_DIST)-$(GIT_PREFIX)/*.deb .
	if [ -n "$(BINTRAY_API_KEY)" ] && (echo "$(CI_COMMIT_REF_NAME)" | grep "^v.*" >/dev/null) ; then \
		for DEB_FILE in *.deb; do \
			curl -T $$DEB_FILE -u$(BINTRAY_ORG):$(BINTRAY_API_KEY) "https://api.bintray.com/content/$(BINTRAY_ORG)/libshadertoy/shadertoy-connector/$(VERS)/$(OS_DIST)/$${DEB_FILE};deb_distribution=$(OS_DIST);deb_component=main;deb_architecture=$$(echo -n $$DEB_FILE | sed 's/.*_\(amd64\|i386\|all\)\.deb$$/\1/')" ; \
		done ; \
	fi

.PHONY: all ci-src ci-pkg $(ALL_DISTS) $(ALL_PKGS)

