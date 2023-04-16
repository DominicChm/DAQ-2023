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

A run file has two major parts: a **header** and the **data**. The header contains metadata about the run and information about how to parse the run's data. The data is encoded as a zero-overhead raw binary stream.

### Data Sources
In the context of the code, a *data source* is conceptually a region of memory that needs to be logged by the DAQ. More practically, it is a struct containing some relevant data (IE `car_data.sus`, `car_data.electrons`). Data sources can be polled at a variable rate, optimizing storage efficiency and performance. For example, `car_data.sus` could be polled every 10ms, while `car_data.electrons` is polled every 100ms.

### Time Base
Every run has a common "*cycle time base*", which is the interval at which all data is polled. Data sources can be polled at any multiple of this time base, including at every cycle. Sources cannot be polled faster than the cycle time base. Additionally, neither the time base nor any source cycle interval can change mid-run. 

The time base and cycle concept is how the 2023 DAQ is able to store data in a binary stream with 0 overhead and enable seeking. Given the ratio of cycles between two values, it's possible to figure out how many of each will occur, and where in the data stream each value is. Consider the following data values we want to store (`D1`, and `D2`)

```
D1: Cycle interval of 1 (every cycle), CI(1)
D2: Cycle interval of 2 (every other cycle), CI(2)

Raw Data Stream: D1 D1 D2 D1 D1 D2 D1 D1 D2

Data Stream Cycles
0: D1
1: D1 D2
2: D1
3: D1 D2
4: D1
5: D1 D2
```

Notice that after the initial cycle (where every data source is polled), `D1` occurs twice as often as `D2`. Knowing the ratio of `D1` to `D2`, therefore, allows a pattern to develop that we can exploit to find data without the need for mid-stream markers. The *cycle time base* then allows us to map from cycle count to time, completing the DAQ.

For example, given a cycle time base of 100ms in addition to the previous example, we can calculate the value of D1 at cycle 5 using the following:
```
D1(5) = size(D1) * (5 // 1) + size(D2) * (5 // 2) 
D1(5) =    1     *    5     +    1     *    2 
D1(5) = 7
``` 
Some special handling in the actual stream will be needed because every value is polled at time=0, but this is the gist of it.
## Header
The header is a struct starting with a 32-bit length field. It contains metadata about the run itself (name, description, location, time, etc) as well as descriptors needed to parse the run's binary data. These descriptors can be found as the header's `entries` field.

## Data