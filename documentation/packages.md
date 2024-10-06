## PAK-ages

Basket packages are where all your application files will be stored in, it's a
useful abstraction over ZIP files, useful for self contained programs and even
self-extractable archives!

Basket packages are usually either regular folders or Zip files with the `.bsk`
file extension.

The package loader built into Basket has a useful and fun feature that allows it
to read zip files appended to other files, in essence, it is able to read the
very end or very bottom of a file to determine if it can find a zip footer or
header, as long as, of course, either one of those is accessible like that.

This feature allows us to have self-extractable archives, or even hiding data on
seemingly normal and innocent files! such as .mp3s, .pngs, .dlls, etc.


## The API

- ### `int pak_mount(const char *name);`
  "Mounts" a Basket package for it to be read by `pak_read`
  - `name`: Either the path to a folder or a file containing zip data.
  - Returns: non-zero on error.

- ### `char *pak_read(const char* name, size_t *size);`
  Reads a file
  - `name`: The path to a certain file within the current open package.
  - `size`: A pointer to a `size_t` integer, set to zero on error, set to file
    size in success, ignored if `NULL` pointer.
  - Returns: `NULL` on error, a file body in success.


## Some examples

- ### Self-extractable archives:
  ```bash
  # shell
  cat data.zip >> final_game
  ```

  ```c
  // c
  bool mounted = pak_mount(eng_executable());
  if (!mounted)
      mounted = pak_mount("fallback.zip");
  ```

- ### Hiding data on mp3 files:
  ```bash
  # shell
  cat data.zip >> cool_music.mp3
  ```

  ```c
  // c
  bool mounted = pak_mount("cool_music.zip");

  if (mounted) {
      const char *hello = pak_read("hello world.txt", NULL);
      printf("%s\n", hello);
  }
  ```
