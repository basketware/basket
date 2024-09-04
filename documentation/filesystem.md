# Filesystem
The filesystem in basket is an optional module that would
abstract out the filesystem disregarding which OS.


## It works in this way
when you run `fs_init()`, it tries to find a "package" in
the following dirs, in the following order:

- Checks `$BASKET_PACKAGE` as a folder
- Checks `$BASKET_PACKAGE` as a .zip file
- Checks `<game binary's directory>/package.zip`
- Checks `<game binary's directory>/package/`

If it couldn't find any suitable package, then simply fails, returning 1.


## Roadmap:
  - Get rid of `$BASKET_PACKAGE` in favour of directly passing arguments
  - Add an ELF and PE parser so I can read the binary itself for appended data
