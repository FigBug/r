#include "mainheaders.h"
#include "r.h"
#include "url.h"
#include "json.h"
#include "ultragetopt.h"
#include "tinyxml.h"

#ifdef _WIN32
#define DIR_SEP "\\"
#endif

#ifdef __APPLE__
#include <CoreFoundation/CFBundle.h>
#include <ApplicationServices/ApplicationServices.h>
#define DIR_SEP "/"
#endif

int gTotal = 0;

string toStr(int v)
{
    std::stringstream ss;
    ss << v;
    return ss.str();
}

json_value* toJson(string json, block_allocator& allocator)
{
	char* errorPos  = 0;
	char* errorDesc = 0;

	int errorLine   = 0;

	char* source = new char[json.size() + 1];        
	strcpy(source, json.c_str());
	json_value* root = json_parse(source, &errorPos, &errorDesc, &errorLine, &allocator);
	//delete source;

	return root;
}

void openUrl(string url)
{
#ifdef _WIN32
	wstring wurl(url.begin(), url.end());
	ShellExecute(NULL, L"open", wurl.c_str(), NULL, NULL, SW_SHOWDEFAULT);
#endif
#ifdef __APPLE__
  	CFURLRef urlref = CFURLCreateWithBytes(NULL, (UInt8*)url.c_str(), url.length(), kCFStringEncodingASCII, NULL);
  	int cnt = 0;
	while (LSOpenCFURLRef(urlref,0) != 0)
	{
		usleep(10000);
		cnt++;
		if (cnt > 100)
			break;
	}
	CFRelease(urlref);
#endif
}

string getDataDir()
{
#ifdef _WIN32
	wchar_t path[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path);

	wcscat(path, L"\\r");

	CreateDirectory(path, NULL);

	wstring tmp(path);

	return string(tmp.begin(), tmp.end());
#endif
#ifdef __APPLE__
	char path[1000];

	strcpy(path, "~/.r");

	wordexp_t exp_result;
	wordexp(path, &exp_result, 0);
	strcpy(path, exp_result.we_wordv[0]);
	wordfree(&exp_result); 

	mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	return path;
#endif
}

void loadHistory(string subreddit, vector<HistoryItem>& history)
{
	string dataDir = getDataDir();
	string xmlFile = dataDir + DIR_SEP + subreddit;

	TiXmlDocument doc(xmlFile.c_str());
	if (doc.LoadFile())
	{
		TiXmlElement* root = doc.RootElement();
		if (root)
		{
			TiXmlElement* historyNode = root->FirstChildElement("history");
			while (historyNode)
			{
				HistoryItem itm;

				itm.id   = historyNode->Attribute("id");
				itm.date = atoi(historyNode->Attribute("date"));

				history.push_back(itm);
				
				historyNode = historyNode->NextSiblingElement("history");
			}
		}
	}
}

void saveHistory(string subreddit, vector<HistoryItem>& history)
{
	string dataDir = getDataDir();
	string xmlFile = dataDir + DIR_SEP + subreddit;

	TiXmlDocument doc(xmlFile.c_str());
	
	TiXmlElement root("histories");

	for (int i = 0; i < (int)history.size(); i++)
	{
		TiXmlElement historyNode("history");

		historyNode.SetAttribute("id", history[i].id.c_str());
		historyNode.SetAttribute("date", (int)history[i].date);

		root.InsertEndChild(historyNode);
	}
	doc.InsertEndChild(root);

	doc.SaveFile();
}

bool isInHistory(vector<HistoryItem>& history, string id)
{
	for (int i = 0; i < (int)history.size(); i++)
	{
		if (history[i].id == id)
			return true;
	}
	return false;
}

bool hasEnding(std::string const &fullString, std::string const &ending)
{
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}


void handleStory(const Options& options, vector<HistoryItem>& history, json_value* story)
{
	string id;
	string title;
	string selftext;
	int score;
	bool over_18;
	string thumbnail;
	string url;
	double created_utc;
	string permalink;

	for (json_value* it = story->first_child; it; it = it->next_sibling)
	{
		if (!strcmp(it->name, "id")) 
			id = it->string_value;
		if (!strcmp(it->name, "title")) 
			title = it->string_value;
		if (!strcmp(it->name, "selftext")) 
			selftext = it->string_value;
		if (!strcmp(it->name, "score")) 
			score = it->int_value;
		if (!strcmp(it->name, "over_18")) 
			over_18 = it->int_value ? true : false;
		if (!strcmp(it->name, "thumbnail")) 
			thumbnail = it->string_value;
		if (!strcmp(it->name, "url")) 
			url = it->string_value;
		if (!strcmp(it->name, "created_utc")) 
			created_utc = it->float_value;
		if (!strcmp(it->name, "permalink")) 
			permalink = it->string_value;
	}

	bool inHistory = isInHistory(history, id);

	if (inHistory && !options.openAll)
		return;

	if (options.imageOnly)
	{
		bool image = false;
		
		if (url.find("imgur.com") != string::npos)	image = true;
		if (url.find("gfycat.com") != string::npos)	image = true;

		if (hasEnding(url, ".gif")) image = true;
		if (hasEnding(url, ".jpg")) image = true;
		if (hasEnding(url, ".png")) image = true;
		
		if (!image)
			return;
	}

	if (options.openPermalink)
		openUrl("http://reddit.com" + permalink);

	if (options.openLink)
		openUrl(url);	

	if (!inHistory)
	{
		HistoryItem hi;
		hi.id   = id;
		hi.date = time(NULL);
		history.push_back(hi);
	}
	gTotal++;
}

bool readSubreddits(const Options& options)
{
	for (int i = 0; i < (int)options.subreddits.size(); i++)
	{
		if (gTotal >= options.maxToOpen)
			break;

		// open subreddit
		if (options.openSubreddit)
			openUrl("http://www.reddit.com/r/" + options.subreddits[i]);

		vector<HistoryItem> history;
		loadHistory(options.subreddits[i], history);

		if (options.clearHistory)
			history.clear();

		// continue if we don't actual need to fetch the json
		if (options.openLink || options.openPermalink)
		{
			// fetch the url from the server
			string server = string("www.reddit.com");
			string path   = string("/r/") + options.subreddits[i] + string("/.json?limit=") + toStr(options.numToOpen);

			string page = fetchUrl(server, path);

			if (page.size() > 0)
			{
				// convert to json
				block_allocator allocator(1 << 10);
				json_value* root = toJson(page, allocator);
				if (root)
				{
					// find the data we want. this is gross.
					for (json_value* it = root->first_child; it; it = it->next_sibling)
					{
						if (!strcmp(it->name, "data") && it->type == JSON_OBJECT)
						{
							json_value* data = it;
							for (json_value* it = data->first_child; it; it = it->next_sibling)
							{
								if (!strcmp(it->name, "children") && it->type == JSON_ARRAY)
								{
									json_value* child = it;
									for (json_value* it = child->first_child; it; it = it->next_sibling)
									{
										json_value* rec = it;
										for (json_value* it = rec->first_child; it; it = it->next_sibling)
										{
											if (!strcmp(it->name, "data"))
												handleStory(options, history, it);
											if (gTotal >= options.maxToOpen)
												goto out;
										}						
									}
								}
							}
						}
					}
				}
			}
		}
out:
		if (history.size() || options.clearHistory)
			saveHistory(options.subreddits[i], history);
	}

	return true;
}

void printUsage()
{
	printf("r: Copyright 2016 Roland Rabien\n");
	printf("  usage: r [-lpiocadv] [-n max] subreddit\n");
	printf("  -l Open links (default)\n");
	printf("  -p Open permalinks\n");
	printf("  -i Open image links only\n");
	printf("  -o Open subreddit\n");
	printf("  -c Clear history\n");
	printf("  -a Open all (including previously opened)\n");
	printf("  -n Maximum number of links to open per subreddit (default 30)\n");
	printf("  -x Maximum number of links to open in total (default 50)\n");
	printf("  -h Display help\n");
	printf("  -v Display version\n");
}

Options parseOptions(int argc, char* argv[])
{
	Options options;

	ultraopterr = 0;

	int c;	
	while ((c = ultragetopt (argc, argv, "lpiocan:x:hv")) != -1)
	{
		switch (c)
		{
			case 'l': options.openLink          	= true; break;
			case 'p': options.openPermalink     	= true; break;
			case 'i': options.imageOnly		= true; break;
			case 'o': options.openSubreddit     	= true; break;
			case 'c': options.clearHistory      	= true; break;
			case 'a': options.openAll		= true; break;
			case 'n': options.numToOpen		= atoi(ultraoptarg); break;
			case 'x': options.maxToOpen		= atoi(ultraoptarg); break;
			case 'h': options.displayHelp		= true; break;
			case 'v': options.displayVersion	= true; break;
			case '?':
				if (ultraoptopt == 'c')
					fprintf (stderr, "Option -%c requires an argument.\n", ultraoptopt);
				else if (isprint (ultraoptopt))
					fprintf (stderr, "Unknown option `-%c'.\n", ultraoptopt);
				else
					fprintf (stderr, "Unknown option character `\\x%x'.\n", ultraoptopt);
				break;
		}
	}

	if (!options.openLink		&& 
		!options.openPermalink	&&
		!options.openSubreddit  &&
		!options.clearHistory   &&
		!options.displayHelp	&&
		!options.displayVersion)
		options.openLink = true;

	if (options.numToOpen == 0)
		options.numToOpen = 30;
	if (options.maxToOpen == 0)
		options.maxToOpen = 50;
	if (options.numToOpen > options.maxToOpen)
		options.maxToOpen = options.numToOpen;

	for (int index = ultraoptind; index < argc; index++)
		options.subreddits.push_back(argv[index]);

	return options;
}

bool showVersion()
{
	printf("r v1.0.1 by Roland Rabien (figbug@gmail.com) %s.\n", __DATE__);

	return true;
}

int main(int argc, char* argv[])
{
	int res = EXIT_SUCCESS;

#ifdef __APPLE__
	curl_global_init(CURL_GLOBAL_ALL);
#endif

	if (argc == 0)
	{
		printUsage();
		return res;
	}

	Options options = parseOptions(argc, argv);

	if (options.displayVersion)
	{
		showVersion();
		return res;
	}

	if (options.displayHelp || options.subreddits.size() == 0)
	{
		printUsage();
		return res;
	}

	readSubreddits(options);

#ifdef __APPLE__
	curl_global_cleanup();
#endif

	return res;
}

