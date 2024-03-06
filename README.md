# SmrtClcltr
A calculator so smart that you will feel dumb when you realize its potential. Or maybe it is just called that to compensate for something.

## Words
Originally written in Python for the flexibility and ease of programming, smrtclcltr was re-imagined as a C++ application that had very few outside dependencies. Then it could be built to run in lower-resource environments. For lowest resource requirements, it is possible to compile it supporting only native types (32 or 64 bits for integers, etc., depending on the target processor). But it also supports arbitrary precision using boost::multiprecision, though that may not be as suitable for low-resource targets.

Also, it is a toy to help me keep up my modern C++ coding skills.

The idea here is that you have a keyboard attached to your HP48GX. Remember the blazing speed and and precision (up to 16 decimal places or something like that). Who cares. Modern arithmetic libraries offer arbitrary precision. Also real keyboards are way better than that daffy non-QWERTY POS that the HP48GX boasted. Ooof. RPN was a must, since that is the best way to deal with a stack. Also it is a nice 1337 group that understands how to use it. Many of the same functions are available, and more may show up if/when I have time. Also it has stuff the HP48GX didn't have. Like:
 - time functions: now, UTC dates, differences, etc.
 - signed/unsigned fixed-bit number interpretation, so you can always check out if your compiler is doing the right thing
 - multiplicative modular inverse (good for RSA debug)
 - netmask math? This was in the python version, but rarely used, so maybe not...

## Building
### Prerequisites
 - meson (1.1.0 or later) and ninja (1.10 or later)
 - clang++-18 or g++-13 (or later)
 - libstdc++ or libc++ that support c++23 features
 - libmpfr-dev (optional, but recommended for speed and usability)
 - libgmp-dev (optional, but second choice, and needed for mpfr anyway)
 - libboost-dev (header-only libraries) (1.85 or later)

### Incantation
SmrtClcltr's build system is meson-based. So build it just like any other meson application.
```
# configure (name assumes gcc is default)
meson setup build-gcc

# build
meson compile -C build-gcc

# configure with an alternate compiler
CC=clang CXX=clang++ meson setup build-clang

# build
meson compile -C build-clang

# configure with an alternate compiler and custom boost
# use the path to the boost root (source), not installed version
BOOST_ROOT=/usr/src/boost CC=clang CXX=clang++ meson setup build-clang-boost

# build
meson compile -C build-clang-boost
```
