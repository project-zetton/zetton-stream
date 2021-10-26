#!/usr/bin/env python3

import cv2

from zetton_stream.stream import GstRtspStreamer


def main():
    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        raise RuntimeError('Failed to open camera')
    writer = GstRtspStreamer()

    try:
        while cap.isOpened():
            ret, frame = cap.read()
            if ret and frame is not None:
                writer.write(frame)
    except KeyboardInterrupt as e:
        print('Pressed Ctrl-C')
    finally:
        cap.release()
        writer.release()


if __name__ == '__main__':
    main()