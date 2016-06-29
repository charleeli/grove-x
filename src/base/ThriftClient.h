#ifndef _THRIFT_CLIENT_H_
#define _THRIFT_CLIENT_H_

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

template <typename ServiceClient>
class ThriftClient {
private:
    string host;
    int port;

    boost::shared_ptr<TTransport> socket;
    boost::shared_ptr<TTransport> transport;
    boost::shared_ptr<TProtocol> protocol;
    boost::shared_ptr<ServiceClient> client;

public:
    ThriftClient(const string & host, int port){
        this->host = host;
        this->port = port;
        this->socket = boost::shared_ptr<TTransport>(new TSocket(this->host, this->port));
        this->transport = boost::shared_ptr<TTransport>(new TFramedTransport(this->socket));
        this->protocol = boost::shared_ptr<TProtocol>(new TBinaryProtocol(this->transport));
        this->client = boost::shared_ptr<ServiceClient>(new ServiceClient(this->protocol));
        this->transport->open();
    }

    ~ThriftClient(){
        this->transport->close();
    }

    boost::shared_ptr<ServiceClient> getClient(){
        return this->client;
    }
};

#endif
