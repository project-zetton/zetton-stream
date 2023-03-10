# zetton-stream

English | [中文](README_zh-CN.md)

## Table of Contents

- [zetton-stream](#zetton-stream)
  - [Table of Contents](#table-of-contents)
  - [Introduction](#introduction)
  - [What's New](#whats-new)
  - [Installation](#installation)
  - [Getting Started](#getting-started)
  - [Overview of Supported Sources and Sinks](#overview-of-supported-sources-and-sinks)
  - [FAQ](#faq)
  - [Contributing](#contributing)
  - [Acknowledgement](#acknowledgement)
  - [License](#license)
  - [Related Projects](#related-projects)

## Introduction

zetton-stream is an open source package for video streaming. It's a part of the [Project Zetton](https://github.com/project-zetton).

<details open>
<summary>Major features</summary>

- **Modular Design**: zetton-stream is designed to be modular, which means that you can easily add new stream source and sinks to the package.

- **Support Multiple Frameworks**: zetton-stream supports multiple video processing and streaming frameworks, such as OpenCV, GStreamer, FFmpeg, etc.

- **High Efficiency**: zetton-stream is designed to be high efficient, which means that you can easily deploy the streaming nodes to CPU/GPU servers or embedded devices.

</details>

## What's New

Please refer to [changelog.md](docs/en/changelog.md) for details and release history.

For compatibility changes between different versions of zetton-stream, please refer to [compatibility.md](docs/en/compatibility.md).

## Installation

Please refer to [Installation](docs/en/get_started.md) for installation instructions.

## Getting Started

Please see [get_started.md](docs/en/get_started.md) for the basic usage of zetton-stream.

## Overview of Supported Sources and Sinks

|  Task  | Protocol | Format | Encoding | V4L2 | GStreamer | FFmpeg |
| :----: | :------: | :----: | :------: | ---- | --------- | ------ |
| Source |   V4L2   | MJPEG  |   JPEG   | ✅    | ❌         | ❌      |
| Source |   V4L2   |  Raw   |    /     | ✅    | ❌         | ❌      |
| Source |   RTSP   |   /    |  H.264   | /    | ✅         | ❌      |
| Source |   RTMP   |   /    |  H.264   | /    | ❌         | ❌      |
| Source |   RTP    |   /    |  H.264   | /    | ✅         | ❌      |
|  Sink  |   RTSP   |   /    |  H.264   | /    | ✅         | ❌      |
|  Sink  |   RTMP   |   /    |  H.264   | /    | ❌         | ❌      |
|  Sink  |   RTP    |   /    |  H.264   | /    | ✅         | ❌      |
|  Sink  |    /     |  MP4   |  H.264   | /    | ❌         | ❌      |

- ✅: Supported and tested
- ❓: Supported but not tested
- ❌: Not supported yet

## FAQ

Please refer to [FAQ](docs/en/faq.md) for frequently asked questions.

## Contributing

We appreciate all contributions to improve zetton-stream. Please refer to [CONTRIBUTING.md](.github/CONTRIBUTING.md) for the contributing guideline.

## Acknowledgement

We appreciate all the contributors who implement their methods or add new features, as well as users who give valuable feedbacks.
We wish that the package and benchmark could serve the growing research and production community by providing a flexible toolkit to stream videos.

- [v4l-utils](https://git.linuxtv.org/v4l-utils.git) for convience wrappers of V4L2 API

## License

- For academic use, this project is licensed under the 2-clause BSD License, please see the [LICENSE file](LICENSE) for details.
- For commercial use, please contact [Yusu Pan](mailto:xxdsox@gmail.com).

## Related Projects

- [zetton-stream-gst](https://github.com/project-zetton/zetton-stream-gst): GStreamer-based streaming extension for zetton-stream package

- [zetton-ros-vendor](https://github.com/project-zetton/zetton-ros-vendor): ROS-based examples
