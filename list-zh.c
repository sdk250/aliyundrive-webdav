#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include "cJSON.h"
#include <netdb.h> //gethostbyname()
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#define ERRMSG "\33[1;31mError: \33[0m"
#define ACCESS_API "auth.aliyundrive.com"
#define GET_INFO_API "api.aliyundrive.com"
#define REFERER "https://www.aliyundrive.com/"
#define CONNECTION "Close"
#define CONTENT_TYPE "application/json; charset: UTF-8"
#define TOKEN ""
#define USER_AGENT "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/101.0.4951.48 Safari/537.36"
#define RESPONSE_SIZE 5000
#define PORT 80
#define BUFSIZE 8192
#define LIMIT "50"
#define TIPS "\33[1m请输入文件夹/文件的索引(索引从0开始，输入-1回到根目录。输入q或任意字母退出软件)\33[0m\33[1;36m->\33[0m "

unsigned long int fn(const char *restrict, const int, const char *restrict, const char *restrict);
char *find_key(const char *restrict, const char *restrict);
cJSON *into(const char *restrict, const char *restrict, const char *restrict, cJSON *restrict);
void show_file_list(const cJSON *restrict);
int i, file_count, folder_count;
cJSON *obj[50];

int main(int argc, char **argv) {
	char msg[400];
	char request_body[100] = "{\"grant_type\": \"refresh_token\", \"refresh_token\": ";
	char *response = (char *)malloc(RESPONSE_SIZE * sizeof(char));
	const char *access_token, *nick_name, *default_drive_id;
	char *index = "root";
	char *command = (char *)malloc(sizeof(char) * 1000);
	unsigned long int recvbyte = 0;
	long int recvb;
	int user_input;
	FILE *read_;
	cJSON *json;

	if (argc == 2)
		strcat(strcat(strcat(request_body, "\""), argv[1]), "\"}");
	else
		strcat(request_body, "\""TOKEN"\"}");
	if (argc > 2)
	{
		fprintf(stderr, "%sToo many parameters.\n", ERRMSG);
		exit(EXIT_FAILURE);
	}

	sprintf(msg, "%s %s HTTP/1.1\n"
		"Host: %s\n"
		"Referer: %s\n"
		"User-Agent: %s\n"
		"Connection: %s\n"
		"Content-Type: %s\n"
		"Content-Length: %lu\n"
		"\n"
		"%s",
		"POST", "/v2/account/token",
		ACCESS_API,
		REFERER,
		USER_AGENT,
		CONNECTION,
		CONTENT_TYPE,
		strlen(request_body),
		request_body
	);

	recvbyte = fn(ACCESS_API, PORT, msg, "Data");

	read_ = fopen("Data", "r");
	if ((fread(response, sizeof(char), recvbyte, read_)) != recvbyte)
		fprintf(stderr, "%sRead response fail: %lu bytes.\n", ERRMSG, recvbyte);
	access_token = find_key(response, "\"access_token\":");
	fclose(read_);
	recvbyte = 0;
	memset(response, 0, sizeof(char));

	char msg_[strlen(access_token) + 400];
	strcpy(request_body, "{}");
	sprintf(msg_, "%s %s HTTP/1.1\n"
		"Host: %s\n"
		"Referer: %s\n"
		"User-Agent: %s\n"
		"Connection: %s\n"
		"Authorization: %s\n"
		"Content-Type: %s\n"
		"Content-Length: %lu\n"
		"\n"
		"%s",
		"POST", "/v2/user/get",
		GET_INFO_API,
		REFERER,
		USER_AGENT,
		CONNECTION,
		access_token,
		CONTENT_TYPE,
		strlen(request_body),
		request_body
	);

	recvbyte = fn(GET_INFO_API, PORT, msg_, "Data_");

	read_ = fopen("Data_", "r");
	if ((fread(response, sizeof(char), recvbyte, read_)) != recvbyte)
		fprintf(stderr, "%sRead response fail: %lu bytes.\n", ERRMSG, recvbyte);
	default_drive_id = find_key(response, "\"default_drive_id\":");
	nick_name = find_key(response, "\"nick_name\":");
	printf("\33[1;35m%s\33[0m, Hello!\n", nick_name);
	fclose(read_);
	recvbyte = 0;
	memset(response, 0, sizeof(char));

	json = into(index, default_drive_id, access_token, json);
	show_file_list(json);
	fputs(TIPS, stdout);
	while (scanf("%d", &user_input) == 1 && user_input <= atoi(LIMIT) - 1) {
		if (user_input > i)
			fprintf(stderr, "%s你输入的索引大于当前目录所有文件总数: %d\n", ERRMSG, i);
		if (user_input < 0) {
			json = into("root", default_drive_id, access_token, json);
			show_file_list(json);
			fputs(TIPS, stdout);
			continue;
		}
		if ((strcmp(cJSON_GetObjectItem(obj[user_input], "type")->valuestring, "folder")) == 0) {
			index = cJSON_GetObjectItem(obj[user_input], "file_id")->valuestring;
			json = into(index, default_drive_id, access_token, json);
			show_file_list(json);
		} else if ((strcmp(cJSON_GetObjectItem(obj[user_input], "type")->valuestring, "file")) == 0)
		{
			puts("\33[35m==\t==\t==\t==\33[0m");
			puts(cJSON_GetObjectItem(obj[user_input], "download_url")->valuestring);
			strcpy(command, "am start -n idm.internet.download.manager.plus/idm.internet.download.manager.Downloader -d \"");
			strcat(command, cJSON_GetObjectItem(obj[user_input], "download_url")->valuestring);
			strcat(command, "\"");
			system(command);
			puts("\33[35m==\t==\t==\t==\33[0m");
		}
		fputs(TIPS, stdout);
	}

	free(response);
	free(command);
	cJSON_Delete(json);
	puts("\33[1;32mDone.\33[0m");
	return 0;
}

unsigned long int fn(const char *restrict addr, const int port, const char *restrict msg, const char *restrict filename) {
	int sockfd;
	struct sockaddr_in addr_;
	struct hostent *host;
	char buffer[80000];
	char *s = NULL;
	unsigned long int recvbyte;
	// struct timeval timeout = {3,0};
	long int recvb;
	FILE *fp;

	sockfd = recvbyte = recvb = 0;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		fprintf(stderr, "%sCreate socket-description fail. socket:%d\n", ERRMSG, sockfd);
	printf("Create socket-file-description suceess: %d\n", sockfd);

	// setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
	if (!(host = gethostbyname(addr)))
		fprintf(stderr, "%sGet IP of %s fail.\n", ERRMSG, addr);
	bzero(&addr_, sizeof(struct sockaddr));
	addr_.sin_family = AF_INET;
	addr_.sin_addr = *((struct in_addr *)host->h_addr_list[0]);
	addr_.sin_port = htons(port);

	if ((connect(sockfd, (struct sockaddr *)&addr_, sizeof(struct sockaddr))) < 0)
		fprintf(stderr, "%sConnect to %s of %s fail.\n", ERRMSG, inet_ntoa(addr_.sin_addr), addr);
	printf("Connect to \33[1;46m%s\33[0m of %s success.\n", inet_ntoa(addr_.sin_addr), addr);
	if ((send(sockfd, msg, strlen(msg), MSG_WAITALL)) < 0)
		fprintf(stderr, "%sRequest %s fail.\n", ERRMSG, addr);
	fp = fopen(filename, "w+");
	while ((recvb = recv(sockfd, buffer, 256, MSG_WAITALL)) > 0) {
		recvbyte += recvb;
		if ((fwrite(buffer, sizeof(char), recvb, fp)) != recvb)
			fprintf(stderr, "%sWrite data fail: %lu\n", ERRMSG, recvb);
		/*if (recvb != BUFSIZE)
			break;*/
	}
	// buffer[recvbyte] = '\0';
	/*if ((s = strstr(buffer, "\n\n")) != NULL)
	{
		puts("Okk");
		s += 9;
	}
	fwrite(s, sizeof(char), recvbyte, fp);*/
	fclose(fp);
	close(sockfd);
	return recvbyte;
}

char *find_key(const char *restrict source, const char *restrict key) {
	char *value, *ch;
	if ((value = strstr(value, key))) {
		if ((ch = strchr(value, ',')))
			*ch = '\0';
		if ((value = strchr(value, ':')))
			value += 2;
		if ((ch = strchr(value, '\"')))
			*ch = '\0';
	} else {
		value = find_key(source, "\"message\":");
		fprintf(stderr, "%sGet \"%s\" error: %s\n", ERRMSG, key, value);
		return NULL;
	}
	return value;
}

cJSON *into(const char *restrict index, const char *restrict drive_id, const char *restrict authorization, cJSON *restrict json) {
	char response[180000], request_body[500], msg[strlen(authorization) + 5000];
	char *s;
	long int recvbyte = 0;
	FILE *read;

	sprintf(request_body, "{\"parent_file_id\": \"%s\", \"drive_id\": \"%s\"}", index, drive_id);
	sprintf(msg, "%s %s HTTP/1.1\n"
		"Host: %s\n"
		"Limit: %s\n"
		"Referer: %s\n"
		"User-Agent: %s\n"
		"Connection: %s\n"
		"Authorization: %s\n"
		"Content-Type: %s\n"
		"Content-Length: %lu\n"
		"\n"
		"%s",
		"POST", "/v2/file/list",
		GET_INFO_API,
		LIMIT,
		REFERER,
		USER_AGENT,
		CONNECTION,
		authorization,
		CONTENT_TYPE,
		strlen(request_body),
		request_body
	);

	recvbyte = fn(GET_INFO_API, PORT, msg, "Data__");

	read = fopen("Data__", "r");
	if ((fread(response, sizeof(char), recvbyte, read)) != recvbyte)
		fprintf(stderr, "%sRead response fail: %lu bytes.\n", ERRMSG, recvbyte);
	s = strchr(response, '{');
	/*if ((d = strstr(s, "}\n")))
		if ((d = strchr(d, '\n')))
			*d = '\0';
	if ((d = strchr(s, '\n')))
		*d = '{';
	if ((d = strchr(s, '\n')))
		*d = '}';*/
	/*while ((d = strchr(s, '\n')))
		*d = '\0';*/
	// puts(s);
	json = cJSON_Parse(s);
	fclose(read);
	return json;
}

void show_file_list(const cJSON *json) {
	memset(obj, 0, sizeof(cJSON));
	file_count = folder_count = i = 0;
	for (obj[i] = cJSON_GetArrayItem(cJSON_GetObjectItem(json, "items"), i); obj[i]->next && i < atoi(LIMIT); i++)
		obj[i + 1] = obj[i]->next;
	for (int j = 0; j != i + 1; j++) {
		printf("\33[46m%d\33[0m\33[1;33m%s\33[0m\t--\t->\t\33[1;34m%s\33[0m\n", j, cJSON_GetObjectItem(obj[j], "name")->valuestring, cJSON_GetObjectItem(obj[j], "type")->valuestring);
		if ((strcmp(cJSON_GetObjectItem(obj[j], "type")->valuestring, "folder")) == 0)
			folder_count++;
		if ((strcmp(cJSON_GetObjectItem(obj[j], "type")->valuestring, "file")) == 0)
			file_count++;
	}
	printf("共有\33[1;33m%u 个文件夹\33[0m, 和 \33[1;34m%u 个文件\33[0m.\n", folder_count, file_count);
}