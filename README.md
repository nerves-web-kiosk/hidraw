# Hidraw

Hidraw is an Elixir interface to Linux hidraw devices.

## Usage

Hidraw can be used to monitor `/dev/hidraw*` devices and report raw data to
the parent process.

Start by looking for the device you want to monitor.

```elixir
iex> Hidraw.enumerate
[
  {"/dev/hidraw2", "ﾩSymbol Technologies, Inc, 2002 Symbol Bar Code Scanner"},
  {"/dev/hidraw1", "DLL082A:01 06CB:76AF"},
  {"/dev/hidraw0", "ELAN Touchscreen"}
]
```

Here we have a Barcode scanner at `/dev/hidraw2`, lets open it.

```elixir
iex> Hidraw.start_link "/dev/hidraw2"
{:ok, #PID<0.197.0>}
```

The first message we will receive is the device's report descriptor

```elixir
iex> flush
{:hidraw, "/dev/hidraw2",
 {:report_descriptor,
  <<6, 69, 255, 10, 0, 75, 161, 1, 10, 1, 74, 117, 8, 149, 11, 21, 0, 38, 255,
    0, 145, 2, 10, 2, 74, 149, 64, 129, 2, 192>>}}
```

All subsequent messages will be triggered off device events.
Here I am scanning a barcode:

```elixir
iex(4)> flush
{:hidraw, "/dev/hidraw2",
 <<16, 16, 3, 0, 65, 67, 67, 49, 55, 49, 49, 50, 79, 0, 24, 11, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   ...>>}
```
