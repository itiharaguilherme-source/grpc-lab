import os
import grpc
import file_processor_pb2
import file_processor_pb2_grpc

os.environ['http_proxy'] = ''
os.environ['https_proxy'] = ''
os.environ['no_proxy'] = '*'

def generate_requests(file_path, request_params):
    file_name = os.path.basename(file_path)
    yield file_processor_pb2.FileRequest(file_name=file_name, **request_params)
    with open(file_path, 'rb') as f:
        while True:
            chunk = f.read(1024)
            if not chunk:
                break
            yield file_processor_pb2.FileRequest(
                file_content=file_processor_pb2.FileChunk(content=chunk)
            )

def save_response_stream(response_stream, output_path):
    with open(output_path, 'wb') as f:
        for response in response_stream:
            if response.HasField('file_content'):
                f.write(response.file_content.content)
    return True

class FileProcessorClient:
    def __init__(self, target='servidor-grpc:50051'):
        self.channel = grpc.insecure_channel(
            target, options=[('grpc.enable_http_proxy', 0)])
        self.stub = file_processor_pb2_grpc.FileProcessorServiceStub(self.channel)

    def compress_pdf(self, input_path, output_path):
        params = {'compress_pdf_params': file_processor_pb2.CompressPDFRequest()}
        requests = generate_requests(input_path, params)
        responses = self.stub.CompressPDF(requests)
        return save_response_stream(responses, output_path)

    def convert_to_txt(self, input_path, output_path):
        pass
    def convert_image(self, input_path, output_path, out_fmt):
        pass
    def resize_image(self, input_path, output_path, w, h):
        pass

if __name__ == '__main__':
    cliente = FileProcessorClient(target='host.docker.internal:50051')
    print("Iniciando compressão...")
    try:
        cliente.compress_pdf('aula10.pdf', 'comprimido.pdf')
        print("Compressão finalizada! Verifique o arquivo comprimido.pdf na sua pasta.")
    except Exception as e:
        print(f"Erro durante a execução: {e}")
