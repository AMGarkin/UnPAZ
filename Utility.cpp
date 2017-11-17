#include "Utility.h"


#if _MSC_VER >= 1910
namespace fs = std::experimental::filesystem;
#else
namespace fs = boost::filesystem;
#endif

///--------------------------------------------------------------------------
/// Based on Alessandro Felice Cantatore's code (szWildMatch7)
/// http://xoomer.virgilio.it/acantato/dev/wildcard/wildmatch.html
/// adapted for use with std::string and wstring
///--------------------------------------------------------------------------
bool WildMatch(std::string sPattern, std::string sString) //ANSI version
{
	if (sPattern.empty())
		return true;
	if (sString.empty())
		return sPattern.empty();

	std::string::iterator iStr = sString.begin();
	std::string::iterator iS;
	std::string::iterator iPat = sPattern.begin();
	std::string::iterator iP;
	bool asterix = false;

loopStart:
	for (iS = iStr, iP = iPat; iS != sString.end(); ++iS, ++iP) {
		switch (*iP) {
		case '?':
			if (*iS == '.')
				goto asterixCheck;
			break;
		case '*':
			asterix = true;
			iStr = iS, iPat = iP;
			do { ++iPat; } while (iPat != sPattern.end() && *iPat == '*');
			if (iPat == sPattern.end())
				return true;
			goto loopStart;
		default:
			if (tolower(*iS) != tolower(*iP))
				goto asterixCheck;
			break;
		}
	}
	while (iP != sPattern.end() && *iP == '*') ++iP;
	return (iP == sPattern.end());

asterixCheck:
	if (!asterix) return false;
	iStr++;
	goto loopStart;
}

bool WildMatch(std::wstring wsPattern, std::wstring wsString) //UNICODE (wide string) version
{
	if (wsPattern.empty())
		return true;
	if (wsString.empty())
		return wsPattern.empty();

	std::wstring::iterator iStr = wsString.begin();
	std::wstring::iterator iS;
	std::wstring::iterator iPat = wsPattern.begin();
	std::wstring::iterator iP;
	bool asterix = false;

loopStart:
	for (iS = iStr, iP = iPat; iS != wsString.end(); ++iS, ++iP) {
		switch (*iP) {
		case L'?':
			if (*iS == L'.')
				goto asterixCheck;
			break;
		case L'*':
			asterix = true;
			iStr = iS, iPat = iP;
			do { ++iPat; } while (iPat != wsPattern.end() && *iPat == L'*');
			if (iPat == wsPattern.end())
				return true;
			goto loopStart;
		default:
			if (towlower(*iS) != towlower(*iP))
				goto asterixCheck;
			break;
		}
	}
	while (iP != wsPattern.end() && *iP == L'*') ++iP;
	return (iP == wsPattern.end());

asterixCheck:
	if (!asterix) return false;
	iStr++;
	goto loopStart;
}


///--------------------------------------------------------------------------
///SIMPLE PROGRESSBAR: "Some text [---|---] cur/max (val%)"
///
///Pattern format: https://docs.microsoft.com/en-us/cpp/c-runtime-library/format-specification-syntax-printf-and-wprintf-functions
///See also: https://docs.microsoft.com/en-us/cpp/c-language/escape-sequences and https://docs.microsoft.com/en-us/cpp/c-language/trigraphs
///
///I made numbers cur/max 6 digits long (it looks better), if you want use dynamic lenght, you will need to count digits in integer: digits = ceil(log10(var+1))
///--------------------------------------------------------------------------
#define ProgressBarPATT  "\r%.*s [%.*s|%.*s] %6d/%6d (%3d%%)"
#define ProgressBarFILL  "--------------------------------------------------------------------------------"
#define ProgressBarWDTH 80 //width of printed formatted string
#define ProgressBarFIXD 25 //length of fixed text in pattern: 10 fixed chars in pattern + 2x6 chars for numbers and 3 chars for percent = 25 chars
#define ProgressBarMINS 9  //min. size of progressbar: [---|---]
#define ProgressBarMAXT 46 //max. size of text = ProgressBarWDTH - ProgressBarFIXD - ProgressBarMINS
///
///Create formated string for progressbar: "Some text [---|---] cur/max (val%)"
std::string getProgressString(uint32_t current, uint32_t maximum, std::string text)
{
	float percentage = current / (float)maximum; //float point magic... https://stackoverflow.com/a/7571378
	int percent = lrint(percentage * 100);
	int barWidth = ProgressBarWDTH - (text.length() + ProgressBarFIXD);
	barWidth = barWidth > ProgressBarMINS ? barWidth : ProgressBarMINS;
	int left = lrint(percentage * barWidth);
	int right = barWidth - left;

	return string_format(ProgressBarPATT, ProgressBarMAXT, text.c_str(), left, ProgressBarFILL, right, ProgressBarFILL, current, maximum, percent);
}

///if no argument is passed to getProgressString, return string to clear line.
std::string getProgressString()
{
	return string_format("\r%*s\r", ProgressBarWDTH, "");
}

clock_t clck = 0; ///variable used for throttling printProgress(...)
clock_t now;      ///variable used for throttling printProgress(...)

///Prints progressbar to std::cerr. I'm using cerr so the relevant output can be redirected to file (program > log.txt)
///Printing to standard output in a loop can have big impact on performance, so I have added simple throttling.
///Function won't print progressbar more then twice per second.
///Returns true if progressbar was printed, false if function was canceled due to throttling
bool printProgress(uint32_t current, uint32_t maximum, std::string text)
{
	bool result = false;
	now = clock();

	if (current == maximum || (now - clck) > (CLOCKS_PER_SEC / 2)) {
		std::cerr << getProgressString(current, maximum, text) << std::flush;

		clck = now;
		result = true;
	}

	return result;
}
///clears progressbar line
bool printProgress()
{
	std::cerr << getProgressString();

	return true;
}

///--------------------------------------------------------------------------
///automatically rename file (it adds [] to the end of file name)
///--------------------------------------------------------------------------
void autoRenameFile(fs::path &FilePath)
{
	fs::path P = FilePath.parent_path();
	std::string sStem = FilePath.stem().string();
	std::string sExt = FilePath.extension().string();
	int i = 1;

	FilePath = P / fs::path(sStem + "[" + std::to_string(i) + "]" + sExt);

	while (fs::exists(FilePath)) {
		i++;
		FilePath = P / fs::path(sStem + "[" + std::to_string(i) + "]" + sExt);
	}
}

///--------------------------------------------------------------------------
///ask if you want to automatically rename file (it adds [] to the end of file name)
///returns char which option was selected - (o)verwrite / (R)ename / overwrite (a)ll / re(n)ame all / (e)xit
///--------------------------------------------------------------------------
char autoRenameFilePrompt(fs::path &FilePath)
{
	std::string sUserInput;
	char c;

	std::cerr << "\nFile \"" << FilePath.filename().string() << "\" already exist in target path.\n> (o)verwrite / (R)ename / overwrite (a)ll / re(n)ame all / (e)xit " << std::endl;
	std::cin.clear();

	while (1) {
		std::getline(std::cin, sUserInput);

		if (sUserInput.empty()) {
			c = 'r';
		}
		else {
			c = tolower(sUserInput[0]);
		}

		switch (c) {
		case 'r':
		case 'n':
			autoRenameFile(FilePath);
		case 'o':
		case 'a':
		case 'e':
			return c;
		}
	}
}

///--------------------------------------------------------------------------
///Ask if you want to create directories needed for file specified in FilePath.
///returns char which option was selected - (Y)es / (a)lways / (s)kip file / (e)xit
///--------------------------------------------------------------------------
char createPathPrompt(fs::path &FilePath)
{
	std::string sUserInput;
	char c;

#if _MSC_VER >= 1910 ///std::experimental::filesystem doesn't have implemented boost::filesystem::relative function, using alternative:
	std::cerr << "\nPath \"" << relativePathTo(fs::current_path(), FilePath.parent_path()) << "\" doesn't exist. Create?\n (Y)es / (a)lways / (s)kip file / (e)xit " << std::endl;
#else
	std::cerr << "\nPath \"" << fs::relative(FilePath.parent_path()).string() << "\" doesn't exist.\n>Create path? (Y)es / (a)lways / (s)kip file / (e)xit " << std::endl;
#endif
	std::cin.clear();

	while (1) {
		std::getline(std::cin, sUserInput);

		if (sUserInput.empty()) {
			c = 'y';
		}
		else {
			c = tolower(sUserInput[0]);
		}

		switch (c) {
		case 'y':
		case 'a':
			fs::create_directories(FilePath.parent_path());
		case 's':
		case 'e':
			return c;
		}
	}
}

#if _MSC_VER >= 1910 ///std::experimental::filesystem doesn't have implemented boost::filesystem::relative function, here is alternative:
///returns relative path (source: https://stackoverflow.com/a/29221546)
static fs::path relativePathTo(fs::path from, fs::path to)
{
	// Start at the root path and while they are the same then do nothing then when they first
	// diverge take the remainder of the two path and replace the entire from path with ".."
	// segments.
	fs::path::const_iterator fromIter = from.begin();
	fs::path::const_iterator toIter = to.begin();

	// Loop through both
	while (fromIter != from.end() && toIter != to.end() && (*toIter) == (*fromIter)) {
		++toIter;
		++fromIter;
	}

	fs::path finalPath;

	while (fromIter != from.end()) {
		finalPath /= "..";
		++fromIter;
	}

	while (toIter != to.end()) {
		finalPath /= *toIter;
		++toIter;
	}

	return finalPath;
}
#endif
