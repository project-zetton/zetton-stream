#include "zetton_stream/base/stream_uri.h"

#include <stdio.h>

#include <algorithm>
#include <clocale>
#include <iostream>

#include "zetton_common/util/filesystem.h"
#include "zetton_common/util/log.h"
#include "zetton_common/util/string.h"

namespace zetton {
namespace stream {

const char* StreamProtocolTypeToStr(StreamProtocolType type) {
  switch (type) {
    case StreamProtocolType::PROTOCOL_DEFAULT:
      return "default";
    case StreamProtocolType::PROTOCOL_V4L2:
      return "v4l2";
    case StreamProtocolType::PROTOCOL_CSI:
      return "csi";
    case StreamProtocolType::PROTOCOL_RTP:
      return "rtp";
    case StreamProtocolType::PROTOCOL_RTSP:
      return "rtsp";
    case StreamProtocolType::PROTOCOL_FILE:
      return "file";
    case StreamProtocolType::PROTOCOL_DISPLAY:
      return "display";
    default:
      return "default";
  }
}

StreamProtocolType StreamProtocolTypeFromStr(const char* str) {
  if (!str) return StreamProtocolType::PROTOCOL_DEFAULT;
  for (int n = 0; n < static_cast<int>(StreamProtocolType::PROTOCOL_MAX_NUM);
       ++n) {
    const auto value = (StreamProtocolType)n;
    if (strcasecmp(str, StreamProtocolTypeToStr(value)) == 0) return value;
  }
  return StreamProtocolType::PROTOCOL_DEFAULT;
}

// constructor
StreamUri::StreamUri() { port = -1; }

// constructor
StreamUri::StreamUri(const char* uri) { Parse(uri); }

// constructor
StreamUri::StreamUri(const std::string& uri) { Parse(uri.c_str()); }

// Parse
bool StreamUri::Parse(const char* uri) {
  if (!uri) return false;

  string = uri;
  extension = "";
  location = "";
  port = -1;

  // look for protocol
  std::size_t pos = string.find("://");
  std::string protocol_string = "";

  if (pos != std::string::npos) {
    protocol_string = string.substr(0, pos);
    location = string.substr(pos + 3, std::string::npos);
  } else {
    // check for some formats without specified protocol

    pos = string.find("/dev/video");

    if (pos == 0) {
      protocol_string = "v4l2";
    } else if (string.find(".") != std::string::npos ||
               string.find("/") != std::string::npos ||
               common::FileExists(string.c_str())) {
      protocol_string = "file";
    } else if (sscanf(string.c_str(), "%i", &port) == 1) {
      protocol_string = "csi";
    } else if (string == "display") {
      protocol_string = "display";
    } else if (string == "appsrc") {
      protocol_string = "appsrc";
    } else if (string == "appsink") {
      protocol_string = "appsink";
    } else {
      AERROR_F("Invalid resource or file path:  {}", string);
      return false;
    }

    location = string;

    // reconstruct full URI string
    string = protocol_string + "://";

    if (protocol_string == "file")
      string +=
          common::GetAbsolutePath(location);  // URI paths should be absolute
    else
      string += location;
  }

  // protocol should be all in lower case for easier parsing
  protocol_string = common::ToLower(protocol_string);

  // parse extra info (device ordinals, IP addresses, ect)
  if (protocol_string == "v4l2") {
    if (sscanf(location.c_str(), "/dev/video%i", &port) != 1) {
      AERROR_F("Failed to parse V4L2 device ID from {}", location);
      return false;
    }
  } else if (protocol_string == "csi") {
    if (sscanf(location.c_str(), "%i", &port) != 1) {
      AERROR_F("Failed to parse MIPI CSI device ID from {}", location);
      return false;
    }
  } else if (protocol_string == "display") {
    if (sscanf(location.c_str(), "%i", &port) != 1) {
      AINFO_F("Using default display device 0");
      port = 0;
    }
  } else if (protocol_string == "file") {
    extension = common::GetFileExtension(location);
  } else {
    // search for ip/port format
    std::string port_str;
    pos = location.find(":");

    if (pos != std::string::npos) {
      // parse uri like "xxx.xxx.xxx.xxx:port"
      if (protocol_string == "rtsp") {
        // parse uri like "user:pass@ip:port"
        const std::size_t host_pos = location.find("@", pos + 1);
        const std::size_t port_pos = location.find(":", pos + 1);
        if (host_pos != std::string::npos && port_pos != std::string::npos)
          pos = port_pos;
      }
      port_str = location.substr(pos + 1, std::string::npos);
      location = location.substr(0, pos);
    } else if (std::count(location.begin(), location.end(), '.') == 0) {
      // parse uri like "port"
      port_str = location;
      location = "127.0.0.1";
    }

    // parse the port number
    if (port_str.size() != 0) {
      // parse mountpoint
      pos = port_str.find("/");
      if (pos != std::string::npos) {
        mountpoint = port_str.substr(pos, std::string::npos);
        port_str = port_str.substr(0, pos);
      }
      if (sscanf(port_str.c_str(), "%i", &port) != 1) {
        if (protocol_string == "rtsp") {
          AWARN_F("Missing/invalid IP port from {}, default to port 554",
                  string);
          port = 554;
        } else {
          AERROR_F("Failed to parse IP port from {}", string);
          return false;
        }
      }
    }

    // convert "@:port" format to localhost
    if (location == "@") location = "127.0.0.1";
  }

  protocol = StreamProtocolTypeFromStr(protocol_string.c_str());

  return true;
}

void StreamUri::Print(const char* prefix) const {
  if (!prefix) prefix = "";

  AINFO_F("{}-- URI: {}", prefix, string);
  AINFO_F("{}   - protocol:  {}", prefix, StreamProtocolTypeToStr(protocol));
  AINFO_F("{}   - location:  {}", prefix, location);
  if (extension.size() > 0) {
    AINFO_F("{}   - extension: {}\n", prefix, extension);
  }
  if (port > 0) {
    AINFO_F("{}   - port:      {}\n", prefix, port);
  }
}
}  // namespace stream
}  // namespace zetton
