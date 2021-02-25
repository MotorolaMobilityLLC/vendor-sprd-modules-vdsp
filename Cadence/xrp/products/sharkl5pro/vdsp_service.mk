PRODUCT_PACKAGES += libvdspservice \
		xrpclient

PRODUCT_COPY_FILES += \
    vendor/sprd/modules/vdsp/Cadence/xrp/init.sprd_vdsp.rc:vendor/etc/init/init.sprd_vdsp.rc \
	vendor/sprd/modules/vdsp/Cadence/xrp/xrp-firmware/sharkl5pro/vdsp_firmware.bin:vendor/firmware/vdsp_firmware.bin \
	vendor/sprd/modules/vdsp/Cadence/xrp/xrp-firmware/sharkl5pro/test_lib.o:/vendor/lib/test_lib.o \
	vendor/sprd/modules/vdsp/Cadence/xrp/xrp-firmware/sharkl5pro/test_lib.bin:vendor/firmware/test_lib.bin \
	vendor/sprd/modules/vdsp/Cadence/xrp/xrp-firmware/sharkl5pro/faceid_fw.bin:vendor/firmware/faceid_fw.bin \
	vendor/sprd/modules/vdsp/Cadence/xrp/xrp-firmware/sharkl5pro/network_coeff_fd_o.bin:vendor/firmware/network_coeff_fd_o.bin \
	vendor/sprd/modules/vdsp/Cadence/xrp/xrp-firmware/sharkl5pro/network_coeff_fd_p.bin:vendor/firmware/network_coeff_fd_p.bin \
	vendor/sprd/modules/vdsp/Cadence/xrp/xrp-firmware/sharkl5pro/network_coeff_fd_r.bin:vendor/firmware/network_coeff_fd_r.bin \
	vendor/sprd/modules/vdsp/Cadence/xrp/xrp-firmware/sharkl5pro/network_coeff_flv.bin:vendor/firmware/network_coeff_flv.bin \
	vendor/sprd/modules/vdsp/Cadence/xrp/xrp-firmware/sharkl5pro/network_coeff_fv.bin:vendor/firmware/network_coeff_fv.bin \
	vendor/sprd/modules/vdsp/Cadence/xrp/xrp-firmware/sharkl5pro/network_coeff_fp.bin:vendor/firmware/network_coeff_fp.bin
