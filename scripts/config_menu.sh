#!/bin/bash

MODULE_NAME="ti-ads114s0xb"
TRIGGER_NAME="sysfstrig123"
IIO_INTERFACE="iio:device0"  # Change this if the device name differs

load_modules() {
    echo "Loading required modules..."
    modprobe industrialio
    modprobe industrialio-buffer-cb
    modprobe industrialio-triggered-buffer
    modprobe iio-trig-sysfs
    echo "Modules loaded successfully."
}

load_adc_driver() {
    echo "Building and loading the ADS114S0XB driver..."
    pushd ../linux-embedded-driver > /dev/null
    make && make insmod
    popd > /dev/null
    echo "Driver loaded successfully."
}

install_adc_driver() {
    echo "Installing the ADS114S0XB driver..."
    pushd ../linux-embedded-driver > /dev/null
    sudo make install
    popd > /dev/null
    echo "Driver installed successfully."
}

configure_iio_interface() {
    echo "Configuring the IIO interface..."
    
    # Ensure the IIO device exists
    if [[ ! -d "/sys/bus/iio/devices/$IIO_INTERFACE" ]]; then
        echo "Error: IIO interface '$IIO_INTERFACE' not found in /sys/bus/iio/devices."
        return 1
    fi

    cd /sys/bus/iio/devices/iio_sysfs_trigger || { echo "Error: iio_sysfs_trigger directory not found."; return 1; }
    echo 123 > add_trigger

    cd /sys/bus/iio/devices/trigger0 || { echo "Error: trigger0 directory not found."; return 1; }
    cat name

    echo "Setting trigger for $IIO_INTERFACE..."
    echo "$TRIGGER_NAME" > "/sys/bus/iio/devices/$IIO_INTERFACE/trigger/current_trigger"
    echo "IIO interface configured."
}

unload_adc_driver() {
    echo "Unloading the ADS114S0XB driver..."
    pushd ../linux-embedded-driver > /dev/null
    make rmmod
    popd > /dev/null
    echo "Driver unloaded successfully."
}

show_menu() {
    while true; do
        echo "=================================="
        echo " ADS114S0XB Configuration Menu"
        echo "=================================="
        echo "1. Load required modules"
        echo "2. Load ADS114S0XB driver"
        echo "3. Install ADS114S0XB driver"
        echo "4. Configure IIO interface"
        echo "5. Unload ADS114S0XB driver"
        echo "6. Exit"
        echo "=================================="
        read -p "Select an option: " option

        case $option in
            1) load_modules ;;
            2) load_adc_driver ;;
            3) install_adc_driver ;;
            4) configure_iio_interface ;;
            5) unload_adc_driver ;;
            6) echo "Exiting..."; exit 0 ;;
            *) echo "Invalid option. Please try again." ;;
        esac
    done
}

show_menu
