#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <unistd.h>

#include <fcntl.h>

#include <sys/inotify.h>

#include <string>
#include <map>
#include <vector>
#include <filesystem>
#include <regex>

namespace fs = std::filesystem;

using namespace std;

class Rule
{
public:
	Rule(regex reg, fs::path dir)
	{
		this->reg = reg;
		this->dir = dir;
	}
	regex reg;
	fs::path dir;
};

vector<string> split(string text, char delim)
{
	vector<string> cont = vector<string>();
	std::size_t current, previous = 0;
	current = text.find(delim);
	while (current != std::string::npos)
	{
		cont.push_back(text.substr(previous, current - previous));
		previous = current + 1;
		current = text.find(delim, previous);
	}
	cont.push_back(text.substr(previous, current - previous));
	return cont;
}

vector<string> lines(string text)
{
	return split(text, '\n');
}

map<fs::path, vector<Rule>> read_config()
{
	map<fs::path, vector<Rule>> result = map<fs::path, vector<Rule>>();
	fs::path configfile = fs::path(getenv("HOME"));
	configfile /= ".config";
	configfile /= "filemover";
	configfile /= "filemoverrc";

	if (fs::exists(configfile))
	{
		ifstream input_file(configfile);
		string content = string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
		vector<string> content_lines = lines(content);
		string dir = "";
		for (string line : content_lines)
		{
			if (line.size() == 0 || line[0] == '#')
				continue;
			else if (line[0] == '[')
			{
				dir = line.substr(1, line.size() - 2);
			}
			else if (!dir.empty())
			{
				vector<string> rule_parts = split(line, '=');
				if (rule_parts.size() != 2)
					continue;
				regex reg = regex(rule_parts[0]);
				fs::path move_dir = fs::path(rule_parts[1]);
				Rule rule = Rule(reg, move_dir);
				fs::path check_dir = fs::path(dir);
				if (result.count(check_dir) == 0)
				{
					result[check_dir] = vector<Rule>();
				}
				result[check_dir].push_back(rule);
			}
		}
	}
	else
	{
		cout << "No config file found\n";
		exit(1);
	}

	return result;
}

void move_to_dir(fs::path path, fs::path dir)
{
	dir /= path.filename();
	try{
		fs::rename(path, dir);
	}
	catch(fs::filesystem_error){
		fs::copy(path, dir);
		fs::remove(path);
	}
}

void check_file(vector<Rule> rules, fs::path file)
{
	string name = file.filename();
	for (auto rule : rules)
	{
		if (std::regex_match(name.begin(), name.end(), rule.reg))
		{
			move_to_dir(file, rule.dir);
			return;
		}
	}
}

#define NAME_MAX 128
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + NAME_MAX + 1))
void listen_rules(map<fs::path, vector<Rule>> dirs)
{
	map<int, fs::path> descriptors = map<int, fs::path>();
	char buffer[BUF_LEN];
	int fd = inotify_init();
	for (auto dir_pair : dirs)
	{
		string dir = dir_pair.first.u8string();
		int wd = inotify_add_watch(fd, dir.c_str(), IN_CREATE | IN_MOVED_TO);
		descriptors[wd] = dir_pair.first;
	}

	while (1)
	{
		int i = 0;
		int length = read(fd, buffer, BUF_LEN);

		if (length < 0)
		{
			perror("read");
		}

		while (i < length)
		{
			struct inotify_event *event =
				(struct inotify_event *)&buffer[i];
			if (event->len)
			{
				fs::path dir = descriptors[event->wd];
				fs::path file = dir;
				file /= event->name;
				check_file(dirs[dir], file);
			}
			i += EVENT_SIZE + event->len;
		}
	}
}

void run_rules(map<fs::path, vector<Rule>> dirs){
	for (auto pair : dirs){
		vector<Rule> rules = dirs[pair.first];
		for (const auto & entry : fs::directory_iterator(pair.first)){
			fs::path file = entry.path();
			check_file(rules, file);
		}
	}
}

int main(int argc, char *argv[])
{
	map<fs::path, vector<Rule>> rules = read_config();

	if (argc > 1 && string(argv[1]) == "--watch")listen_rules(rules);
	else run_rules(rules);
}
