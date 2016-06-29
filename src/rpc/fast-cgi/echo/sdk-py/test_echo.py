#!/usr/bin/python
# coding = utf-8

import sys
sys.path.append('gen_py')

from HttpClient import HttpClient
from ThriftHelper import ThriftToSimpleJSON
from HttpCmd.ttypes import Header
from EchoCmd.ttypes import EchoReq, EchoRsp

server_ip = "127.0.0.1"
server_port = 80
server_module = "echo_module"


def main():
    result = EchoRsp()
    try:
        echoReq = EchoReq()
        echoReq.command = "Echo"
        echoReq.header = Header()
        echoReq.foo = "hello world!"

        result = HttpClient(server_ip, server_port, server_module).request(echoReq, EchoRsp)
    except Exception, e:
        result.error = -1
        result.errmsg = str(e)
    finally:
        print ThriftToSimpleJSON(result)

if __name__ == '__main__':
    main()
