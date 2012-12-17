#include "mainheaders.h"
#include "url.h"
#include "json.h"

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

void handleStory(json_value* story)
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

	openUrl(url);	
}

bool readSubreddit(string subredditName, int tofetch)
{
	// fetch the url from the server
	string server = string("www.reddit.com");
	string path   = string("/r/") + subredditName + string("/.json?limit=") + toStr(tofetch);

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
								handleStory(it);
						}						
					}
				}
			}
		}
	}

	return true;
}

int main(int argc, char* argv[])
{
	int res = EXIT_SUCCESS;

#ifdef __APPLE__
	curl_global_init(CURL_GLOBAL_ALL);
#endif

	if (argc == 2)
	{
		readSubreddit(argv[1], 30);
	}
	else 
	{
		printf("r: Copyright 2012 Roland Rabien\n");
		printf("  usage: r <subreddit>\n");
	}

#ifdef __APPLE__
	curl_global_cleanup();
#endif

	return res;
}

