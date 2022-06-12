# -*- coding: UTF-8 -*-
import requests
import json

class webdav(object):
	def __init__(self):
		# print("Initial")
		self.folder_count = 0
		self.file_count = 0
		self.index = 0
		self.limit = 50
		self.files = [[[None], [None]]] * limit
		self.referer = "https://www.aliyundrive.com/"
		self.content_type = "application/json; charset=UTF-8"
		self.user_agent = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/101.0.4951.48 Safari/537.36"
		self.user_refresh_token = None
		self.user_authorization = None
		self.user_drive_id = None

	def get_user_info(self, url, data):
		if !auth():
			return false;
		return requests.post(
			url = url,
			data = json.dumps(data),
			headers = {
				"User-Agent": self.user_agent,
				"Referer": self.referer,
				"Content-Type": self.content_type,
				"Authorization": self.user_authorization
			}
		)

	# def auth


	def run(self):
		print("Run")

if __name__ == "__main__":
	sdk250 = webdav()
	sdk250.run()