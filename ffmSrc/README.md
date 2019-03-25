{
	"video_stream_center_common": {
		"log_error_format": "./log/video_stream_error",
		"log_info_format": "./log/video_stream_info"
	},
	"video_stream_center": [
		{
			"video_stream_center_name": "JinNiuBa",
			"out_video_stream_url": {
				"out_url_ip": "udp://127.0.0.1",
				"out_url_port": "30010"
			},
			"out_video_stream_file": {
				"video_path": "/DATACENTER1/WJ/video_cache",
				"write_time": 2,
				"filename_format": "YYYYMMDDTHHMMSS",
				"filename_prefix": "JinNiuBa_",
				"video_format": ".264"
			},
			"in_video_stream": {
				"in_url": "/DATACENTER1/WJ/video/A103.mp4",
				"mat_height": 720,
				"mat_width": 1080
			}
		}
	]
}

rtsp://admin:swjtu9422@192.168.9.37:554