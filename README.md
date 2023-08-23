# C++20 Coordinated Looping Threadpool

A `CoordinatedLoopingThreadpool` constitutes zero or more threads, each of which executes the same function on a unique argument each time the threadpool is triggered.

## Member Functions

| Function                                         | Purpose |
| ------------------------------------------------ | ------- |
| `CoordinatedLoopingThreadpool(auto F, auto args)`| create a threadpool that will execute `F(arg)` for each `arg` in `args` each time the threadpool is triggered. |
| `trigger()`                                      | run `F` on each of the supplied arguments one time asynchronously |
| `sync()`                                         | block until all running instances of `F` complete |
| `trigger_sync()`                                 | `trigger` synchronously; equivalent to `trigger(); sync();` |

`sync()`, `trigger()`, and `trigger_sync()` can be safely called willy-nilly--repeated calls to `sync()` without corresponding calls to `trigger()` have no ill effect, and `trigger()` will block if the previous iteration is still underway.

## Usage

Given function `F`, iterable `A`, and unrelated function `do_stuff()`, the following code blocks will yield equivalent results:

```c++
for (auto a: A) F(a);
do_stuff();
for (auto a: A) F(a);
do_stuff();
```

```c++
CoordinatedLoopingThreadpool tp(F, A);
tp.trigger_sync();
do_stuff();
tp.trigger_sync();
do_stuff();
```

```c++
CoordinatedLoopingThreadpool tp(F, A);
tp.trigger();
do_stuff();
tp.sync();
tp.trigger();
do_stuf();
tp.sync();
```

The last block will be fastest if `F(a)` doesn't take significantly longer than `do_stuff()` does. If there are multiple cores available and `F(a)` takes long enough to overcome the overhead of threads, the second and third blocks will be faster than the first.
