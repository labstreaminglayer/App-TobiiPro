# TobiiPro Connector

This app connects to a Tobii Pro device to the LabRecorder.

Users of the Tobii Glasses 3 should check [here](https://github.com/tobiipro/Tobii.Glasses3.SDK/releases) for an official application from Tobii.

## Dependencies

It requires you to get the [TobiiPro C SDK](https://www.tobiipro.com/product-listing/tobii-pro-sdk/#Download) and
make sure your use case is covered by the end user agreement.
Extract the C SDK folder (e.g. `Linux/64` for 64 bit Linux) here and rename it
to `tobii` or set the `TOBIIPRO_ROOT_DIR` variable to its path

## Building

This application can be built following general
[LSL Application build instructions](https://labstreaminglayer.readthedocs.io/dev/app_build.html).
