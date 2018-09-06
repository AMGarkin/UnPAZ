#pragma once

#include <string>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <filesystem>
#include "IceKey.h"
#include "BDO.h"
#include "Utility.h"
#include "zlib.h"


namespace BDO
{
	struct FileEntry
	{
		FileEntry(); //constructor
		uint32_t uiFileHash;
		uint32_t uiFolderNum;
		uint32_t uiFileNum;
		uint32_t uiPazNum;
		uint32_t uiOffset;
		uint32_t uiCompressedSize;
		uint32_t uiOriginalSize;
		std::string sFileName;
		std::string sFilePath;

		///operator overload for sorting
		bool operator<(const FileEntry &a) const
		{
			return sFilePath < a.sFilePath;
		}
	};

	class BDOFile
		: private IceKey
	{
	public:
		///constructor
		BDOFile();
		///functions
		void ExtractFile(std::experimental::filesystem::path FilePath, std::experimental::filesystem::path PazName, uint32_t uiOffset, uint32_t uiCompressedSize, uint32_t uiOriginalSize);
		uint32_t ExtractFileMask(std::string sFileMask, std::experimental::filesystem::path OutputPath);
		uint32_t List();
		uint32_t ListFileMask(std::string sFileMask);
		bool GetQuiet();
		void SetQuiet(bool bQuiet);
		bool GetNoFolders();
		void SetNoFolders(bool bNoFolders);
		bool GetYesToAll();
		void SetYesToAll(bool bYesToAll);
		bool GetMobile();
		void SetMobile(bool bMobile);
		std::experimental::filesystem::path GetArchivePath();
		void SetArchivePath(std::experimental::filesystem::path ArchivePath);
	protected:
		///functions
		void ICEdecrypt(uint8_t *encrypted, uint8_t *decrypted, uint32_t dataSize);
		void ICEencrypt(uint8_t *decrypted, uint8_t *encrypted, uint32_t dataSize);
		void exitError(int errCode, std::string sDetail);
		void exitError(int errCode);
		std::experimental::filesystem::path GetPazName(uint32_t uiPazNum);
		///variables
		std::vector<FileEntry> vFilesTable;
		std::map<uint32_t, std::experimental::filesystem::path> mPazNames;
	private:
		///functions
		void internalExtractFile(std::experimental::filesystem::path FilePath, std::experimental::filesystem::path PazName, uint32_t uiOffset, uint32_t uiCompressedSize, uint32_t uiOriginalSize);
		///variables
        bool bQuiet;
        bool bNoFolders;
        bool bYesToAll;
        bool bRenameFiles;
        bool bOverwriteFiles;
        bool bCreatePath;
		bool bMobile;
        std::experimental::filesystem::path ArchivePath;
	};


	class MetaFile
		: public BDOFile
	{
	public:
		///constructors
		MetaFile();
		MetaFile(std::experimental::filesystem::path FileName, bool bQuiet);
		///functions
		uint32_t GetClientVersion();
		uint32_t GetFilesCount();
		uint32_t GetFileNamesCount();
		uint32_t GetFoldersCount();
	protected:
		///functions
		void ReadSource(std::experimental::filesystem::path FileName);
	private:
		///variables
		uint32_t uiClientVersion;
		uint32_t uiFilesCount;
		uint32_t uiFileNamesCount;
		uint32_t uiFoldersCount;
	};


	class PazFile
		: public BDOFile
	{
	public:
		///constructors
		PazFile();
		PazFile(std::experimental::filesystem::path FileName, bool bQuiet);
		///functions
		uint32_t GetPazHash();
		uint32_t GetFilesCount();
	protected:
		///functions
		void ReadSource(std::experimental::filesystem::path FileName);
	private:
		///variables
		uint32_t uiPazHash;
		uint32_t uiFilesCount;
	};

}
