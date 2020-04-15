#include <cinttypes>
#include <cassert>
#include <unordered_set>

#include "acbextract.h"
#include "../common/utils.h"

using namespace std;
using namespace cgss;

AcbWalkCallbackParams::AcbWalkCallbackParams()
    : walkOptions(nullptr), acb(nullptr), formatVersion(0),
      archive(nullptr), archiveDataStream(nullptr), entryDataStream(nullptr),
      isInternal(FALSE) {
}

AcbWalkOptions::AcbWalkOptions()
    : decoderConfig({}), useCueName(FALSE), callback(nullptr) {
}

static int ProcessAllBinaries(CAcbFile &acb, uint32_t formatVersion, AcbWalkOptions *options, const string &extractDir, CAfs2Archive *archive, IStream *archiveDataStream, bool_t isInternal, unordered_set<uint32_t> &extractedCueIds);

int AcbWalk(const std::string &inputAcbFile, AcbWalkOptions *options) {
    const auto baseExtractDirPath = CPath::Combine(CPath::GetDirectoryName(inputAcbFile), "_acb_" + CPath::GetFileName(inputAcbFile));

    CFileStream fileStream(inputAcbFile.c_str(), FileMode::OpenExisting, FileAccess::Read);
    CAcbFile acb(&fileStream, inputAcbFile.c_str());

    acb.Initialize();

    CAfs2Archive *archive = nullptr;
    uint32_t formatVersion = acb.GetFormatVersion();
    int r;

    try {
        archive = acb.GetInternalAwb();
    } catch (CException &ex) {
        fprintf(stderr, "%s (%d)\n", ex.GetExceptionMessage().c_str(), ex.GetOpResult());
        archive = nullptr;
    }

    unordered_set<uint32_t> extractedCueIds;

    if (archive) {
        const auto extractDir = CPath::Combine(baseExtractDirPath, "internal");

        try {
            r = ProcessAllBinaries(acb, formatVersion, options, extractDir, archive, acb.GetStream(), TRUE, extractedCueIds);
        } catch (CException &ex) {
            fprintf(stderr, "%s (%d)\n", ex.GetExceptionMessage().c_str(), ex.GetOpResult());
            r = -1;
        }

        delete archive;

        if (r != 0) {
            return r;
        }
    }

    try {
        archive = acb.GetExternalAwb();
    } catch (CException &ex) {
        fprintf(stderr, "%s (%d)\n", ex.GetExceptionMessage().c_str(), ex.GetOpResult());
        archive = nullptr;
    }

    if (archive) {
        const auto extractDir = CPath::Combine(baseExtractDirPath, "external");

        try {
            CFileStream fs(archive->GetFileName(), FileMode::OpenExisting, FileAccess::Read);

            r = ProcessAllBinaries(acb, formatVersion, options, extractDir, archive, &fs, FALSE, extractedCueIds);
        } catch (CException &ex) {
            fprintf(stderr, "%s (%d)\n", ex.GetExceptionMessage().c_str(), ex.GetOpResult());
            r = -1;
        }

        delete archive;

        if (r != 0) {
            return r;
        }
    }

    return 0;
}

static int ProcessAllBinaries(CAcbFile &acb, uint32_t formatVersion, AcbWalkOptions *options, const string &extractDir, CAfs2Archive *archive, IStream *archiveDataStream, bool_t isInternal, unordered_set<uint32_t> &extractedCueIds) {
    if (!CFileSystem::DirectoryExists(extractDir)) {
        if (!CFileSystem::MkDir(extractDir)) {
            fprintf(stderr, "Failed to create directory %s.\n", extractDir.c_str());
            return -1;
        }
    }

    // Set up key modifier
    uint16_t keyModifier;

    if (formatVersion >= CAcbFile::KEY_MODIFIER_ENABLED_VERSION) {
        keyModifier = archive->GetHcaKeyModifier();
    } else {
        keyModifier = 0;
    }

    if (options->callback) {
        AcbWalkCallbackParams p;

        p.acb = &acb;
        p.archive = archive;
        p.archiveDataStream = archiveDataStream;
        p.formatVersion = formatVersion;
        p.isInternal = isInternal;
        p.walkOptions = options;

        p.walkOptions->decoderConfig.cipherConfig.keyModifier = keyModifier;

        // First try to extract files with readable cue names
        {
            const auto &fileNames = acb.GetFileNames();
            uint32_t i = 0;

            for (const auto &extractFileName : fileNames) {
                if (extractFileName.empty()) {
                    continue;
                }

                p.extractPathHint = CPath::Combine(extractDir, extractFileName);

                auto entryDataStream = acb.OpenDataStream(extractFileName.c_str());

                if (entryDataStream) {
                    p.entryDataStream = entryDataStream;

                    const auto fileRecord = acb.GetFileRecord(extractFileName.c_str());

                    assert(fileRecord != nullptr);

                    p.cueInfo.id = fileRecord->cueId;
                    p.cueInfo.offset = fileRecord->fileOffsetAligned;
                    p.cueInfo.size = fileRecord->fileSize;

                    options->callback(&p);

                    extractedCueIds.insert(fileRecord->cueId);
                } else {
                    fprintf(stderr, "Cue #%" PRIu32 " (%s) cannot be retrieved.\n", i + 1, extractFileName.c_str());
                }

                ++i;
                p.entryDataStream = nullptr;
                p.extractPathHint = "";

                delete entryDataStream;
            }
        }

        // Then try on not-yet-exported files
        {
            const auto &fileList = archive->GetFiles();

            for (auto &entry : fileList) {
                auto &record = entry.second;

                if (extractedCueIds.find(record.cueId) != extractedCueIds.end()) {
                    continue;
                }

                string extractFileName;

                if (options->useCueName) {
                    extractFileName = acb.GetCueNameFromCueId(record.cueId);
                } else {
                    extractFileName = CAcbFile::GetSymbolicFileNameFromCueId(record.cueId);
                }

                auto entryDataStream = CAcbHelper::ExtractToNewStream(archiveDataStream, record.fileOffsetAligned, (uint32_t)record.fileSize);

                p.entryDataStream = entryDataStream;
                p.extractPathHint = CPath::Combine(extractDir, extractFileName);
                p.cueInfo.id = record.cueId;
                p.cueInfo.offset = record.fileOffsetAligned;
                p.cueInfo.size = record.fileSize;

                options->callback(&p);

                p.entryDataStream = nullptr;
                p.extractPathHint = "";
                p.cueInfo.id = 0;
                p.cueInfo.offset = 0;
                p.cueInfo.size = 0;

                delete entryDataStream;
            }
        }
    }

    options->decoderConfig.cipherConfig.keyModifier = 0;

    return 0;
}