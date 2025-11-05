# Black Hole Simulator

_work-in-progress_

## Planned features

- Change metric

## Implementation Details

- Uses C Standard Library.
- Uses Raylib installed as shared library.
- Use ninja/samurai build system.

## Build with system-wide shared library

Copy the build.ninja.shared to build.ninja and run:

```
$ ninja
```

or

```
$ gcc -lm -lraylib main.c -o main
```

## Build with statically linked library

- Make an empty directory called `raylib`.
- Download the raylib version for your specific platform from the Github releases page.
- Inside the downloaded raylib build, locate the `include` and `lib` directories.
- Copy them into the new `raylib` directory.

Copy the build.ninja.shared to build.ninja and run:
```
$ ninja
```

or

```
$ gcc -lm -Iraylib/include main.c raylib/lib/libraylib.a -o main
```

## Run

```
./main
```
