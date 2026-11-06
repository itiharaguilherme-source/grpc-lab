#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <chrono>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <grpcpp/grpcpp.h>
#include "file_processor.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using grpc::Status;
using namespace file_processor;

class FileProcessorServiceImpl final : public FileProcessorService::Service {
private:
  void LogOperation(const std::string& type, const std::string& service,
                    const std::string& file, const std::string& msg) {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char ts[32];
    std::strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    std::ofstream log("server.log", std::ios::app);
    log << "[" << ts << "] [" << type << "] [" << service << "] [file="
        << file << "] " << msg << std::endl;
  }

  bool ReceiveFile(ServerReaderWriter<FileResponse, FileRequest>* stream,
                   std::string& filepath, std::string& original_name,
                   FileRequest& last_req) {
    FileRequest req;
    std::ofstream outfile;
    bool first = true;
    while (stream->Read(&req)) {
      if (first) {
        original_name = req.file_name();
        filepath = "/tmp/input_" + original_name;
        outfile.open(filepath, std::ios::binary);
        last_req = req;
        first = false;
      }
      if (req.has_file_content()) {
        outfile.write(req.file_content().content().c_str(),
                      req.file_content().content().size());
      }
    }
    if (outfile.is_open()) outfile.close();
    return !first;
  }

  Status SendFile(ServerReaderWriter<FileResponse, FileRequest>* stream,
                  const std::string& out_filepath, const std::string& out_filename) {
    std::ifstream infile(out_filepath, std::ios::binary);
    if (!infile) return Status(grpc::StatusCode::INTERNAL, "Falha ao ler output");
    FileResponse resp;
    resp.set_success(true);
    resp.set_file_name(out_filename);
    resp.set_status_message("Sucesso");
    char buffer[1024];
    while (infile.read(buffer, sizeof(buffer)) || infile.gcount() > 0) {
      FileChunk* chunk = new FileChunk();
      chunk->set_content(buffer, infile.gcount());
      resp.set_allocated_file_content(chunk);
      stream->Write(resp);
      resp.clear_file_content();
    }
    return Status::OK;
  }

public:
  Status CompressPDF(ServerContext* context,
                     ServerReaderWriter<FileResponse, FileRequest>* stream) override {
    std::string in_path, name;
    FileRequest first_req;

    if (!ReceiveFile(stream, in_path, name, first_req)) {
      LogOperation("ERRO", "CompressPDF", "-", "Nenhum dado recebido do cliente");
      return Status(grpc::StatusCode::INVALID_ARGUMENT, "Nenhum arquivo recebido");
    }

    std::string out_path = "/tmp/output_" + name;
    std::string cmd = "gs -sDEVICE=pdfwrite -dCompatibilityLevel=1.4 "
                      "-dPDFSETTINGS=/ebook -dNOPAUSE -dQUIET -dBATCH "
                      "-sOutputFile='" + out_path + "' '" + in_path + "' 2>/tmp/gs_err.txt";
    int rc = std::system(cmd.c_str());

    if (rc != 0) {
      LogOperation("ERRO", "CompressPDF", name,
                   "Falha no Ghostscript, codigo de retorno=" + std::to_string(rc));
      std::remove(in_path.c_str());
      std::remove(out_path.c_str());
      return Status(grpc::StatusCode::INTERNAL, "Falha ao comprimir o PDF");
    }

    Status s = SendFile(stream, out_path, "comprimido_" + name);
    if (s.ok()) LogOperation("SUCESSO", "CompressPDF", name, "PDF comprimido e enviado");
    else LogOperation("ERRO", "CompressPDF", name, s.error_message());

    std::remove(in_path.c_str());
    std::remove(out_path.c_str());
    return s;
  }

  Status ConvertToTXT(ServerContext*, ServerReaderWriter<FileResponse, FileRequest>*) override {
    return Status::OK;
  }
  Status ConvertImageFormat(ServerContext*, ServerReaderWriter<FileResponse, FileRequest>*) override {
    return Status::OK;
  }
  Status ResizeImage(ServerContext*, ServerReaderWriter<FileResponse, FileRequest>*) override {
    return Status::OK;
  }
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  FileProcessorServiceImpl service;
  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Servidor gRPC operacional em " << server_address << std::endl;
  server->Wait();
}

int main() {
  RunServer();
  return 0;
}
