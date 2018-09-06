#if _MSC_VER >= 1910
#include "BDOFiles-exp.h"

namespace fs = std::experimental::filesystem;
#else
#include "BDOFiles-boost.h"

namespace fs = boost::filesystem;
#endif

using namespace BDO;

/// ***** FileEntry Structure ***** ///
FileEntry::FileEntry() :
	uiFileHash(0),
	uiFolderNum(0),
	uiFileNum(0),
	uiPazNum(0),
	uiOffset(0),
	uiCompressedSize(0),
	uiOriginalSize(0),
	sFileName(""),
	sFilePath("")
	{};


/// ***** BDOFile Class ***** ///

///constructor
BDOFile::BDOFile() :
	IceKey(0),
	bQuiet(false),
	bNoFolders(false),
	bYesToAll(false),
	bRenameFiles(false),
	bOverwriteFiles(false),
	bCreatePath(false),
	bMobile(false),
	vFilesTable(),
	mPazNames(),
	ArchivePath()
{
	const unsigned char decrypction_key[8] = { 0x51, 0xF3, 0x0F, 0x11, 0x04, 0x24, 0x6A, 0x00 }; /// The Black Desert ICE decryption key
	this->set(decrypction_key);
}

///Public functions
void BDOFile::ExtractFile(fs::path FilePath, fs::path PazName, uint32_t uiOffset, uint32_t uiCompressedSize, uint32_t uiOriginalSize)
{
	this->internalExtractFile(FilePath, PazName, uiOffset, uiCompressedSize, uiOriginalSize);
}

uint32_t BDOFile::ExtractFileMask(std::string sFileMask, fs::path OutputPath)
{
	std::vector<FileEntry>::iterator it;
	uint32_t counter = 0;
	bool bProgress = false;
	bool bIsQuiet = bQuiet;

	this->bOverwriteFiles = this->GetYesToAll();
	this->bCreatePath = this->GetYesToAll();

	if (sFileMask.empty()) { ///turn off progressbar when listing all files
		this->SetQuiet(true);
	}

	///make sure that output folder exists
	if (!fs::exists(OutputPath)) {
		if (this->bCreatePath) {
			fs::create_directories(OutputPath);
		}
		else {
			std::string sUserInput;
			char c;
			bool bInput = true;

			std::cerr << "\nPath \"" << OutputPath.string() << "\" doesn't exist.\n>Create path? (Y)es / (n)o" << std::endl;
			std::cin.clear();

			while (bInput) {
				std::getline(std::cin, sUserInput);

				if (sUserInput.empty()) {
					c = 'y';
				}
				else {
					c = tolower(sUserInput[0]);
				}

				switch (c) {
				case 'n':
				case 'e':
					exit(0);
				case 'y':
					fs::create_directories(OutputPath);
					bInput = false;
				}
			}
		}
	}
	///set working path to OutputPath because I want shorter path in user prompt if directory doesn't exist
	fs::path currentPath = fs::current_path();
	fs::current_path(OutputPath);

	for (it = this->vFilesTable.begin(); it != this->vFilesTable.end(); ++it) {
		if (WildMatch(sFileMask, it->sFilePath)) {
			fs::path FilePath = OutputPath;

			if (this->GetNoFolders()) {
				FilePath /= it->sFileName;
			} else {
				FilePath /= it->sFilePath;
			}

			this->internalExtractFile(FilePath, this->GetPazName(it->uiPazNum), it->uiOffset, it->uiCompressedSize, it->uiOriginalSize);

			if (bProgress) {
				printProgress(); ///delete current line (progress bar)
				bProgress = false;
			}
			if (!bIsQuiet) {
				std::cout << "> " << it->sFilePath << " (size: " << it->uiOriginalSize << ")\n";
			}

			counter++;
		}
		if (!this->GetQuiet()) {
			bProgress = printProgress(it - this->vFilesTable.begin() + 1, this->vFilesTable.size(), "Searching: ") || bProgress;
		}
	}

	///restore original working path
	fs::current_path(currentPath);

	this->SetQuiet(bIsQuiet);

	if (!this->GetQuiet()) {
		if (bProgress) { ///delete current line (progress bar)
			printProgress(); ///delete current line (progress bar)
		}
		std::cout << "\nExtracted files: " << counter << ", total files: " << this->vFilesTable.size() << std::endl;
	}

	return counter;
}

uint32_t BDOFile::List()
{
	return this->ListFileMask("");
}

uint32_t BDOFile::ListFileMask(std::string sFileMask)
{
	std::vector<FileEntry>::iterator it;
	uint32_t counter = 0;
	bool bProgress = false;
	bool bIsQuiet = bQuiet;

	if (sFileMask.empty()) { ///turn off progressbar when listing all files
		this->SetQuiet(true);
	}

	for (it = this->vFilesTable.begin(); it != this->vFilesTable.end(); ++it) {
		if (WildMatch(sFileMask, it->sFilePath)) {
			if (bProgress) {
				printProgress(); ///delete current line (progress bar)
				bProgress = false;
			}

			std::cout << "[" << this->GetPazName(it->uiPazNum).filename().string() << "] " << it->sFilePath << " (size: " << it->uiOriginalSize << ")\n";
			counter++;

		}
		if (!this->GetQuiet()) {
			bProgress = printProgress(it - this->vFilesTable.begin() + 1, this->vFilesTable.size(), "Searching: ") || bProgress;
		}
	}

	this->SetQuiet(bIsQuiet);

	if (!this->GetQuiet()) {
		if (bProgress) {
			printProgress(); ///delete current line (progress bar)
		}
		std::cout << "\nListed files: " << counter << ", total files: " << vFilesTable.size() << std::endl;
	}

	return counter;
}

bool BDOFile::GetQuiet()
{
	return this->bQuiet;
}

void BDOFile::SetQuiet(bool bQuiet)
{
	this->bQuiet = bQuiet;
}

bool BDOFile::GetNoFolders()
{
	return this->bNoFolders;
}

void BDOFile::SetNoFolders(bool bNoFolders)
{
	this->bNoFolders = bNoFolders;
}

bool BDOFile::GetYesToAll()
{
	return this->bYesToAll;
}

void BDOFile::SetYesToAll(bool bYesToAll)
{
	this->bYesToAll = bYesToAll;
}

fs::path BDOFile::GetArchivePath()
{
	return this->ArchivePath;
}

void BDOFile::SetArchivePath(fs::path ArchivePath)
{
	this->ArchivePath = ArchivePath;
	this->mPazNames.clear();
}

bool BDOFile::GetMobile()
{
	return this->bMobile;
}

void BDOFile::SetMobile(bool bMobile)
{
	this->bMobile = bMobile;
}

///protected functions
void BDOFile::exitError(int errCode, std::string sDetail) {
	switch (errCode) {
	case -2:
		std::cerr << "ERROR: Can't open file " << sDetail << std::endl;
	case -3:
		std::cerr << "ERROR: Insufficient memory." << std::endl;
	case -4:
		std::cerr << "ERROR: Invalid compressed size." << std::endl;
	default:
		std::cerr << "ERROR: " << sDetail << " (code: " << errCode << ")." << std::endl;
	}
	exit(errCode);
}
void BDOFile::exitError(int errCode)
{
	this->exitError(errCode, "");
}

void BDOFile::ICEdecrypt(uint8_t *encrypted, uint8_t *decrypted, uint32_t dataSize)
{
	uint8_t uBlockSize = this->blockSize();

	if (dataSize % uBlockSize != 0)
		this->exitError(-4);

	uint32_t blocks = dataSize / uBlockSize;

	while (blocks--) {
		this->decrypt(encrypted, decrypted);

		encrypted += uBlockSize;
		decrypted += uBlockSize;
	}

	encrypted -= dataSize; ///reset the pointers back to the beginning.
	decrypted -= dataSize;
}

void BDOFile::ICEencrypt(uint8_t *decrypted, uint8_t *encrypted, uint32_t dataSize)
{
	uint8_t uBlockSize = this->blockSize();

	if (dataSize % uBlockSize != 0)
		exitError(-4);

	uint32_t blocks = dataSize / uBlockSize;

	while (blocks--) {
		this->encrypt(decrypted, encrypted);

		encrypted += uBlockSize;
		decrypted += uBlockSize;
	}

	encrypted -= dataSize; ///reset the pointers back to the beginning.
	decrypted -= dataSize;
}

///returns path for uiPazNum, paths are stored in map and created on demand
fs::path BDOFile::GetPazName(uint32_t uiPazNum)
{
	std::map<uint32_t, fs::path>::iterator it;
	fs::path PazName;

	it = this->mPazNames.lower_bound(uiPazNum);

	if (it != mPazNames.end()) {
		if (it->first == uiPazNum) {
			return it->second;
		}
		else {
			it++; ///map.insert -> C++11 The function optimizes its insertion time if position points to the element that will follow the inserted element (or to the end, if it would be the last).
		}
	}

	std::string sPazName;
	std::stringstream ss(sPazName);

	ss << "pad" << std::setw(5) << std::setfill('0') << uiPazNum << ".paz";

	PazName = this->GetArchivePath() / ss.str();

	this->mPazNames.insert(it, std::pair<uint32_t, fs::path>(uiPazNum, PazName));

	return PazName;
}

///Private functions
void BDOFile::internalExtractFile(fs::path FilePath, fs::path PazName, uint32_t uiOffset, uint32_t uiCompressedSize, uint32_t uiOriginalSize)
{
	if (!this->GetMobile()) {
		if (uiCompressedSize % 8 != 0)
			this->exitError(-4);
	}

	///make sure that output folder exists
	if (FilePath.has_parent_path() && !fs::exists(FilePath.parent_path())) {
		if (this->bCreatePath) {
			fs::create_directories(FilePath.parent_path());
		}
		else {
			switch (createPathPrompt(FilePath)) { ///(Y)es / (a)lways / (s)kip file / (e)xit
			case 'e':
				exit(0);
			case 'a':
				this->bCreatePath = true;
			}
		}
	}

	if (uiCompressedSize == 0) { //just create empty file
		std::ofstream ofsFile(FilePath.string(), std::ios::binary);
		ofsFile.close();
	}
	else {
		std::ifstream ifsPazFile(PazName.string(), std::ios::binary);
		if (!ifsPazFile.is_open()) {
			this->exitError(-2, PazName.string());
		}

		///check if output file exists, ask user if he wants to overwrite it
		if (!this->bOverwriteFiles) {
			if (fs::exists(FilePath)) {
				if (this->bRenameFiles) {
					autoRenameFile(FilePath);
				}
				else {
					switch (autoRenameFilePrompt(FilePath)) { ///options: (o)verwrite / (R)ename / overwrite (a)ll / re(n)ame all / (e)xit
					case 'e':
						exit(0);
					case 'a':
						this->bOverwriteFiles = true;
						break;
					case 'n':
						this->bRenameFiles = true;
					}
				}
			}
		}

		std::ofstream ofsFile(FilePath.string(), std::ios::binary);
		if (!ofsFile.is_open()) {
			this->exitError(-2, FilePath.string());
		}

		///decrypt data
		uint8_t *decrypted = new uint8_t[uiCompressedSize];
		if (decrypted == 0) exitError(-3);

		ifsPazFile.seekg(uiOffset);

		if (!this->GetMobile()) {
			uint8_t *encrypted = new uint8_t[uiCompressedSize];
			if (encrypted == 0) exitError(-3);

			ifsPazFile.read(reinterpret_cast<char *>(encrypted), uiCompressedSize);
			this->ICEdecrypt(encrypted, decrypted, uiCompressedSize);

			delete[] encrypted;

			///check if data have header, valid header is 9 bytes long and contains:
			///- ID (unit8_t) = 0x6E for uncompressed data or 0x6F for compressed data
			///- data size (uint32_t)
			///- original file size (unit32_t)
			if ((decrypted[0] == 0x6F || decrypted[0] == 0x6E) && uiCompressedSize > 9) {
				uint32_t uiSize = 0;
				memcpy(&uiSize, decrypted + 1 + 4, 4);	///copy original file size from decrypted data
				if (uiSize == uiOriginalSize) {			///We can consider data header as valid. Size in data header is the same as size in .meta/.paz file.
					uint8_t *decompressed = new uint8_t[uiOriginalSize];
					if (decompressed == 0) exitError(-3);

					BDO::decompress(decrypted, decompressed);
					delete[] decrypted;
					decrypted = decompressed;
				}
			}
		} else {
			ifsPazFile.read(reinterpret_cast<char *>(decrypted), uiCompressedSize);

			if (uiOriginalSize != uiCompressedSize) {
				uint8_t *decompressed = new uint8_t[uiOriginalSize];
				if (decompressed == 0) exitError(-3);

				int result = uncompress(decompressed, reinterpret_cast<uLongf *>(&uiOriginalSize), decrypted, uiCompressedSize);

				if (result == Z_OK) {
					delete[] decrypted;
					decrypted = decompressed;
				} else if (result == Z_MEM_ERROR) {
					exitError(-5, "zlib - Not enough memory.");
				} else if (result == Z_BUF_ERROR) {
					exitError(-5, "zlib - Output buffer is too small.");
				} else if (result == Z_DATA_ERROR) {
					exitError(-5, "zlib - Input data are corrupted or incomplete.");
				}
			}
		}


		ofsFile.write(reinterpret_cast<char *>(decrypted), uiOriginalSize);
		ofsFile.close();

		delete[] decrypted;
	}
}




/// ***** MetaFile class ***** ///
///constructor
MetaFile::MetaFile() :
	uiClientVersion(0),
	uiFilesCount(0),
	uiFileNamesCount(0),
	uiFoldersCount(0)
{
}

MetaFile::MetaFile(fs::path FileName, bool bQuiet) :
	uiClientVersion(0),
	uiFilesCount(0),
	uiFileNamesCount(0),
	uiFoldersCount(0)
{
	this->SetQuiet(bQuiet);

	this->ReadSource(FileName);
};

///public functions
uint32_t MetaFile::GetClientVersion()
{
	return uiClientVersion;
}
uint32_t MetaFile::GetFilesCount()
{
	return uiFilesCount;
}
uint32_t MetaFile::GetFileNamesCount()
{
	return uiFileNamesCount;
}
uint32_t MetaFile::GetFoldersCount()
{
	return uiFoldersCount;
}

///private functions
void MetaFile::ReadSource(fs::path FileName)
{
	std::ifstream ifsMetaFile;
	uint32_t uiFolderNamesLength;
	uint32_t uiFileNamesLength;
	uint32_t uiPazCount;
	uint8_t readBuffer[32];
	uint8_t *pFilesInfo;
	uint8_t *pFolderNamesEncrypted, *pFolderNamesDecrypted;
	uint8_t *pFileNamesEncrypted, *pFileNamesDecrypted;
	uint8_t *ptr, *pEnd;
	std::vector<std::string> vFolderNames;
	std::vector<std::string> vFileNames;

	ifsMetaFile.open(FileName.string(), std::ios::binary);
	if (!ifsMetaFile.is_open()) this->exitError(-2, FileName.string());

	this->SetArchivePath(FileName.parent_path());

	ifsMetaFile.read(reinterpret_cast<char *>(readBuffer), 8);
	memcpy(&this->uiClientVersion, readBuffer, 4);
	memcpy(&uiPazCount, readBuffer + 4, 4);

	if (!this->GetQuiet()) {
		std::cout << "\nClient version: " << uiClientVersion << std::endl;
	}

	///Game client doesn't use all .paz files, so uiPazCount is less then actual number of .paz files in BDO folder.
	///For example NA/EU client v478 doesn't use 43 .paz numbers: 5504-5525, 5535-5552, 5591, 5601 and 5602.
	///as there is no real use of PazInfo, just skip it...
	ifsMetaFile.seekg(uiPazCount * 12, std::ios::cur);

	ifsMetaFile.read(reinterpret_cast<char *>(readBuffer), 4);
	memcpy(&this->uiFilesCount, readBuffer, 4);

	if (!this->GetQuiet()) {
		std::cout << "Number of stored files: " << this->uiFilesCount << std::endl;
	}

	pFilesInfo = new uint8_t[this->uiFilesCount * 28];
	if (pFilesInfo == 0) this->exitError(-3);
	ifsMetaFile.read(reinterpret_cast<char *>(pFilesInfo), this->uiFilesCount * 28);

	//folder names
	ifsMetaFile.read(reinterpret_cast<char *>(readBuffer), 4);
	memcpy(&uiFolderNamesLength, readBuffer, 4);

	pFolderNamesEncrypted = new uint8_t[uiFolderNamesLength];
	if (pFolderNamesEncrypted == 0) this->exitError(-3);
	pFolderNamesDecrypted = new uint8_t[uiFolderNamesLength];
	if (pFolderNamesDecrypted == 0) this->exitError(-3);

	ifsMetaFile.read(reinterpret_cast<char *>(pFolderNamesEncrypted), uiFolderNamesLength);

	///test if meta file is from BDO mobile (names are not encrypted and all folder names starts with "res/")
	if (pFolderNamesEncrypted[8] == 'r' && pFolderNamesEncrypted[9] == 'e' && pFolderNamesEncrypted[10] == 's') {
		this->SetMobile(true);
		memcpy(pFolderNamesDecrypted, pFolderNamesEncrypted, uiFolderNamesLength);
	} else {
		this->ICEdecrypt(pFolderNamesEncrypted, pFolderNamesDecrypted, uiFolderNamesLength);
	}
	delete[] pFolderNamesEncrypted;

	ptr = pFolderNamesDecrypted;
	pEnd = ptr + uiFolderNamesLength - 8;
	while (ptr < pEnd) {
		ptr += 8; ///skip 2x int
		vFolderNames.push_back(reinterpret_cast<char *>(ptr));
		ptr += vFolderNames.back().length() + 1; ///length + zero terminator
	}
	delete[] pFolderNamesDecrypted;

	this->uiFoldersCount = vFolderNames.size();

	if (!this->GetQuiet()) {
		std::cout << "Number of stored folder names: " << uiFoldersCount << std::endl;
	}

	///file names
	ifsMetaFile.read(reinterpret_cast<char *>(readBuffer), 4);
	memcpy(&uiFileNamesLength, readBuffer, 4);

	pFileNamesDecrypted = new uint8_t[uiFileNamesLength];
	if (pFileNamesDecrypted == 0) this->exitError(-3);

	if (!this->GetMobile()) {
		pFileNamesEncrypted = new uint8_t[uiFileNamesLength];
		if (pFileNamesEncrypted == 0) this->exitError(-3);

		ifsMetaFile.read(reinterpret_cast<char *>(pFileNamesEncrypted), uiFileNamesLength);
		ICEdecrypt(pFileNamesEncrypted, pFileNamesDecrypted, uiFileNamesLength);

		delete[] pFileNamesEncrypted;
	} else {
		ifsMetaFile.read(reinterpret_cast<char *>(pFileNamesDecrypted), uiFileNamesLength);
	}

	ptr = pFileNamesDecrypted;
	pEnd = ptr + uiFileNamesLength;
	while (ptr < pEnd) {
		vFileNames.push_back(reinterpret_cast<char *>(ptr));
		ptr += vFileNames.back().length() + 1;
	}
	delete[] pFileNamesDecrypted;

	this->uiFileNamesCount = vFileNames.size();

	if (!this->GetQuiet()) {
		std::cout << "Number of stored file names: " << this->uiFileNamesCount << "\n" << std::endl;
	}

	///fill vFilesTable
	for (uint32_t i = 0; i < this->uiFilesCount; ++i) {
		FileEntry fileEntry;

		memcpy(&fileEntry.uiFileHash, pFilesInfo + i * 28 + 0, 4);
		memcpy(&fileEntry.uiFolderNum, pFilesInfo + i * 28 + 4, 4);
		memcpy(&fileEntry.uiFileNum, pFilesInfo + i * 28 + 8, 4);
		memcpy(&fileEntry.uiPazNum, pFilesInfo + i * 28 + 12, 4);
		memcpy(&fileEntry.uiOffset, pFilesInfo + i * 28 + 16, 4);
		memcpy(&fileEntry.uiCompressedSize, pFilesInfo + i * 28 + 20, 4);
		memcpy(&fileEntry.uiOriginalSize, pFilesInfo + i * 28 + 24, 4);

		fileEntry.sFileName = vFileNames.at(fileEntry.uiFileNum);
		fileEntry.sFilePath = vFolderNames.at(fileEntry.uiFolderNum) + fileEntry.sFileName;

		if (!this->GetQuiet()) {
			printProgress(i + 1, this->uiFilesCount, "Building files info: ");
		}

		this->vFilesTable.push_back(fileEntry);
	}

	if (!this->GetQuiet()) {
		printProgress(); //delete current line (progress bar)
	}

	delete[] pFilesInfo;
}


/// ***** PazFile class ***** ///
///constructors
PazFile::PazFile() :
	uiPazHash(0),
	uiFilesCount(0)
{
}

PazFile::PazFile(fs::path FileName, bool bQuiet) :
	uiPazHash(0),
	uiFilesCount(0)
{
	this->SetQuiet(bQuiet);

	this->ReadSource(FileName);
};

void PazFile::ReadSource(fs::path FileName)
{
	std::ifstream ifsPazFile;
	uint32_t uiPazNum;
	uint32_t uiNamesLength;
	uint8_t readBuffer[32];
	uint8_t *pFilesInfo;
	uint8_t *pNamesEncrypted, *pNamesDecrypted;
	uint8_t *ptr, *pEnd;
	std::vector<std::string> vNames;

	ifsPazFile.open(FileName.string(), std::ios::binary);
	if (!ifsPazFile.is_open()) this->exitError(-2, FileName.string());

	this->SetArchivePath(FileName.parent_path());

	std::istringstream iss(FileName.stem().string()); ///stem (file name without extension) should be "pad012345"
	iss.seekg(3, std::istringstream::beg); ///move to the first digit
	iss >> uiPazNum;

	ifsPazFile.read(reinterpret_cast<char *>(readBuffer), 12);
	memcpy(&this->uiPazHash, readBuffer, 4);
	memcpy(&this->uiFilesCount, readBuffer + 4, 4);
	memcpy(&uiNamesLength, readBuffer + 8, 4);

	if (!this->GetQuiet()) {
		std::cout << "\nNumber of stored files: " << this->uiFilesCount << "\n" << std::endl;
	}

	///read files info
	pFilesInfo = new uint8_t[this->uiFilesCount * 24];
	if (pFilesInfo == 0) this->exitError(-3);
	ifsPazFile.read(reinterpret_cast<char *>(pFilesInfo), this->uiFilesCount * 24);

	///read & decrypt file names
	pNamesEncrypted = new uint8_t[uiNamesLength];
	if (pNamesEncrypted == 0) exitError(-3);
	pNamesDecrypted = new uint8_t[uiNamesLength];
	if (pNamesDecrypted == 0) exitError(-3);

	ifsPazFile.read(reinterpret_cast<char *>(pNamesEncrypted), uiNamesLength);

	///test if meta file is from BDO mobile (names are not encrypted and always starts with folder name "res/")
	if (pNamesEncrypted[0] == 'r' && pNamesEncrypted[1] == 'e' && pNamesEncrypted[2] == 's') {
		this->SetMobile(true);
		memcpy(pNamesDecrypted, pNamesEncrypted, uiNamesLength);
	} else {
		this->ICEdecrypt(pNamesEncrypted, pNamesDecrypted, uiNamesLength);
	}
	delete[] pNamesEncrypted;

	ptr = pNamesDecrypted;
	pEnd = ptr + uiNamesLength;
	while (ptr < pEnd) {
		vNames.push_back(reinterpret_cast<char *>(ptr));
		ptr += vNames.back().length() + 1; ///length + zero terminator
	}
	delete[] pNamesDecrypted;

	///fill vFilesTable
	for (uint32_t i = 0; i < uiFilesCount; ++i) {
		FileEntry fileEntry;

		memcpy(&fileEntry.uiFileHash, pFilesInfo + i * 24 + 0, 4);
		memcpy(&fileEntry.uiFolderNum, pFilesInfo + i * 24 + 4, 4);
		memcpy(&fileEntry.uiFileNum, pFilesInfo + i * 24 + 8, 4);
		memcpy(&fileEntry.uiOffset, pFilesInfo + i * 24 + 12, 4);
		memcpy(&fileEntry.uiCompressedSize, pFilesInfo + i * 24 + 16, 4);
		memcpy(&fileEntry.uiOriginalSize, pFilesInfo + i * 24 + 20, 4);

		fileEntry.uiPazNum = uiPazNum;

		fileEntry.sFileName = vNames.at(fileEntry.uiFileNum);
		fileEntry.sFilePath = vNames.at(fileEntry.uiFolderNum) + fileEntry.sFileName;

		if (!this->GetQuiet()) {
			printProgress(i + 1, uiFilesCount, "Building files info: ");
		}

		this->vFilesTable.push_back(fileEntry);
	}

	if (!this->GetQuiet()) {
		printProgress(); ///delete current line (progress bar)
	}

	delete[] pFilesInfo;
}

///public functions
uint32_t PazFile::GetPazHash()
{
	return this->uiPazHash;
}

uint32_t PazFile::GetFilesCount()
{
	return this->uiFilesCount;
}
