Builds a Throughput Test Binary
===============================

The throughput test is a very simple, RIME based test that streams packets
from a sender node to a sink node. On test completion, the sink will output
test results on its serial port (the same port used by printf).

To start the test, press the button on the source node which maps to
`button_sensor`. This project will not currently work on platforms without
any buttons.

Building
========

This project builds just like any other Contiki project. Just use
`make TARGET=<target>`. If, however, the contiki source tree does not reside at
`../../contiki_src/contiki` then you will need to use
`make CONTIKI=path/to/contiki/ TARGET=<target>` or edit the Makefile.
