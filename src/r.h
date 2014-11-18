#pragma once

struct Options
{
	Options()
      : openLink(false),
		openPermalink(false),
		imageOnly(false),
		openSubreddit(false),
		clearHistory(false),
		openAll(false),
		displayHelp(false),
		displayVersion(false),
		numToOpen(30),
		maxToOpen(50)
	{
	}

	bool openLink;
	bool openPermalink;
	bool imageOnly;
	bool openSubreddit;
	bool clearHistory;
	bool openAll;
	bool displayHelp;
	bool displayVersion;
	int numToOpen;
	int maxToOpen;
	vector<string> subreddits;
};

struct HistoryItem
{
	HistoryItem()
	  : date(0)
	{
	}

	string id;
	time_t date;
};
