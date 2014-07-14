T5
==

Description
-----------

T5 is a powerful file I/O library. It provides a unique Data Source API for general I/O, built in compression and transparent archive access, and simple configuration file manipulation.
T5 is intended to provide unified, transparent access to files that are in compressed or uncompressed archives. As part of this, T5 provides a unified Data Source I/O api, which is similar to C file handles and Unix file descriptors.
T5 also provides configuration file loading and saving from arbitrary formats. INI and JSON are provided by default.

Data Sources
------------

A Data Source is an abstraction over any object that can either be written to or read from. Most notably, Data Sources can be recursive.
An example of a using recursive Data Sources is the T5 hello world program:

```
#include <t5.h>
#include <cstdio>

int main(int argc, char *argv[]){
    // Create a data source that uses a C file handle.
    t5::DataSource *OutSource = t5::DataSource::FromCFileHandle(stdout);

    // Create a chained data source that can only write.
    t5::DataSourcePusher *StdOut = OutSource->AsPusher();

    StdOut->WriteS("Hello, world!\n");

    delete StdOut;
    delete OutSource;
}
```

As shown, you can make a Data Source from a file handle (other creation methods include directly from paths), and create a write-only (pusher) Data Source from this.
You may wonder if the precise Data Source freeing order is important. While it is _not_ important which of the two Data Sources are freed first, freeing a Data Source on which other Data Sources are based will close all the children.
