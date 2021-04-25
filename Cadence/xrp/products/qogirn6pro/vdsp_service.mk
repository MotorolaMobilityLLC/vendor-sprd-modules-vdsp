PRODUCT_PACKAGES += libvdspservice

PRODUCT_COPY_FILES += \
	vendor/sprd/modules/vdsp/Cadence/xrp/init.sprd_vdsp.rc:vendor/etc/init/init.sprd_vdsp.rc \
	vendor/sprd/modules/vdsp/Cadence/xrp/xrp-firmware/qogirn6pro/vdsp_firmware.bin:vendor/firmware/vdsp_firmware.bin \
	vendor/sprd/modules/vdsp/Cadence/xrp/xrp-firmware/qogirn6pro/faceid_fw.bin:vendor/firmware/faceid_fw.bin \
	vendor/sprd/modules/vdsp/Cadence/xrp/xrp-firmware/qogirn6pro/network_coeff_fa.bin:vendor/firmware/network_coeff_fa.bin \
	vendor/sprd/modules/vdsp/Cadence/xrp/xrp-firmware/qogirn6pro/network_coeff_foc.bin:vendor/firmware/network_coeff_foc.bin \
	vendor/sprd/modules/vdsp/Cadence/xrp/xrp-firmware/qogirn6pro/network_coeff_fp.bin:vendor/firmware/network_coeff_fp.bin \
