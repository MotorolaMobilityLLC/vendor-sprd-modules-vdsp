PRODUCT_PACKAGES += libvdspservice \
		xrpclient

PRODUCT_COPY_FILES += \
	vendor/sprd/modules/vdsp/Cadence/xrp/init.sprd_vdsp.rc:vendor/etc/init/init.sprd_vdsp.rc \
	vendor/sprd/modules/vdsp/Cadence/xrp/xrp-firmware/qogirn6pro/vdsp_firmware.bin:vendor/firmware/vdsp_firmware.bin \
