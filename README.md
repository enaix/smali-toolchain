# smali-toolchain

Simple, but reliable POSIX utilities for .smali reverse engineering written in C. The main purpose of this project is to quickly build and test .smali code using a command-line toolchain.

## Utilities

- `inject`: a simple utility which populates .smali template file. Allows to fill in the init (main) code and extra class methods. Number of registers needs to be set manually.

- `compile`: build the payload into an .apk file. Automatically handles keychains, signing and aligning.

- `deploy`: install the payload to the attached device, execute the payload and run logcat to gather payload output.

### `inject`

```
Usage: inject [OPTION]... [COMMAND] [FILE]
Inject smali payload code into the FILE template. If INPUT is not given, reads code from STDIN.

COMMAND specifies where to inject the code:
  locals         	set the number of local variables, can be injected only once
  init           	inject the code to the main class constructor, can be done multiple times
  methods        	inject new methods to the main class, can be done multiple ones

OPTION may be the following:
  -h, --help     	print this message and exit
  -f, --file=INPUT	inject the code from INPUT
```

Compile the app by running `make`, the binary will be placed in `build/`.

### `compile`

A bash script, so will work as-is.

```
Usage: APKTOOL=path-to-apktool ANDROID_HOME=path-to-sdk-bin compile
Compiles and signs the apk
```


### `deploy`

Simply execute the script, no extra action required.
