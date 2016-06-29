#!/usr/bin/python
# coding = utf-8

import time
import httplib
from ThriftHelper import ThriftToJSON, JSONToThrift


class HttpClient(object):
    def __init__(self, server_ip, server_port, server_module, timeout=2000):
        self.server_module = server_module
        self.http_client = httplib.HTTPConnection(server_ip, server_port, timeout)
        self.headers = {"Content-type": "application/x-www-form-urlencoded", "Accept": "text/json"}

    def request(self, req_ins, rsp_cls):
        t1 = time.time()
        self.http_client.request("POST", "/" + self.server_module + ".cgi", ThriftToJSON(req_ins), self.headers)
        response = self.http_client.getresponse()
        print req_ins.command + " cost %s ms" % ((time.time() - t1) * 1000)
        return JSONToThrift(rsp_cls(), response.read())

    def __del__(self):
        self.http_client.close()

    def close(self):
        self.http_client.close()
