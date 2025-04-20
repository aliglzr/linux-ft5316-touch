# FT5316 Touchscreen Driver

This is a Linux kernel driver for the FocalTech FT5316 capacitive touch controller.

## Prerequisites

- Linux kernel source headers
- Make
- GCC
- I2C support enabled in kernel

## Building the Driver

### As a Loadable Module

1. Make sure you have the kernel headers installed:
   ```bash
   sudo apt-get install linux-headers-$(uname -r)
   ```

2. Build the driver:
   ```bash
   make
   ```

3. Load the driver:
   ```bash
   sudo insmod ft5316.ko
   ```

### As a Built-in Module

To build the driver directly into the kernel:

1. Copy the driver source to your kernel source tree:
   ```bash
   cp ft5316.c /path/to/linux/drivers/input/touchscreen/
   ```

2. Add the driver to the Kconfig:
   ```bash
   echo 'config TOUCHSCREEN_FT5316' >> /path/to/linux/drivers/input/touchscreen/Kconfig
   echo '	tristate "FocalTech FT5316 Touchscreen"' >> /path/to/linux/drivers/input/touchscreen/Kconfig
   echo '	depends on I2C' >> /path/to/linux/drivers/input/touchscreen/Kconfig
   echo '	help' >> /path/to/linux/drivers/input/touchscreen/Kconfig
   echo '	  Say Y here if you have a FocalTech FT5316 touchscreen' >> /path/to/linux/drivers/input/touchscreen/Kconfig
   echo '	  connected to your system.' >> /path/to/linux/drivers/input/touchscreen/Kconfig
   ```

3. Add the driver to the Makefile:
   ```bash
   echo 'obj-$(CONFIG_TOUCHSCREEN_FT5316) += ft5316.o' >> /path/to/linux/drivers/input/touchscreen/Makefile
   ```

4. Configure the kernel:
   ```bash
   cd /path/to/linux
   make menuconfig
   ```
   Navigate to:
   ```
   Device Drivers
     -> Input device support
       -> Touchscreens
         -> FocalTech FT5316 Touchscreen
   ```
   Select 'Y' to build it into the kernel.

5. Build the kernel:
   ```bash
   make
   ```

6. Install the new kernel:
   ```bash
   sudo make modules_install install
   ```

7. Reboot to use the new kernel with the built-in driver.

## Device Tree Configuration

Add the following to your device tree:

```dts
&i2c1 {
    status = "okay";
    
    touchscreen@38 {
        compatible = "focaltech,ft5316";
        reg = <0x38>;
        interrupt-parent = <&gpio>;
        interrupts = <PIN_NUMBER IRQ_TYPE_EDGE_FALLING>;
        reset-gpios = <&gpio PIN_NUMBER GPIO_ACTIVE_LOW>;
    };
};
```

Replace `PIN_NUMBER` with the actual GPIO pin numbers for your hardware.

## Features

- Multi-touch support (up to 5 touch points)
- I2C interface
- Interrupt-driven operation
- Device tree support

## Debugging

To check if the driver is loaded:
```bash
lsmod | grep ft5316
```

To view kernel messages:
```bash
dmesg | grep ft5316
```

## License

This driver is licensed under the GPL v2. 