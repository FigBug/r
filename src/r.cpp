#include "mainheaders.h"
#include "r.h"
#include "url.h"
#include "json.h"
#include "ultragetopt.h"

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

#endif
}

void handleStory(const Options& options, json_value* story)
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

	if (options.imgurOnly && url.find("imgur.com") == string::npos)
		return;

	if (options.openPermalink)
		openUrl("http://reddit.com" + permalink);

	if (options.openLink)
		openUrl(url);	
}

bool readSubreddits(const Options& options)
{
	for (int i = 0; i < (int)options.subreddits.size(); i++)
	{
		// open subreddit
		if (options.openSubreddit)
			openUrl("http://www.reddit.com/r/" + options.subreddits[i]);

		// continue if we don't actual need to fetch the json
		if (!options.openLink && !options.openPermalink)
			continue;

		// fetch the url from the server
		string server = string("www.reddit.com");
		string path   = string("/r/") + options.subreddits[i] + string("/.json?limit=") + toStr(options.numToOpen);

		string page = fetchUrl(server, path);

		if (page.size() == 0)
			return false;

		// convert to json
		block_allocator allocator(1 << 10);
		json_value* root = toJson(page, allocator);
		if (!root)
			return false;

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
									handleStory(options, it);
							}						
						}
					}
				}
			}
		}
	}

	return true;
}

	bool openLink;
	bool openPermalink;
	bool imgurOnly;
	bool openSubreddit;
	bool clearHistory;
	bool openAll;
	int numToOpen;
	string subreddit;

void printUsage()
{
	printf("r: Copyright 2012 Roland Rabien\n");
	printf("  usage: r [-lpiocadv] [-n max] subreddit\n");
	printf("  -l Open links (default)\n");
	printf("  -p Open permalinks\n");
	printf("  -i Open Imgur links only\n");
	printf("  -o Open subreddit\n");
	printf("  -c Clear history\n");
	printf("  -a Open all (including previously opened)\n");
	printf("  -n Maximum number of links to open (default 30)\n");
	printf("  -d Display help");
	printf("  -v Display version");
}

Options parseOptions(int argc, char* argv[])
{
	Options options;

	ultraopterr = 0;

	int c;	
	while ((c = ultragetopt (argc, argv, "lpiocandv:")) != -1)
	{
		switch (c)
		{
			case 'l': options.openLink          = true; break;
			case 'p': options.openPermalink     = true; break;
			case 'i': options.imgurOnly			= true; break;
			case 'o': options.openSubreddit     = true; break;
			case 'c': options.clearHistory      = true; break;
			case 'a': options.openAll			= true; break;
			case 'n': options.numToOpen			= atoi(ultraoptarg); break;
			case 'd': options.displayHelp		= true; break;
			case 'v': options.displayVersion	= true; break;
			case '?':
				if (ultraoptopt == 'c')
					fprintf (stderr, "Option -%c requires an argument.\n", ultraoptopt);
				else if (isprint (ultraoptopt))
					fprintf (stderr, "Unknown option `-%c'.\n", ultraoptopt);
				else
					fprintf (stderr, "Unknown option character `\\x%x'.\n", ultraoptopt);
				break;
			default:
				abort();
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

	for (int index = ultraoptind; index < argc; index++)
		options.subreddits.push_back(argv[index]);

	return options;
}

bool showVersion()
{
	printf("r v1.0.0 by Roland Rabien (figbug@gmail.com) %s.\n", __DATE__);

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

	if (options.displayHelp || options.subreddits.size() == 0)
	{
		printUsage();
		return res;
	}

	if (options.displayVersion)
	{
		showVersion();
		return res;
	}

	readSubreddits(options);

#ifdef __APPLE__
	curl_global_cleanup();
#endif

	return res;
}

