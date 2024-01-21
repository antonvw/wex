# bindings

This directory will be used if wexBUILD_BINDINGS was specified,
and [swig](https://www.swig.org/) is installed.

This directory contains language bindings for wex.
At this moment python is supported using swig, but stil under
development.

For OSX you need to make a symbolic link '_wex.so' to '_wex.dylib' in
build/bindings, or in the installed python directory.

As an exampe of using the bindings: install wex, run python and do:

```python
>>> import wex'
>>> wex.now()
'2024-01-21 14:00:05'
>>> r=wex.regex('([0-9])+xxx')
>>> print(r.match('1444xxx'))
1
```
