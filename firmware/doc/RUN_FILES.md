# Run Files
These are the random alphanumeric files (IE `7AF3887778205E8C3E032BD2062136E7`) in the SD card's root directory. They store run data, as well as run metadata. 

## Motivation
To understand the file format, consider my guiding principals for this format:
- Metadata should be stored with run data in the same file
- Data should be "seekable". That is, knowing only information provided in the file metadata, a byte offset can be calculated to get a value at a given time.
- Data should be kept as small as possible.
  - This is ***extremely*** important, because if datafiles can be kept small enough, the strategy for displaying data becomes much, much easier. Small files (~300MB) can be loaded and manipulated in a browser without the need for chunking or streaming, which reduces load and code complexity on the ESP.
- The storage layer can be considered reliable (subject to change)
  - Just means that we don't bother with checksums or similar.
- One-time overheads (I.E. header size) are not as important to optimize as ongoing overheads (Data)

That being said, let's get into the format.
## Format
***NOTE: Full format structs can be found in [run_format.h](../src/run_format.h)***

A run file has two major parts: a **header** and **the data**. 
### Header
The header is a **fixed-size** struct 

### Data