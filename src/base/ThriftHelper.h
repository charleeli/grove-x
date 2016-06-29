/*
 * author:charlee
 */
#ifndef _THRIFT_HELPER_H_
#define _THRIFT_HELPER_H_

#include <thrift/config.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/transport/TTransportUtils.h>

template<typename ThriftStruct>
std::string ThriftToString(const ThriftStruct& ts) {
    using namespace apache::thrift::transport;
    using namespace apache::thrift::protocol;
    TMemoryBuffer* buffer = new TMemoryBuffer;
    boost::shared_ptr<TTransport> trans(buffer);
    TBinaryProtocol protocol(trans);
    ts.write(&protocol);
    uint8_t* buf;
    uint32_t size;
    buffer->getBuffer(&buf, &size);
    return std::string((char*)buf, (unsigned int)size);
}

template<typename ThriftStruct>
bool StringToThrift(const std::string& buff,
    ThriftStruct* ts) {
    using namespace apache::thrift::transport;
    using namespace apache::thrift::protocol;
    TMemoryBuffer* buffer = new TMemoryBuffer;
    buffer->write((const uint8_t*)buff.data(), buff.size());
    boost::shared_ptr<TTransport> trans(buffer);
    TBinaryProtocol protocol(trans);
    ts->read(&protocol);
    return true;
}

template<typename ThriftStruct>
std::string ThriftToJSON(const ThriftStruct& ts) {
    using namespace apache::thrift::transport;
    using namespace apache::thrift::protocol;
    TMemoryBuffer* buffer = new TMemoryBuffer;
    boost::shared_ptr<TTransport> trans(buffer);
    TJSONProtocol protocol(trans);
    ts.write(&protocol);
    uint8_t* buf;
    uint32_t size;
    buffer->getBuffer(&buf, &size);
    return std::string((char*)buf, (unsigned int)size);
}

template<typename ThriftStruct>
bool JSONToThrift(const std::string& buff,
    ThriftStruct* ts) {
    using namespace apache::thrift::transport;
    using namespace apache::thrift::protocol;
    TMemoryBuffer* buffer = new TMemoryBuffer;
    buffer->write((const uint8_t*)buff.data(), buff.size());
    boost::shared_ptr<TTransport> trans(buffer);
    TJSONProtocol protocol(trans);
    ts->read(&protocol);
    return true;
}

template<typename ThriftStruct>
std::string ThriftToCompact(const ThriftStruct& ts) {
    using namespace apache::thrift::transport;
    using namespace apache::thrift::protocol;
    TMemoryBuffer* buffer = new TMemoryBuffer;
    boost::shared_ptr<TTransport> trans(buffer);
    TCompactProtocol protocol(trans);
    ts.write(&protocol);
    uint8_t* buf;
    uint32_t size;
    buffer->getBuffer(&buf, &size);
    return std::string((char*)buf, (unsigned int)size);
}

template<typename ThriftStruct>
bool CompactToThrift(const std::string& buff,
    ThriftStruct* ts) {
    using namespace apache::thrift::transport;
    using namespace apache::thrift::protocol;
    TMemoryBuffer* buffer = new TMemoryBuffer;
    buffer->write((const uint8_t*)buff.data(), buff.size());
    boost::shared_ptr<TTransport> trans(buffer);
    TCompactProtocol protocol(trans);
    ts->read(&protocol);
    return true;
}

#endif
