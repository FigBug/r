#pragma once

struct Options
{
	Options()
      : openLink(false),
		openPermalink(false),
		imgurOnly(false),
		openSubreddit(false),
		clearHistory(false),
		openAll(false),
		displayHelp(false),
		displayVersion(false),
		numToOpen(30)
	{
	}

	bool openLink;
	bool openPermalink;
	bool imgurOnly;
	bool openSubreddit;
	bool clearHistory;
	bool openAll;
	bool displayHelp;
	bool displayVersion;
	int numToOpen;
	vector<string> subreddits;
};