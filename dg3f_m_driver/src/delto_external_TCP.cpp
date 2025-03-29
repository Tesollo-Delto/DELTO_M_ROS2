// Copyright 2025 TESOLLO
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the TESOLLO nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include "dg3f_driver/delto_external_TCP.hpp"

namespace DeltoTCP {
// Initializer Helper Function
int Communication::GetMotorCount(uint16_t model) {
  switch (model) {
    case 0x3F01:
      return 12;
    case 0x3F02:
      return 12;
    case 0x4F02:
      return 18;
    case 0x5F02:
      return 20;
    default:
      std::cerr << "Unknown model type: " << std::hex << model << std::endl;
      return 12;
      // throw std::invalid_argument("Unknown model type");
  }
}

int Communication::GetBytePerMotor(bool fingertip_sensor, bool io) {
  if (!fingertip_sensor && !io) return 8;
  if (fingertip_sensor && !io) return 9;
  if (!fingertip_sensor && io) return 9;
  if (fingertip_sensor && io) return 10;

  throw std::invalid_argument("Invalid fingertip sensor or IO configuration");
}

Communication::Communication(const std::string& ip, int port, int16_t model,
                             bool fingertip_sensor, bool io)
    : ip_(ip),
      port_(port),
      socket_(io_context_),
      model_(model),
      fingertip_sensor_(fingertip_sensor),
      io_(io),
      motor_count_(GetMotorCount(model)),
      byte_per_motor_(GetBytePerMotor(fingertip_sensor, io)),
      total_packet_size_(HEADER_SIZE + motor_count_ * byte_per_motor_),
      expected_response_length_(99) {}

Communication::~Communication() { socket_.close(); }

void Communication::Connect() {
  tcp::resolver resolver(io_context_);
  boost::system::error_code ec;

  boost::asio::connect(socket_, resolver.resolve(ip_, std::to_string(port_)),
                       ec);

  if (ec) {
    std::cerr << "Could not connect: " << ec.message() << std::endl;
    return;
  }

  std::cout << "Connected to Delto Gripper" << std::endl;
}

void Communication::Disconnect() {
  socket_.close();
  std::cout << "Disconnected from Delto Gripper" << std::endl;
}
// 안정적인 패킷 수신 함수: 정확히 TOTAL_PACKET_SIZE만큼 읽을 때까지 대기

bool Communication::ReadFullPacket(boost::asio::ip::tcp::socket& socket,
                                   std::array<uint8_t, 99>& buffer) {
  boost::system::error_code ec;
  // std::size_t total_read = 0;
  // while (total_read < total_packet_size_) {
  //   std::size_t bytes_read =
  //       socket.read_some(boost::asio::buffer(buffer.data() + total_read,
  //                                            total_packet_size_ - total_read),
  //                        ec);
      
  //   if (ec) {
  //     std::cerr << "Read error: " << ec.message() << std::endl;
  //     return false;
  //   }
  //   total_read += bytes_read;
  // }
  // return true;

std::size_t bytes_read = boost::asio::read(
    socket, 
    boost::asio::buffer(buffer.data(), total_packet_size_),
    boost::asio::transfer_exactly(total_packet_size_),
    ec
);

if (ec) {
  std::cerr << "Read error: " << ec.message() << std::endl;
  return false;
}

return (bytes_read == total_packet_size_);

}


DeltoReceivedData Communication::GetData() {
  std::array<uint8_t, 3> request;  // Length(2) + CMD(1)
  request[0] = 0x00;               // Length_h
  request[1] = 0x03;               // Length_l
  request[2] = GET_DATA_CMD;       // CMD

  {
    boost::system::error_code ec;
    socket_.write_some(boost::asio::buffer(request), ec);
    if (ec) {
      std::cerr << "Error sending request: " << ec.message() << std::endl;
      return DeltoReceivedData{};
    }
  }

  std::array<uint8_t, 99> response;
  if (!ReadFullPacket(socket_, response)) {
    std::cerr << "Failed to read full packet" << std::endl;
    return DeltoReceivedData{};
  }
  
  uint16_t length = CombineMsg(response[0], response[1]);
  uint8_t cmd = response[2];

  if (cmd != request[2] || (int) response.size() != expected_response_length_) {
    // std::cout << "Invalid header (CMD or LENGTH mismatch) "
    //           << static_cast<int>(cmd) << " " << static_cast<int>(request[2])
    //           << " " << static_cast<int>(requset[2]) << " "
    //           << static_cast<int>(length) << " test" << std::endl;
    std:: cerr<< "expected_response_length_: " << expected_response_length_ << std::endl;
    std::cerr << "Invalid header (CMD or LENGTH mismatch)" << std::endl;
    std::cerr << "Request bytes: ";
    for (const auto& byte : request) {
      std::cerr << "0x" << std::hex << static_cast<int>(byte) << " ";
    }
    std::cerr << "Response bytes: ";
    for (const auto& byte : response) {
      std::cerr << "0x" << std::hex << static_cast<int>(byte) << " ";
    }
    // std::cerr << std::dec << std::endl;
    std::cerr << "Received header: CMD = 0x" << std::hex
              << static_cast<int>(cmd) << ", Length = 0x"
              << static_cast<int>(length) << std::dec << std::endl;
    return DeltoReceivedData{};
  }

  // 모터 데이터 파싱
  DeltoReceivedData received_data;
  received_data.joint.resize(motor_count_);
  received_data.current.resize(motor_count_);

  if (model_ == 0x3F02 || model_ == 0x4F02 || model_ == 0x5F12 ||
      model_ == 0x5F22) {
    received_data.velocity.resize(motor_count_);
    received_data.temperature.resize(motor_count_);
  }

  // 5바이트
  for (size_t i = 0; i < motor_count_; i++) {
    size_t base = HEADER_SIZE + i * byte_per_motor_;
    uint8_t motor_id = response[base];  // ID

    uint8_t posL = response[base + 1];
    uint8_t posH = response[base + 2];

    uint8_t curL = response[base + 3];
    uint8_t curH = response[base + 4];

    int16_t raw_position = CombineMsg(posL, posH);
    int16_t raw_current = CombineMsg(curL, curH);

    // 위치는 rad로 환산 (조건에 따라 스케일 변경)
    double position_rad = raw_position * POSITION_SCALE;

    received_data.joint[i] = position_rad;
    received_data.current[i] = raw_current * CURRENT_SCALE;

    // std::cout << static_cast<int>(model_) << std::endl;
    if (model_ == 0x3F02 || model_ == 0x4F02 || model_ == 0x5F12) {
      uint8_t tempL = response[base + 5];
      uint8_t tempH = response[base + 6];
      int16_t raw_temperature = CombineMsg(tempL, tempH);
      received_data.temperature[i] = raw_temperature * 0.1;

      int8_t raw_vel = convertByte(response[base + 7]);
      received_data.velocity[i] = static_cast<double>(raw_vel) * VELOCITY_SCALE * -1;
      // std::cout << "raw_vel: " << static_cast<int>(raw_vel)  << std::endl;
    }
  }

  return received_data;
}

void Communication::SendDuty(std::vector<int>& duty) {
  std::vector<uint8_t> tcp_data_send;

  uint16_t total_packet_size =
      3 +                // Length(2) + CMD(1) +
      motor_count_ * 3;  // motor_count_ *(ID(1) + Duty(2))

  tcp_data_send.resize(total_packet_size);

  tcp_data_send[0] = (total_packet_size >> 8) & 0xFF;  // Length_h
  tcp_data_send[1] = (total_packet_size) & 0xFF;       // Length_l
  tcp_data_send[2] = SET_DUTY_CMD;                     // CMD

  for (size_t i = 0; i < motor_count_; ++i) {
    tcp_data_send[3 + i * 3] = i + 1;                  // ID
    tcp_data_send[4 + i * 3] = (duty[i] >> 8) & 0xFF;  // 상위바이트
    tcp_data_send[5 + i * 3] = (duty[i]) & 0xFF;       // 하위바이트
  }

  boost::system::error_code ec;

  socket_.write_some(boost::asio::buffer(tcp_data_send), ec);

  if (ec) {
    std::cerr << "Error sending request: " << ec.message() << std::endl;
  }
}

int16_t Communication::CombineMsg(uint8_t data1, uint8_t data2) {
  int16_t combined = static_cast<int16_t>((data1 << 8) | (data2));

  if (combined >= 0x8000) {
    combined -= 0x10000;
  }

  return combined;
}

int8_t Communication::convertByte(uint8_t byte) {
  if (byte >= 0x80) {
    return static_cast<int8_t>(byte - 0x100);
  }
  return static_cast<int8_t>(byte);
}
}  // namespace DeltoTCP
