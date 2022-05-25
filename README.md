# hw-test-agent
An application that allows running tests on hardware devices.

# Commands
Show Help:
```
build/release/sr-hw-test-agent --help
```
Run a test file on hardware:
```
build/release/sr-hw-test-agent --run --board BOARD --file FILE
  -bXX, --board=XX         Specify the target board for tests
  -fXX, --file=XX          Specify the firmware .bin file to test
  -u, --unlocked           Don't ensure only one agent runs at a time
  -lXX, --lock-timeout=XX  Seconds to wait for exclusive agent access
  -tXX, --test-timeout=XX  Seconds to wait for test to complete
```
Run the agent as a network server that will listen for test requests over the network.  NOTE: There is no security.  Any software that can reach this port can run any file.  Only expose this software in a trusted environment.
```
build/release/sr-hw-test-agent --server --port PORT
  -s, --server             Run as a network server
  -pXX, --port=XX          Port for the server to listen on
  -u, --unlocked           Don't ensure only one agent runs at a time
  -lXX, --lock-timeout=XX  Seconds to wait for exclusive agent access
  -tXX, --test-timeout=XX  Seconds to wait for test to complete
```
Show the system's current configuration:
```
build/release/sr-hw-test-agent --show-config
```

# Configuration
Configuration is stored in the file '/etc/hardware-test-agent.conf'.  This file consists of multiple sections where each defines a hardware device to run tests on.  Currently the local boards only support running with stlink.

## Local Boards (stlink)
```
[<board>]
type=stlink
id=<Programmer Serial Number>
flash-base=<Address to load firmware to, such as: 0x08000000>
```
## Network Boards
```
[<board>]
type=network
host=<hostname>
port=<port>
```
