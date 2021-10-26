import cv2
import gi
import threading
from collections import deque
import subprocess

gi.require_version('Gst', '1.0')
gi.require_version('GstRtspServer', '1.0')
from gi.repository import GObject, Gst, GstRtspServer


class GstRtspServerWrapper:

    def __init__(self,
                 rtsp_port=8554,
                 rtsp_mountpoint='/test',
                 udp_port=5400) -> None:

        self.rtsp_port = rtsp_port
        self.rtsp_mountpoint = rtsp_mountpoint
        self.udp_port = udp_port

        self.rtsp_server = GstRtspServer.RTSPServer.new()
        self.rtsp_server.props.service = str(self.rtsp_port)
        self.rtsp_server.attach(None)

        self.rtsp_factory = GstRtspServer.RTSPMediaFactory.new()
        test_launch_pipeline = "( udpsrc name=pay0 port={} caps=\"application/x-rtp, media=video, encoding-name=H264, payload=96 \" )".format(
            self.udp_port)
        self.rtsp_factory.set_launch(test_launch_pipeline)
        self.rtsp_factory.set_shared(True)

        self.rtsp_server.get_mount_points().add_factory(self.rtsp_mountpoint,
                                                        self.rtsp_factory)
        # print('\nTest launch pipeline: ', test_launch_pipeline)

        # start gst mainloop
        self.gst_mainloop = None
        print('\n Launched RTSP Streaming at rtsp://localhost:{}{} \n'.format(
            self.rtsp_port, self.rtsp_mountpoint))

    def run(self):
        self.gst_mainloop = GObject.MainLoop()
        GObject.threads_init()
        Gst.init(None)
        self.gst_mainloop.run()

    def stop(self):
        self.gst_mainloop.quit()


class GstRtspStreamer:

    def __init__(self,
                 rtsp_port=8554,
                 rtsp_mountpoint='/test',
                 udp_port=5400,
                 width=1280,
                 height=720,
                 framerate=30,
                 bitrate=2000000,
                 enable_gst_rtsp=True,
                 buffer_size=50) -> None:
        self.rtsp_port = rtsp_port
        self.rtsp_mountpoint = rtsp_mountpoint
        self.udp_port = udp_port
        self.width = width
        self.height = height
        self.framerate = framerate
        self.bitrate = bitrate
        self.enable_gst_rtsp = enable_gst_rtsp
        self.buffer_size = buffer_size

        # udp writer
        self.udp_writer = cv2.VideoWriter(*self._video_writer_args())
        if not self.udp_writer.isOpened():
            raise RuntimeError('UdpWriter not opened')

        self.output_frame_queue = deque([], maxlen=self.buffer_size)
        self.output_cond = threading.Condition()
        self.output_exit_event = threading.Event()
        self.thread_output = threading.Thread(target=self._writing_frames)

        # gst rtsp server
        if self.enable_gst_rtsp:
            self.rtsp_server_wrapper = GstRtspServerWrapper(
                rtsp_port=rtsp_port,
                rtsp_mountpoint=rtsp_mountpoint,
                udp_port=udp_port)
            self.thread_gst = threading.Thread(
                target=self.rtsp_server_wrapper.run)
        else:
            self.rtsp_server_wrapper, self.thread_gst = None, None

        self.start_writing()

    def release(self):
        self.stop_writing()
        self.udp_writer.release()

    def write(self, frame):
        with self.output_cond:
            self.output_frame_queue.append(frame)
            self.output_cond.notify()

    def start_writing(self):
        """
        Start writing to video file.
        """
        if not self.udp_writer.isOpened():
            self.udp_writer.open(*self._video_writer_args())
        if not self.thread_output.is_alive():
            self.thread_output.start()
        if self.enable_gst_rtsp and not self.thread_gst.is_alive():
            self.thread_gst.start()

    def stop_writing(self):
        """
        Stop writing to video file.
        """
        with self.output_cond:
            self.output_exit_event.set()
            self.output_cond.notify()
        self.output_frame_queue.clear()
        self.thread_output.join()
        if self.enable_gst_rtsp:
            self.rtsp_server_wrapper.stop()
            self.thread_gst.join(timeout=1.)

    def _video_writer_args(self):
        return (self._gst_write_pipeline(), cv2.CAP_GSTREAMER, 0,
                self.framerate, (self.width, self.height), True)

    def _gst_write_pipeline(self):
        gst_elements = str(subprocess.check_output('gst-inspect-1.0'))
        # use hardware encoder if found
        if 'omxh264enc' in gst_elements:
            encoder = 'omxh264enc bitrate={} insert-sps-pps=true profile=high'.format(
                self.bitrate)
        elif 'x264enc' in gst_elements:
            encoder = 'x264enc bitrate={} zero-latency=true'.format(
                self.bitrate)
        else:
            raise RuntimeError('GStreamer H.264 encoder not found')

        writer_pipeline = 'appsrc do-timestamp=true min-latency=0 max-latency=0 max-bytes=1000 is-live=true ! ' +\
            'videoconvert ! ' +\
            'nvvidconv ! video/x-raw(memory:NVMM),format=I420,width={},height={},framerate={}/1 ! '.format(self.width, self.height, self.framerate) +\
            '{} ! h264parse ! video/x-h264,stream-format=byte-stream ! rtph264pay name=pay0 pt=96 ! '.format(encoder) +\
            'udpsink host=127.0.0.1 port={} async=false sync=false'.format(self.udp_port)
        return writer_pipeline

    def _write(self, frame):
        """
        Writes the next video frame.
        """
        self.udp_writer.write(cv2.resize(frame, (self.width, self.height)))

    def _writing_frames(self):
        while not self.output_exit_event.is_set():
            with self.output_cond:
                while len(self.output_frame_queue
                         ) == 0 and not self.output_exit_event.is_set():
                    self.output_cond.wait()
                if len(self.output_frame_queue
                      ) == 0 and self.output_exit_event.is_set():
                    continue
                frame = self.output_frame_queue.popleft()
                self._write(frame)
                self.output_cond.notify()