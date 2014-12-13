# free_mem

Tiny command line tool to free memory on Mac OS X. It will gradually reserve specified amount of memory forcing OS to page if required then release it.
Amount can be specified as a fraction of total or explicitly.

```sh
free_mem

free_mem 1/4
free_mem /4

free_mem 4G
```

## Building

```sh
xcodebuild && cp build/Release/free_mem /path/where/you/want/it
```
