# -*- coding: UTF-8 -*-
# Version 2.0
import requests
import json
import os

class webdav(object):
	def __init__(self):
		# print("Initial")
		self.folder_count = 0
		self.file_count = 0
		self.index = 0
		self.limit = 50
		self.files = [[[None], [None], [None], [None]]] * self.limit # id + type + name + url
		self.inputi = None
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
			"http://auth.aliyundrive.com/v2/account/token",
			self.user_refresh_token
		))["access_token"]
		
		user_info = json.loads(self.get_user_info(
			"http://api.aliyundrive.com/v2/user/get",
			{},
			self.user_authorization
		))
		self.user_drive_id = user_info["default_drive_id"]
		self.user_nickname = user_info["nick_name"]
		print("\33[1;35m", self.user_nickname, "\33[0m" + ", Hello!")
		file_list = self.get_file_list(
			"http://api.aliyundrive.com/v2/file/list",
			"root",
			self.user_drive_id,
			self.user_authorization
		)
		self.show_file_list(file_list)
		# into_folder = input("Please enter the name of folder, or index number(start from 0):")
		# into_folder_index = input("\33[1mPlease enter the index number of folder/files(start from 0):\33[0m")
		self.get_user_input()
		while self.inputi <= 50:
			if self.inputi > (self.index - 1):
				print("\33[1;31mError: \33[0mMemory overflow.")
				break
			if self.inputi < 0:
				into_folder = self.get_file_list(
					"https://api.aliyundrive.com/v2/file/list",
					"root",
					self.user_drive_id,
					self.user_authorization
				)
				self.show_file_list(into_folder)
				self.get_user_input()
				continue
			if self.files[self.inputi][1] == "folder":
				into_folder = self.get_file_list(
					"https://api.aliyundrive.com/v2/file/list",
					self.files[self.inputi][0],
					self.user_drive_id,
					self.user_authorization
				)
				self.show_file_list(into_folder)
			elif self.files[self.inputi][1] == "file":
				print("File name:", self.files[self.inputi][2])
				print("Download url:", self.files[self.inputi][3])
				os.system("am start -n idm.internet.download.manager.plus/idm.internet.download.manager.Downloader -d " + "\"" + str(self.files[self.inputi][3]) + "\"")
			self.get_user_input()
		print("Done.")

	def get_user_input(self):
		try:
			self.inputi = int(input("\33[1mPlease enter the index number of folder/files(start from 0. enter -1 to return root):\33[0m"))
		except:
			print("Bye~")
			quit()

	def show_file_list(self, folder_id):
		self.folder_count = self.file_count = self.index = 0
		self.files = [[[None], [None]]] * self.limit
		for i in json.loads(folder_id)["items"]:
			if i["type"] == "folder":
				self.folder_count += 1
			elif i["type"] == "file":
				self.file_count += 1
			if (self.folder_count + self.file_count) < self.limit:
				if i["type"] == "folder":
					self.files[self.index] = [str(i["file_id"]), str(i["type"]), str(i["name"]), None]
				else:
					self.files[self.index] = [str(i["file_id"]), str(i["type"]), str(i["name"]), str(i["download_url"])]
				self.index += 1
			else:
				print("\33[31mError: \33[0mOnly support <= 50 files/folders.")
			print("\33[1;33m", i["name"], "\33[0m" + "\t\t--\t->\t" + "\33[1;34m", i["type"], "\33[0m")
		print("Three are", "\33[1;33m", self.folder_count, "folders\33[0m", ", and", "\33[1;34m", self.file_count, "files\33[0m.")

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
				"grant_type": "refresh_token",
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
