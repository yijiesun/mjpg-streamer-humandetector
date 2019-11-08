/usr/local/bin/mjpg_streamer -i "/usr/local/lib/mjpg-streamer/input_opencv.so -f 30  -r 640x480" -o "/usr/local/lib/mjpg-streamer/output_http.so -p 8080 -w /usr/local/share/mjpg-streamer/www"

/usr/local/bin/mjpg_streamer -i "/usr/local/lib/mjpg-streamer/input_uvc.so -f 30 -r 640x480" -o "/usr/local/lib/mjpg-streamer/output_http.so -p 8080 -w /usr/local/share/mjpg-streamer/www"