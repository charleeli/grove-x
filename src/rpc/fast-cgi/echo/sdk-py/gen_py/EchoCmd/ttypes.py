#
# Autogenerated by Thrift Compiler (0.9.2)
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#
#  options string: py
#

from thrift.Thrift import TType, TMessageType, TException, TApplicationException
import HttpCmd.ttypes


from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol, TProtocol
try:
  from thrift.protocol import fastbinary
except:
  fastbinary = None



class EchoReq:
  """
  Attributes:
   - command
   - header
   - foo
  """

  thrift_spec = (
    None, # 0
    (1, TType.STRING, 'command', None, "Echo", ), # 1
    (2, TType.STRUCT, 'header', (HttpCmd.ttypes.Header, HttpCmd.ttypes.Header.thrift_spec), None, ), # 2
    (3, TType.STRING, 'foo', None, None, ), # 3
  )

  def __init__(self, command=thrift_spec[1][4], header=None, foo=None,):
    self.command = command
    self.header = header
    self.foo = foo

  def read(self, iprot):
    if iprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and isinstance(iprot.trans, TTransport.CReadableTransport) and self.thrift_spec is not None and fastbinary is not None:
      fastbinary.decode_binary(self, iprot.trans, (self.__class__, self.thrift_spec))
      return
    iprot.readStructBegin()
    while True:
      (fname, ftype, fid) = iprot.readFieldBegin()
      if ftype == TType.STOP:
        break
      if fid == 1:
        if ftype == TType.STRING:
          self.command = iprot.readString();
        else:
          iprot.skip(ftype)
      elif fid == 2:
        if ftype == TType.STRUCT:
          self.header = HttpCmd.ttypes.Header()
          self.header.read(iprot)
        else:
          iprot.skip(ftype)
      elif fid == 3:
        if ftype == TType.STRING:
          self.foo = iprot.readString();
        else:
          iprot.skip(ftype)
      else:
        iprot.skip(ftype)
      iprot.readFieldEnd()
    iprot.readStructEnd()

  def write(self, oprot):
    if oprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and self.thrift_spec is not None and fastbinary is not None:
      oprot.trans.write(fastbinary.encode_binary(self, (self.__class__, self.thrift_spec)))
      return
    oprot.writeStructBegin('EchoReq')
    if self.command is not None:
      oprot.writeFieldBegin('command', TType.STRING, 1)
      oprot.writeString(self.command)
      oprot.writeFieldEnd()
    if self.header is not None:
      oprot.writeFieldBegin('header', TType.STRUCT, 2)
      self.header.write(oprot)
      oprot.writeFieldEnd()
    if self.foo is not None:
      oprot.writeFieldBegin('foo', TType.STRING, 3)
      oprot.writeString(self.foo)
      oprot.writeFieldEnd()
    oprot.writeFieldStop()
    oprot.writeStructEnd()

  def validate(self):
    if self.command is None:
      raise TProtocol.TProtocolException(message='Required field command is unset!')
    if self.header is None:
      raise TProtocol.TProtocolException(message='Required field header is unset!')
    if self.foo is None:
      raise TProtocol.TProtocolException(message='Required field foo is unset!')
    return


  def __hash__(self):
    value = 17
    value = (value * 31) ^ hash(self.command)
    value = (value * 31) ^ hash(self.header)
    value = (value * 31) ^ hash(self.foo)
    return value

  def __repr__(self):
    L = ['%s=%r' % (key, value)
      for key, value in self.__dict__.iteritems()]
    return '%s(%s)' % (self.__class__.__name__, ', '.join(L))

  def __eq__(self, other):
    return isinstance(other, self.__class__) and self.__dict__ == other.__dict__

  def __ne__(self, other):
    return not (self == other)

class EchoRsp:
  """
  Attributes:
   - error
   - errmsg
   - foo
   - pid
  """

  thrift_spec = (
    None, # 0
    (1, TType.I32, 'error', None, None, ), # 1
    (2, TType.STRING, 'errmsg', None, None, ), # 2
    (3, TType.STRING, 'foo', None, "", ), # 3
    (4, TType.I32, 'pid', None, None, ), # 4
  )

  def __init__(self, error=None, errmsg=None, foo=thrift_spec[3][4], pid=None,):
    self.error = error
    self.errmsg = errmsg
    self.foo = foo
    self.pid = pid

  def read(self, iprot):
    if iprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and isinstance(iprot.trans, TTransport.CReadableTransport) and self.thrift_spec is not None and fastbinary is not None:
      fastbinary.decode_binary(self, iprot.trans, (self.__class__, self.thrift_spec))
      return
    iprot.readStructBegin()
    while True:
      (fname, ftype, fid) = iprot.readFieldBegin()
      if ftype == TType.STOP:
        break
      if fid == 1:
        if ftype == TType.I32:
          self.error = iprot.readI32();
        else:
          iprot.skip(ftype)
      elif fid == 2:
        if ftype == TType.STRING:
          self.errmsg = iprot.readString();
        else:
          iprot.skip(ftype)
      elif fid == 3:
        if ftype == TType.STRING:
          self.foo = iprot.readString();
        else:
          iprot.skip(ftype)
      elif fid == 4:
        if ftype == TType.I32:
          self.pid = iprot.readI32();
        else:
          iprot.skip(ftype)
      else:
        iprot.skip(ftype)
      iprot.readFieldEnd()
    iprot.readStructEnd()

  def write(self, oprot):
    if oprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and self.thrift_spec is not None and fastbinary is not None:
      oprot.trans.write(fastbinary.encode_binary(self, (self.__class__, self.thrift_spec)))
      return
    oprot.writeStructBegin('EchoRsp')
    if self.error is not None:
      oprot.writeFieldBegin('error', TType.I32, 1)
      oprot.writeI32(self.error)
      oprot.writeFieldEnd()
    if self.errmsg is not None:
      oprot.writeFieldBegin('errmsg', TType.STRING, 2)
      oprot.writeString(self.errmsg)
      oprot.writeFieldEnd()
    if self.foo is not None:
      oprot.writeFieldBegin('foo', TType.STRING, 3)
      oprot.writeString(self.foo)
      oprot.writeFieldEnd()
    if self.pid is not None:
      oprot.writeFieldBegin('pid', TType.I32, 4)
      oprot.writeI32(self.pid)
      oprot.writeFieldEnd()
    oprot.writeFieldStop()
    oprot.writeStructEnd()

  def validate(self):
    if self.error is None:
      raise TProtocol.TProtocolException(message='Required field error is unset!')
    if self.errmsg is None:
      raise TProtocol.TProtocolException(message='Required field errmsg is unset!')
    return


  def __hash__(self):
    value = 17
    value = (value * 31) ^ hash(self.error)
    value = (value * 31) ^ hash(self.errmsg)
    value = (value * 31) ^ hash(self.foo)
    value = (value * 31) ^ hash(self.pid)
    return value

  def __repr__(self):
    L = ['%s=%r' % (key, value)
      for key, value in self.__dict__.iteritems()]
    return '%s(%s)' % (self.__class__.__name__, ', '.join(L))

  def __eq__(self, other):
    return isinstance(other, self.__class__) and self.__dict__ == other.__dict__

  def __ne__(self, other):
    return not (self == other)