---
sidebar_position: 1
---
# Overview
### Update Consistency
Data can be categorized by how consistently it updates. Many sensors - like GPS, accelerometers, DACs, and countless others -  update at a fixed rate. These rates might differ by multiple orders of magnitude (many DACs can operate at 10ksps+, while GPS chips are lucky to hit 10hz) but the key is that these rates are _consistent_. For a 10hz GPS, for example, you're guaranteed a new position every 100ms.  

Some other pieces of data, however, update at unknown intervals (or not at all!). Data like settings, flags, and so on, might only update once or twice over the course of a data collection period.
