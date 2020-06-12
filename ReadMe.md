# node-prlimit

Native POSIX `prlimit` command adapted for Node.JS.

Command itself can replace `setrlimit` and `getrlimit`. No need to run `ulimit` and `prlimit` in command line.

## Installation

Simply do `npm install prlimit`. Package uses `node-gyp`, so make sure you have all needed tools (`make`, C++, macos: xcode).

Also, I am looking forward to add `@types/prlimit`.

## Usage

`require('prlimit')` returns a function:
```ts
prlimit(
    pid : number, 
    resource : string | number, 
    new_limit? : {soft: number, hard: number}
) : {soft: number, hard: number}
```

I tried to not change [function logic](https://linux.die.net/man/2/prlimit), so the logic remains the same:
- If `pid` is `0`, then the call applies to the calling process. (similar to `setrlimit`, `getrlimit`)
- `resource`, as a number, refers to rlimit enum.
- `resource`, as a string, tries to find appropriate enum value:
    - `"cpu"` — RLIMIT_CPU
    - `"data"` — RLIMIT_DATA
    - `"fsize"` — RLIMIT_FSIZE
    - `"locks"` — RLIMIT_LOCKS
    - `"memlock"` — RLIMIT_MEMLOCK
    - `"msgqueue"` — RLIMIT_MSGQUEUE
    - `"nice"` — RLIMIT_NICE
    - `"nofile"` — RLIMIT_NOFILE
    - `"nproc"` — RLIMIT_NPROC
    - `"rss"` — RLIMIT_RSS
    - `"rtprio"` — RLIMIT_RTPRIO
    - `"rttime"` — RLIMIT_RTTIME
    - `"sigpending"` — RLIMIT_SIGPENDING

(resource type can be missing, according whether your OS supports it or not)
- `new_limit` changes limit of resource to new one
- function always returns old limit (even if you changed it)
