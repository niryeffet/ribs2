TARGET=bin/ribeye
INSTALL_DIR=/usr/local/bin
DEB_VERSION=$(shell git describe --tags --long | cut -d '-' -f -2 | cut -c2-)
DEB_PKG_DIR=ribeye-$(DEB_VERSION)
SYSTEMD_UNIT_DIR=$(shell pkg-config systemd --variable=systemdsystemunitdir)

all:
	@make -s -C src
install: all
	sudo install -s -groot -oroot ${TARGET} ${INSTALL_DIR}

install-service: install
	sudo install -groot -oroot init/ribeye.service $(SYSTEMD_UNIT_DIR)
	sudo install -d /var/run/ribeye

deb: all
	@rm -rf $(DEB_PKG_DIR)
	mkdir -p $(DEB_PKG_DIR)
	mkdir -p $(DEB_PKG_DIR)/DEBIAN
	mkdir -p $(DEB_PKG_DIR)/usr/local/bin
	mkdir -p $(DEB_PKG_DIR)/etc/default
	mkdir -p $(DEB_PKG_DIR)$(SYSTEMD_UNIT_DIR)
	mkdir -p $(DEB_PKG_DIR)/usr/lib/tmpfiles.d
	echo 'd    /var/run/ribeye   0755 ribeye  nogroup  -    -' > $(DEB_PKG_DIR)/usr/lib/tmpfiles.d/ribeye.conf
	cp DEBIAN/* $(DEB_PKG_DIR)/DEBIAN
	sed -i "s/<VERSION_PLACEHOLDER>/${DEB_VERSION}/g" ${DEB_PKG_DIR}/DEBIAN/control
	cp bin/ribeye $(DEB_PKG_DIR)/usr/local/bin
	cp init/ribeye.service $(DEB_PKG_DIR)$(SYSTEMD_UNIT_DIR)
	cp init/etc/default/* $(DEB_PKG_DIR)/etc/default/
	fakeroot dpkg-deb --build $(DEB_PKG_DIR)

