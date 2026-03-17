# Distributed-Cuckoo-Filter-with-ESP32-WebSocket-Architecture
This project focuses on the design and implementation of a distributed Cuckoo Filter across multiple ESP32 devices. A Cuckoo
Filter is a type of probabilistic data structure which allows efficient membership testing with a
low memory footprint. Unlike traditional hash tables, Cuckoo Filters are designed to
support insertions, deletions, and approximate membership queries while minimizing false
positives.

In a distributed setting (multiple ESP32s), each ESP32 node maintains a local partition of the
filter, which allows the system to handle larger datasets than a single device could accommodate.
The nodes communicate with a “master node”, which coordinates operations such as insertion,
deletion, and lookup across the network. Communication can be implemented using ESP-NOW
(low-latency peer-to-peer protocol for ESP32 devices) or WebSocket-based messaging (real-time
communication over Wi-Fi). For now, we are choosing to stick with WebSocket-based
communication using an existing WebSocket library.

The main objectives of this project are:
• Distributed data structure: Partition the filter data across multiple devices to effectively
utilize memory and processing capabilities of each node.
• Message passing & coordination: Implement a master-slave architecture that ensures all
operations are correctly routed to the responsible nodes, maintaining consistency, and
minimizing latency.
• Fault tolerance & replication: Incorporate replication mechanisms to handle node
failures, network partitions, or message loss, while ensuring consistency across nodes.
• Performance evaluation: Measure and analyze key metrics such as false positive rates,
operation latency, and the overall scalability of the distributed filter when nodes are
added or removed.
