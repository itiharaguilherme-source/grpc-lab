#include <iostream>
#include <memory>
#include <grpcpp/grpcpp.h>
#include "calculator.grpc.pb.h"

class CalculadoraClient {
 public:
  CalculadoraClient(std::shared_ptr<grpc::Channel> channel)
      : stub_(calculadora::Calculadora::NewStub(channel)) {}

  void Somar(double a, double b) {
    calculadora::SomaRequest request;
    request.set_a(a);
    request.set_b(b);
    calculadora::SomaResponse response;
    grpc::ClientContext context;
    std::cout << "Cliente C++ -> Chamando Somar(" << a << ", " << b << ")" << std::endl;
    grpc::Status status = stub_->Somar(&context, request, &response);
    if (status.ok()) {
      std::cout << "Cliente C++ <- Resultado: " << response.resultado() << std::endl;
    } else {
      std::cout << "RPC falhou: " << status.error_message() << std::endl;
    }
  }

 private:
  std::unique_ptr<calculadora::Calculadora::Stub> stub_;
};

int main(int argc, char** argv) {
  std::string target = (argc > 1) ? argv[1] : "localhost:50052";
  CalculadoraClient client(grpc::CreateChannel(target, grpc::InsecureChannelCredentials()));
  client.Somar(100.5, 50.2);
  return 0;
}
