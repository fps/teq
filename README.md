teq
===

A library implementing a jack midi client for sequencing purposes. The organization of the musical data is tracker-like, i.e. patterns with columns.

Requirements
============

* A C++11 compatible compiler
* libboost-python for the python module
* python2 C library for the python module


# Building

<pre>
mkdir build
cd build
cmake ..
make
cd ..
</pre>

If you have python3 installed in parallel to python2, you can try altering some cmake variables: Substitute this command for <code>cmake ..</code> above:

<pre>
cmake -DPYTHON_LIBRARY=/usr/lib/libpython2.7.so -DPYTHON_INCLUDE_DIR=/usr/include/python2.7
</pre>

Change the exact names and locations to your python2 install. This is nessecary if you get e.g. linker errors when running

<pre>
python2 example.py
</pre>

API Docs
========

The C++ API should be documented here: 

http://fps.github.com/teq

If you find these to be out of date, use

<code>
doxygen
</code>

to generate up to date documentation.

The Python API is generated from the file <code>python.cc</code>. So take a look at that file to see which functions and classes are exposed. Also take a look at <code>test_teq.py</code>. That file should serve as an example of how to use the API from a python program.
