import sys
import grpc
import calculator_pb2
import calculator_pb2_grpc

def run(target):
    with grpc.insecure_channel(target) as channel:
        stub = calculator_pb2_grpc.CalculadoraStub(channel)
        print(f"Cliente Python -> Chamando Somar(10, 5) em {target}")
        response = stub.Somar(calculator_pb2.SomaRequest(a=10, b=5))
        print(f"Cliente Python <- Resultado: {response.resultado}")

if __name__ == '__main__':
    run(sys.argv[1] if len(sys.argv) > 1 else 'localhost:50051')
