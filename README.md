# gst-printNvDsMeta
GStreamer plugin that prints Nvidia Deepstream metadata to terminal

## Installation

Set DEEPSTREAM_VERSION X.X in CMakeLists.txt

```
mkdir build && cd build
cmake ..
make
sudo make install
```

## Usage

This plugin can only be used downstream of nvstreammux, and ideally after nvinfer

```
gst-launch-1.0 ........ ! nvstreammux ! ..... ! nvinfer ! plzprintmeta ! ....
