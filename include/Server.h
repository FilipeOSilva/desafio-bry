#ifndef SERVER_H
#define SERVER_H

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/MessageHeader.h>
#include <Poco/Net/PartHandler.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Util/ServerApplication.h>
#include <map>
#include <vector>
#include <string>

class MultPartHandler : public Poco::Net::PartHandler {
public:
    std::map<std::string, std::vector<unsigned char>> files;
    std::map<std::string, std::string> fields;

    void handlePart(const Poco::Net::MessageHeader& header, std::istream& stream) override;
};

class SignatureHandler : public Poco::Net::HTTPRequestHandler {
public:
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                       Poco::Net::HTTPServerResponse& response) override;
};

class VerifyHandler : public Poco::Net::HTTPRequestHandler {
public:
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                       Poco::Net::HTTPServerResponse& response) override;
};

class NotFoundHandler : public Poco::Net::HTTPRequestHandler {
public:
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                       Poco::Net::HTTPServerResponse& response) override;
};

class SignVerifyRequestHandler : public Poco::Net::HTTPRequestHandlerFactory {
public:
    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request) override;
};

class SignVerifyServerApp : public Poco::Util::ServerApplication {
protected:
    int main(const std::vector<std::string>& args) override;
};

#endif // SERVER_H