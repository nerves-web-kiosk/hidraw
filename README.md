# Hidraw

Hidraw is an Elixir interface to Linux hidraw devices.

## Installation

If [available in Hex](https://hex.pm/docs/publish), the package can be installed as:

  1. Add `hidraw` to your list of dependencies in `mix.exs`:

    ```elixir
    def deps do
      [{:hidraw, "~> 0.1.0"}]
    end
    ```

  2. Ensure `hidraw` is started before your application:

    ```elixir
    def application do
      [applications: [:hidraw]]
    end
    ```
