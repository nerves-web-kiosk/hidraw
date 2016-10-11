defmodule Hidraw do
  use GenServer

  def start_link(fd) do
    GenServer.start_link(__MODULE__, [fd, self])
  end

  def enumerate() do
    executable = :code.priv_dir(:hidraw) ++ '/ex_hidraw'
    port = Port.open({:spawn_executable, executable},
      [{:args, ["enumerate"]},
        {:packet, 2},
        :use_stdio,
        :binary])
    receive do
      {^port, {:data, <<?r, message::binary>>}} ->
        :erlang.binary_to_term(message)
    after
        5_000 ->
          Port.close(port)
          []
    end
  end

  def init([fd, caller]) do
    executable = :code.priv_dir(:hidraw) ++ '/ex_hidraw'
    port = Port.open({:spawn_executable, executable},
      [{:args, [fd]},
        {:packet, 2},
        :use_stdio,
        :binary,
        :exit_status])
    state = %{port: port, name: fd, callback: caller, buffer: []}

    {:ok, state}
  end

  def handle_info({_, {:data, <<?n, message::binary>>}}, state) do
    msg = :erlang.binary_to_term(message)
    handle_port(msg, state)
  end

  def handle_info({_, {:data, <<?e, message::binary>>}}, state) do
    error = :erlang.binary_to_term(message)
    send state.callback,  {:hidraw, state.name, error}
    {:stop, error, state}
  end

  defp handle_port({:data, value}, state) do
    IO.puts "Received Data: #{inspect value}"
    {:noreply, state}
  end
end
