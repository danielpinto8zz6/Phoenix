# Phoenix

## TODO
- [x] Send received data on gateway to server
- [x] Finish pipe comunication between gateway and clients
- [x] Handle errors (Server close, Client close, etc)
- [x] Move gateway logic to server (gateway is just a bridge to pass data)
- [x] Add events to know when to read and write on shared memory
- [x] Server only run once
- [x] Gateway only run once + doesn't start without server
- [x] Client doesn't start without gateway
- [ ] Add registry keys to store players points, etc...
- [ ] Close client pipe when he exists