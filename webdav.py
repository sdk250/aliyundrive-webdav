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
		self.files = [[[None], [None]]] * self.limit
		self.referer = "https://www.aliyundrive.com/"
		self.content_type = "application/json; charset=UTF-8"
		self.user_agent = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/101.0.4951.48 Safari/537.36"
		self.user_refresh_token = None
		self.user_authorization = None
		self.user_nickname = None
		self.user_drive_id = None

	def run(self):
		self.user_refresh_token = input("Please enter your \"Refresh Token\" to continue:")
		if self.user_refresh_token == "":
			print("Enter error.")
			quit()
		self.user_authorization = json.loads(self.get_access_token(
			"https://websv.aliyundrive.com/token/refresh",
			self.user_refresh_token
		))["access_token"]
		# print(json.loads(self.user_access_token)["access_token"])
		user_info = json.loads(self.get_user_info(
			"https://api.aliyundrive.com/v2/user/get",
			{},
			self.user_authorization
		))
		self.user_drive_id = user_info["default_drive_id"]
		self.user_nickname = user_info["nick_name"]
		print("\33[1;35m", self.user_nickname, "\33[0m" + ", Hello!")
		file_list = self.get_file_list(
			"https://api.aliyundrive.com/v2/file/list",
			"root",
			self.user_drive_id,
			self.user_authorization
		)
		for i in json.loads(file_list)["items"]:
			if i["type"] == "folder":
				self.folder_count += 1
			elif i["type"] == "file":
				self.file_count += 1
			if (self.folder_count + self.file_count) < self.limit:
				self.files[self.index] = [str(i["file_id"]), str(i["type"])]
				self.index += 1
			else:
				print("\33[31mError: \33[0mOnly support <= 50 files/folders.")
			print("\33[1;33m", i["name"], "\33[0m" + "\t\t--\t->\t" + "\33[1;34m", i["type"], "\33[0m")
		print("Three are", "\33[1;33m", self.folder_count, "folders\33[0m", ", and", "\33[1;34m", self.file_count, "files\33[0m.")
		# into_folder = input("Please enter the name of folder, or index number(start from 0):")
		into_folder = input("\33[1mPlease enter the index number of folder/files(start from 0):\33[0m")




	def get_user_info(self, url, data, authorization):
		if authorization == "":
			return False;
		return requests.post(
			url = url,
			data = json.dumps(data),
			headers = {
				"User-Agent": self.user_agent,
				"Referer": self.referer,
				"Content-Type": self.content_type,
				"Authorization": str(authorization)
			}
		).text

	def get_access_token(self, url, token):
		if token == "":
			return False
		return requests.post(
			url = url,
			data = json.dumps({
				"refresh_token": str(token)
			}),
			headers = {
				"User-Agent": self.user_agent,
				"Referer": self.referer,
				"Content-Type": self.content_type
			}
		).text

	def get_file_list(self, url, index, drive_id, authorization):
		if index == "":
			return False
		return requests.post(
			url = url,
			data = json.dumps({
				"parent_file_id": str(index),
				"drive_id": str(drive_id)
			}),
			headers = {
				"User-Agent": self.user_agent,
				"Referer": self.referer,
				"Content-Type": self.content_type,
				"limit": str(self.limit),
				"Authorization": str(authorization)
			}
		).text

if __name__ == "__main__":
	sdk250 = webdav()
	sdk250.run()