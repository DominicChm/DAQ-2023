# Goals
For the best balance between format efficiency, simplicity, and longevity, this is a **binary file format**, with all **metadata embedded**.

Features:
- Constant-size data, which allows data indices to be directly seeked.
- Variable-rate data
  - Structs with different data can occur at a fixed ratio.
  - IE accelerometer data can occur at 100hz, while other sensors are stored at only 20hz.


# Format
## Overall: `<HEADER><DATA>`

## Header
Note: Exact header definition is contained in [DataManager.h](src/DataManager.h)
