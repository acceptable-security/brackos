% Virtual File systemA
% Avi Saven
% January 26th 2019

The virtual file system of the brackos kernel is designed to maximize
interoperability between file systems while also maintaining a strict heirarchy
of systems and files.

## RESTful VFS

While most devices will output some form of common data format, the hardest part
of getting interoperability is having them speak a common format and present 
the various configuration options/functionality in a common format. There are
two mechanisms that are applied in order to achieve this functionality. The
first is by exposing functionality that a device implements through various
paths. This is similar to the RESTful APIs that are exposed over HTTP. The idea
is a novel but powerful construction: an object (file, device, process, or any
other kernel level object) has a name which