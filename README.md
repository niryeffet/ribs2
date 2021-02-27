Introduction
============

RIBS2 (Robust Infrastructure for Backend Systems, ver. 2) is a 
library which allows building high-performance internet serving
systems, while simplifying the flow of events into the user
application without sacrificing performance.

Up until the creation of RIBS2, there were generally two ways to
write internet serving engines - threads (or forks) per client/-
request and event-driven.

Using threads makes it easy to write applications (no need to handle
callbacks of events) with the price of performance (and performance
predictability) and with the price of either locking and code 
synchronization, or, in the case of fork per client/request, highly
inefficient engine.

On the other hand, event-driven will render very efficient engine,
with the price of needing to manually handle callbacks (manual
"stack management", "callback pyramid of doom"). In complex
application this can slow down development significantly. As of
today, this is the least preferred method and for years had very
limited support in popular languages, like java.

With RIBS2, the development of application is actually easier than
with threads, since there is no need for locking or code
synchronization, and the performance is as good (and sometimes
better) as with event-driven engines. This is possible by simply
replacing the preemptive scheduling of threads with events.
Cooperative "threads" are often called "fibers". Fibers that are
being controlled exclusively by events are called here "Ribbons".

        +----------------------------+
        |      Task Management       |
        +-------------+--------------+
        | Cooperative |  Preemptive  |
+---+---+-------------+--------------+
|   |   |             |              |
| S | M |             |              |
| t | a |             |              |
| a | n |   Event-    |              |
| c | u |   Driven    |              |
| k | a |             |              |
|   | l |             |              |
| M |   |             |              |
| a +---+-------------+--------------+
| n | A |             |              |
| a | u |             |              |
| g | t |             |              |
| e | o |    Ribs2    |              |
| m | m |   Ribbons   |   Threads    |
| e | a |             |              |
| n | t |             |              |
| t | e |             |              |
|   | d |             |              |
+---+---+-------------+--------------+
Illustration of the differences between
threads, event-driven and ribbons.

The framework designed to run as single threaded but not limited to.
You can 'fork' or run multiple instances in order to utilize all
cores on your system, it is recommended to avoid the use of threads.

In addition, RIBS provides vmbuf, a simple yet powerful way to manage
large memory buffers.

RIBS also provides http client and server. (pools, smart persistent
connection handling)

With RIBS you can take any library that is provided with ‘.a’ archive
file, and “ribify” it to use RIBS provided function calls instead of
the default libc. This way you can take a library that is written in
a “blocking” paradigm, and run inside a complete event driven system
without modifications to the original code. This is proven to be
working well with libmysqlclient.a and other libraries. RIBS includes
a libmysqlclient example.

Prerequisites
=============
Ubuntu 10.04 or higher, preferably 12.04. Although it was tested on
Ubuntu, any Linux distro which is running 2.6.32 or above should be
fine as well, however, it was never tested. Please report if you
encounter problems.

Although 32bit is also supported, 64bit is highly recommended due to
address space limitation of 32bit.

In order to build the library you will need to install the following
packages (under Ubuntu):
* build-essential

Optional:
* libmysqlclient-dev
* zlib1g-dev

Tutorials
=========
The following tutorials can be found in doc/
* [TUTORIAL01](./docs/TUTORIAL01) - hello world
* [TUTORIAL02](./docs/TUTORIAL02) - simple file server
* [TUTORIAL03](./docs/TUTORIAL03) - Using MySQL via ribify
* [TUTORIAL04](./docs/TUTORIAL04) - log player (http client)
* [TUTORIAL05](./docs/TUTORIAL05) - dumping data and creating index
* [TUTORIAL06](./docs/TUTORIAL06) - using a ribified mongodb
=========
