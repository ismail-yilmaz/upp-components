# NetProxy package for Ultimate++

This package implements a client-side network proxy class, encapsulating two widely used network proxy protocols: HTTP tunneling and SOCKS

## Features and Highlights

- Uses `HTTP_CONNECT` method for http tunneling.
- Encapsulates SOCKS proxy protocol version 4/4a, and version 5, as defined in RFC 1928 and RFC 1929. 
- In SOCKS mode, NetProxy can work with both IPv4 and IPv6 address families.
- In SOCKS mode, NetProxy allows BIND requests.
- In SOCKS mode, NetProxy allows remote name lookups (DNS).
- Supports both synchronous and asynchronous operation modes.
- Allows SSL connections to the target machine (not to proxy itself) in both Http and SOCKS modes.
- Package comes with full public API document for Topic++, and has a typical BSD license.

## Examples

Two examples are provided with the package.

|**Name**            | **Description**                                                             |
|:---                |:---                                                                         |
| SocksProxyExample  | Demonstrates a simple, blocking `SOCKS5` connection to a target machine.    |
| SocksProxtExampleNB | Demonstrates a simple, non-blocking `SOCKS5` connection to a target machine |
