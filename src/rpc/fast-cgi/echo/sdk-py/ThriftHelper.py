# -*- coding:utf-8 -*-
from thrift.protocol import TJSONProtocol
from thrift.protocol import TBinaryProtocol
from thrift.protocol import TCompactProtocol
from thrift.transport import TTransport
import json

class MyJSONEncoder(json.JSONEncoder):
    def default(self,obj,display_meta = False):
        #convert object to a dict
        d = {}
        if display_meta == True:
            d['__class__'] = obj.__class__.__name__
            d['__module__'] = obj.__module__
        d.update(obj.__dict__)
        return d

class MyJSONDecoder(json.JSONDecoder):
    def __init__(self):
        json.JSONDecoder.__init__(self,object_hook=self.dict2object)
    def dict2object(self,d):
        #convert dict to object
        if'__class__' in d:
            class_name = d.pop('__class__')
            module_name = d.pop('__module__')
            module = __import__(module_name)
            class_ = getattr(module,class_name)
            args = dict((key.encode('ascii'), value) for key, value in d.items()) #get args
            inst = class_(**args) #create new instance
        else:
            inst = d
        return inst

def ObjectToJSON(thrift_object):
    return MyJSONEncoder().encode(thrift_object)


def JSONToObject(base):
    return MyJSONDecoder().decode(base)

def ThriftToSimpleJSON(thrift_object,
              protocol_factory=TJSONProtocol.TSimpleJSONProtocolFactory()):
    transport = TTransport.TMemoryBuffer()
    protocol = protocol_factory.getProtocol(transport)
    thrift_object.write(protocol)
    return transport.getvalue()

def SimpleJSONToThrift(base,
                buf,
                protocol_factory=TJSONProtocol.TSimpleJSONProtocolFactory()):
    transport = TTransport.TMemoryBuffer(buf)
    protocol = protocol_factory.getProtocol(transport)
    base.read(protocol)
    return base


def ThriftToJSON(thrift_object,
              protocol_factory=TJSONProtocol.TJSONProtocolFactory()):
    transport = TTransport.TMemoryBuffer()
    protocol = protocol_factory.getProtocol(transport)
    thrift_object.write(protocol)
    return transport.getvalue()


def JSONToThrift(base,
                buf,
                protocol_factory=TJSONProtocol.TJSONProtocolFactory()):
    transport = TTransport.TMemoryBuffer(buf)
    protocol = protocol_factory.getProtocol(transport)
    base.read(protocol)
    return base

def ThriftToTBinary(thrift_object,
              protocol_factory=TBinaryProtocol.TBinaryProtocolFactory()):
    transport = TTransport.TMemoryBuffer()
    protocol = protocol_factory.getProtocol(transport)
    thrift_object.write(protocol)
    return transport.getvalue()


def BinaryToThrift(base,
                buf,
                protocol_factory=TBinaryProtocol.TBinaryProtocolFactory()):
    transport = TTransport.TMemoryBuffer(buf)
    protocol = protocol_factory.getProtocol(transport)
    base.read(protocol)
    return base

def ThriftToCompact(thrift_object,
              protocol_factory=TCompactProtocol.TCompactProtocolFactory()):
    transport = TTransport.TMemoryBuffer()
    protocol = protocol_factory.getProtocol(transport)
    thrift_object.write(protocol)
    return transport.getvalue()

def CompactToThrift(base,
                buf,
                protocol_factory=TCompactProtocol.TCompactProtocolFactory()):
    transport = TTransport.TMemoryBuffer(buf)
    protocol = protocol_factory.getProtocol(transport)
    base.read(protocol)
    return base

def JSONToSimpleJSON(thrift_cls,buf):
    thrift_instance = thrift_cls()
    return ThriftToSimpleJSON(JSONToThrift(thrift_instance, buf))

def CompactToSimpleJSON(thrift_cls,buf):
    thrift_instance = thrift_cls()
    return ThriftToSimpleJSON(CompactToThrift(thrift_instance, buf))