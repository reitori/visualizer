# Visualizer

A visualization tool for ATLAS ITK pixel detectors.

## Installation

```bash
git clone https://github.com/reitori/visualizer.git
# OPTIONAL: for LBL pc installs
source /opt/rh/devtoolset-9/enable
cd visualizer
mkdir build/
cd build/
cmake3 ../
make
cd ../
```

Installed binaries will be available in `visualizer/bin`, and can be executed with e.g.
```
./bin/<executable> <args>
```

## Troubleshooting

### GUI rendering errors
**Problem**: you cannot render GUI windows over SSH and see an error like this:

```
libGL error: No matching fbConfigs or visuals found
libGL error: failed to load driver: swrast
X Error of failed request:  GLXBadContext
  Major opcode of failed request:  152 (GLX)
  Minor opcode of failed request:  6 (X_GLXIsDirect)
  Serial number of failed request:  628
  Current serial number in output stream:  627
```

**Possible solution for MACOS:**
try (on macos - probably an analog exists for windows/linux) adding this line to your `.bashrc` or equivalent:
```
defaults write org.xquartz.X11 enable_iglx -bool YES
```

Then, quit and restart your X11 server.


## Structure

- `core`: core functionality
- `util`: utility functions
- `datasets`: dataset loaders and adapters

## `dataloader_test`

Run this binary to test the functionality of the data reader. Some example configuration files covering the `YarrBinaryFile` format are included, with some test data. 

For a "fixed-frame" test, where the only 100 events at a time are read, run (optionally with `-v`` for verbose output)
```
./bin/dataloader_test -c configs/event_count_example.json
```

For a "real-time" test, where all available data is read at once, run
```
./bin/dataloader_test -c configs/realtime_example.json -v
```