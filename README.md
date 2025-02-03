# Lamport’s distributed mutual exclusion protocol

## Overview

Once it has mutex, the server verifies if it has enough balance to issue this transfer using the local Balance Table. If the server can afford the transfer, then it inserts the transaction block at the head of the blockchain and send that block directly to all other clients, who also insert the block at the head of their local copy of the blockchain. Once inserted in the blockchain, the local copy of the Balance Table is updated. Then mutex is released. If the server does not have enough balance, the transaction is aborted and mutex is released.

## Requirement
   - Consistency

## Techstack
   - C++
   - CMake, Make
   - epoll + thread pool
   - Singleton Pattern + Abstract Factory Pattern

## Building
Build the system with 1 interface and 3 lamport clients with default balance of {100, 200, 300}:
```bash
make build
```
To build system with more lamport clients and with self-defined balance, you need to change parameters in `include/configs.hpp`.

Clean everything you have built:
```bash
make clean
```

## Running
Run and interact with the system:

```bash
./build/LamportAlgorithm
```


## User Interface

transfer(client_id, sender_id, receiver_id, amount): SUCCESS or FAIL
```bash
transfer <client_id> <sender_id> <receiver_id> <amount>
```

balance(client_id): balance returned from the server also print the blockchain in server
```bash
balance <client_id>
```

## Config

client_num: 3 + 1(interface)

​	**Q: How to build a server**

​	**A: a master thread with workers in thread pool**

local blockchain

​	**Q: When to update**

​	**A: On release**

local balance table: K(server name):V(corresponding balance) store

​	**Q: When to update**

​	**A: On release**

local Lamport logical clock: ⟨Lamportclock, P rocessid⟩

​	**Q: When to update**

​	**A: On requst**

local request queue: queue by Lamport logical clock

​	**Q: When to update**

​	**A: On request (add) / release (remove)**

TCP/UDP/RPC