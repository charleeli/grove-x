#!/usr/bin/python
# coding=utf-8

from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol


class ThriftClient(object):

    def __init__(self, service, server_ip, server_port, timeout=2000):
        self.service = service
        self.server_ip = server_ip
        self.server_port = server_port

        trans = TSocket.TSocket(self.server_ip, self.server_port)
        trans.setTimeout(timeout)
        self.transport = TTransport.TFramedTransport(trans)

        self.protocol = TBinaryProtocol.TBinaryProtocol(self.transport)
        self.client = self.service.Client(self.protocol)

        self.transport.open()

    def __del__(self):
        self.transport.close()

    def close(self):
        self.transport.close()
