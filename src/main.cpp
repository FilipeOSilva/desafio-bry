#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/PartHandler.h>
#include <Poco/Net/MessageHeader.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/StreamCopier.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Stringifier.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/Application.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>

#include "DocToSignature.h"
#include "DocSignInfo.h"

using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;
using namespace Poco::JSON;

class MyPartHandler : public PartHandler {
public:
    std::map<std::string, std::vector<unsigned char>> files;
    std::map<std::string, std::string> fields;

    void handlePart(const MessageHeader& header, std::istream& stream) override {
        std::string contentDisposition = header.get("Content-Disposition", "");
        std::string dispositionValue;
        NameValueCollection params;
        MessageHeader::splitParameters(contentDisposition, dispositionValue, params);

        std::string field = params.get("name", "");
        std::string filename = params.get("filename", "");

        std::ostringstream ss;
        StreamCopier::copyStream(stream, ss);
        std::string data = ss.str();

        if (!filename.empty()) {
            files[field] = std::vector<unsigned char>(data.begin(), data.end());
        } else {
            fields[field] = data;
        }
    }
};

class SignatureHandler : public HTTPRequestHandler {
public:
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        response.setContentType("application/json");
        std::ostream& out = response.send();

        MyPartHandler partHandler;
        HTMLForm form(request, request.stream(), partHandler);

        if (partHandler.files.count("document") == 0 || partHandler.files.count("pkcs12") == 0) { //} || partHandler.fields.count("password") == 0) {
            response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
            Object::Ptr error = new Object;
            error->set("error", "Campos obrigatórios ausentes: document, pkcs12 ou password.");
            Stringifier::stringify(error, out);
            return;
        }

        std::vector<unsigned char>& docData = partHandler.files["document"];
        std::vector<unsigned char>& docPKCS12 = partHandler.files["pkcs12"];

        std::string base64PKCS12(docPKCS12.begin(), docPKCS12.end());
        std::string docStr(docData.begin(), docData.end());

        DocToSignature doc2sign(base64PKCS12, "bry123456", docStr);

        std::cout << docStr.c_str() << "\n";

        // DocToSignature doc2sign(docPKCS12, "bry123456", docData.data());

        // Aqui você faria o processamento real com OpenSSL (assinar CMS)
        DocSignInfo validDoc(doc2sign.getDocSignBase64());


        Object::Ptr result = new Object;

        result->set("cms", doc2sign.getDocSignBase64());
        Stringifier::stringify(result, out);
    }
};

class VerifyHandler : public HTTPRequestHandler {
public:
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");

        Poco::JSON::Object::Ptr json = new Poco::JSON::Object;
        MyPartHandler partHandler;
        HTMLForm form(request, request.stream(), partHandler);

        std::ostream& out = response.send();
        if (partHandler.files.count("cms")) {

            const std::vector<unsigned char>& cmsData = partHandler.files["cms"];


            // json->set("status", assinaturaValida ? "VALIDO" : "INVALIDO");
            // if (assinaturaValida) {
            //     json->set("signatario", nomeSignatario);
            //     json->set("dataAssinatura", dataAssinatura);
            //     json->set("hashDocumento", hashDocumento);
            //     json->set("algoritmoHash", algoritmoHash);
            // }
        } else {
            json->set("status", "INVALIDO");
            json->set("error", "File CMS not found!");
        }

        Poco::JSON::Stringifier::stringify(json, out);
    }
};

class SignRequestHandlerFactory : public HTTPRequestHandlerFactory {
public:
    HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request) override {
        if (request.getURI() == "/signature/" && request.getMethod() == HTTPRequest::HTTP_POST) {
            return new SignatureHandler;
        } else if (request.getURI() == "/verify/" && request.getMethod() == HTTPRequest::HTTP_POST) {
            return new VerifyHandler;
        }
        return nullptr;
    }
};

class SignServerApp : public ServerApplication {
protected:
    int main(const std::vector<std::string>&) override {
        HTTPServer server(new SignRequestHandlerFactory, ServerSocket(8080), new HTTPServerParams);
        server.start();
        std::cout << "Servidor HTTP rodando na porta 8080...\n";
        waitForTerminationRequest(); // CTRL+C para sair
        server.stop();
        return Application::EXIT_OK;
    }
};

int main(int argc, char** argv) {
    SignServerApp app;
    return app.run(argc, argv);
}
