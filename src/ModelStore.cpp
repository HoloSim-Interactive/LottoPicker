#include "lottopicker/ModelStore.h"

#include <fstream>
#include <optional>
#include <sstream>

#include "lottopicker/CooccurrenceScorer.h"
#include "lottopicker/EraTagger.h"
#include "lottopicker/Errors.h"
#include "lottopicker/ModelSerializer.h"
#include "lottopicker/PoolSizeNormalizer.h"
#include "lottopicker/Sha256.h"

namespace lottopicker {

namespace {

// Builds a fresh ModelArtifact from an already-ingested, already-era-
// tagged `history` -- CORE-200/201's raw scores run through CORE-206's
// normalization, per docs/SDD.md's pipeline. Only groups CooccurrenceScorer
// actually observed are persisted (its sparse-storage contract, carried
// through unchanged by normalization -- see PoolSizeNormalizer.h).
ModelArtifact buildArtifact(const std::vector<DrawRecord> &history, const std::string &sourceHash,
                            int poolMin, int poolMax, int halfLifeDraws) {
    ModelArtifact artifact;
    artifact.sourceHash = sourceHash;
    artifact.drawCount = history.size();

    if (!history.empty()) {
        std::string earliest = history.front().date;
        std::string latest = history.front().date;
        for (const DrawRecord &record : history) {
            if (record.date < earliest) {
                earliest = record.date;
            }
            if (record.date > latest) {
                latest = record.date;
            }
        }
        artifact.earliestDate = earliest;
        artifact.latestDate = latest;
    }

    const PoolSizeNormalizer::Result normalized =
        PoolSizeNormalizer::normalize(history, poolMin, poolMax, halfLifeDraws);
    for (int number = poolMin; number <= poolMax; ++number) {
        artifact.perNumber[number] = normalized.normDecay(number);
    }

    // PoolSizeNormalizer::Result doesn't expose which groups were
    // actually observed (only normCooc() lookups by key), so the raw,
    // sparse group set is recomputed here directly from CooccurrenceScorer
    // -- CORE-206's already-Verified code is used strictly through its
    // public API (normCooc()), not modified to expose internal state.
    const CooccurrenceScores rawCooc = CooccurrenceScorer::score(history, halfLifeDraws);
    for (int groupSize = kMinGroupSize; groupSize <= kMaxGroupSize; ++groupSize) {
        const GroupScoreMap &observedGroups =
            rawCooc[static_cast<std::size_t>(groupSize - kMinGroupSize)];
        if (observedGroups.empty()) {
            continue;
        }
        ModelGroupScoreMap &out = artifact.groupScores[groupSize];
        for (const auto &[group, unusedRawScore] : observedGroups) {
            (void)unusedRawScore;
            out[group] = normalized.normCooc(group);
        }
    }

    return artifact;
}

} // namespace

std::string ModelStore::computeSourceHash(const std::filesystem::path &dataFile) {
    std::ifstream file(dataFile, std::ios::binary);
    if (!file.is_open()) {
        throw ModelStoreError("cannot open data file to compute source hash: " + dataFile.string());
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return Sha256::hexDigest(buffer.str());
}

std::filesystem::path ModelStore::defaultModelPath(const std::filesystem::path &dataFile) {
    std::filesystem::path modelPath = dataFile;
    modelPath += ".model"; // concatenation (path::operator+=), not extension replacement.
    return modelPath;
}

ModelStore::LoadOrBuildResult ModelStore::loadOrBuild(const std::filesystem::path &dataFile,
                                                      const std::filesystem::path &modelPath,
                                                      int poolMin, int poolMax, int halfLifeDraws) {
    const std::string currentHash = computeSourceHash(dataFile);

    if (std::filesystem::exists(modelPath)) {
        const std::optional<ModelArtifact> persisted = ModelSerializer::tryRead(modelPath);
        if (persisted.has_value() && persisted->sourceHash == currentHash) {
            LoadOrBuildResult result;
            result.artifact = *persisted;
            result.wasRebuilt = false;
            return result;
        }
    }

    const IngestResult ingest = CsvIngestor::ingest(dataFile, poolMin, poolMax);
    std::vector<DrawRecord> history = ingest.records;
    EraTagger::tag(history);

    LoadOrBuildResult result;
    result.wasRebuilt = true;
    result.ingestErrors = ingest.errors;
    result.artifact = buildArtifact(history, currentHash, poolMin, poolMax, halfLifeDraws);

    ModelSerializer::write(result.artifact, modelPath);
    return result;
}

} // namespace lottopicker
