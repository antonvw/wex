# bindings

This directory will be used if switch -i was specified with build-gen,
and [swig](https://www.swig.org/) is installed.

This directory contains language bindings for wex.
At this moment python is supported using swig, but stil under
development.

For OSX you need to make a symbolic link '_wex.so' to '_wex.dylib' in
swig/bindings, or in the installed python directory.

As an example of using the bindings: install wex, run python and do:

```python
>>> import wex
>>> wex.now()
'2024-01-21 14:00:05'
>>> r=wex.regex('([0-9])+xxx')
>>> print(r.match('1444xxx'))
1
>>> c=wex.config("config-item")
>>> c.set("xyz")
>>> print(c.get())
xyz
```
