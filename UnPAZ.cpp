#include "UnPAZ.h"
#include "Utility.h"

#if _MSC_VER >= 1910
#include "BDOFiles-exp.h"

namespace fs = std::experimental::filesystem;
#else
#include "BDOFiles-boost.h"

namespace fs = boost::filesystem;
#endif

using namespace std;

namespace {
	fs::path SourcePath;
	fs::path TargetPath;
	string sMask;
	bool bList = false;
	bool bNoFolders = false;
	bool bYesToAll = false;
	bool bQuiet = false;


	///print version header
	void printVersion()
	{
		cout << "Garkin's UnPAZ v1.1 - tool for extracting Black Desert Online archives.\n";
	}

	///print help text
	void printHelp()
	{
		printVersion();

		cout << "\nUnPAZ <input file> <commands>\n\n" <<
			"<input file>:  name of.meta or .paz file(default: pad00000.meta)\n" <<
			"<commands>:\n" <<
			"  -f <mask>:  Filter, this argument must be followed by mask. Mask supports wildcards * and ?.\n" <<
			"  -o <path>:  Output folder, this argument must be followed by path.\n" <<
			"  -h:  Print this help text\n" <<
			"  -l:  List file names without extracting them.\n" <<
			"  -n:  No folder structure, extract files directly to output folder.\n" <<
			"  -y:  Yes to all questions(creating folders, overwritting files).\n" <<
			"  -q:  Quiet(limit printed messages to file names)\n\n" <<
			"Examples:\n" <<
			"  UnPAZ pad00001.paz -f *.luac \n" <<
			"  UnPAZ pad00000.meta -y -n -f *languagedata_??.txt -o Extracted\n" <<
			"  UnPAZ D:\\Games\\BlackDesert\\live\\Paz\\pad00000.meta -l" <<
			endl;
	}

	///Parse commandline and store results to global variables
	void parseCommandline(int argc, char **argv)
	{
		if (argc > 1) {
			SourcePath = argv[1];

			if (!fs::exists(SourcePath)) {
				cerr << "ERROR: " << SourcePath.string() << " doesn't exist." << endl;
				exit(-1);
			}

			SourcePath = fs::canonical(argv[1]);

			int i = 2; ///iterator for commands, starting with 2 - skip name of executable file (0) and SourcePath (1)
			
			while (i < argc) {
				if ((argv[i][0] == '-' || argv[i][0] == '/') && argv[i][2] == '\0') { ///argument is command
					switch (tolower(argv[i][1])) {
					case 'f': ///filter, next argument must be mask
						i++;
						if (i >= argc) {
							cerr << "ERROR: Missing filter mask for argument -f." << endl;
							exit(-1);
						}
						sMask = argv[i];
						break;
					case 'o': ///output, next argument must be path
						i++;
						if (i >= argc) {
							cerr << "ERROR: Missing output path for argument -o." << endl;
							exit(-1);
						}
						TargetPath = argv[i];
						if (!fs::exists(TargetPath)) {
							TargetPath = fs::absolute(TargetPath);
						}
						else {
							TargetPath = fs::canonical(TargetPath);
						}
						break;
					case 'h':  ///help
					case '?':
						printHelp();
						exit(0);
					case 'l': ///list
						bList = true;
						break;
					case 'n': ///no folders
						bNoFolders = true;
						break;
					case 'y': ///overwrite files, create folders
						bYesToAll = true;
						break;
					case 'q': ///quiet
						bQuiet = true;
						break;
					}
				}
				else {
					cout << "Ignoring unexpected argument: " << argv[i] << endl;
				}
				i++;
			}
		}

		if (SourcePath.empty()) {
			printHelp();
			exit(0);
		}

		if (TargetPath.empty()) {
			TargetPath = fs::current_path();
		}

		SourcePath.make_preferred(); ///replace forward slashes with back slashes.
		TargetPath.make_preferred();
	}
}

///main function
int main(int argc, char **argv)
{
	///parse commandline, function will set values of sSourceFileName, sTargetPath, sMask, bList and bVerbose
	parseCommandline(argc, argv);
	if (!bQuiet) {
		printVersion();

		cout << "Type UnPAZ -h for help.\n" <<
			"\nSource file: " << SourcePath.string() <<
			"\nTarget path: " << TargetPath.string() <<
			"\nFilter mask: " << sMask << endl;
	}


	if (_stricmp(SourcePath.extension().string().c_str(), ".meta") == 0) {
		///get files info from .meta
		BDO::MetaFile MetaFile(SourcePath, bQuiet);

		MetaFile.SetQuiet(bQuiet);
		MetaFile.SetNoFolders(bNoFolders);
		MetaFile.SetYesToAll(bYesToAll);

		if (bList) {
			if (sMask == "*.*" || sMask == "*") {
				sMask.clear();
			}
			MetaFile.ListFileMask(sMask);
		}
		else {
			MetaFile.ExtractFileMask(sMask, TargetPath);
		}
	}
	else if (_stricmp(SourcePath.extension().string().c_str(), ".paz") == 0) {
		///get files info from .paz
		BDO::PazFile PazFile(SourcePath, bQuiet);

		PazFile.SetQuiet(bQuiet);
		PazFile.SetNoFolders(bNoFolders);
		PazFile.SetYesToAll(bYesToAll);

		if (bList) {
			if (sMask == "*.*" || sMask == "*") {
				sMask.clear();
			}
			PazFile.ListFileMask(sMask);
		}
		else {
			PazFile.ExtractFileMask(sMask, TargetPath);
		}
	}
	else {
		cerr << "ERROR: Input file must have extension .meta or .paz. File extension " << SourcePath.extension().string() << " is not supported." << endl;
		exit(-1);
	}


	return 0;
}
