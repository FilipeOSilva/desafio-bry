#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/StreamCopier.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Stringifier.h>
#include <iostream>
#include <sstream>

#include "Constants.h"
#include "Server.h"
#include "DocToSignature.h"
#include "DocSignInfo.h"

using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;

// cppcheck-suppress unusedFunction
void MultPartHandler::handlePart(const MessageHeader& header, std::istream& stream) {
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

// cppcheck-suppress unusedFunction
void SignatureHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) {
    response.setContentType("application/json");
    std::ostream& out = response.send();
    Poco::JSON::Object::Ptr json = new Poco::JSON::Object;
    MultPartHandler partHandler;

    HTMLForm form(request, request.stream(), partHandler);
    std::string passw = form.get("password", "");

    if (partHandler.files.count("document") && partHandler.files.count("pkcs12") && !passw.empty()) {
        std::vector<unsigned char>& docData = partHandler.files["document"];
        std::vector<unsigned char>& docPKCS12 = partHandler.files["pkcs12"];

        std::string signerCertPKCS12(docPKCS12.begin(), docPKCS12.end());
        std::string docStr(docData.begin(), docData.end());

        try {
            DocToSignature doc2sign(signerCertPKCS12, passw, docStr);
            json->set("cms", doc2sign.getDocSignBase64());
        } catch (const std::runtime_error& e) {
            response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
            json->set("error", e.what());
        } catch (...) {
            response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
            json->set("error", "Internal Error.");
        }
    } else {
        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
        json->set("error", "Need data: document, pkcs12 or password.");
    }

    Poco::JSON::Stringifier::stringify(json, out);
}

void VerifyHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) {
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");
    std::ostream& out = response.send();
    Poco::JSON::Object::Ptr json = new Poco::JSON::Object;
    MultPartHandler partHandler;

    HTMLForm form(request, request.stream(), partHandler);

    if (partHandler.files.count("cms")) {
        const std::vector<unsigned char>& cmsData = partHandler.files["cms"];
        std::string docCMS(cmsData.begin(), cmsData.end());

        try {
            DocSignInfo docVerifyCMS(docCMS);

            json->set("status", docVerifyCMS.getStatusCert() ? "VALIDO" : "INVALIDO");
            json->set("CN", docVerifyCMS.getCN());
            json->set("signingTime", docVerifyCMS.getSigningTime());
            json->set("encapContentInfo", docVerifyCMS.getEncapContentInfoHEXA());
            json->set("digestAlgorithm", docVerifyCMS.getDigestAlgorithm());
        } catch (const std::runtime_error& e) {
            response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
            json->set("error", e.what());
        } catch (...) {
            response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
            json->set("error", "Internal Error.");
        }
    } else {
        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
        json->set("status", "INVALIDO");
        json->set("error", "File CMS not found!");
    }

    Poco::JSON::Stringifier::stringify(json, out);
}

void NotFoundHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) {
    response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
    response.setContentType("application/json");

    Poco::JSON::Object::Ptr json = new Poco::JSON::Object;
    json->set("error", "Endpoint not found!");

    std::ostream& out = response.send();
    Poco::JSON::Stringifier::stringify(json, out);
}

// cppcheck-suppress unusedFunction
HTTPRequestHandler* SignVerifyRequestHandler::createRequestHandler(const HTTPServerRequest& request) {
    if (request.getURI() == Constants::ENDPOINT_SIGNATURE && request.getMethod() == HTTPRequest::HTTP_POST) {
        return new SignatureHandler;
    } else if (request.getURI() == Constants::ENDPOINT_VERIFY && request.getMethod() == HTTPRequest::HTTP_POST) {
        return new VerifyHandler;
    }
    return new NotFoundHandler;
}

int SignVerifyServerApp::main(const std::vector<std::string>&) {
    HTTPServer server(new SignVerifyRequestHandler, ServerSocket(Constants::HTTP_PORT), new HTTPServerParams);
    server.start();
    std::cout << "Server HTTP in Port " << Constants::HTTP_PORT << " ...\n";
    waitForTerminationRequest();
    server.stop();
    return Application::EXIT_OK;
}