cmake_minimum_required(VERSION 3.10)
project(grpc_calculator_cpp CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GRPC REQUIRED grpc++)
pkg_check_modules(PROTOBUF REQUIRED protobuf)

set(PB_CC ${CMAKE_CURRENT_BINARY_DIR}/calculator.pb.cc)
set(PB_H ${CMAKE_CURRENT_BINARY_DIR}/calculator.pb.h)
set(GRPC_CC ${CMAKE_CURRENT_BINARY_DIR}/calculator.grpc.pb.cc)
set(GRPC_H ${CMAKE_CURRENT_BINARY_DIR}/calculator.grpc.pb.h)
set(PROTO_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/calculator.proto)

add_custom_command(
  OUTPUT ${PB_CC} ${GRPC_CC} ${PB_H} ${GRPC_H}
  COMMAND /usr/bin/protoc
    --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
    --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
    --plugin=protoc-gen-grpc=/usr/bin/grpc_cpp_plugin
    -I${CMAKE_CURRENT_SOURCE_DIR}
    ${PROTO_SOURCE}
  DEPENDS ${PROTO_SOURCE}
  COMMENT "Gerando stubs C++ do Protobuf/gRPC para calculator.proto"
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(servidor_cpp servidor_cpp.cpp ${PB_CC} ${GRPC_CC})
target_link_libraries(servidor_cpp grpc++ grpc protobuf)

add_executable(cliente_cpp cliente_cpp.cpp ${PB_CC} ${GRPC_CC})
target_link_libraries(cliente_cpp grpc++ grpc protobuf)
